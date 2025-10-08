# Installer Library Modules

This directory contains reusable bash library modules for the midiplay installer scripts. Each module follows the **Single Responsibility Principle (SRP)** and contains functions related to a single aspect of the build/packaging process.

## Design Principles

- **DRY (Don't Repeat Yourself):** Each function exists in exactly one place
- **SRP (Single Responsibility Principle):** Each module has one reason to change
- **Composability:** Scripts source and compose these libraries as needed

## Library Modules

### version.sh - Version Detection

**Responsibility:** Detect and extract semantic version numbers from various sources

**Reason to Change:** Version detection strategy changes

**Key Functions:**
- `get_version([provided_version])` - Multi-tier version detection with fallbacks
- `extract_semantic_version(git_output)` - Extract X.Y.Z from git describe output
- `validate_version(version)` - Validate semantic version format

**Fallback Tiers:**
1. Manual override (--version flag)
2. Git tags (`git describe --tags`)
3. .VERSION file
4. Binary extraction (strings command)
5. Directory name parsing

### translations.sh - Translation Compilation

**Responsibility:** Compile .po translation files to .mo binaries

**Reason to Change:** Translation tooling or process changes

**Key Functions:**
- `compile_translations([package_locale_dir])` - Compile and install translations
- `check_translations_exist([package_locale_dir])` - Verify translations present

### packaging.sh - Archive Creation

**Responsibility:** Create distributable tar.gz and zip archives

**Reason to Change:** Archive format requirements change

**Key Functions:**
- `create_archives(version, [archive_dir])` - Create tar.gz and zip archives
- `list_archive_contents(archive_file)` - Display archive contents

### metadata.sh - Metadata Updates

**Responsibility:** Update version strings in various metadata files

**Reason to Change:** Metadata file formats change

**Key Functions:**
- `update_control_version(version, [control_file])` - Update DEBIAN/control
- `update_install_script_version(version, [install_script])` - Update install.sh
- `create_version_file(version, [version_file])` - Create .VERSION file
- `update_all_metadata(version)` - Update all metadata files at once

### validation.sh - Package Validation

**Responsibility:** Verify package structure completeness

**Reason to Change:** Required files or structure changes

**Key Functions:**
- `validate_package_structure([package_dir])` - Validate all required files exist
- `check_required_tools()` - Verify required system tools available
- `verify_binary([binary_path])` - Verify binary exists and is executable

## Usage Examples

### Basic Version Detection

```bash
#!/bin/bash
source lib/version.sh

VERSION=$(get_version "$USER_PROVIDED_VERSION")
if [[ $? -ne 0 ]]; then
    echo "Failed to detect version"
    exit 1
fi

echo "Using version: $VERSION"
```

### Complete Package Update Workflow

```bash
#!/bin/bash
source lib/version.sh
source lib/translations.sh
source lib/metadata.sh
source lib/packaging.sh

# Detect version
VERSION=$(get_version)

# Compile translations
compile_translations

# Update metadata
update_all_metadata "$VERSION"

# Create archives
create_archives "$VERSION"
```

### Package Validation

```bash
#!/bin/bash
source lib/validation.sh

# Check tools
if ! check_required_tools; then
    exit 1
fi

# Validate structure
if ! validate_package_structure; then
    exit 1
fi
```

## Testing

Each library can be tested independently:

```bash
# Test version detection
source lib/version.sh
get_version
echo "Detected: $?"

# Test validation
source lib/validation.sh
validate_package_structure midiplay-installer
```

## Dependencies

### System Tools Required
- `git` (for version detection from tags)
- `dpkg-deb` (for package building)
- `tar` (for archive creation)
- `zip` (for zip archive creation)
- `strings` (for binary version extraction)

### Optional Tools
- `msgfmt` (for translation compilation)

## Error Handling

All functions follow these conventions:
- Return `0` on success
- Return `1` on failure
- Print error messages to stdout with emoji indicators:
  - ✅ Success
  - ❌ Error
  - ⚠️  Warning
  - 🔍 Information

## Integration with Scripts

### update-installer-package.sh
Sources: version.sh, translations.sh, metadata.sh, packaging.sh

### create-installer-archive.sh
Sources: version.sh, validation.sh, packaging.sh, metadata.sh

### install.sh
Uses inline version detection (for distribution without lib/ directory)

## Maintenance

When modifying these libraries:
1. Ensure the single responsibility is maintained
2. Update this README if interfaces change
3. Test with all dependent scripts
4. Verify backward compatibility

## Version History

- v1.0.0 - Initial library structure with SRP/DRY compliance