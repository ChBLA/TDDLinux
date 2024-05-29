# TDDLinux
A Linux cmake version of TDD (CUDA Version)

## Installation:
These are required to compile and run the C++ TDD library file:
* Python 3.9
* Pytorch 2.1.0
* Clang 14.0.0
* CMake 3.22.1

### Libtorch CUDA
```
wget https://download.pytorch.org/libtorch/cu121/libtorch-cxx11-abi-shared-with-deps-2.1.0%2Bcu121.zip
unzip libtorch-cxx11-abi-shared-with-deps-2.1.0+cu121.zip -d ~/libtorch_cuda/
```
### Pytorch for cpu
```
conda install pytorch-cpu==2.1.2 torchvision==0.16.2 torchaudio==2.1.2 cpuonly -c pytorch
```

### xtensor
```
mamba install -c conda-forge xtensor
```

### xtl
```
mamba install -c conda-forge xtl
```

### CUDA
Installing CUDA in root as:
```
wget https://developer.download.nvidia.com/compute/cuda/12.4.1/local_installers/cuda_12.4.1_550.54.15_linux.run
sudo sh cuda_12.4.1_550.54.15_linux.run
```
And then install CUDA Pytorch for Python as:
```
sudo apt-get -y install cuda
conda install pytorch-gpu==2.1.2
conda install pytorch-cuda=12.1 -c pytorch -c nvidia
```

### CUDNN
From root, run:
```
wget https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2004/x86_64/cuda-keyring_1.1-1_all.deb
sudo dpkg -i cuda-keyring_1.1-1_all.deb
sudo apt-get update
sudo apt-get -y install cudnn
```

## Running CMAKE
Using the path where the libtorch zip is unpacked to.
```
cmake -DCMAKE_PREFIX_PATH=/path/to/libtorch_cuda .
e.g.
cmake -DCMAKE_PREFIX_PATH=~/libtorch_cuda/libtorch/share/cmake/ .
```

```
make TDDLinux
```

## Compiling to .so
Locate the conda xtl, xtensor, and libtorch from conda (typically in miniforge3/pkgs/). Find and insert the path to the include folder within each in:
```
clang -std=c++17 -std=gnu++17 -fPIC -I ~/miniforge3/pkgs/xtensor/include -I ~/miniforge3/pkgs/xtl/include -I ~/libtorch_cuda/libtorch/include -I ~/libtorch_cuda/libtorch/include/torch/csrc/api/include -c -o TDDLinux.o TDDLinux.cpp -Wno-everything -D_GLIBCXX_USE_CXX11_ABI=0
```
e.g.
```
clang -std=c++17 -std=gnu++17 -fPIC -I ~/miniforge3/pkgs/xtensor-0.25.0-h00ab1b0_0/include -I ~/miniforge3/pkgs/xtl-0.7.7-h00ab1b0_0/include -I ~/libtorch_cuda/libtorch/include -I ~/libtorch_cuda/libtorch/include/torch/csrc/api/include -c -o TDDLinux.o TDDLinux.cpp -Wno-everything -D_GLIBCXX_USE_CXX11_ABI=0
```
And then run:
```
clang -o libTDDLinuxCUDA.so TDDLinux.o -shared -pthread -L ~/libtorch_cuda/libtorch/lib -ltorch_cuda -lc10 -ltorch_python -Wl,-rpath ~/libtorch_cuda/libtorch/lib -D_GLIBCXX_USE_CXX11_ABI=0
```
This final step should create the library file called: libTDDLinuxCUDA.so

## Issues:
You may need to manually add C++ path and Cuda path as, e.g.:
```
export CUDACXX=/usr/local/cuda/bin/nvcc
export CPLUS_INCLUDE_PATH=/usr/include/c++/11:/usr/include/x86_64-linux-gnu/c++/11
```
Otherwise make and issue and the authors will try to help.