# Adizilla Mobile OS 🇰🇪

<p align="center">
  <img src="https://img.shields.io/badge/Version-1.0.0-blue.svg" alt="Version">
  <img src="https://img.shields.io/badge/License-MIT-green.svg" alt="License">
  <img src="https://img.shields.io/badge/Built-in-Nairobi-red.svg" alt="Built in Nairobi">
  <img src="https://img.shields.io/badge/Platforms-Web%20%7C%20ISO-purple.svg" alt="Platforms">
</p>

<p align="center">
  <strong>A revolutionary web-based mobile operating system built with ❤️ in Nairobi, Kenya</strong>
</p>

---

## 📱 What is Adizilla OS?

Adizilla is a **full-featured mobile operating system** that runs entirely in your phone, web, USB drive and your computer. It provides a complete desktop experience with window management, terminal, file system, and more. Additionally, it can be compiled into a **bootable ISO** to run on real hardware or virtual machines.

---

## ✨ Features

### 🖥️ Desktop Environment
- **Window Management** - Drag, resize, minimize, maximize, close windows
- **Taskbar** - Application launcher, system tray, clock
- **Desktop Icons** - Quick access to apps
- **Multiple Themes** - Dark, Light, Matrix-style
- **Boot Animation** - Animated ASCII art boot sequence

### 📦 Built-in Applications

| App | Description |
|-----|-------------|
| **Terminal** | Full-featured command-line with history, autocomplete, aliases |
| **File Manager** | Virtual file system with create, read, write, delete operations |
| **Settings** | System configuration, theme switching, user management |
| **Browser** | Web browser within the OS |
| **Notepad** | Text editor |
| **Calculator** | Basic and scientific calculator |
| **Weather** | Real-time weather via OpenWeatherMap API |
| **Crypto Prices** | Live cryptocurrency prices via CoinGecko API |
| **News** | Latest headlines via News API |
| **AI Assistant** | Claude-powered AI terminal assistant |

### 🔧 Core OS Features

#### Process Management
- Process scheduler (run multiple tasks simultaneously)
- Process kill/suspend/resume commands
- CPU & memory usage per process
- Background jobs (& operator like real Linux)

#### File System
- Virtual file system (create, read, write, delete files)
- Directory tree navigation (cd, ls, mkdir, rm)
- File permissions system (read/write/execute)
- Trash/recycle bin

#### User System
- Multi-user support with login/logout
- User roles (root, admin, guest)
- Password hashing and authentication
- Session management

#### Networking Layer
- Ping command
- Network status monitor
- Port scanner
- SSH-like terminal connections between nodes

### 🔐 Security Features

- Firewall module — allow/block connections
- Intrusion detection — alert when unusual activity detected
- Encryption layer — encrypt files within the OS
- Audit log — track every command with timestamps
- Sandbox mode — run untrusted processes in isolation
- 2FA login — TOTP-based authentication
- Vulnerability scanner

### 🌐 APIs Integrated

| API | Feature |
|-----|---------|
| OpenWeatherMap | Weather command |
| NewsAPI | news command - headlines |
| GitHub API | git status, push/pull |
| CoinGecko | crypto prices |
| IP-API | myip - location & network |
| Shodan API | Network intelligence |
| HaveIBeenPwned | Check breached emails |
| VirusTotal | Scan files for malware |
| Anthropic API (Claude) | AI assistant |
| NASA API | Astronomy Picture of the Day |
| Dictionary API | define <word> command |
| Exchange Rate API | Currency conversion |

### 🏞️ Kenya-Specific Features (Unique!)

- 🇰🇪 **M-Pesa API** - Check balance, send money (Kenya-specific)
- 🗣️ **Swahili Support** - First OS with Swahili commands!
- 📱 **Safaricom API** - Data balance checker

### 📊 Dashboard & Monitoring

- Real-time CPU, RAM, disk usage graphs
- Network traffic monitor
- Node health dashboard
- System uptime tracker
- Battery/power monitor

