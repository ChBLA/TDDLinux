# TDDLinux
A Linux cmake version of TDD

## Installation:

### Libtorch
```
wget https://download.pytorch.org/libtorch/cpu/libtorch-cxx11-abi-shared-with-deps-2.2.1%2Bcpu.zip
unzip libtorch-cxx11-abi-shared-with-deps-2.2.1%2Bcpu.zip
```

```
conda install pytorch-cpu
```

### xtensor


### xtl


## Running CMAKE
```
cmake -DCMAKE_PREFIX_PATH=/path/to/libtorch -S .
e.g.
cmake -DCMAKE_PREFIX_PATH=~/libtorch/share/cmake/ -S .
```

```
make TDDLinux
```

## Compiling to .so
```
g++ -I ~/miniforge3/pkgs/xtensor-0.25.0-h00ab1b0_0/include -I ~/miniforge3/pkgs/xtl-0.7.7-h00ab1b0_0/include -std=c++20 -c -fPIC -shared TDDLinux.cpp -o TDDLinux.o -w

g++ -I ~/miniforge3/pkgs/xtensor-0.25.0-h00ab1b0_0/include -I ~/miniforge3/pkgs/xtl-0.7.7-h00ab1b0_0/include -std=c++20 -fPIC -shared -o libTDDLinux.so TDDLinux.cpp -w
```
