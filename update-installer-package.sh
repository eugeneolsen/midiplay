#!/bin/bash
# Script to update the midiplay installer package with a new binary version
# Refactored to use SRP/DRY-compliant library modules

set -e

# Get script directory for sourcing libraries
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Source required library modules
source "$SCRIPT_DIR/lib/version.sh"
source "$SCRIPT_DIR/lib/translations.sh"
source "$SCRIPT_DIR/lib/metadata.sh"
source "$SCRIPT_DIR/lib/packaging.sh"
source "$SCRIPT_DIR/lib/validation.sh"

# Colors for output (kept for backward compatibility)
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Default values
PLAY_BINARY="./play"
INSTALLER_DIR="midiplay-installer"
PACKAGE_DIR="$INSTALLER_DIR/debian-package"
BINARY_DEST="$PACKAGE_DIR/usr/local/bin/play"

# Function to print colored output (kept for backward compatibility)
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
PROVIDED_VERSION=""
CREATE_ARCHIVES=false

while [[ $# -gt 0 ]]; do
    case $1 in
        -b|--binary)
            BINARY_PATH="$2"
            shift 2
            ;;
        -v|--version)
            PROVIDED_VERSION="$2"
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

# Detect version using library function
DETECTED_VERSION=$(get_version "$PROVIDED_VERSION")
if [[ $? -ne 0 ]]; then
    print_error "Failed to detect version"
    echo "Please provide version explicitly with: $0 -v X.Y.Z"
    exit 1
fi

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

# Compile and install translations using library function
echo ""
if ! compile_translations; then
    print_warning "Translation compilation had issues, but continuing..."
fi

# Update all metadata files using library function
echo ""
if ! update_all_metadata "$DETECTED_VERSION"; then
    print_error "Failed to update metadata files"
    exit 1
fi

# Verify the update
echo ""
echo "Verifying package update..."
if ! verify_binary "$BINARY_DEST"; then
    print_error "Binary verification failed"
    exit 1
fi

# Show binary info
BINARY_SIZE=$(du -h "$BINARY_DEST" | cut -f1)
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
    if ! create_archives "$DETECTED_VERSION"; then
        print_warning "Archive creation had issues"
    fi
fi

echo ""
print_status "Package update completed successfully!"
echo ""
print_info "Package contents:"
echo "  • Binary: $BINARY_DEST"
echo "  • Version: $DETECTED_VERSION"

# Check if translations exist
if check_translations_exist; then
    TRANSLATION_COUNT=$(find "$PACKAGE_DIR/usr/share/locale" -name "*.mo" 2>/dev/null | wc -l)
    echo "  • Translations: $TRANSLATION_COUNT language(s)"
fi

echo ""
print_info "Next steps:"
echo "  1. Test the updated installer: cd $INSTALLER_DIR && ./install.sh"
if [[ "$CREATE_ARCHIVES" != true ]]; then
    echo "  2. Create distribution archives: ./create-installer-archive.sh"
    echo "  3. Distribute the updated package"
else
    echo "  2. Distribute the updated package archives"
fi
echo ""
print_info "Package ready for version $DETECTED_VERSION"