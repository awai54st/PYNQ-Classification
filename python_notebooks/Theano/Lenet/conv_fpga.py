import im2col_lasagne_cython
import acc8
import numpy as np
from collections import namedtuple
from pynq import PL
from pynq import MMIO
import inspect
import math
import time


import theano.tensor as T
import lasagne
from lasagne import init
from lasagne import nonlinearities
from lasagne.utils import as_tuple
from lasagne.theano_extensions import conv, padding

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
    self.functions['xilinx.com:hls:lenet:1.0'] = Function(in_ports=[2],out_ports=[2],name=None)
    self.functions['Xilinx:hls:stream_mult:1.0'] = Function(in_ports=[3,4],out_ports=[3],name=None)
    self.functions['Xilinx:hls:simple_sum:1.0']=Function(in_ports=[1],out_ports=[1],name=None)
    self.functions['Xilinx:hls:mult_constant:1.0']=Function(in_ports=[6],out_ports=[5],name='mult_constant_0')
    
metadata=FunctionMetadata()
    
class StreamingSwitch:
  def __init__(self,name):
    base_addr=int(PL.ip_dict["SEG_{0}_Reg".format(name)][0],16)
    self.mmio=MMIO(base_addr,256)
    self.reset()

  def set_route(self,in_port,out_port):
    #print('SWITCH: setting route{0} to {1}'.format(in_port,out_port))
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
    #print('Send DMA: create index {0} name {1}'.format(index,metadata.DMA_names[index]))
    base_addr=int(PL.ip_dict["SEG_{0}_Reg".format(metadata.DMA_names[index])][0],16)
    #print('Send DMA: base_address {0:x}'.format(base_addr))
    self.dma=DMA(base_addr,0)
    self.ports=metadata.DMA[index]

  def set_data(self,data,dtype):
    self.length=len(data) * dtype.itemsize
    #print('Send DMA: sending {0} bytes'.format(self.length))
    self.dma.create_buf(self.length)
    ffi=pynq.drivers.dma.ffi
    buf=ffi.buffer(self.dma.buf,self.length)
    view=np.frombuffer(buf,dtype,-1)
    np.copyto(view,data,casting='same_kind')

  def transfer(self):
    #print('Send DMA: transfer started')
    self.dma.transfer(self.length,0)

  def wait(self):
    self.dma.wait()
    #print('Send DMA: transfer finished')

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
Overlay('/home/xilinx/jupyter_notebooks/PYNQ_CNN/Theano/Lenet/Bitstream/lenet.bit').download()

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
  ## Timer Start
  start_time = time.process_time()
  ret_dma.transfer(8388607,1)
  for d in dma:
    d.transfer()
  for d in dma:
    d.wait()
  ## Timer End
  end_time = time.process_time()
  print("Elapsed Test Time: ", end_time-start_time)
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

@hardware_function('xilinx.com:hls:lenet:1.0')
def double(vs:[np.int32]) -> [np.int32]:
  print("In double")
  return [v*2 for v in vs]

@hardware_function('Xilinx:hls:stream_mult:1.0')
def mult(a:[np.int32], b:[np.int32]) -> [np.int32]:
  return [a1 * b1 for (a1,b1) in zip (a,b)]

def conv_output_length(input_length, filter_size, stride, pad=0):
    """Helper function to compute the output size of a convolution operation

    This function computes the length along a single axis, which corresponds
    to a 1D convolution. It can also be used for convolutions with higher
    dimensionalities by using it individually for each axis.

    Parameters
    ----------
    input_length : int
        The size of the input.

    filter_size : int
        The size of the filter.

    stride : int
        The stride of the convolution operation.

    pad : int, 'full' or 'same' (default: 0)
        By default, the convolution is only computed where the input and the
        filter fully overlap (a valid convolution). When ``stride=1``, this
        yields an output that is smaller than the input by ``filter_size - 1``.
        The `pad` argument allows you to implicitly pad the input with zeros,
        extending the output size.

        A single integer results in symmetric zero-padding of the given size on
        both borders.

        ``'full'`` pads with one less than the filter size on both sides. This
        is equivalent to computing the convolution wherever the input and the
        filter overlap by at least one position.

        ``'same'`` pads with half the filter size on both sides (one less on
        the second side for an even filter size). When ``stride=1``, this
        results in an output size equal to the input size.

    Returns
    -------
    int
        The output size corresponding to the given convolution parameters.

    Raises
    ------
    RuntimeError
        When an invalid padding is specified, a `RuntimeError` is raised.
    """
    if input_length is None:
        return None
    if pad == 'valid':
        output_length = input_length - filter_size + 1
    elif pad == 'full':
        output_length = input_length + filter_size - 1
    elif pad == 'same':
        output_length = input_length
    elif isinstance(pad, int):
        output_length = input_length + 2 * pad - filter_size + 1
    else:
        raise ValueError('Invalid pad: {0}'.format(pad))

    # This is the integer arithmetic equivalent to
    # np.ceil(output_length / stride)
    output_length = (output_length + stride - 1) // stride

    return output_length

