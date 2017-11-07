import json
import caffe
import im2col_cython
import acc8
import numpy as np
from collections import namedtuple
from pynq import PL
from pynq import MMIO
import inspect
import math
import time

class Wrapper:
  def __init__(self,wrapped,dtype=np.int32):
    self.wrapped=wrapped
    self.dtype=dtype
  def value(self):
    return self.wrapped

class Call:
  def __init__(self,func,stream_args,scalar_args,return_type=np.int32):
    self.func=func
    self.args=stream_args
    self.scalar_args=scalar_args
    self.dtype=return_type
    self.cached=None

  def value(self):
    return self.func(*[a.value() for a in self.args])

  def hw_value(self):
    return execute_hardware(self)

  def __str__(self):
    if self.cached is None:
      self.cached=self.hw_value()
    return str(self.cached)

  def __getitem__(self,index):
    if self.cached is None:
      self.cached=self.hw_value()
    return len(self.cached)

class FunctionMetadata:
  def __init__(self):
    Function=namedtuple('Function','in_ports out_ports name')
    self.DMA=[([0],[0]),([5],[4])]
    self.DMA_names=['axi_dma_0','axi_dma_1']
    self.functions={}
    self.functions['Xilinx:hls:stream_double:1.0']=Function(in_ports=[2],out_ports=[2],name=None)
    #self.functions['Xilinx:hls:stream_mult:1.0']=Function(in_ports=[3,4],out_ports=[3],name=None)
    #self.functions['xilinx.com:hls:wrapped_mmult_hw:1.0']=Function(in_ports=[3,4],out_ports=[3],name=None)
    self.functions['xilinx.com:hls:wrapped_conv_im2col_hw:1.0']=Function(in_ports=[3,4],out_ports=[3],name=None)
    self.functions['Xilinx:hls:simple_sum:1.0']=Function(in_ports=[1],out_ports=[1],name=None)
    self.functions['Xilinx:hls:mult_constant:1.0']=Function(in_ports=[6],out_ports=[5],name='mult_constant_0')
    
metadata=FunctionMetadata()
    
class StreamingSwitch:
  def __init__(self,name):
    base_addr=int(PL.ip_dict["SEG_{0}_Reg".format(name)][0],16)
    self.mmio=MMIO(base_addr,256)
    self.reset()

  def set_route(self,in_port,out_port):
    print('SWITCH: setting route{0} to {1}'.format(in_port,out_port))
    self.mmio.write(0x40+out_port*4,in_port)

  def reset(self):
    for i in range(16):
      # Disable the output on every port
      self.mmio.write(0x40+i*4,0x80000000)

  def commit(self):
    # Causes the switch to update automatically to the new routing
    self.mmio.write(0,2)

class DMAWrapper:
  def __init__(self,index):
    print('Send DMA: create index {0} name {1}'.format(index,metadata.DMA_names[index]))
    base_addr=int(PL.ip_dict["SEG_{0}_Reg".format(metadata.DMA_names[index])][0],16)
    print('Send DMA: base_address {0:x}'.format(base_addr))
    self.dma=DMA(base_addr,0)
    self.ports=metadata.DMA[index]

  def set_data(self,data,dtype):
    self.length=len(data) * dtype.itemsize
    print('Send DMA: sending {0} bytes'.format(self.length))
    self.dma.create_buf(self.length)
    ffi=pynq.drivers.dma.ffi
    buf=ffi.buffer(self.dma.buf,self.length)
    view=np.frombuffer(buf,dtype,-1)
    np.copyto(view,data,casting='same_kind')

  def transfer(self):
    print('Send DMA: transfer started')
    self.dma.transfer(self.length,0)

  def wait(self):
    self.dma.wait()
    print('Send DMA: transfer finished')

def wrap_arg(a,dtype=np.int32):
  if type(a) is Call or type(a) is Wrapper:
    return a
  else:
    return Wrapper(a,dtype)

def hardware_function(vlnv):
  def decorator(func):
    sig=inspect.signature(func)
    ret_type=sig.return_annotation[0]
    def wrapped_function(*args,**kwargs):
      ba=sig.bind(*args,**kwargs)
      if vlnv in metadata.functions:
        stream_args=[]
        scalar_args=[]
        for param in sig.parameters.values():
          if type(param.annotation) is list:
            stream_args.append(wrap_arg(ba.arguments[param.name],param.annotation[0]))
          else:
            scalar_args.append(ba.arguments[param.name])
        return Call(vlnv,stream_args,scalar_args,return_type=ret_type)
      else:
        return func(*args,**kwargs)
    return wrapped_function
  return decorator

