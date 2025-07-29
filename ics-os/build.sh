#!/bin/bash

# ICS-OS Build and Test Script
# Comprehensive script for building and testing ICS-OS on modern systems

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if we're in the right directory
if [ ! -f "Makefile" ] || [ ! -d "kernel" ]; then
    print_error "This script must be run from the ics-os directory"
    exit 1
fi

# Function to build using Docker
build_docker() {
    print_status "Building ICS-OS using Docker..."
    if ! command -v docker >/dev/null 2>&1; then
        print_error "Docker not found. Please install Docker first."
        exit 1
    fi
    
    if ! docker-compose run --rm ics-os-build bash -c "cd /home/ics-os && make clean && make"; then
        print_error "Docker build failed"
        exit 1
    fi
    print_success "Docker build completed"
}

# Function to build natively
build_native() {
    print_status "Building ICS-OS natively..."
    
    # Check for required tools
    for tool in gcc nasm ld; do
        if ! command -v $tool >/dev/null 2>&1; then
            print_error "$tool not found. Please install build dependencies."
            exit 1
        fi
    done
    
    # Check for 32-bit support
    if ! gcc -m32 -E - </dev/null >/dev/null 2>&1; then
        print_error "32-bit GCC support not found. Install gcc-multilib and libc6-dev-i386"
        exit 1
    fi
    
    make clean
    make
    print_success "Native build completed"
}

# Function to create floppy image
create_floppy() {
    print_status "Creating floppy disk image..."
    if [ "$EUID" -ne 0 ]; then
        print_warning "Floppy creation requires root privileges"
        sudo make floppy
    else
        make floppy
    fi
    print_success "Floppy image created: ics-os-floppy.img"
}

# Function to test the OS
test_os() {
    local mode=$1
    print_status "Testing ICS-OS in $mode mode..."
    
    if ! command -v qemu-system-i386 >/dev/null 2>&1 && ! command -v qemu-system-x86_64 >/dev/null 2>&1; then
        print_error "QEMU not found. Please install qemu-system-x86"
        exit 1
    fi
    
    case $mode in
        "32bit")
            make boot-floppy
            ;;
        "amd64")
            make boot-floppy-amd64
            ;;
        "modern")
            make boot-floppy-modern
            ;;
        *)
            print_error "Unknown test mode: $mode"
            exit 1
            ;;
    esac
}

# Function to show usage
show_usage() {
    echo "ICS-OS Build and Test Script"
    echo "Usage: $0 [OPTIONS] [COMMAND]"
    echo ""
    echo "Commands:"
    echo "  build-docker    Build using Docker (recommended)"
    echo "  build-native    Build natively on host system"
    echo "  floppy         Create floppy disk image"
    echo "  test-32bit     Test in 32-bit mode"
    echo "  test-amd64     Test on AMD64 with 32-bit compatibility"
    echo "  test-modern    Test with modern QEMU features"
    echo "  full-docker    Complete build and test cycle with Docker"
    echo "  full-native    Complete build and test cycle natively"
    echo "  clean          Clean build artifacts"
    echo ""
    echo "Options:"
    echo "  -h, --help     Show this help message"
    echo "  -v, --verbose  Enable verbose output"
}

# Parse command line arguments
VERBOSE=false
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_usage
            exit 0
            ;;
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        build-docker)
            build_docker
            exit 0
            ;;
        build-native)
            build_native
            exit 0
            ;;
        floppy)
            create_floppy
            exit 0
            ;;
        test-32bit)
            test_os "32bit"
            exit 0
            ;;
        test-amd64)
            test_os "amd64"
            exit 0
            ;;
        test-modern)
            test_os "modern"
            exit 0
            ;;
        full-docker)
            build_docker
            create_floppy
            print_success "Build complete! You can now test with:"
            print_status "  ./build.sh test-32bit    # Test in 32-bit mode"
            print_status "  ./build.sh test-amd64    # Test on AMD64"
            exit 0
            ;;
        full-native)
            build_native
            create_floppy
            print_success "Build complete! You can now test with:"
            print_status "  ./build.sh test-32bit    # Test in 32-bit mode"
            print_status "  ./build.sh test-amd64    # Test on AMD64"
            exit 0
            ;;
        clean)
            make clean
            print_success "Build artifacts cleaned"
            exit 0
            ;;
        *)
            print_error "Unknown command: $1"
            show_usage
            exit 1
            ;;
    esac
done

# If no command provided, show usage
show_usage