def FPGAWeightLoader(W, index, IFMDim, OFMDim, PadDim, flip_filters=True):
    print('weight shape', W.shape)
    KerDim = W.shape[2]
    IFMCH = W.shape[1]
    OFMCH = W.shape[0]
    batch_size = 0
    
    ## flip
    if flip_filters==True:
        W = W[...,::-1,::-1]
    
    print('Loading Started for Layer ',index)
    kernel_val = W.transpose(0,2,3,1).ravel()
    kernel_val = kernel_val * 128
    kernel = np.append([index, batch_size, KerDim, IFMCH, IFMDim, OFMCH, OFMDim, PadDim], kernel_val)
    kernel = kernel.astype(int)
    ker_param = double(kernel).hw_value()
    print('Loading Finished for Layer ',index)
    
    return
    
def FPGAQuickTest(test_data, batch_size, OFMDim, OFMCH):
    input_mat_tmp = test_data[0:batch_size]
    input_val = input_mat_tmp.transpose(0,2,3,1).ravel()
    input_val = input_val * 18
    input_val = np.append([0, batch_size, 5, 1, 28, 10, 1, 2], input_val)
    input_val = input_val.astype(int)
    output_fpga = double(input_val).hw_value()
    output_mat_tmp = output_fpga[8:].reshape(batch_size, OFMDim*OFMDim, OFMCH)
    output_mat_tmp = output_mat_tmp.transpose(0, 2, 1)
    output_mat_tmp = output_mat_tmp.reshape(batch_size, OFMCH, OFMDim, OFMDim)
    output_mat_tmp = output_mat_tmp / 18
    
    return output_mat_tmp

class FPGA_LENET(lasagne.layers.Layer):
    def get_output_for(self, input, **kwargs):
        input_mat = input.eval()
        batch_size = input_mat.shape[0]
        input_val = input_mat.transpose(0,2,3,1).ravel()
        input_val = input_val * 18
        input_val = np.append([0, batch_size, 5, 1, 28, 10, 1, 2], input_val)
        input_val = input_val.astype(int)
        output_fpga = double(input_val).hw_value()
        output_mat_tmp = output_fpga[8:].reshape(batch_size, -1)
        output_mat_tmp = output_mat_tmp / 18
        return output_mat_tmp
    
