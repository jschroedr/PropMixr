# Use the bleeding-edge GCC image containing the reflection merge
FROM gcc:latest

# Install standard workspace and data backend dependencies
RUN apt-get update && apt-get install -y \
    cmake \
    git \
    nlohmann-json3-dev \
    libyaml-cpp-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace

# Force the environment to use GCC with C++26 reflection flags
ENV CC=gcc
ENV CXX=g++
ENV CXXFLAGS="-std=c++26 -freflection"