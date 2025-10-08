#!/bin/bash
# lib/metadata.sh - Metadata updates (Single Responsibility)
#
# Purpose: Update version strings in various metadata files
# Reason to change: Metadata file formats change

# Function: update_control_version
# Updates the Version field in DEBIAN/control file
# Parameters:
#   $1 - Version string (required)
#   $2 - Control file path (optional, default: midiplay-installer/debian-package/DEBIAN/control)
# Returns: 0 on success, 1 on failure

update_control_version() {
    local version="$1"
    local control_file="${2:-midiplay-installer/debian-package/DEBIAN/control}"
    
    if [[ -z "$version" ]]; then
        echo "❌ Version is required for control file update"
        return 1
    fi
    
    if [[ ! -f "$control_file" ]]; then
        echo "❌ Control file not found: $control_file"
        return 1
    fi
    
    sed -i "s/^Version:.*/Version: $version/" "$control_file"
    echo "✅ Updated DEBIAN/control to version $version"
    
    return 0
}

# Function: update_install_script_version
# Updates the PACKAGE_VERSION variable in install.sh
# Parameters:
#   $1 - Version string (required)
#   $2 - Install script path (optional, default: midiplay-installer/install.sh)
# Returns: 0 on success, 1 on failure

update_install_script_version() {
    local version="$1"
    local install_script="${2:-midiplay-installer/install.sh}"
    
    if [[ -z "$version" ]]; then
        echo "❌ Version is required for install script update"
        return 1
    fi
    
    if [[ ! -f "$install_script" ]]; then
        echo "⚠️  Install script not found: $install_script"
        return 1
    fi
    
    sed -i "s/^PACKAGE_VERSION=.*/PACKAGE_VERSION=\"$version\"/" "$install_script"
    echo "✅ Updated install.sh to version $version"
    
    return 0
}

# Function: create_version_file
# Creates a .VERSION file containing the version number
# Parameters:
#   $1 - Version string (required)
#   $2 - Version file path (optional, default: midiplay-installer/.VERSION)
# Returns: 0 on success, 1 on failure

create_version_file() {
    local version="$1"
    local version_file="${2:-midiplay-installer/.VERSION}"
    
    if [[ -z "$version" ]]; then
        echo "❌ Version is required for .VERSION file creation"
        return 1
    fi
    
    # Create directory if it doesn't exist
    local version_dir=$(dirname "$version_file")
    if [[ ! -d "$version_dir" ]]; then
        mkdir -p "$version_dir"
    fi
    
    echo "$version" > "$version_file"
    echo "✅ Created .VERSION file: $version"
    
    return 0
}

# Function: update_all_metadata
# Updates all metadata files with the new version
# Parameters:
#   $1 - Version string (required)
# Returns: 0 on success, 1 on failure

update_all_metadata() {
    local version="$1"
    local errors=0
    
    if [[ -z "$version" ]]; then
        echo "❌ Version is required for metadata update"
        return 1
    fi
    
    echo "📝 Updating metadata files to version $version..."
    
    # Update control file
    if ! update_control_version "$version"; then
        ((errors++))
    fi
    
    # Update install script
    if ! update_install_script_version "$version"; then
        ((errors++))
    fi
    
    # Create .VERSION file
    if ! create_version_file "$version"; then
        ((errors++))
    fi
    
    if [[ $errors -gt 0 ]]; then
        echo "⚠️  $errors metadata update(s) failed"
        return 1
    fi
    
    echo "✅ All metadata updated successfully"
    return 0
}