class FPGAConv2DLayer(lasagne.layers.Layer):
    """
    lasagne.layers.Conv2DLayer(incoming, num_filters, filter_size,
    stride=(1, 1), pad=0, untie_biases=False,
    W=lasagne.init.GlorotUniform(), b=lasagne.init.Constant(0.),
    nonlinearity=lasagne.nonlinearities.rectify,
    convolution=theano.tensor.nnet.conv2d, **kwargs)

    2D convolutional layer

    Performs a 2D convolution on its input and optionally adds a bias and
    applies an elementwise nonlinearity.

    Parameters
    ----------
    incoming : a :class:`Layer` instance or a tuple
        The layer feeding into this layer, or the expected input shape. The
        output of this layer should be a 4D tensor, with shape
        ``(batch_size, num_input_channels, input_rows, input_columns)``.

    num_filters : int
        The number of learnable convolutional filters this layer has.

    filter_size : int or iterable of int
        An integer or a 2-element tuple specifying the size of the filters.

    stride : int or iterable of int
        An integer or a 2-element tuple specifying the stride of the
        convolution operation.

    pad : int, iterable of int, 'full', 'same' or 'valid' (default: 0)
        By default, the convolution is only computed where the input and the
        filter fully overlap (a valid convolution). When ``stride=1``, this
        yields an output that is smaller than the input by ``filter_size - 1``.
        The `pad` argument allows you to implicitly pad the input with zeros,
        extending the output size.

        A single integer results in symmetric zero-padding of the given size on
        all borders, a tuple of two integers allows different symmetric padding
        per dimension.

        ``'full'`` pads with one less than the filter size on both sides. This
        is equivalent to computing the convolution wherever the input and the
        filter overlap by at least one position.

        ``'same'`` pads with half the filter size on both sides (one less on
        the second side for an even filter size). When ``stride=1``, this
        results in an output size equal to the input size.

        ``'valid'`` is an alias for ``0`` (no padding / a valid convolution).

        Note that ``'full'`` and ``'same'`` can be faster than equivalent
        integer values due to optimizations by Theano.

    untie_biases : bool (default: False)
        If ``False``, the layer will have a bias parameter for each channel,
        which is shared across all positions in this channel. As a result, the
        `b` attribute will be a vector (1D).

        If True, the layer will have separate bias parameters for each
        position in each channel. As a result, the `b` attribute will be a
        3D tensor.

    W : Theano shared variable, numpy array or callable
        An initializer for the weights of the layer. This should initialize the
        layer weights to a 4D array with shape
        ``(num_filters, num_input_channels, filter_rows, filter_columns)``.
        See :func:`lasagne.utils.create_param` for more information.

    b : Theano shared variable, numpy array, callable or None
        An initializer for the biases of the layer. If None is provided, the
        layer will have no biases. This should initialize the layer biases to
        a 1D array with shape ``(num_filters,)`` if `untied_biases` is set to
        ``False``. If it is set to ``True``, its shape should be
        ``(num_filters, input_rows, input_columns)`` instead.
        See :func:`lasagne.utils.create_param` for more information.

    nonlinearity : callable or None
        The nonlinearity that is applied to the layer activations. If None
        is provided, the layer will be linear.

    convolution : callable
        The convolution implementation to use. Usually it should be fine to
        leave this at the default value.

    **kwargs
        Any additional keyword arguments are passed to the `Layer` superclass.

    Attributes
    ----------
    W : Theano shared variable
        Variable representing the filter weights.

    b : Theano shared variable
        Variable representing the biases.

    Notes
    -----
    Theano's underlying convolution (:func:`theano.tensor.nnet.conv.conv2d`)
    only supports ``pad=0`` and ``pad='full'``. This layer emulates other modes
    by cropping a full convolution or explicitly padding the input with zeros.
    """
    def __init__(self, incoming, num_filters, filter_size, stride=(1, 1),
                 pad=0, untie_biases=False,
                 W=init.GlorotUniform(), b=init.Constant(0.),
                 nonlinearity=nonlinearities.rectify,
                 convolution=T.nnet.conv2d, **kwargs):
        super(FPGAConv2DLayer, self).__init__(incoming, **kwargs)
        if nonlinearity is None:
            self.nonlinearity = nonlinearities.identity
        else:
            self.nonlinearity = nonlinearity

        self.num_filters = num_filters
        self.filter_size = as_tuple(filter_size, 2)
        self.stride = as_tuple(stride, 2)
        self.untie_biases = untie_biases
        self.convolution = convolution

        if pad == 'valid':
            self.pad = (0, 0)
        elif pad in ('full', 'same'):
            self.pad = pad
        else:
            self.pad = as_tuple(pad, 2, int)

        self.W = self.add_param(W, self.get_W_shape(), name="W")
        if b is None:
            self.b = None
        else:
            if self.untie_biases:
                biases_shape = (num_filters, self.output_shape[2], self.
                                output_shape[3])
            else:
                biases_shape = (num_filters,)
            self.b = self.add_param(b, biases_shape, name="b",
                                    regularizable=False)

    def get_W_shape(self):
        """Get the shape of the weight matrix `W`.

        Returns
        -------
        tuple of int
            The shape of the weight matrix.
        """
        num_input_channels = self.input_shape[1]
        return (self.num_filters, num_input_channels, self.filter_size[0],
                self.filter_size[1])

    def get_output_shape_for(self, input_shape):
        pad = self.pad if isinstance(self.pad, tuple) else (self.pad,) * 2

        output_rows = conv_output_length(input_shape[2],
                                         self.filter_size[0],
                                         self.stride[0],
                                         pad[0])

        output_columns = conv_output_length(input_shape[3],
                                            self.filter_size[1],
                                            self.stride[1],
                                            pad[1])

        return (input_shape[0], self.num_filters, output_rows, output_columns)

    def get_output_for(self, input, input_shape=None, **kwargs):
        # The optional input_shape argument is for when get_output_for is
        # called directly with a different shape than self.input_shape.
        if input_shape is None:
            input_shape = self.input_shape

        #print("Input Shape",input_shape)
        #print("Filter Shape",self.get_W_shape())
        
        ############################################################
        
        if self.stride == (1, 1) and self.pad == 'same':
            shift_x = (self.filter_size[0] - 1) // 2
            shift_y = (self.filter_size[1] - 1) // 2
        else:
            # no padding needed, or explicit padding of input needed
            if self.pad == 'full':
                pad = [(0, 0), (0, 0)]
            elif self.pad == 'same':
                pad = [(self.filter_size[0] // 2,
                        (self.filter_size[0] - 1) // 2),
                       (self.filter_size[1] // 2,
                        (self.filter_size[1] - 1) // 2)]
            else:
                pad = [(self.pad[0], self.pad[0]), (self.pad[1], self.pad[1])]

        input_mat = input.eval()
        if input_mat.ndim == 3:
            input_mat = input_mat.reshape(1, input_mat.shape)
        print('input_mat shape', input_mat.shape)
            
        #elif input_mat.ndim == 4:
        batch_size = input_mat.shape[0]
        KerDim = self.filter_size[0]
        print('KerDim', KerDim)
        IFMCH = input_mat.shape[1]
        print('IFMCH', IFMCH)
        IFMDim = input_mat.shape[2]
        print('IFMDim', IFMDim)
        OFMCH = self.num_filters
        print('OFMCH', OFMCH)
        OFMDim = self.output_shape[2]
        print('OFMDim', OFMDim)
        PadDim = pad[0][0]
        print('PadDim', PadDim)
          
        batch_size_sub = 40
        inx_processed = 0
        for i in range(int(batch_size/batch_size_sub)):
            input_mat_tmp = input_mat[batch_size_sub*i:batch_size_sub*i+batch_size_sub]
            input_val = input_mat_tmp.transpose(0,2,3,1).ravel()
            input_val = input_val * 6
            input_val = np.append([0, batch_size_sub, KerDim, IFMCH, IFMDim, OFMCH, OFMDim, PadDim], input_val)
            input_val = input_val.astype(int)
            output_fpga = double(input_val).hw_value()
            output_mat_tmp = output_fpga[8:].reshape(batch_size_sub, OFMDim*OFMDim, OFMCH)
            output_mat_tmp = output_mat_tmp.transpose(0, 2, 1)
            output_mat_tmp = output_mat_tmp.reshape(batch_size_sub, OFMCH, OFMDim, OFMDim)
            output_mat_tmp = output_mat_tmp / 768
            if i==0:
                conved = output_mat_tmp
            else:
                conved = np.append(conved, output_mat_tmp, axis=0)
            inx_processed = batch_size_sub*i+batch_size_sub
        
        if inx_processed < batch_size:
            batch_size_sub = batch_size - inx_processed
            input_mat_tmp = input_mat[inx_processed:]
            input_val = input_mat_tmp.transpose(0,2,3,1).ravel()
            input_val = input_val * 6
            input_val = np.append([0, batch_size_sub, KerDim, IFMCH, IFMDim, OFMCH, OFMDim, PadDim], input_val)
            input_val = input_val.astype(int)
            output_fpga = double(input_val).hw_value()
            output_mat_tmp = output_fpga[8:].reshape(batch_size_sub, OFMDim*OFMDim, OFMCH)
            output_mat_tmp = output_mat_tmp.transpose(0, 2, 1)
            output_mat_tmp = output_mat_tmp.reshape(batch_size_sub, OFMCH, OFMDim, OFMDim)
            output_mat_tmp = output_mat_tmp / 768
            if inx_processed==0:
                conved = output_mat_tmp
            else:
                conved = np.append(conved, output_mat_tmp, axis=0)
        
        
        

        #################################################################
            
        if self.b is None:
            activation = conved
        elif self.untie_biases:
            activation = conved + self.b.dimshuffle('x', 0, 1, 2)
        else:
            activation = conved + self.b.dimshuffle('x', 0, 'x', 'x')

        return self.nonlinearity(activation)

    
    
    
class Conv2DLayer(lasagne.layers.Layer):
    def __init__(self, incoming, num_filters, filter_size, stride=(1, 1),
                 pad=0, untie_biases=False,
                 W=init.GlorotUniform(), b=init.Constant(0.),
                 nonlinearity=nonlinearities.rectify,
                 convolution=T.nnet.conv2d, **kwargs):
        super(Conv2DLayer, self).__init__(incoming, **kwargs)
        if nonlinearity is None:
            self.nonlinearity = nonlinearities.identity
        else:
            self.nonlinearity = nonlinearity

        self.num_filters = num_filters
        self.filter_size = as_tuple(filter_size, 2)
        self.stride = as_tuple(stride, 2)
        self.untie_biases = untie_biases
        self.convolution = convolution

        if pad == 'valid':
            self.pad = (0, 0)
        elif pad in ('full', 'same'):
            self.pad = pad
        else:
            self.pad = as_tuple(pad, 2, int)

        self.W = self.add_param(W, self.get_W_shape(), name="W")
        if b is None:
            self.b = None
        else:
            if self.untie_biases:
                biases_shape = (num_filters, self.output_shape[2], self.
                                output_shape[3])
            else:
                biases_shape = (num_filters,)
            self.b = self.add_param(b, biases_shape, name="b",
                                    regularizable=False)

    def get_W_shape(self):
        """Get the shape of the weight matrix `W`.

        Returns
        -------
        tuple of int
            The shape of the weight matrix.
        """
        num_input_channels = self.input_shape[1]
        return (self.num_filters, num_input_channels, self.filter_size[0],
                self.filter_size[1])

    def get_output_shape_for(self, input_shape):
        pad = self.pad if isinstance(self.pad, tuple) else (self.pad,) * 2

        output_rows = conv_output_length(input_shape[2],
                                         self.filter_size[0],
                                         self.stride[0],
                                         pad[0])

        output_columns = conv_output_length(input_shape[3],
                                            self.filter_size[1],
                                            self.stride[1],
                                            pad[1])

        return (input_shape[0], self.num_filters, output_rows, output_columns)

    def get_output_for(self, input, input_shape=None, **kwargs):
        # The optional input_shape argument is for when get_output_for is
        # called directly with a different shape than self.input_shape.
        if input_shape is None:
            input_shape = self.input_shape
        
        ############################################################
        
        if self.stride == (1, 1) and self.pad == 'same':
            # simulate same convolution by cropping a full convolution
            conved = self.convolution(input, self.W, subsample=self.stride,
                                      image_shape=input_shape,
                                      filter_shape=self.get_W_shape(),
                                      border_mode='full')
            shift_x = (self.filter_size[0] - 1) // 2
            shift_y = (self.filter_size[1] - 1) // 2
            conved = conved[:, :, shift_x:input.shape[2] + shift_x,
                            shift_y:input.shape[3] + shift_y]
        else:
            # no padding needed, or explicit padding of input needed
            if self.pad == 'full':
                border_mode = 'full'
                pad = [(0, 0), (0, 0)]
            elif self.pad == 'same':
                border_mode = 'valid'
                pad = [(self.filter_size[0] // 2,
                        (self.filter_size[0] - 1) // 2),
                       (self.filter_size[1] // 2,
                        (self.filter_size[1] - 1) // 2)]
            else:
                border_mode = 'valid'
                pad = [(self.pad[0], self.pad[0]), (self.pad[1], self.pad[1])]
            if pad != [(0, 0), (0, 0)]:
                input = padding.pad(input, pad, batch_ndim=2)
                input_shape = (input_shape[0], input_shape[1],
                               None if input_shape[2] is None else
                               input_shape[2] + pad[0][0] + pad[0][1],
                               None if input_shape[3] is None else
                               input_shape[3] + pad[1][0] + pad[1][1])
            conved = self.convolution(input, self.W, subsample=self.stride,
                                      image_shape=input_shape,
                                      filter_shape=self.get_W_shape(),
                                      border_mode=border_mode)
            
        if self.b is None:
            activation = conved
        elif self.untie_biases:
            activation = conved + self.b.dimshuffle('x', 0, 1, 2)
        else:
            activation = conved + self.b.dimshuffle('x', 0, 'x', 'x')
        activation = conved

        return self.nonlinearity(activation)


