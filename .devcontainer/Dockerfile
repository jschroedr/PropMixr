# Use the bleeding-edge GCC image containing the reflection merge
FROM gcc:latest

# Install standard workspace and data backend dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libpugixml-dev \
    && rm -rf /var/lib/apt/lists/*

# Clone the repository and its nested submodules
RUN git clone --recursive https://github.com/biojppm/rapidyaml.git && \
    cd rapidyaml && \
    mkdir build && cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release -DRYML_BUILD_TESTS=OFF .. && \
    make -j$(nproc) && \
    make install

WORKDIR /workspace

# Force the environment to use GCC with C++26 reflection flags
ENV CC=gcc
ENV CXX=g++
ENV CXXFLAGS="-std=c++26 -freflection"