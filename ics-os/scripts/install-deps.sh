#!/bin/bash
# Install the dependencies for ICS-OS on Ubuntu/Debian (Updated for modern systems)

echo "Installing ICS-OS dependencies for Ubuntu/Debian..."
echo "This script has been updated for modern 64-bit systems with 32-bit cross-compilation support."

sudo apt-get update
sudo apt-get install -y \
    build-essential \
    nasm \
    qemu-system-x86 \
    qemu-utils \
    git \
    gcc-multilib \
    g++-multilib \
    libc6-dev-i386 \
    grub-common \
    grub-pc-bin \
    xorriso \
    mtools \
    dosfstools

echo "Dependencies installed successfully!"
echo "Note: TinyCC (tcc) is now installed via Docker for better compatibility."
echo "Use the Docker build environment for best results: docker-compose run ics-os-build" 
