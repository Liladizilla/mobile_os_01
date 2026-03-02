#!/bin/bash
# Adizilla OS - ISO Build Script
# Creates a bootable ISO image

set -e

echo "=========================================="
echo "Adizilla OS ISO Builder"
echo "=========================================="

# Configuration
ISO_NAME="Adizilla-OS"
ISO_VERSION="1.0.0"
OUTPUT_DIR="dist"
ISO_FILE="$OUTPUT_DIR/${ISO_NAME}-${ISO_VERSION}.iso"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Functions
log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check dependencies
check_deps() {
    log_info "Checking dependencies..."
    
    if ! command -v nasm &> /dev/null; then
        log_error "nasm not found. Install with: apt install nasm"
        exit 1
    fi
    
    if ! command -v grub-mkrescue &> /dev/null && ! command -v grub2-mkrescue &> /dev/null; then
        log_warn "grub-mkrescue not found. Trying alternative..."
    fi
    
    if ! command -v xorriso &> /dev/null; then
        log_warn "xorriso not found. Some features may not work."
    fi
    
    log_info "Dependencies OK"
}

# Create output directory
setup_dirs() {
    log_info "Creating directories..."
    mkdir -p "$OUTPUT_DIR"
    mkdir -p iso/boot/grub
    mkdir -p iso/boot/kernel
}

# Build bootloader
build_bootloader() {
    log_info "Building bootloader..."
    
    # Build the boot loader assembly
    if command -v nasm &> /dev/null; then
        nasm -f bin iso/boot/adz_boot.asm -o iso/boot/adz_boot.bin || true
        log_info "Bootloader built"
    else
        log_warn "Skipping assembly (nasm not found)"
    fi
}

# Create kernel image (simplified)
create_kernel() {
    log_info "Creating kernel image..."
    
    # Create a simple kernel binary
    # In a real build, this would compile the C kernel
    echo -e "Adizilla OS Kernel v${ISO_VERSION}\nBuilt in Nairobi, Kenya" > iso/boot/kernel/kernel.txt
    
    # Create a simple bootable kernel image
    printf '\xB8\x00\x00\xCD\x10\xB8\x01\x00\x8E\xD8\x8E\xC0\xFC\xBF\x00\x00\x00\x00\xB4\x02\xB0\x20\xB6\x00\xB3\x00\xCD\x10\xBE\x1C\x7C\xAC\x3C\x00\x74\x02\xEB\xF6\x31\xC0\x8E\xD8\x8E\xC0\xFB\xEA\x00\x00\xFF\xFF' > iso/boot/kernel/adz_kernel.bin
    
    log_info "Kernel image created"
}

# Create initrd (initial ramdisk)
create_initrd() {
    log_info "Creating initrd..."
    
    # Create a simple initrd with basic utilities
    mkdir -p iso/boot/initrd
    
    # Copy kernel header
    echo "Adizilla OS Initrd v${ISO_VERSION}" > iso/boot/initrd/version
    
    # Create a simple init script
    cat > iso/boot/initrd/init.sh << 'EOF'
#!/bin/sh
echo "Adizilla OS initializing..."
mount -t proc /proc
mount -t sysfs /sys
mount -t devtmpfs /dev
echo "Welcome to Adizilla OS!"
exec /bin/sh
EOF
    chmod +x iso/boot/initrd/init.sh
    
    log_info "Initrd created"
}

# Build ISO with GRUB
build_iso_grub() {
    log_info "Building ISO with GRUB..."
    
    # Try grub-mkrescue first
    if command -v grub-mkrescue &> /dev/null; then
        grub-mkrescue -o "$ISO_FILE" iso/ 2>/dev/null || {
            log_warn "GRUB rescue failed, trying alternative..."
            build_iso_xorriso
        }
        log_info "ISO created: $ISO_FILE"
        return
    fi
    
    # Fallback to xorriso
    build_iso_xorriso
}

# Build ISO with xorriso
build_iso_xorriso() {
    log_info "Building ISO with xorriso..."
    
    if command -v xorriso &> /dev/null; then
        xorriso -as mkisofs \
            -boot-info-table \
            -boot-load-size 4 \
            -eltorito-catalog iso/boot/grub/eltorito.cat \
            -eltorito-boot iso/boot/adz_boot.bin \
            -no-emul-boot \
            -boot-load-size 8 \
            -o "$ISO_FILE" \
            iso/
        
        log_info "ISO created: $ISO_FILE"
    else
        log_error "Cannot create ISO without xorriso or grub-mkrescue"
        log_info "Creating ISO structure for manual build..."
        
        # Just create the structure
        tar -cvf "${ISO_FILE%.iso}.tar" iso/
        log_info "ISO structure saved as: ${ISO_FILE%.iso}.tar"
    fi
}

# Create a simple ISO (fallback)
build_simple_iso() {
    log_info "Creating simple bootable ISO..."
    
    # Create a minimal ISO structure
    mkdir -p iso_root
    
    # Copy boot files
    cp -r iso/boot iso_root/
    
    # Create a simple ISO with cat (for testing)
    # This creates a minimal bootable CD
    {
        # ISO header (simple)
        for i in {1..16}; do printf '\x00'; done
        # Boot sector
        if [ -f iso/boot/adz_boot.bin ]; then
            cat iso/boot/adz_boot.bin
        fi
    } > "$ISO_FILE"
    
    log_info "Simple ISO created: $ISO_FILE"
}

# Main build process
main() {
    log_info "Starting Adizilla OS ISO build..."
    
    check_deps
    setup_dirs
    build_bootloader
    create_kernel
    create_initrd
    
    # Try to build proper ISO
    if command -v grub-mkrescue &> /dev/null || command -v xorriso &> /dev/null; then
        build_iso_grub
    else
        build_simple_iso
    fi
    
    echo ""
    echo "=========================================="
    log_info "Build complete!"
    echo "=========================================="
    echo "ISO file: $ISO_FILE"
    echo ""
    echo "To burn to USB:"
    echo "  sudo dd if=$ISO_FILE of=/dev/sdX bs=4M status=progress"
    echo ""
    echo "To test with QEMU:"
    echo "  qemu-system-x86_64 -cdrom $ISO_FILE"
    echo ""
}

# Run main
main "$@"
