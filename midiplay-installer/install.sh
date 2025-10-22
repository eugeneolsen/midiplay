#!/bin/bash
# Organ Pi MIDI File Player - Installation Script
# Creates and installs a Debian package for the midiplay binary

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Try to source version library if available (development environment)
# Otherwise use inline fallback (distributed environment)
if [[ -f "../lib/version.sh" ]]; then
    source "../lib/version.sh"
else
    # Inline version detection for distributed archives
    get_version() {
        local provided_version="$1"
        
        # Tier 0: Manual override
        if [[ -n "$provided_version" ]]; then
            echo "$provided_version"
            return 0
        fi
        
        # Tier 1: .VERSION file (most common for distributed archives)
        if [[ -f ".VERSION" ]]; then
            local version=$(cat ".VERSION" | head -1)
            if [[ -n "$version" ]]; then
                echo "$version"
                return 0
            fi
        fi
        
        # Tier 2: Extract from binary
        if [[ -f "debian-package/usr/local/bin/play" ]]; then
            local version=$(strings "debian-package/usr/local/bin/play" 2>/dev/null | \
                           grep -E '^[0-9]+\.[0-9]+\.[0-9]+$' | head -1)
            if [[ -n "$version" ]]; then
                echo "$version"
                return 0
            fi
        fi
        
        # Tier 3: Parse from parent directory
        local parent=$(basename "$(dirname "$(pwd)")")
        if [[ "$parent" =~ midiplay-installer-v([0-9]+\.[0-9]+\.[0-9]+) ]]; then
            echo "${BASH_REMATCH[1]}"
            return 0
        fi
        
        # All fallbacks failed
        return 1
    }
fi

# Function to print colored output
print_status() {
    echo -e "${GREEN}✅ $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}⚠️  $1${NC}"
}

print_error() {
    echo -e "${RED}❌ $1${NC}"
}

print_info() {
    echo -e "${BLUE}ℹ️  $1${NC}"
}

# Function to show usage
show_usage() {
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  -v, --version VERSION Specify version number (auto-detected if not provided)"
    echo "  -h, --help           Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0                   # Auto-detect version from .VERSION file or binary"
    echo "  $0 -v 1.6.0         # Use specific version"
}

# Parse command line arguments
PROVIDED_VERSION=""

while [[ $# -gt 0 ]]; do
    case $1 in
        -v|--version)
            PROVIDED_VERSION="$2"
            shift 2
            ;;
        -h|--help)
            show_usage
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            show_usage
            exit 1
            ;;
    esac
done

echo -e "${BLUE}🎹 Organ Pi MIDI File Player - Installation Script${NC}"
echo -e "${BLUE}=================================================${NC}"
echo ""

# Detect version
PACKAGE_VERSION="1.5.9"
if [[ $? -ne 0 ]]; then
    print_error "Unable to detect version automatically"
    echo ""
    echo "Attempted:"
    echo "  ✗ .VERSION file (file not found or empty)"
    echo "  ✗ Binary extraction (binary not found or no version string)"
    echo "  ✗ Directory name parsing (directory name doesn't match pattern)"
    echo ""
    echo "Solutions:"
    echo "  1. Provide version manually: ./install.sh --version X.Y.Z"
    echo "  2. Ensure you're in the midiplay-installer directory"
    echo "  3. Check that the package structure is complete"
    exit 1
fi

print_info "Installing version: $PACKAGE_VERSION"

# Package information
PACKAGE_NAME="midiplay"
PACKAGE_ARCH="arm64"
DEB_FILE="${PACKAGE_NAME}_${PACKAGE_VERSION}_${PACKAGE_ARCH}.deb"

# Check if running as root for installation
if [[ $EUID -eq 0 ]]; then
    print_warning "Running as root. This is required for system installation."
else
    print_warning "Not running as root. You may be prompted for sudo password."
fi

# Check if we're on the right architecture
CURRENT_ARCH=$(dpkg --print-architecture)
if [[ "$CURRENT_ARCH" != "arm64" ]]; then
    print_warning "This package is built for arm64, but you're running $CURRENT_ARCH"
    echo "The installation may not work correctly."
    read -p "Continue anyway? (y/N): " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        echo "Installation cancelled."
        exit 1
    fi
fi

# Check if dpkg-deb is available
if ! command -v dpkg-deb &> /dev/null; then
    print_error "dpkg-deb is not installed. Please install dpkg-dev:"
    echo "sudo apt-get update && sudo apt-get install dpkg-dev"
    exit 1
