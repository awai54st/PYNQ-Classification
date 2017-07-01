# PYNQ-Classification
Python on Zynq FPGA for Convolutional Neural Networks (Alpha)

This repository presents a fast prototyping framework, which is an Open Source framework designed to enable fast deployment of embedded Convolutional Neural Network (CNN) applications on PYNQ platforms.

The project demo accepts pre-trained CNN models in either Caffe or Theano syntax, hence the step 1 and 2 introduces how to install Caffe and Theano (with Lasagne) on PYNQ. Step 3 explains how to download and run the demo for LeNet and CIFAR-10 (Caffe "quick" version) models.

For details on the project please watch my video at: 

https://youtu.be/DoA8hKBltV4

## 1. Install Caffe on PYNQ

- Install dependencies

```
sudo apt-get install libprotobuf-dev libleveldb-dev libsnappy-dev libopencv-dev libhdf5-serial-dev protobuf-compiler
sudo apt-get install --no-install-recommends libboost-all-dev
sudo apt-get install libgflags-dev libgoogle-glog-dev liblmdb-dev
```

- Install Protobuf 3

`pip3 install protobuf`

- Add SWAP USB

```
mkswap /dev/sda
swapon /dev/sda
```

- Install Caffe

```
cd /home/xilinx
git clone https://github.com/BVLC/caffe.git`
```

Copy the Makefile.config (which is provided by our project) to caffe root directory

```
make all
make test
make runtest
```

- Unmount SWAP USB

`swapoff /dev/sda`

- Install pycaffe with Python 3

```
cd python
for req in $(cat requirements.txt); do sudo pip install $req; done
echo "export PYTHONPATH=$(pwd):$PYTHONPATH " >> ~/.bash_profile # to be able to call "import caffe" from Python after reboot
source ~/.bash_profile # Update shell 
cd .
```

- Export PYTHONPATH

```
vim .bashrc
export PYTHONPATH=/home/xilinx/caffe/python
```

## 2. Install Theano with Lasagne on PYNQ

```
pip install -r https://raw.githubusercontent.com/Lasagne/Lasagne/v0.1/requirements.txt
pip install Lasagne==0.1
```


