/**
 * Adizilla OS - ISO Builder
 * Creates a bootable ISO image using pure JavaScript
 * Built by Lila, Nairobi 🇰🇪
 */

const fs = require('fs');
const path = require('path');

console.log('==========================================');
console.log('Adizilla OS ISO Builder');
console.log('==========================================');

// Configuration
const ISO_NAME = 'Adizilla-OS';
const ISO_VERSION = '1.0.0';
const OUTPUT_DIR = path.join(__dirname, '..', 'dist');
const ISO_FILE = path.join(OUTPUT_DIR, `${ISO_NAME}-${ISO_VERSION}.iso`);

// ISO constants
const SECTOR_SIZE = 2048;
const ISO_VOLUME_NAME = 'ADIZILLA_OS';

// Helper functions
function logInfo(msg) {
    console.log(`[INFO] ${msg}`);
}

function logWarn(msg) {
    console.log(`[WARN] ${msg}`);
}

function logError(msg) {
    console.log(`[ERROR] ${msg}`);
}

// Create output directory
function setupDirs() {
    logInfo('Creating directories...');
    if (!fs.existsSync(OUTPUT_DIR)) {
        fs.mkdirSync(OUTPUT_DIR, { recursive: true });
    }
}

// Create bootloader (simple boot sector)
function createBootSector() {
    logInfo('Creating boot sector...');
    
    // Simple 16-bit boot sector
    const bootSector = Buffer.alloc(512);
    
    // Assembly: simple boot code that prints "Adizilla OS"
    const bootCode = [
        0xB8, 0x00, 0x00, // mov ax, 0
        0x8E, 0xD8,        // mov ds, ax
        0x8E, 0xC0,        // mov es, ax
        0xBC, 0x00, 0x7C,  // mov sp, 0x7C00
        0xB4, 0x03,        // mov ah, 0x03 (get cursor position)
        0xCD, 0x10,        // int 0x10
        0xB8, 0x03, 0x00, // mov ax, 0x0003 (80x25 text mode)
        0xCD, 0x10,        // int 0x10
        
        // Print "Adizilla OS" using BIOS interrupt
        0xB4, 0x0E,        // mov ah, 0x0E
        0xB0, 'A',         // mov al, 'A'
        0xCD, 0x10,        // int 0x10
        0xB0, 'd',         // mov al, 'd'
        0xCD, 0x10,        // int 0x10
        0xB0, 'i',         // mov al, 'i'
        0xCD, 0x10,        // int 0x10
        0xB0, 'z',         // mov al, 'z'
        0xCD, 0x10,        // int 0x10
        0xB0, 'i',         // mov al, 'i'
        0xCD, 0x10,        // int 0x10
        0xB0, 'l',         // mov al, 'l'
        0xCD, 0x10,        // int 0x10
        0xB0, 'l',         // mov al, 'l'
        0xCD, 0x10,        // int 0x10
        0xB0, 'a',         // mov al, 'a'
        0xCD, 0x10,        // int 0x10
        
        0xB0, ' ',         // mov al, ' '
        0xCD, 0x10,        // int 0x10
        
        0xB0, 'O',         // mov al, 'O'
        0xCD, 0x10,        // int 0x10
        0xB0, 'S',         // mov al, 'S'
        0xCD, 0x10,        // int 0x10
        
        // Newline
        0xB0, 0x0D,        // mov al, 0x0D (carriage return)
        0xCD, 0x10,        // int 0x10
        0xB0, 0x0A,        // mov al, 0x0A (line feed)
        0xCD, 0x10,        // int 0x10
        
        // Infinite loop
        0xEB, 0xFE         // jmp $
    ];
    
    bootCode.forEach((byte, i) => {
        if (i < 510) bootSector[i] = byte;
    });
    
    // Boot signature
    bootSector[510] = 0x55;
    bootSector[511] = 0xAA;
    
    return bootSector;
}