from pynq import Overlay
Overlay('base.bit').download()
from pynq.drivers import DMA
import pynq.drivers.dma
#Overlay('/home/xilinx/decorator_conv.bit').download()
Overlay('/home/xilinx/decorator_conv_im2col.bit').download()

def prepare_execution(plan,dma,return_port):
  if type(plan) is Wrapper:
    d=DMAWrapper(len(dma))
    d.set_data(plan.wrapped,plan.dtype())
    dma.append(d)
    hw_switch.set_route(d.ports[1][0],return_port)
  elif type(plan) is Call:
    in_ports=metadata.functions[plan.func].in_ports
    out_ports=metadata.functions[plan.func].out_ports
    name=metadata.functions[plan.func].name
    mmio=None
    if name:
      mmio=MMIO(int(PL,ip_dict['SEG_{0}_Reg'.format(name)][0],16),256)
    for i,a in enumerate(plan.args):
      prepare_execution(a,dma,in_ports[i])
    for i,a in enumerate(plan.scalar_args):
      mmio.write(0x10+4*i,a)
    hw_switch.set_route(out_ports[0],return_port)
  else:
    print("Unknown plan type: "+repr(plan))

hw_switch=StreamingSwitch('axis_switch_0')
def execute_hardware(plan):
  dma=[]
  hw_switch.reset()
  ret_dma_base=int(PL.ip_dict["SEG_{0}_Reg".format(metadata.DMA_names[0])][0],16)
  ret_dma_mmio=MMIO(ret_dma_base,256)
  ret_dma=DMA(ret_dma_base,1)
  ret_dma.create_buf(8388607)
  prepare_execution(plan,dma,metadata.DMA[0][0][0])
  hw_switch.commit()
  ret_dma.transfer(8388607,1)
  for d in dma:
    d.transfer()
  for d in dma:
    d.wait()
  ret_dma.wait()
  bytes_read=ret_dma_mmio.read(0x58)
  ffi=pynq.drivers.dma.ffi
  buf=ffi.buffer(ret_dma.buf,bytes_read)
  view=np.frombuffer(buf,plan.dtype,-1).copy()
  return view

@hardware_function('Xilinx:hls:simple_sum:1.0')
def total(vs:[np.int32]) -> [np.int32]:
  print("In total")
  return sum(vs)

@hardware_function('Xilinx:hls:stream_double:1.0')
def double(vs:[np.int32]) -> [np.int32]:
  print("In double")
  return [v*2 for v in vs]

#@hardware_function('xilinx.com:hls:wrapped_conv_hw:1.0')
@hardware_function('xilinx.com:hls:wrapped_conv_im2col_hw:1.0')
def mult(a:[np.float32],b:[np.float32]) -> [np.float32]:
  return [a1 * b1 for (a1,b1) in zip (a,b)]

#def get_im2col_indices(x_shape,field_height,field_width,padding=1,stride=1):
#  # First figure out what the size of the output should be
#  N,C,H,W=x_shape
#  assert(H+2*padding-field_height)%stride == 0
#  assert(W+2*padding-field_height)%stride == 0
#  out_height = (H + 2 * padding - field_height) / stride + 1
#  out_width = (W + 2 * padding - field_width) / stride + 1
#  
#  i0 = np.repeat(np.arange(field_height),field_width)
#  i0 = np.tile(i0,C)
#  i1 = stride * np.repeat(np.arange(out_height),out_width)
#  j0 = np.tile(np.arange(field_width),field_height * C)
#  j1 = stride * np.tile(np.arange(out_width),out_height)
#  i = i0.reshape(-1,1) + i1.reshape(1,-1)
#  j = j0.reshape(-1,1) + j1.reshape(1,-1)
#
#  k = np.repeat(np.arange(C),field_height * field_width).reshape(-1,1)
#
#  return (k,i,j)
#
#def im2col_indices(x, field_height, field_width, padding=1, stride=1):
#  """ An implementation of im2col based on some fancy indexing """
#  # Zero-pad the input
#  p = padding
#  x_padded = np.pad(x, ((0,0),(0,0),(p,p),(p,p)), mode='constant')
#
#  k,i,j = get_im2col_indices(x.shape, field_height, field_width, padding, stride)
#  i = i.astype(int)
#  j = j.astype(int)
#  cols = x_padded[:, k, i, j]
#  C = x.shape[1]
#  print(cols.shape)
#  #cols = cols.transpose(1,2,0).reshape(field_height * field_width * C, -1)
#  cols = cols.transpose(2,1,0).reshape(-1, field_height * field_width * C)
#  return cols
#
#def col2im_indices(cols, x_shape, field_height=3, field_width=3, padding=1, stride=1):
#  """ An implementation of col2im based on fancy indexing and np.add.at """
#  N,C,H,W = x_shape
#  H_padded, W_padded = H + 2 * padding, W + 2 * padding
#  x_padded = np.zeros((N, C, H_padded, W_padded), dtype=cols.dtype)
#  k, i, j = get_im2col_indices(x_shape, field_height * field_width, padding, stride)
#  cols_reshaped = cols.reshape(C * field_height * field_width, -1, N)
#  cols_reshaped = cols_reshaped.transpose(2,0,1)
#  np.add.at(x_paddeed, (slice(None), k, i, j), cols_reshaped)
#  if padding == 0:
#    return x_padded
#  return x_padded[:, :, padding:-padding, padding:-padding]