### 📅 Productivity Tools

- **todo** - Task manager in terminal
- **note** - Quick notes saved to virtual filesystem
- **timer** and **alarm** commands
- **calc** - Calculator
- **calendar** - Calendar command

---

## 🚀 Getting Started

### Web Version (Run in Browser)

```
bash
# Install dependencies
npm install

# Start the server
npm start

# Open in browser
# http://localhost:3000
```

### Bootable ISO

```
bash
# Build the ISO
npm run build:iso

# Test with QEMU
qemu-system-x86_64 -cdrom dist/Adizilla-OS-1.0.0.iso

# Or burn to USB (Linux)
sudo dd if=dist/Adizilla-OS-1.0.0.iso of=/dev/sdX bs=4M status=progress
```

---

## 🏗️ Project Structure

```
adizilla-mob-os/
├── asm/                    # Assembly files (boot, syscalls, interrupts)
│   ├── boot.asm           # Boot sector code
│   ├── syscall.asm        # System call handlers
│   └── interrupt.asm      # Interrupt handlers
├── backend/               # Express.js backend server
│   ├── server.js          # Main server file
│   ├── middleware/       # Auth, logging middleware
│   ├── routes/           # API routes
│   └── utils/            # Helper functions
├── frontend/              # Web frontend
│   ├── index.html        # Main HTML
│   ├── css/              # Stylesheets
│   └── js/               # JavaScript files
├── kernel/               # C kernel code
│   ├── adz_core.c       # Core kernel
│   ├── adz_mem.c        # Memory management
│   ├── adz_proc.c       # Process management
│   ├── adz_net.c        # Network stack
│   └── include/          # Headers
├── iso/                  # ISO build files
│   └── boot/             # Boot configuration
├── scripts/              # Build scripts
│   ├── build_iso.js     # ISO builder
│   └── build_iso.sh     # Shell ISO builder
└── dist/                 # Output directory
    └── Adizilla-OS-1.0.0.iso
```

---

## 📋 System Requirements

### For Web Version
- Modern web browser (Chrome, Firefox, Edge, Safari)
- Node.js v14+

### For ISO/Hardware
- x86_64 compatible processor
- 512MB+ RAM
- QEMU, VirtualBox, or real hardware

---

## 🔧 Available Commands

### Terminal Commands
```
bash
help              # Show all commands
ls                # List files
cd                # Change directory
mkdir             # Create directory
rm                # Remove file/directory
cat               # Read file
touch             # Create file
echo              # Write to file
pwd               # Print working directory
whoami            # Current user
ps                # Process list
kill              # Kill process
ping              # Ping host
clear             # Clear terminal
history           # Command history
alias             # Create command aliases
piping            # command1 | command2
```

### Productivity Commands
```
bash
weather <city>    # Get weather
crypto <coin>     # Get crypto prices
news              # Get latest news
define <word>     # Dictionary lookup
calc              # Calculator
calendar          # Show calendar
todo              # Task manager
note              # Quick notes
convert           # Currency conversion
```

---

## 🐛 Known Issues

- Some APIs require API keys (add to .env)
- ISO is basic - full kernel compilation requires cross-compiler
- Some security features are demo-only

---

## 🤝 Contributing

Contributions welcome! Please submit a pull request or open an issue.

---

## 📄 License

MIT License - See LICENSE file for details.

---

## 👏 Credits

- **Creator**: Charis, Nairobi, Kenya 🇰🇪
- **Built with**: Node.js, Express, Vanilla JS, C, Assembly
- **Inspired by**: Chrome OS, Windows, Linux

---

## 🔗 Links

- [Website](#) (coming soon)
- [Documentation](#) (coming soon)
- [Discord](#) (coming soon)

---

<p align="center">
  <strong>Adizilla OS - The Future of Operating Systems 🇰🇪</strong>
</p>
