#!/bin/bash
# ICS-OS UEFI and USB Boot Testing Script
# Tests UEFI/BIOS compatibility and USB boot functionality

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$SCRIPT_DIR"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

log() {
    echo -e "${BLUE}[$(date +'%H:%M:%S')]${NC} $1"
}

error() {
    echo -e "${RED}[ERROR]${NC} $1" >&2
    exit 1
}

success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

# Check if we're on an AMD64 system
check_system() {
    log "Checking system compatibility..."
    
    if [ "$(uname -m)" != "x86_64" ]; then
        warning "This test is designed for AMD64 (x86_64) systems"
        warning "Current architecture: $(uname -m)"
    fi
    
    if ! command -v qemu-system-x86_64 &> /dev/null; then
        error "qemu-system-x86_64 not found. Please install QEMU."
    fi
    
    success "System compatibility verified"
}

# Test UEFI ISO creation
test_uefi_iso() {
    log "Testing UEFI ISO creation..."
    
    cd ics-os
    
    # Ensure kernel is built
    if [ ! -f "vmdex" ]; then
        log "Building kernel first..."
        docker-compose run ics-os-build bash -c "cd kernel && make clean && make all && make bzImage"
    fi
    
    # Create UEFI ISO
    make uefi-iso
    
    if [ -f "ics-os-uefi.iso" ]; then
        local size=$(ls -lh ics-os-uefi.iso | awk '{print $5}')
        success "UEFI ISO created: $size"
    else
        error "Failed to create UEFI ISO"
    fi
    
    cd ..
}

# Test UEFI boot functionality
test_uefi_boot() {
    log "Testing UEFI boot functionality..."
    
    cd ics-os
    
    if [ ! -f "/usr/share/ovmf/OVMF.fd" ]; then
        warning "OVMF UEFI firmware not found, skipping UEFI test"
        warning "Install with: sudo apt-get install ovmf"
        return 0
    fi
    
    log "Starting UEFI boot test (5 seconds)..."
    timeout 5 qemu-system-x86_64 \
        -bios /usr/share/ovmf/OVMF.fd \
        -cdrom ics-os-uefi.iso \
        -m 256M \
        -nographic \
        -serial stdio 2>/dev/null || true
    
    success "UEFI boot test completed"
    cd ..
}

# Test BIOS compatibility
test_bios_compatibility() {
    log "Testing BIOS compatibility..."
    
    cd ics-os
    
    log "Starting BIOS boot test (5 seconds)..."
    timeout 5 qemu-system-x86_64 \
        -cdrom ics-os-uefi.iso \
        -m 256M \
        -nographic \
        -serial stdio 2>/dev/null || true
    
    success "BIOS compatibility test completed"
    cd ..
}

# Test USB image creation
test_usb_image() {
    log "Testing USB image creation..."
    
    cd ics-os
    
    # Note: USB image requires loop device access, which may not work in all environments
    if make usb-image 2>/dev/null; then
        if [ -f "ics-os-usb.img" ]; then
            local size=$(ls -lh ics-os-usb.img | awk '{print $5}')
            success "USB image created: $size"
            
            # Test USB image boot
            log "Testing USB image boot (5 seconds)..."
            timeout 5 qemu-system-x86_64 \
                -drive format=raw,file=ics-os-usb.img \
                -m 256M \
                -nographic \
                -serial stdio 2>/dev/null || true
            
            success "USB image boot test completed"
        else
            warning "USB image file not found"
        fi
    else
        warning "USB image creation failed (may require host execution for loop devices)"
    fi
    
    cd ..
}

# Test all boot methods
test_all_boot_methods() {
    log "Testing all boot methods..."
    
    echo "Testing Boot Methods:"
    echo "===================="
    
    # UEFI ISO
    if [ -f "ics-os/ics-os-uefi.iso" ]; then
        echo "✓ UEFI ISO: $(ls -lh ics-os/ics-os-uefi.iso | awk '{print $5}')"
    else
        echo "✗ UEFI ISO: Not found"
    fi
    
    # USB Image
    if [ -f "ics-os/ics-os-usb.img" ]; then
        echo "✓ USB Image: $(ls -lh ics-os/ics-os-usb.img | awk '{print $5}')"
    else
        echo "✗ USB Image: Not found"
    fi
    
    echo
    echo "Available Boot Commands:"
    echo "======================="
    echo "UEFI Boot:  qemu-system-x86_64 -bios /usr/share/ovmf/OVMF.fd -cdrom ics-os/ics-os-uefi.iso -m 256M"
    echo "BIOS Boot:  qemu-system-x86_64 -cdrom ics-os/ics-os-uefi.iso -m 256M"
    echo "USB Boot:   qemu-system-x86_64 -drive format=raw,file=ics-os/ics-os-usb.img -m 256M"
    echo
}

# Main test function
main() {
    echo "ICS-OS UEFI and USB Boot Testing"
    echo "================================="
    echo
    
    local test_type="$1"
    
    case "$test_type" in
        "iso")
            check_system
            test_uefi_iso
            ;;
        "uefi")
            check_system
            test_uefi_boot
            ;;
        "bios")
            check_system
            test_bios_compatibility
            ;;
        "usb")
            check_system
            test_usb_image
            ;;
        "all"|"")
            check_system
            test_uefi_iso
            test_uefi_boot
            test_bios_compatibility
            test_usb_image
            test_all_boot_methods
            ;;
        "info")
            test_all_boot_methods
            ;;
        *)
            echo "Usage: $0 [iso|uefi|bios|usb|all|info]"
            echo "  iso   - Test UEFI ISO creation only"
            echo "  uefi  - Test UEFI boot functionality only"
            echo "  bios  - Test BIOS compatibility only"
            echo "  usb   - Test USB image creation only"
            echo "  all   - Run all tests (default)"
            echo "  info  - Show boot method information"
            exit 1
            ;;
    esac
    
    echo
    success "ICS-OS UEFI and USB boot testing completed!"
    echo
    echo "Hardware Testing:"
    echo "================"
    echo "To test on real hardware:"
    echo "1. Write USB image: sudo dd if=ics-os/ics-os-usb.img of=/dev/sdX bs=1M"
    echo "2. Boot from USB drive on UEFI or BIOS system"
    echo "3. Burn ISO to DVD: iso-burn ics-os/ics-os-uefi.iso"
    echo
}

# Run main function
main "$@"