// Create ISO 9660 volume descriptor
function createVolumeDescriptor() {
    logInfo('Creating volume descriptor...');
    
    const volumeDescriptor = Buffer.alloc(SECTOR_SIZE);
    
    // Type code (1 = Primary Volume Descriptor)
    volumeDescriptor[0] = 0x01;
    
    // Standard identifier "CD001"
    volumeDescriptor.write('CD001', 1);
    
    // Version
    volumeDescriptor[6] = 0x01;
    
    // Volume name
    volumeDescriptor.write(ISO_VOLUME_NAME, 40);
    
    // Volume space size (little-endian)
    volumeDescriptor.writeUInt32LE(400, 80);  // 400 sectors
    
    // Volume set size
    volumeDescriptor.writeUInt16LE(1, 120);
    
    // Volume sequence number
    volumeDescriptor.writeUInt16LE(1, 124);
    
    // Logical block size
    volumeDescriptor.writeUInt16LE(SECTOR_SIZE, 128);
    
    // Path table size
    volumeDescriptor.writeUInt32LE(10, 132);
    
    // Type L path table
    volumeDescriptor[140] = 0x01;  // One entry
    
    // Volume set identifier
    volumeDescriptor.write(ISO_NAME, 190);
    
    // Publisher identifier
    volumeDescriptor.write('Lila, Nairobi, Kenya', 318);
    
    // Data preparation identifier
    volumeDescriptor.write('Adizilla OS Builder', 446);
    
    // Copyright file identifier
    volumeDescriptor.write(';1', 574);
    
    // Abstract file identifier
    volumeDescriptor.write(';1', 702);
    
    // Bibliographic file identifier
    volumeDescriptor.write(';1', 830);
    
    // Volume creation date
    const now = new Date();
    const dateStr = now.toISOString().slice(0, 16).replace(/[-:T]/g, '').padEnd(17, '0');
    volumeDescriptor.write(dateStr, 958);
    
    // Volume modification date
    volumeDescriptor.write(dateStr, 975);
    
    // Volume expiration date
    volumeDescriptor.write(dateStr, 992);
    
    // Volume effective date
    volumeDescriptor.write(dateStr, 1009);
    
    return volumeDescriptor;
}

// Create directory record
function createDirectoryRecord(name, size, location) {
    const record = Buffer.alloc(33 + name.length);
    
    // Length of directory record
    record[0] = 33 + name.length;
    
    // Extended attribute record length
    record[1] = 0;
    
    // Location of extent (little-endian)
    record.writeUInt32LE(location, 2);
    
    // Data extent length (little-endian)
    record.writeUInt32LE(size, 6);
    
    // Recording date and time (7 bytes)
    const now = new Date();
    record[11] = now.getFullYear() - 1900;
    record[12] = now.getMonth() + 1;
    record[13] = now.getDate();
    record[14] = now.getHours();
    record[15] = now.getMinutes();
    record[16] = now.getSeconds();
    
    // File flags
    record[18] = 0;
    
    // File unit size
    record[20] = 0;
    
    // Interleave gap size
    record[21] = 0;
    
    // Volume sequence number
    record.writeUInt16LE(1, 22);
    
    // Length of file identifier
    record[25] = name.length;
    
    // File identifier
    record.write(name, 26);
    
    return record;
}

// Build ISO file
async function buildISO() {
    logInfo('Building ISO image...');
    
    setupDirs();
    
    const bootSector = createBootSector();
    const volumeDescriptor = createVolumeDescriptor();
    
    // Calculate ISO size
    const totalSectors = 400;
    const isoBuffer = Buffer.alloc(totalSectors * SECTOR_SIZE);
    
    // Write boot sector at sector 17 (El Torito)
    bootSector.copy(isoBuffer, 17 * SECTOR_SIZE);
    
    // Write volume descriptor at sector 16
    volumeDescriptor.copy(isoBuffer, 16 * SECTOR_SIZE);
    
    // Write volume descriptor terminator at sector 17
    isoBuffer[18 * SECTOR_SIZE] = 0xFF;
    'CD001'.split('').forEach((c, i) => {
        isoBuffer[18 * SECTOR_SIZE + 1 + i] = c.charCodeAt(0);
    });
    isoBuffer[18 * SECTOR_SIZE + 6] = 0x01;
    
    // Add a simple kernel file
    const kernelContent = `Adizilla OS v${ISO_VERSION}\nBuilt with love in Nairobi, Kenya 🇰🇪\n\nThis is a simple bootable ISO.`;
    const kernelBuffer = Buffer.from(kernelContent);
    
    // Copy kernel to ISO (at sector 20)
    kernelBuffer.copy(isoBuffer, 20 * SECTOR_SIZE);
    
    // Write ISO file
    fs.writeFileSync(ISO_FILE, isoBuffer);
    
    logInfo(`ISO created: ${ISO_FILE}`);
    logInfo(`ISO size: ${(isoBuffer.length / 1024).toFixed(2)} KB`);
    
    console.log('');
    console.log('==========================================');
    console.log('[SUCCESS] ISO Build Complete!');
    console.log('==========================================');
    console.log(`Output: ${ISO_FILE}`);
    console.log('');
    console.log('To burn to USB (Linux):');
    console.log('  sudo dd if=' + ISO_FILE + ' of=/dev/sdX bs=4M status=progress');
    console.log('');
    console.log('To test with QEMU (Linux/Mac/Windows):');
    console.log('  qemu-system-x86_64 -cdrom ' + ISO_FILE);
    console.log('');
    console.log('Note: This is a basic bootable ISO. For a full');
    console.log('      OS, you would need to compile the C kernel');
    console.log('      and include proper boot loaders.');
    console.log('');
}

// Run builder
buildISO().catch(err => {
    logError(`Build failed: ${err.message}`);
    process.exit(1);
});