fi

# Check if yaml-cpp is available
if ! dpkg -l | grep -q libyaml-cpp; then
    print_warning "libyaml-cpp0.7 not found. This will be installed as a dependency."
fi

# Verify package structure
echo "Verifying package structure..."
if [[ ! -d "debian-package/DEBIAN" ]]; then
    print_error "Package structure is incomplete. Missing DEBIAN directory."
    exit 1
fi

if [[ ! -f "debian-package/usr/local/bin/play" ]]; then
    print_error "Binary not found in package structure."
    exit 1
fi

print_status "Package structure verified"

# Check for translation files
echo "Checking for translation files..."
TRANSLATIONS_FOUND=()
for lang_dir in debian-package/usr/share/locale/*/LC_MESSAGES/midiplay.mo; do
    if [[ -f "$lang_dir" ]]; then
        # Extract language code from path
        lang=$(basename $(dirname $(dirname "$lang_dir")))
        TRANSLATIONS_FOUND+=("$lang")
    fi
done

if [[ ${#TRANSLATIONS_FOUND[@]} -gt 0 ]]; then
    print_status "Found ${#TRANSLATIONS_FOUND[@]} translation(s): ${TRANSLATIONS_FOUND[*]}"
else
    print_warning "No translation files found in package"
fi

# Set proper permissions on DEBIAN scripts
chmod 755 debian-package/DEBIAN/postinst
chmod 755 debian-package/DEBIAN/prerm
print_status "Set permissions on package scripts"

# Build the Debian package
echo "Building Debian package..."
if dpkg-deb --build debian-package "$DEB_FILE"; then
    print_status "Package built successfully: $DEB_FILE"
else
    print_error "Failed to build package"
    exit 1
fi

# Install the package
echo ""
echo "Installing package..."
if [[ $EUID -eq 0 ]]; then
    # Running as root
    if dpkg -i "$DEB_FILE"; then
        print_status "Package installed successfully"
    else
        print_warning "Package installation had issues. Attempting to fix dependencies..."
        apt-get update && apt-get -f install -y
        print_status "Dependencies resolved"
    fi
else
    # Not running as root, use sudo
    if sudo dpkg -i "$DEB_FILE"; then
        print_status "Package installed successfully"
    else
        print_warning "Package installation had issues. Attempting to fix dependencies..."
        sudo apt-get update && sudo apt-get -f install -y
        print_status "Dependencies resolved"
    fi
fi

# Verify installation
echo ""
echo "Verifying installation..."
if command -v play &> /dev/null; then
    print_status "Installation verified - 'play' command is available"
    echo ""
    echo -e "${BLUE}Installation Summary:${NC}"
    echo "• Binary installed to: /usr/local/bin/play"
    echo "• Symlink created: /usr/local/bin/p -> /usr/local/bin/play"
    echo "• System config: /etc/midiplay/midi_devices.yaml"
    echo "• User config: ~/.config/midiplay/midi_devices.yaml (optional)"
    echo "• Version: $(play --version 2>/dev/null || echo "$PACKAGE_VERSION")"
    if [[ ${#TRANSLATIONS_FOUND[@]} -gt 0 ]]; then
        echo "• Languages: ${#TRANSLATIONS_FOUND[@]} translation(s) installed (${TRANSLATIONS_FOUND[*]})"
        echo "  → Automatic language detection enabled"
    fi
    echo ""
    echo -e "${GREEN}Usage:${NC}"
    echo "  play <filename> [options]"
    echo "  p <filename> [options]"
    echo ""
    echo -e "${GREEN}Examples:${NC}"
    echo "  play 19.mid"
    echo "  play 19 -n2    # Play 2 verses"
    echo "  p 19 -p8       # Prelude at 80% tempo"
    echo ""
    echo "For more options: play --help"
else
    print_error "Installation verification failed - 'play' command not found"
    echo "You may need to restart your terminal or run: source ~/.bashrc"
    exit 1
fi

# Clean up
if [[ -f "$DEB_FILE" ]]; then
    echo ""
    read -p "Remove the generated .deb file? (Y/n): " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Nn]$ ]]; then
        echo "Keeping $DEB_FILE for future use."
    else
        rm "$DEB_FILE"
        print_status "Cleaned up temporary files"
    fi
fi

echo ""
print_status "Installation completed successfully!"
echo -e "${BLUE}🎵 Ready to play MIDI files! Connect your MIDI device and enjoy! 🎵${NC}"