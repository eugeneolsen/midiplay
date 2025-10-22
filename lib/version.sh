#!/bin/bash
# lib/version.sh - Version detection and extraction (Single Responsibility)
#
# Purpose: Detect and extract semantic version numbers from various sources
# Reason to change: Version detection strategy changes

# Function: get_version
# Returns: Semantic version string (X.Y.Z)
# Exit code: 0 on success, 1 on failure
# Usage: VERSION=$(get_version "$PROVIDED_VERSION")

get_version() {
    local provided_version="$1"  # Optional: --version override
    local version=""
    
    # Tier 0: Manual override (highest priority)
    if [[ -n "$provided_version" ]]; then
        echo "$provided_version"
        return 0
    fi
    
    # Tier 1: Git tags (development environment)
    if command -v git &> /dev/null && [[ -d .git ]]; then
        local git_version=$(git describe --tags 2>/dev/null)
        if [[ -n "$git_version" ]]; then
            version=$(extract_semantic_version "$git_version")
            if [[ -n "$version" ]]; then
                echo "$version"
                return 0
            fi
        fi
    fi
    
    # Tier 2: .VERSION file (distributed archives)
    if [[ -f ".VERSION" ]]; then
        version=$(cat ".VERSION" | head -1)
        if [[ -n "$version" ]]; then
            echo "$version"
            return 0
        fi
    fi
    
    # Tier 3: Extract from binary (if available)
    if [[ -f "midiplay-installer/debian-package/usr/local/bin/play" ]]; then
        version=$(strings "midiplay-installer/debian-package/usr/local/bin/play" 2>/dev/null | \
                  grep -E '^[0-9]+\.[0-9]+\.[0-9]+$' | head -1)
        if [[ -n "$version" ]]; then
            echo "$version"
            return 0
        fi
    fi
    
    # Tier 4: Parse from parent directory name
    local parent_dir=$(basename "$(pwd)")
    if [[ "$parent_dir" =~ midiplay-installer-v([0-9]+\.[0-9]+\.[0-9]+) ]]; then
        echo "${BASH_REMATCH[1]}"
        return 0
    fi
    
    # All fallbacks failed
    return 1
}

# Helper function to extract semantic version from git describe output
# Matches pattern from options.hpp: (?:^|-|n|v|V)([0-9]+\.[0-9]+\.[0-9]+)
extract_semantic_version() {
    local git_output="$1"
    
    if [[ "$git_output" =~ (^|-|n|v|V)([0-9]+\.[0-9]+\.[0-9]+) ]]; then
        echo "${BASH_REMATCH[2]}"
        return 0
    fi
    
    return 1
}

# Validate semantic version format
validate_version() {
    local version="$1"
    
    if [[ "$version" =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
        return 0
    fi
    
    return 1
}