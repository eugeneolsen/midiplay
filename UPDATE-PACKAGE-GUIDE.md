# 🔄 Updating the Installer Package

## Quick Reference

When you build a new version of the [`play`](play) binary, use this script to update the installer package:

```bash
# Basic update (auto-detects version)
./update-installer-package.sh

# Update with specific version
./update-installer-package.sh -v 1.5.0

# Update and create distribution archives
./update-installer-package.sh -v 1.5.0 -a

# Use binary from different location
./update-installer-package.sh -b /path/to/new/play -v 1.5.0 -a
```

## What the Script Does

1. ✅ **Backs up** the existing binary (with timestamp)
2. ✅ **Copies** the new binary into the package structure
3. ✅ **Updates version** in DEBIAN/control and install.sh
4. ✅ **Sets permissions** correctly (755 for binary, 755 for scripts)
5. ✅ **Verifies** the update was successful
6. ✅ **Optionally creates** distribution archives

## Complete Workflow

```bash
# 1. Build your new binary (however you normally do it)
make  # or your build command

# 2. Update the installer package
./update-installer-package.sh -v 1.5.0 -a

# 3. Test the installer (optional but recommended)
cd midiplay-installer && ./install.sh

# 4. Distribute the new archives
# Upload midiplay-installer-v1.5.0.tar.gz and/or .zip
```

## Script Options

- `-b, --binary PATH` - Path to the play binary (default: ./play)
- `-v, --version VERSION` - Specify version number (auto-detected if not provided)
- `-a, --archive` - Create distribution archives after updating
- `-h, --help` - Show help message

## Files Updated

The script automatically updates:
- `midiplay-installer/debian-package/usr/local/bin/play` (the binary)
- `midiplay-installer/debian-package/DEBIAN/control` (version field)
- `midiplay-installer/install.sh` (PACKAGE_VERSION variable)

## Error Fixes Applied

The installer package has been fixed to resolve:
- ✅ Missing final newline in DEBIAN/control
- ✅ Incorrect permissions on DEBIAN scripts (now 755)
- ✅ Proper executable permissions on binary (755)

## Ready to Use!

The installer package is now fully functional and ready for distribution. The `./install.sh` script should work without errors.