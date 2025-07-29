#!/bin/bash
# ICS-OS USB and UEFI Boot Image Creator
# Creates bootable USB images and UEFI-compatible ISOs

set -e  # Exit on error

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$SCRIPT_DIR"
TMP_DIR="$PROJECT_ROOT/tmp-uefi"
MOUNT_DIR="$PROJECT_ROOT/mnt-uefi"

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

# Check dependencies
check_dependencies() {
    log "Checking dependencies..."
    
    local deps=("grub-mkrescue" "xorriso" "mtools" "mkfs.fat" "parted")
    for dep in "${deps[@]}"; do
        if ! command -v "$dep" &> /dev/null; then
            error "$dep is required but not installed. Please install GRUB and related tools."
        fi
    done
    
    if [ ! -f "vmdex" ]; then
        error "vmdex kernel not found. Please build the kernel first: make -C kernel && make -C kernel bzImage"
    fi
    
    success "All dependencies found"
}

# Clean up temporary directories
cleanup() {
    log "Cleaning up temporary files..."
    sudo umount "$MOUNT_DIR" 2>/dev/null || true
    sudo rm -rf "$TMP_DIR" "$MOUNT_DIR" 2>/dev/null || true
}

# Prepare build environment
prepare_environment() {
    log "Preparing build environment..."
    cleanup
    
    mkdir -p "$TMP_DIR"
    mkdir -p "$MOUNT_DIR"
    
    # Create directory structure for bootable image
    mkdir -p "$TMP_DIR/boot/grub"
    mkdir -p "$TMP_DIR/EFI/BOOT"
    mkdir -p "$TMP_DIR/apps"
    mkdir -p "$TMP_DIR/tcc1"
    mkdir -p "$TMP_DIR/lib1"
    
    success "Environment prepared"
}

