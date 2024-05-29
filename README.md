# TDDLinux
A Linux cmake version of TDD

## Installation:
These are required to compile and run the C++ TDD library file:
* Python 3.9
* Pytorch 2.1.0
* Clang 14.0.0
* CMake 3.22.1

### Libtorch
```
wget https://download.pytorch.org/libtorch/cpu/libtorch-cxx11-abi-shared-with-deps-2.2.1%2Bcpu.zip
unzip libtorch-cxx11-abi-shared-with-deps-2.2.1%2Bcpu.zip
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


## Running CMAKE
Using the path where the libtorch zip is unpacked to.
```
cmake -DCMAKE_PREFIX_PATH=/path/to/libtorch -S .
e.g.
cmake -DCMAKE_PREFIX_PATH=~/libtorch/share/cmake/ -S .
```

```
make TDDLinux
```

## Compiling to .so
Locate the conda xtl, xtensor, and libtorch from conda (typically in miniforge3/pkgs/). Find and insert the path to the include folder within each in:
```
clang -std=c++17 -std=gnu++17 -fPIC -I ~/miniforge3/pkgs/xtensor/include -I ~/miniforge3/pkgs/xtl/include -I ~/libtorch/include -I ~/libtorch/include/torch/csrc/api/include -c -o TDDLinux.o TDDLinux.cpp -Wno-everything -D_GLIBCXX_USE_CXX11_ABI=0
```
e.g.
```
clang -std=c++17 -std=gnu++17 -fPIC -I ~/miniforge3/pkgs/xtensor-0.25.0-h00ab1b0_0/include -I ~/miniforge3/pkgs/xtl-0.7.7-h00ab1b0_0/include -I ~/libtorch/include -I ~/libtorch/include/torch/csrc/api/include -c -o TDDLinux.o TDDLinux.cpp -Wno-everything -D_GLIBCXX_USE_CXX11_ABI=0
```
And then run:
```
clang -o libTDDLinux.so TDDLinux.o -shared -pthread -L ~/libtorch/lib -ltorch_cpu -lc10 -ltorch_python -Wl,-rpath ~/libtorch/lib -D_GLIBCXX_USE_CXX11_ABI=0
```
This final step should create the library file called: libTDDLinux.so


