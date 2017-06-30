# PYNQ-Classification
Python on Zynq FPGA for Convolutional Neural Networks (Beta)

This repository presents a fast prototyping framework, which is an Open Source framework designed to enable fast deployment of embedded Convolutional Neural Network (CNN) applications on PYNQ platforms.

For details on the project please watch my video at: 

https://youtu.be/DoA8hKBltV4

# 1. Install Caffe

- Install dependencies

sudo apt-get install libprotobuf-dev libleveldb-dev libsnappy-dev libopencv-dev libhdf5-serial-dev protobuf-compiler
sudo apt-get install --no-install-recommends libboost-all-dev
sudo apt-get install libgflags-dev libgoogle-glog-dev liblmdb-dev

- Install Protobuf 3

# Make sure you grab the latest version
curl -OL https://github.com/google/protobuf/releases/download/v3.2.0/protoc-3.2.0-linux-x86_64.zip

# Unzip
unzip protoc-3.2.0-linux-x86_64.zip -d protoc3

# Move only protoc* to /usr/bin/
sudo mv protoc3/bin/protoc /usr/bin/protoc

OR

pip3.4 install protobuf

- Add SWAP USB

mkswap /dev/sda
swapon /dev/sda

- Install Caffe

git clone https://github.com/BVLC/caffe.git

Copy my Makefile.config

make -j8 all
make test
make runtest

- Unmount SWAP USB

swapoff /dev/sda

- Python support

# Prepare Python binding (pycaffe)
cd python
for req in $(cat requirements.txt); do sudo pip install $req; done
echo "export PYTHONPATH=$(pwd):$PYTHONPATH " >> ~/.bash_profile # to be able to call "import caffe" from Python after reboot
source ~/.bash_profile # Update shell 
cd .

vim .bashrc
export PYTHONPATH=/home/xilinx/caffe/python