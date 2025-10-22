#!/bin/bash
# lib/packaging.sh - Archive creation (Single Responsibility)
#
# Purpose: Create distributable tar.gz and zip archives
# Reason to change: Archive format requirements change

# Function: create_archives
# Creates both tar.gz and zip archives of the installer package
# Parameters:
#   $1 - Version string (required)
#   $2 - Archive directory (optional, default: midiplay-installer)
# Returns: 0 on success, 1 on failure

create_archives() {
    local version="$1"
    local base_name="midiplay-installer"
    local archive_dir="${2:-midiplay-installer}"
    
    if [[ -z "$version" ]]; then
        echo "❌ Error: Version is required for archive creation"
        return 1
    fi
    
    if [[ ! -d "$archive_dir" ]]; then
        echo "❌ Error: Archive directory not found: $archive_dir"
        return 1
    fi
    
    echo "📦 Creating distribution archives..."
    
    # Create tar.gz archive
    local tar_file="${base_name}-v${version}.tar.gz"
    echo "  Creating $tar_file..."
    
    if tar -czf "$tar_file" "$archive_dir/"; then
        local tar_size=$(du -h "$tar_file" | cut -f1)
        echo "  ✅ Created: $tar_file ($tar_size)"
    else
        echo "  ❌ Failed to create tar.gz archive"
        return 1
    fi
    
    # Create zip archive
    local zip_file="${base_name}-v${version}.zip"
    echo "  Creating $zip_file..."
    
    if zip -r "$zip_file" "$archive_dir/" > /dev/null 2>&1; then
        local zip_size=$(du -h "$zip_file" | cut -f1)
        echo "  ✅ Created: $zip_file ($zip_size)"
    else
        echo "  ❌ Failed to create zip archive"
        return 1
    fi
    
    echo ""
    echo "📊 Archive Information:"
    echo "  • Tar.gz: $tar_file ($tar_size)"
    echo "  • Zip:    $zip_file ($zip_size)"
    echo "  • Version: $version"
    
    return 0
}

# Function: list_archive_contents
# Lists contents of the most recent archive
# Parameters:
#   $1 - Archive file path
# Returns: 0 on success, 1 on failure

list_archive_contents() {
    local archive_file="$1"
    
    if [[ ! -f "$archive_file" ]]; then
        echo "❌ Archive file not found: $archive_file"
        return 1
    fi
    
    echo "📋 Archive Contents:"
    if [[ "$archive_file" == *.tar.gz ]]; then
        tar -tzf "$archive_file" | head -20
        local total=$(tar -tzf "$archive_file" | wc -l)
        if [[ $total -gt 20 ]]; then
            echo "... and $(($total - 20)) more files"
        fi
    elif [[ "$archive_file" == *.zip ]]; then
        unzip -l "$archive_file" | head -25
    fi
    
    return 0
}