class FPGAConvLayer(caffe.Layer):#prototype does not update weight in the backward propagation
  def setup(self, bottom, top):
    #print('start setup')
    #if self.kernel_size:
    #  self.weight=np.random.randn(self.kernel_size*self.kernel_size)*math.sqrt(2.0/(self.kernel_size*self.kernel_size))
    #else:
    #  self.weight=np.random.randn(5*5)*math.sqrt(2.0/(5*5))
    
    if hasattr(self,'param_str') and self.param_str:
      print(self.param_str)
      params = json.loads(self.param_str)
    else:
      params = {}
    self.num_out = params.get('num_out',1)
    self.pad = params.get('pad',1)
    self.st = params.get('stride',1)
    self.ks = params.get('kernel_size',1)

    self.C = bottom[0].data.shape[1]
    self.H = bottom[0].data.shape[2]
    self.W = bottom[0].data.shape[3]

    if not self.blobs:
      self.blobs.add_blob(self.num_out,self.C,self.ks,self.ks)#use add_blob to create weight blob
      #self.blobs[0].data[...]=np.random.randn(self.num_out,self.C,self.ks,self.ks)*math.sqrt(2.0/(self.ks*self.ks))

      self.blobs.add_blob(self.num_out,self.C,self.ks,self.ks)#use add_blob to create weight blob
      #self.blobs[1].data[...]=np.random.randn(self.num_out,self.C,self.ks,self.ks)*math.sqrt(2.0/(self.ks*self.ks))


    
  def reshape(self, bottom, top):
    #called before forward. adjust size of top blob based on bottom blob
    #print("reshape fpga_conv")
    #top[0].reshape(*bottom[0].data.shape)
    top[0].reshape(1,self.num_out,self.H,self.W)

  def forward(self, bottom, top):
    #forward propagation
    #print('start forward')
    #val = [5,5,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1]
    #my_mult = mult
    #print(my_mult(val,val))

    time1 = time.process_time();
    #bottom_stream=np.append(bottom[0].data.shape[1:],bottom[0].data.ravel())
    #bottom_stream = im2col_indices(bottom[0].data, 5, 5, 2, 1)
    bottom_mat = im2col_cython.im2col_cython(bottom[0].data,self.ks,self.ks,self.pad,self.st)
    bottom_mat = np.transpose(bottom_mat)
    bottom_stream = bottom_mat.ravel()
    bottom_stream = np.append((bottom_mat.shape[0],bottom_mat.shape[1]),bottom_stream)
    time2 = time.process_time();
    print("ravel time");
    print(time2 - time1);
    #bottom_stream=bottom_stream[2:]
    #print("bottom_stream: ")
    #print(bottom_stream)

    time3 = time.process_time();
    weight_stream=np.append((self.num_out,bottom_mat.shape[1]),self.blobs[0].data.ravel())
    time4 = time.process_time();
    print("ravel time");
    print(time4 - time3);

    #print("weight_stream: ")
    #print(weight_stream)

    #print("top_stream: ")
    time5 = time.process_time();
    result_stream=mult(bottom_stream,weight_stream).hw_value()
    time6 = time.process_time();
    print("hw time");
    print(time6 - time5);

    #print(result)

    time7 = time.process_time();
    #result = np.reshape(result, (-1, 32))
    #result = result.transpose(1,0)
    #result = result.ravel()
    result_stream=result_stream.astype(float)
    result = np.zeros(self.H*self.W*self.num_out)
    result=result.astype(float)
    acc8.acc8_func(result_stream,result);

    result = np.reshape(result, (-1, self.num_out))
    result = result.transpose(1,0)
    result = result.ravel()

    result=np.reshape(result,top[0].data.shape)
    time8 = time.process_time();
    print("reshape time");
    print(time8 - time7);

    # check correctness
    print("bottom_mat shape")
    print(bottom_mat.shape)
    bottom_mat=np.transpose(bottom_mat)
    # end

    top[0].data[...]=result

  def backward(self, top, propagate_down, bottom):
    #backward propagation
    #print("start backward")
    pass


