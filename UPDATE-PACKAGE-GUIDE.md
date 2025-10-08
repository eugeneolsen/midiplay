# 🔄 Updating the Installer Package

## When to Use Which Script

### `update-installer-package.sh` - **Use After Building New Binary**
**Use this when:** You've built a new version of the `play` binary and need to update the entire installer package.

**What it does:**
- Copies the new binary to the installer package
- Compiles translations
- Updates all version metadata
- Optionally creates distribution archives (with `-a` flag)

**Example:**
```bash
# Just built a new play binary
./update-installer-package.sh -a
```

### `create-installer-archive.sh` - **Use to Package for Distribution**
**Use this when:** The installer package is already up-to-date, and you just need to create distributable archives.

**What it does:**
- Validates package structure (including translations)
- Creates .tar.gz and .zip archives
- Does NOT update binary or translations

**Example:**
```bash
# Package is ready, just need archives
./create-installer-archive.sh
```

### Quick Decision Guide
- **New binary built?** → Use `update-installer-package.sh -a` (does everything)
- **Just need archives?** → Use `create-installer-archive.sh`
- **Made manual changes to installer files?** → Use `create-installer-archive.sh`

---

## Quick Reference

When you build a new version of the [`play`](play) binary, use this script to update the installer package:

```bash
# Basic update (auto-detects version from Git tags)
./update-installer-package.sh

# Update with specific version override
./update-installer-package.sh -v 1.6.0

# Update and create distribution archives
./update-installer-package.sh -a

# Use binary from different location
./update-installer-package.sh -b /path/to/new/play -a
```

## What the Script Does

1. ✅ **Detects version** automatically from Git tags (or manual override)
2. ✅ **Backs up** the existing binary (with timestamp)
3. ✅ **Copies** the new binary into the package structure
4. ✅ **Compiles translations** (.po → .mo files)
5. ✅ **Updates version** in DEBIAN/control, install.sh, and .VERSION file
6. ✅ **Sets permissions** correctly (755 for binary, 755 for scripts)
7. ✅ **Verifies** the update was successful
8. ✅ **Optionally creates** distribution archives

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
- `-v, --version VERSION` - Specify version number (auto-detected from Git tags if not provided)
- `-a, --archive` - Create distribution archives after updating
- `-h, --help` - Show help message

## Version Detection

The script now automatically detects the version using a **four-tier fallback strategy**:

1. **Manual Override:** Use `--version X.Y.Z` flag
2. **Git Tags:** Extracts semantic version from `git describe --tags`
3. **Binary Extraction:** Reads version from the binary using `strings`
4. **Directory Parsing:** Parses version from directory name pattern

This ensures version consistency across your build system and installer.

## Files Updated

The script automatically updates:
- `midiplay-installer/debian-package/usr/local/bin/play` (the binary)
- `midiplay-installer/debian-package/usr/share/locale/*/LC_MESSAGES/midiplay.mo` (translations)
- `midiplay-installer/debian-package/DEBIAN/control` (version field)
- `midiplay-installer/install.sh` (PACKAGE_VERSION variable)
- `midiplay-installer/.VERSION` (version file for distributed archives)

## Library Architecture

The installer system now uses modular, SRP-compliant libraries:

```
lib/
├── version.sh        # Version detection and extraction
├── translations.sh   # Translation compilation
├── packaging.sh      # Archive creation
├── metadata.sh       # Metadata updates
└── validation.sh     # Package validation
```

Each library has a single responsibility and can be tested independently. See [`lib/README.md`](lib/README.md) for details.

## Error Fixes Applied

The installer package has been fixed to resolve:
- ✅ Missing final newline in DEBIAN/control
- ✅ Incorrect permissions on DEBIAN scripts (now 755)
- ✅ Proper executable permissions on binary (755)

## Creating Distribution Archives

After updating the package, create distribution archives:

```bash
./create-installer-archive.sh
```

This script:
- Validates package structure (including translations)
- Updates the .VERSION file
- Creates both tar.gz and zip archives
- Uses version from Git tags automatically

## Ready to Use!

The installer package is now fully functional and ready for distribution. The `./install.sh` script should work without errors and will automatically detect the version from the `.VERSION` file included in the archive.