# 🎹 Organ Pi MIDI File Player - Installation Package

This package contains everything needed to install the Organ Pi MIDI File Player on your Raspberry Pi or compatible ARM64 Linux system.

## 📦 Package Contents

```
midiplay-installer/
├── install.sh                 # Main installation script
├── uninstall.sh              # Uninstallation script
├── README.md                 # This file
└── debian-package/           # Complete Debian package structure
    ├── DEBIAN/
    │   ├── control          # Package metadata and dependencies
    │   ├── postinst         # Post-installation script
    │   └── prerm            # Pre-removal script
    └── usr/
        └── local/
            └── bin/
                └── play     # The midiplay binary (v1.4.7)
```

## 🖥️ System Requirements

- **Hardware**: Raspberry Pi 4B or better (ARM64 architecture)
- **OS**: Debian 12 (Bookworm) 64-bit or later
  - Raspberry Pi OS based on Debian 12 works fine
- **RAM**: 4GB recommended (may work with less)
- **Storage**: NVME SSD recommended, fast SD card acceptable
- **MIDI Device**: USB-to-MIDI converter and compatible instrument
  - Allen Protégé-16 organ (primary target)
  - Casio USB MIDI keyboards (tested)
  - Yamaha consumer keyboards (tested)

## 🚀 Quick Installation

1. **Extract the package** (if downloaded as archive):
   ```bash
   tar -xzf midiplay-installer.tar.gz
   # or
   unzip midiplay-installer.zip
   ```

2. **Navigate to the installer directory**:
   ```bash
   cd midiplay-installer
   ```

3. **Run the installation script**:
   ```bash
   ./install.sh
   ```

The script will:
- ✅ Verify system compatibility
- ✅ Build the Debian package
- ✅ Install the package with proper dependencies
- ✅ Set up command aliases and permissions
- ✅ Verify the installation

## 📋 Dependencies

The installer automatically handles these system dependencies:
- `libasound2` (>= 1.0.16) - ALSA sound library
- `libc6` (>= 2.17) - GNU C Library
- `libgcc-s1` (>= 3.0) - GCC support library
- `libstdc++6` (>= 5.2) - GNU Standard C++ Library

## 🎵 Usage

After installation, you can use the player with:

```bash
# Full command
play <filename> [options]

# Short alias
p <filename> [options]
```

### Examples

```bash
# Play a MIDI file
play hymn001.mid

# Play without .mid extension (auto-added)
play hymn001

# Play 3 verses
play hymn001 -n3

# Play prelude at 80% tempo
play hymn001 -p8

# Play without introduction, 2 verses
play hymn001 -x2

# Override tempo to 120 BPM
play hymn001 --tempo=120
```

### Command Line Options

- `-v, --version` - Display version number
- `-p[n], --prelude=[n]` - Play as prelude/postlude without intro (default speed: 8/10)
- `-n[n]` - Number of verses to play after introduction
- `-x[n]` - Number of verses to play without introduction
- `-t[bpm], --tempo=[bpm]` - Override tempo in beats per minute
- `--help` - Show detailed help

## 🔧 Troubleshooting

### Installation Issues

**Permission denied**:
```bash
chmod +x install.sh
./install.sh
```

**Missing dependencies**:
```bash
sudo apt-get update
sudo apt-get install dpkg-dev
```

**Architecture mismatch**:
- This package is built for ARM64 (aarch64)
- Verify with: `dpkg --print-architecture`

### Runtime Issues

**No MIDI device found**:
- Connect your USB-to-MIDI adapter
- Wait a few seconds for device recognition
- Check with: `lsusb` or `aconnect -l`

**File not found**:
- Ensure MIDI files have `.mid` extension or omit extension
- Check file permissions and path

**Audio issues**:
- Verify ALSA is working: `aplay -l`
- Check MIDI connections: `aconnect -l`

## 🗑️ Uninstallation

To remove the Organ Pi MIDI File Player:

```bash
./uninstall.sh
```

Or manually:
```bash
sudo dpkg -r midiplay
```

## 📁 File Locations

After installation:
- **Binary**: `/usr/local/bin/play`
- **Alias**: `/usr/local/bin/p` → `/usr/local/bin/play`
- **Package info**: `/var/lib/dpkg/info/midiplay.*`

## 🔄 Updates

To update to a newer version:
1. Uninstall the current version
2. Install the new package

## 🐛 Support

For issues and support:
- Check the troubleshooting section above
- Verify system requirements
- Ensure MIDI hardware is properly connected
- Check system logs: `journalctl -u midiplay` (if applicable)

## 📄 License

This software is distributed under the terms specified in the original project license.

## 🎼 About

The Organ Pi MIDI File Player is designed specifically for church organists and musicians who need a reliable, simple MIDI file player for live performance. It supports custom MIDI markers for introductions, multiple verses, and various playback options optimized for liturgical use.

**Version**: 1.4.7  
**Architecture**: ARM64  
**Target Platform**: Raspberry Pi 4B+ with Debian 12

---

*🎵 Ready to make beautiful music! Connect your MIDI device and enjoy! 🎵*