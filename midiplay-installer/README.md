# 🎹 Organ Pi<sup style="font-size: smaller">TM</sup> 🎜 MIDI File Player - Installation Package

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
    ├── etc/
    │   └── midiplay/
    │       └── midi_devices.yaml    # System MIDI device configuration
    └── usr/
        └── local/
            └── bin/
                └── play     # The midiplay binary (v1.5.7)
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
   tar -xzf midiplay-installer-v1.6.0.tar.gz
   ```

2. **Navigate to the installer directory**:
   ```bash
   cd midiplay-installer
   ```

3. **Run the installation script**:
   ```bash
   sudo ./install.sh
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
- `libyaml-cpp0.7` - YAML configuration parsing library *(NEW in v1.5.0)*

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

## ⚙️ YAML Configuration (NEW in v1.5.0)

The player now uses YAML configuration files for device management, providing greater flexibility and easier customization.

### Configuration File Locations

The player searches for configuration files in this priority order:
1. `./midi_devices.yaml` (current directory - highest priority)
2. `~/.config/midiplay/midi_devices.yaml` (user-specific)
3. `/etc/midiplay/midi_devices.yaml` (system-wide - installed by default)

### Default Configuration

The installer creates a system-wide configuration at `/etc/midiplay/midi_devices.yaml` with support for:
- **Allen Protégé-16** organ (primary target)
- **Casio CTX-3000 series** keyboards
- **Yamaha PSR-EW425** keyboards

### Custom Configuration

To create a user-specific configuration:

```bash
# Create user config directory
mkdir -p ~/.config/midiplay

# Copy system config as starting point
cp /etc/midiplay/midi_devices.yaml ~/.config/midiplay/

# Edit with your preferred editor
nano ~/.config/midiplay/midi_devices.yaml
```

### YAML Configuration Format

```yaml
devices:
  - name: "Allen Protégé-16"
    detection_strings: ["Protege", "PROTEGE"]
    channels:
      - number: 1
        bank_msb: 32
        bank_lsb: 0
        program: 19  # Pipe Organ
      - number: 2
        bank_msb: 32
        bank_lsb: 0
        program: 19  # Pipe Organ
```

For detailed configuration options, see the comments in the installed YAML file.

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

**YAML library missing**:
```bash
sudo apt-get update
sudo apt-get install libyaml-cpp0.7
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

**YAML configuration issues**:
- Check if config file exists: `ls /etc/midiplay/midi_devices.yaml`
- Validate YAML syntax: `python3 -c "import yaml; yaml.safe_load(open('/etc/midiplay/midi_devices.yaml'))"`
- Check configuration priority: `play --version` (shows config file being used)
- Reset to defaults: `sudo cp /etc/midiplay/midi_devices.yaml ~/.config/midiplay/`

**Device not recognized**:
- Check if your device is in the YAML configuration
- Add custom device detection strings to user config
- See YAML configuration section above for customization

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
- **System Configuration**: `/etc/midiplay/midi_devices.yaml` *(NEW in v1.5.0)*
- **User Configuration**: `~/.config/midiplay/midi_devices.yaml` (optional override)
- **Package info**: `/var/lib/dpkg/info/midiplay.*`

## 🔄 Updates

### Upgrading from v1.4.7 to v1.5.0

**⚠️ Breaking Changes in v1.5.0:**
- **New dependency**: `libyaml-cpp0.6` library required
- **Configuration**: YAML device configuration now mandatory
- **Behavior**: Device detection now uses YAML configuration instead of hardcoded logic

**Migration Steps:**
1. **Automatic**: The installer will install the new YAML dependency and configuration
2. **Manual**: If you had custom device configurations, that were not in `~/.config/midiplay/midi_devices.yaml` you may need to recreate them in YAML format
3. **Verification**: Test your devices after upgrade to ensure proper recognition

**What's New in v1.5.0:**
- ✅ **Configurable device management** via YAML files
- ✅ **User-specific configuration** support (`~/.config/midiplay/`)
- ✅ **System-wide defaults** with user override capability
- ✅ **Easier device addition** without code changes
- ✅ **Better device detection** with custom detection strings

### Upgrading to v1.5.7

**✅ No Breaking Changes**

**What's New in v1.5.7:**
- ✅ **The code has been internationalized.**
- ✅ **Support for five different languages:**
1. English
2. Spanish
3. Brazilian Portuguese
4. French (France)
5. French (Canada)
- ✅ **Many internal changes to enhance reliability and extensibility.**

### Upgrading to v1.5.9

**✅ No Breaking Changes**

**What's New in v1.5.9:**
- Minor bug fixes
- Code cleanup
- Add unit tests
- Automate user acceptance testing

### Upgrading to v1.6.0

**✅ No Breaking Changes**

**What's New in v1.6.0:**
- D.C. al Fine uses the between-verse pause before starting *al fine*
- Improved readability and clarity for command line options


### General Updates

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

**Version**: 1.6.0  
**Architecture**: ARM64  
**Target Platform**: Raspberry Pi 4B+ with Debian 12

---

*🎵 Ready to make beautiful music! Connect your MIDI device and enjoy! 🎵*