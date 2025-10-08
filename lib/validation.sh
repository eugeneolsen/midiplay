#!/bin/bash
# lib/validation.sh - Package validation (Single Responsibility)
#
# Purpose: Verify package structure completeness
# Reason to change: Required files or structure changes

# Function: validate_package_structure
# Validates that all required files exist in the package
# Parameters:
#   $1 - Package directory (optional, default: midiplay-installer)
# Returns: 0 on success, 1 on failure

validate_package_structure() {
    local package_dir="${1:-midiplay-installer}"
    local errors=0
    
    # Required files
    local required_files=(
        "$package_dir/install.sh"
        "$package_dir/uninstall.sh"
        "$package_dir/README.md"
        "$package_dir/debian-package/DEBIAN/control"
        "$package_dir/debian-package/DEBIAN/postinst"
        "$package_dir/debian-package/DEBIAN/prerm"
        "$package_dir/debian-package/usr/local/bin/play"
        "$package_dir/debian-package/etc/midiplay/midi_devices.yaml"
    )
    
    echo "🔍 Validating package structure..."
    
    # Check required files
    for file in "${required_files[@]}"; do
        if [[ ! -f "$file" ]]; then
            echo "  ❌ Missing: $file"
            ((errors++))
        fi
    done
    
    # Check for translations (warning only, not an error)
    if [[ ! -d "$package_dir/debian-package/usr/share/locale" ]]; then
        echo "  ⚠️  No translations found (optional but recommended)"
        echo "     Run: ./po/compile-translations.sh to compile translations"
    else
        local translation_count=$(find "$package_dir/debian-package/usr/share/locale" -name "*.mo" 2>/dev/null | wc -l)
        if [[ $translation_count -gt 0 ]]; then
            echo "  ✅ Found $translation_count translation file(s)"
        else
            echo "  ⚠️  Translation directory exists but no .mo files found"
        fi
    fi
    
    # Check binary permissions
    if [[ -f "$package_dir/debian-package/usr/local/bin/play" ]]; then
        local perms=$(stat -c "%a" "$package_dir/debian-package/usr/local/bin/play" 2>/dev/null || stat -f "%OLp" "$package_dir/debian-package/usr/local/bin/play" 2>/dev/null)
        if [[ "$perms" != "755" ]]; then
            echo "  ⚠️  Binary permissions are $perms (expected 755)"
        fi
    fi
    
    # Report results
    if [[ $errors -gt 0 ]]; then
        echo "❌ Package validation failed with $errors error(s)"
        return 1
    fi
    
    echo "✅ Package structure is valid"
    return 0
}

# Function: check_required_tools
# Checks if required tools are available
# Returns: 0 if all tools available, 1 if any missing

check_required_tools() {
    local missing_tools=()
    
    # Tools needed for packaging
    local required_tools=(
        "dpkg-deb"
        "tar"
        "zip"
    )
    
    echo "🔧 Checking required tools..."
    
    for tool in "${required_tools[@]}"; do
        if ! command -v "$tool" &> /dev/null; then
            echo "  ❌ Missing: $tool"
            missing_tools+=("$tool")
        fi
    done
    
    if [[ ${#missing_tools[@]} -gt 0 ]]; then
        echo "❌ Missing ${#missing_tools[@]} required tool(s)"
        echo "   Install with: sudo apt-get install ${missing_tools[*]}"
        return 1
    fi
    
    echo "✅ All required tools are available"
    return 0
}

# Function: verify_binary
# Verifies the binary exists and is executable
# Parameters:
#   $1 - Binary path (optional, default: midiplay-installer/debian-package/usr/local/bin/play)
# Returns: 0 on success, 1 on failure

verify_binary() {
    local binary_path="${1:-midiplay-installer/debian-package/usr/local/bin/play}"
    
    if [[ ! -f "$binary_path" ]]; then
        echo "❌ Binary not found: $binary_path"
        return 1
    fi
    
    if [[ ! -x "$binary_path" ]]; then
        echo "⚠️  Binary is not executable: $binary_path"
        echo "   Fixing permissions..."
        chmod 755 "$binary_path"
    fi
    
    local size=$(du -h "$binary_path" | cut -f1)
    echo "✅ Binary verified: $binary_path ($size)"
    
    return 0
}