# Copy ICS-OS files
copy_ics_files() {
    log "Copying ICS-OS files..."
    
    # Copy kernel
    cp vmdex "$TMP_DIR/"
    
    # Copy applications and libraries
    if [ -d "apps" ]; then
        cp apps/* "$TMP_DIR/apps/" 2>/dev/null || true
    fi
    
    if [ -d "sdk" ]; then
        cp sdk/* "$TMP_DIR/tcc1/" 2>/dev/null || true
    fi
    
    if [ -d "lib" ]; then
        cp lib/* "$TMP_DIR/lib1/" 2>/dev/null || true
    fi
    
    if [ -d "base" ]; then
        cp base/* "$TMP_DIR/" 2>/dev/null || true
    fi
    
    # Generate help files
    if [ -f "scripts/gen-help.sh" ]; then
        bash scripts/gen-help.sh
    fi
    
    success "ICS-OS files copied"
}

# Create GRUB configuration for hybrid boot
create_grub_config() {
    log "Creating GRUB configuration..."
    
    cat > "$TMP_DIR/boot/grub/grub.cfg" << 'EOF'
set timeout=10
set default=0

# Simple configuration for both BIOS and UEFI
if [ x$grub_platform = xefi ]; then
    menuentry 'ICS Operating System (UEFI)' {
        echo 'Loading ICS-OS kernel via UEFI...'
        multiboot /vmdex
        boot
    }
    
    menuentry 'ICS Operating System (UEFI Safe Mode)' {
        echo 'Loading ICS-OS kernel in safe mode...'
        multiboot /vmdex safe
        boot
    }
else
    menuentry 'ICS Operating System (BIOS)' {
        echo 'Loading ICS-OS kernel via BIOS...'
        multiboot /vmdex
        boot
    }
    
    menuentry 'ICS Operating System (BIOS Safe Mode)' {
        echo 'Loading ICS-OS kernel in safe mode...'
        multiboot /vmdex safe
        boot
    }
fi

menuentry 'System Information' {
    echo 'ICS Operating System - Educational OS'
    echo 'Platform: '$grub_platform
    echo 'CPU: '$grub_cpu
    echo 'Version: 2024 Modernized Edition'
    echo ''
    echo 'Press any key to continue...'
    read
}

menuentry 'Reboot' {
    reboot
}

menuentry 'Shutdown' {
    halt
}
EOF

    # Create simple EFI boot configuration
    cat > "$TMP_DIR/EFI/BOOT/grub.cfg" << 'EOF'
search --set=root --file /vmdex
configfile /boot/grub/grub.cfg
EOF

    success "GRUB configuration created"
}

# Build UEFI-compatible ISO
build_iso() {
    log "Building UEFI-compatible ISO image..."
    
    local iso_name="ics-os-uefi.iso"
    
    # Use grub-mkrescue to create hybrid BIOS/UEFI ISO with minimal modules
    grub-mkrescue \
        --output="$iso_name" \
        --compress=xz \
        "$TMP_DIR"
    
    if [ -f "$iso_name" ]; then
        local size=$(ls -lh "$iso_name" | awk '{print $5}')
        success "ISO created: $iso_name ($size)"
        
        # Verify the ISO
        log "Verifying ISO structure..."
        xorriso -indev "$iso_name" -find / 2>/dev/null | head -10
        
        return 0
    else
        error "Failed to create ISO image"
    fi
}

# Create USB bootable image
create_usb_image() {
    log "Creating USB bootable image..."
    
    local usb_image="ics-os-usb.img"
    local image_size="64M"  # 64MB should be enough for ICS-OS
    
    # Create empty image file
    dd if=/dev/zero of="$usb_image" bs=1M count=64 status=progress
    
    # Create partition table and format
    parted -s "$usb_image" mklabel msdos
    parted -s "$usb_image" mkpart primary fat32 1MiB 100%
    parted -s "$usb_image" set 1 boot on
    
    # Set up loop device (requires Docker privileged mode or host execution)
    if command -v losetup &> /dev/null && [ -w /dev ]; then
        log "Setting up loop device for USB image..."
        
        local loop_device=$(sudo losetup --find --show "$usb_image")
        local partition="${loop_device}p1"
        
        # Wait for partition to appear
        sleep 1
        
        # Format partition
        sudo mkfs.fat -F32 "$partition"
        
        # Mount and copy files
        sudo mount "$partition" "$MOUNT_DIR"
        sudo cp -r "$TMP_DIR"/* "$MOUNT_DIR/"
        
        # Install GRUB
        sudo grub-install --target=i386-pc --boot-directory="$MOUNT_DIR/boot" "$loop_device"
        
        # Cleanup
        sudo umount "$MOUNT_DIR"
        sudo losetup -d "$loop_device"
        
        success "USB image created: $usb_image"
    else
        warning "Cannot create USB image: requires loop device access"
        warning "Run this script on the host system (not in Docker) for USB image creation"
        return 1
    fi
}

# Main function
main() {
    echo "ICS-OS USB and UEFI Boot Image Creator"
    echo "======================================"
    
    local mode="$1"
    
    case "$mode" in
        "iso")
            check_dependencies
            prepare_environment
            copy_ics_files
            create_grub_config
            build_iso
            cleanup
            ;;
        "usb")
            check_dependencies
            prepare_environment
            copy_ics_files
            create_grub_config
            create_usb_image
            cleanup
            ;;
        "all"|"")
            check_dependencies
            prepare_environment
            copy_ics_files
            create_grub_config
            build_iso
            create_usb_image
            cleanup
            ;;
        "clean")
            cleanup
            success "Cleaned up temporary files"
            ;;
        *)
            echo "Usage: $0 [iso|usb|all|clean]"
            echo "  iso   - Create UEFI-compatible ISO only"
            echo "  usb   - Create USB bootable image only"
            echo "  all   - Create both ISO and USB images (default)"
            echo "  clean - Clean up temporary files"
            exit 1
            ;;
    esac
    
    echo
    success "ICS-OS boot image creation completed!"
    echo
    echo "Test the images with:"
    echo "  UEFI ISO: qemu-system-x86_64 -bios /usr/share/ovmf/OVMF.fd -cdrom ics-os-uefi.iso -m 256M"
    echo "  BIOS ISO: qemu-system-x86_64 -cdrom ics-os-uefi.iso -m 256M"
    echo "  USB Image: qemu-system-x86_64 -drive format=raw,file=ics-os-usb.img -m 256M"
}

# Handle script termination
trap cleanup EXIT

# Run main function
main "$@"
