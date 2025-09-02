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

# Package information
PACKAGE_NAME="midiplay"
PACKAGE_VERSION="1.4.7"
PACKAGE_ARCH="arm64"
DEB_FILE="${PACKAGE_NAME}_${PACKAGE_VERSION}_${PACKAGE_ARCH}.deb"

echo -e "${BLUE}🎹 Organ Pi MIDI File Player - Installation Script${NC}"
echo -e "${BLUE}=================================================${NC}"
echo ""

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
    echo "• Version: $(play --version 2>/dev/null || echo '1.4.7')"
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