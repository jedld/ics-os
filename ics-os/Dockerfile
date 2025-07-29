############################################################
# ICS-OS Development
############################################################
FROM ubuntu:22.04

# Prevent interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Update package lists and install basic dependencies
RUN apt-get update && apt-get upgrade -y

# Install modern build tools with 32-bit support
RUN apt-get install -y \
    build-essential \
    nasm \
    qemu-system-x86 \
    qemu-utils \
    git \
    gcc-multilib \
    g++-multilib \
    libc6-dev-i386 \
    sudo \
    wget \
    curl \
    grub-common \
    xorriso \
    mtools \
    dosfstools \
    && rm -rf /var/lib/apt/lists/*

# Try to install TinyCC from package manager (simpler and more compatible)
RUN apt-get update && apt-get install -y tcc || echo "TCC not available from packages, skipping..."

RUN mkdir -p /home/ics-os

