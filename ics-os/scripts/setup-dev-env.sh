#!/bin/bash

# ICS-OS Development Environment Setup Script
# This script helps set up the development environment for ICS-OS

set -e

echo "ICS-OS Development Environment Setup"
echo "===================================="

# Check if running on 64-bit system
if [ "$(uname -m)" != "x86_64" ]; then
    echo "Warning: This script is designed for 64-bit (x86_64) systems."
    echo "Current architecture: $(uname -m)"
fi

# Check OS
if [ -f /etc/os-release ]; then
    . /etc/os-release
    echo "Detected OS: $NAME $VERSION"
else
    echo "Cannot detect OS version"
fi

# Function to install dependencies on Ubuntu/Debian
install_ubuntu_deps() {
    echo "Installing dependencies for Ubuntu/Debian..."
    sudo apt update
    sudo apt install -y \
        build-essential \
        nasm \
        qemu-system-x86 \
        qemu-utils \
        git \
        gcc-multilib \
        g++-multilib \
        libc6-dev-i386 \
        grub-common \
        xorriso \
        mtools \
        dosfstools \
        docker.io \
        docker-compose
    
    # Add user to docker group
    sudo usermod -a -G docker $USER
    echo "Added $USER to docker group. You may need to log out and back in."
}

# Function to test the build environment
test_build() {
    echo "Testing build environment..."
    
    # Test 32-bit compilation
    echo "Testing 32-bit cross-compilation..."
    cat > test_32bit.c << 'EOF'
#include <stdio.h>
int main() {
    printf("32-bit test successful\n");
    return 0;
}
EOF
    
    if gcc -m32 -o test_32bit test_32bit.c 2>/dev/null; then
        echo "✓ 32-bit compilation test passed"
        rm -f test_32bit test_32bit.c
    else
        echo "✗ 32-bit compilation test failed"
        echo "Make sure gcc-multilib and libc6-dev-i386 are installed"
        rm -f test_32bit test_32bit.c
        exit 1
    fi
    
    # Test NASM
    if command -v nasm >/dev/null 2>&1; then
        echo "✓ NASM assembler found: $(nasm --version | head -n1)"
    else
        echo "✗ NASM assembler not found"
        exit 1
    fi
    
    # Test QEMU
    if command -v qemu-system-i386 >/dev/null 2>&1; then
        echo "✓ QEMU i386 emulator found: $(qemu-system-i386 --version | head -n1)"
    else
        echo "✗ QEMU i386 emulator not found"
        exit 1
    fi
    
    # Test Docker
    if command -v docker >/dev/null 2>&1; then
        echo "✓ Docker found: $(docker --version)"
        if docker ps >/dev/null 2>&1; then
            echo "✓ Docker is running and accessible"
        else
            echo "! Docker found but not accessible (try: sudo usermod -a -G docker $USER)"
        fi
    else
        echo "✗ Docker not found"
    fi
}

# Main installation logic
case "$ID" in
    ubuntu|debian)
        install_ubuntu_deps
        ;;
    *)
        echo "Unsupported OS: $ID"
        echo "Please install the following packages manually:"
        echo "- build-essential (or equivalent)"
        echo "- nasm"
        echo "- qemu-system-x86"
        echo "- gcc with 32-bit support"
        echo "- docker and docker-compose"
        ;;
esac

echo ""
test_build

echo ""
echo "Setup complete! You can now:"
echo "1. Build using Docker: docker-compose run ics-os-build"
echo "2. Build natively: make clean && make"
echo "3. Test on AMD64: make boot-floppy-amd64"
echo ""
echo "For more information, see the updated README.md"
