FROM ubuntu:20.04

ENV HOME /app

WORKDIR /build

### Install requirements and dev tools
ENV DEBIAN_FRONTEND=noninteractive
RUN apt update && apt install -y \
        wget git build-essential autoconf libtool pkg-config \
        cmake libboost-all-dev \
        curl lsb-release wget software-properties-common \
        python3 ninja-build
RUN wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add
RUN apt-add-repository "deb http://apt.llvm.org/focal/ llvm-toolchain-focal-15 main"
RUN apt install -y llvm-15 clang-15 lld-15

RUN rm -rf /build

WORKDIR $HOME

COPY entrypoint.sh /entrypoint.sh
ENTRYPOINT ["/entrypoint.sh"]
