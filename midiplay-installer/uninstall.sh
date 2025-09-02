#!/bin/bash
# Organ Pi MIDI File Player - Uninstallation Script

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

PACKAGE_NAME="midiplay"

echo -e "${BLUE}🎹 Organ Pi MIDI File Player - Uninstallation Script${NC}"
echo -e "${BLUE}===================================================${NC}"
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

# Check if package is installed
if ! dpkg -l | grep -q "^ii.*$PACKAGE_NAME"; then
    print_warning "Package '$PACKAGE_NAME' is not installed."
    exit 0
fi

echo "Found installed package: $PACKAGE_NAME"
echo ""

# Confirm uninstallation
read -p "Are you sure you want to uninstall Organ Pi MIDI File Player? (y/N): " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo "Uninstallation cancelled."
    exit 0
fi

# Remove the package
echo "Removing package..."
if [[ $EUID -eq 0 ]]; then
    # Running as root
    if dpkg -r "$PACKAGE_NAME"; then
        print_status "Package removed successfully"
    else
        print_error "Failed to remove package"
        exit 1
    fi
else
    # Not running as root, use sudo
    if sudo dpkg -r "$PACKAGE_NAME"; then
        print_status "Package removed successfully"
    else
        print_error "Failed to remove package"
        exit 1
    fi
fi

# Verify removal
if ! command -v play &> /dev/null; then
    print_status "Uninstallation verified - 'play' command is no longer available"
else
    print_warning "The 'play' command is still available. There may be another installation."
fi

echo ""
print_status "Uninstallation completed successfully!"
echo -e "${BLUE}Thank you for using Organ Pi MIDI File Player! 🎵${NC}"