#!/bin/bash
# lib/translations.sh - Translation compilation (Single Responsibility)
#
# Purpose: Compile .po translation files to .mo binaries and install them
# Reason to change: Translation tooling or process changes

# Function: compile_translations
# Compiles all .po files and copies .mo files to package structure
# Parameters:
#   $1 - Target locale directory (optional, default: midiplay-installer/debian-package/usr/share/locale)
# Returns: 0 on success, 1 on failure

compile_translations() {
    local package_locale_dir="${1:-midiplay-installer/debian-package/usr/share/locale}"
    
    # Check if compilation script exists
    if [[ ! -f "./po/compile-translations.sh" ]]; then
        echo "⚠️  Translation compilation script not found at ./po/compile-translations.sh"
        return 1
    fi
    
    echo "🌍 Compiling translations..."
    
    # Compile all .po files
    if ! ./po/compile-translations.sh; then
        echo "⚠️  Translation compilation failed"
        return 1
    fi
    
    # Copy compiled .mo files to package structure
    local count=0
    for lang_dir in po/*/LC_MESSAGES; do
        if [[ -d "$lang_dir" && -f "$lang_dir/midiplay.mo" ]]; then
            # Extract language code from path (e.g., po/es/LC_MESSAGES -> es)
            local lang=$(basename $(dirname "$lang_dir"))
            
            # Create destination directory
            mkdir -p "$package_locale_dir/$lang/LC_MESSAGES"
            
            # Copy .mo file
            cp "$lang_dir/midiplay.mo" "$package_locale_dir/$lang/LC_MESSAGES/"
            echo "  ✅ Installed translation: $lang"
            ((count++))
        fi
    done
    
    if [[ $count -gt 0 ]]; then
        echo "✅ Installed $count translation(s) to package"
    else
        echo "⚠️  No translation files found to install"
    fi
    
    return 0
}

# Function: check_translations_exist
# Checks if translation files exist in the package
# Parameters:
#   $1 - Package locale directory (optional)
# Returns: 0 if translations found, 1 if not

check_translations_exist() {
    local package_locale_dir="${1:-midiplay-installer/debian-package/usr/share/locale}"
    
    if [[ ! -d "$package_locale_dir" ]]; then
        return 1
    fi
    
    local count=$(find "$package_locale_dir" -name "*.mo" 2>/dev/null | wc -l)
    
    if [[ $count -gt 0 ]]; then
        return 0
    fi
    
    return 1
}