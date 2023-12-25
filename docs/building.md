# Building

## Checkout source code

Checkout the source code and its dependencies from GitHub:

```shell
git clone --recursive https://github.com/fuzzuf/fuzzuf-cc
```

## Building with a Development Container

We provide a container image to set up a development environment for fuzzuf-cc quickly. Install Docker and run:

```shell
./shell.sh
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

## Building Manually

### Recommended Environment

- Ubuntu 22.04
- Ubuntu 20.04

### Dependencies

The following dependencies must be met to build fuzzuf-cc.

- [gcc](https://gcc.gnu.org/) 7 or higher
  - 8 or higher is recommended
  - 10 or higher is required for static analysis
- [CMake](https://cmake.org/) 3.10 or higher
- [Boost C++ library](https://www.boost.org/) 1.53.0 or higher
- [CPython](https://www.python.org/) 3.7 or higher
- [LLVM](https://llvm.org/) 15 or higher

### Manual Build

To build fuzzuf-cc on Ubuntu 22.04 and 20.04, install the dependencies as follows:

```shell
sudo apt update
sudo apt install -y build-essential cmake git libboost-all-dev python3 software-properties-common wget
wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add
sudo apt-add-repository "deb http://apt.llvm.org/focal/ llvm-toolchain-focal-15 main" # On Ubuntu 20.04
sudo apt install -y llvm-15 clang-15 lld-15
```

See [LLVM Debian/Ubuntu nightly packages](https://apt.llvm.org) to find the appropriate apt repository for your environment.

Next, clone the repository and build fuzzuf-cc:

```shell
git clone --recursive https://github.com/fuzzuf/fuzzuf-cc
cd fuzzuf-cc
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)
```

### Run Unit Tests

Build the `test` target to run unit tests:

```shell
cmake --build build --target test
```
