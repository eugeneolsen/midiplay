#!/bin/bash
# Script to update the midiplay installer package with a new binary version

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Default values
PLAY_BINARY="./play"
INSTALLER_DIR="midiplay-installer"
PACKAGE_DIR="$INSTALLER_DIR/debian-package"
CONTROL_FILE="$PACKAGE_DIR/DEBIAN/control"
BINARY_DEST="$PACKAGE_DIR/usr/local/bin/play"

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

# Function to extract version from binary or use provided version
get_version() {
    local binary_path="$1"
    local provided_version="$2"
    
    if [[ -n "$provided_version" ]]; then
        echo "$provided_version"
        return
    fi
    
    # Try to extract version from binary
    if [[ -f "$binary_path" ]]; then
        # Look for version string in the binary (this might need adjustment based on how version is stored)
        local version=$(strings "$binary_path" 2>/dev/null | grep -E '^[0-9]+\.[0-9]+\.[0-9]+$' | head -1)
        if [[ -n "$version" ]]; then
            echo "$version"
            return
        fi
    fi
    
    # Fallback to current version in control file if it exists
    if [[ -f "$CONTROL_FILE" ]]; then
        local current_version=$(grep "^Version:" "$CONTROL_FILE" | cut -d' ' -f2)
        if [[ -n "$current_version" ]]; then
            echo "$current_version"
            return
        fi
    fi
    
    # Final fallback
    echo "1.4.7"
}

# Function to update version in control file
update_control_version() {
    local new_version="$1"
    local control_file="$2"
    
    if [[ -f "$control_file" ]]; then
        sed -i "s/^Version:.*/Version: $new_version/" "$control_file"
        print_status "Updated version in control file to $new_version"
    else
        print_error "Control file not found: $control_file"
        return 1
    fi
}

# Function to update version in install script
update_install_script_version() {
    local new_version="$1"
    local install_script="$INSTALLER_DIR/install.sh"
    
    if [[ -f "$install_script" ]]; then
        sed -i "s/^PACKAGE_VERSION=.*/PACKAGE_VERSION=\"$new_version\"/" "$install_script"
        print_status "Updated version in install script to $new_version"
    else
        print_warning "Install script not found: $install_script"
    fi
}

# Function to show usage
show_usage() {
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  -b, --binary PATH     Path to the play binary (default: ./play)"
    echo "  -v, --version VERSION Specify version number (auto-detected if not provided)"
    echo "  -a, --archive         Create distribution archives after updating"
    echo "  -h, --help           Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0                           # Update with ./play binary, auto-detect version"
    echo "  $0 -v 1.5.0                 # Update with version 1.5.0"
    echo "  $0 -b /path/to/play -v 1.5.0 # Use specific binary and version"
    echo "  $0 -a                        # Update and create archives"
}

# Parse command line arguments
BINARY_PATH=""
VERSION=""
CREATE_ARCHIVES=false

while [[ $# -gt 0 ]]; do
    case $1 in
        -b|--binary)
            BINARY_PATH="$2"
            shift 2
            ;;
        -v|--version)
            VERSION="$2"
            shift 2
            ;;
        -a|--archive)
            CREATE_ARCHIVES=true
            shift
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

# Use default binary path if not specified
if [[ -z "$BINARY_PATH" ]]; then
    BINARY_PATH="$PLAY_BINARY"
fi

echo -e "${BLUE}🔄 Updating Organ Pi MIDI File Player Installer Package${NC}"
echo -e "${BLUE}====================================================${NC}"
echo ""

# Check if binary exists
if [[ ! -f "$BINARY_PATH" ]]; then
    print_error "Binary not found: $BINARY_PATH"
    echo "Please ensure the play binary exists or specify the correct path with -b"
    exit 1
fi

# Check if installer directory exists
if [[ ! -d "$INSTALLER_DIR" ]]; then
    print_error "Installer directory not found: $INSTALLER_DIR"
    echo "Please run this script from the directory containing the installer package."
    exit 1
fi

# Get version
DETECTED_VERSION=$(get_version "$BINARY_PATH" "$VERSION")
print_info "Using version: $DETECTED_VERSION"

# Check if binary is executable
if [[ ! -x "$BINARY_PATH" ]]; then
    print_warning "Binary is not executable, making it executable..."
    chmod +x "$BINARY_PATH"
fi

# Create backup of current binary if it exists
if [[ -f "$BINARY_DEST" ]]; then
    BACKUP_FILE="${BINARY_DEST}.backup.$(date +%Y%m%d_%H%M%S)"
    cp "$BINARY_DEST" "$BACKUP_FILE"
    print_info "Backed up existing binary to: $BACKUP_FILE"
fi

# Copy new binary
echo "Copying new binary..."
cp "$BINARY_PATH" "$BINARY_DEST"
chmod 755 "$BINARY_DEST"
print_status "Updated binary in package structure"

# Update version in control file
if [[ -f "$CONTROL_FILE" ]]; then
    update_control_version "$DETECTED_VERSION" "$CONTROL_FILE"
else
    print_error "Control file not found. Package structure may be incomplete."
    exit 1
fi

# Update version in install script
update_install_script_version "$DETECTED_VERSION"

# Verify the update
echo ""
echo "Verifying package update..."
if [[ -f "$BINARY_DEST" ]] && [[ -x "$BINARY_DEST" ]]; then
    BINARY_SIZE=$(du -h "$BINARY_DEST" | cut -f1)
    print_status "Binary updated successfully (size: $BINARY_SIZE)"
else
    print_error "Binary update verification failed"
    exit 1
fi

# Show binary info
echo ""
print_info "Binary Information:"
echo "  Source: $BINARY_PATH"
echo "  Destination: $BINARY_DEST"
echo "  Version: $DETECTED_VERSION"
echo "  Size: $BINARY_SIZE"
echo "  Permissions: $(ls -l "$BINARY_DEST" | cut -d' ' -f1)"

# Create archives if requested
if [[ "$CREATE_ARCHIVES" == true ]]; then
    echo ""
    print_info "Creating distribution archives..."
    if [[ -f "./create-installer-archive.sh" ]]; then
        ./create-installer-archive.sh
    else
        print_warning "Archive creation script not found. Skipping archive creation."
        print_info "You can manually run: ./create-installer-archive.sh"
    fi
fi

echo ""
print_status "Package update completed successfully!"
echo ""
print_info "Next steps:"
echo "  1. Test the updated installer: cd $INSTALLER_DIR && ./install.sh"
echo "  2. Create distribution archives: ./create-installer-archive.sh"
echo "  3. Distribute the updated package"
echo ""
print_info "Package ready for version $DETECTED_VERSION"