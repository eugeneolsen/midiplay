#!/bin/bash
# Script to create a distributable archive of the midiplay installer
# Refactored to use SRP/DRY-compliant library modules

set -e

# Get script directory for sourcing libraries
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Source required library modules
source "$SCRIPT_DIR/lib/version.sh"
source "$SCRIPT_DIR/lib/validation.sh"
source "$SCRIPT_DIR/lib/packaging.sh"
source "$SCRIPT_DIR/lib/metadata.sh"

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

ARCHIVE_NAME="midiplay-installer"

# Function to print colored output
print_status() {
    echo -e "${GREEN}✅ $1${NC}"
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
    echo "  $0                   # Auto-detect version from Git tags or .VERSION"
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

echo -e "${BLUE}📦 Creating Organ Pi MIDI File Player Installer Archive${NC}"
echo -e "${BLUE}====================================================${NC}"
echo ""

# Check if the installer directory exists
if [[ ! -d "$ARCHIVE_NAME" ]]; then
    print_error "Installer directory not found: $ARCHIVE_NAME"
    echo "Please run this script from the project root directory."
    exit 1
fi

# Detect version using library function
VERSION=$(get_version "$PROVIDED_VERSION")
if [[ $? -ne 0 ]]; then
    print_error "Failed to detect version"
    echo ""
    echo "Attempted:"
    echo "  ✗ Git tags (git not available or no tags found)"
    echo "  ✗ .VERSION file (file not found)"
    echo "  ✗ Binary extraction (binary not found or no version string)"
    echo "  ✗ Directory name parsing (directory name doesn't match pattern)"
    echo ""
    echo "Solutions:"
    echo "  1. Provide version manually: $0 --version X.Y.Z"
    echo "  2. Ensure you're in a Git repository with version tags"
    echo "  3. Run update-installer-package.sh first to set up the package"
    exit 1
fi

print_info "Using version: $VERSION"

# Validate package structure using library function
echo ""
if ! validate_package_structure "$ARCHIVE_NAME"; then
    print_error "Package validation failed"
    echo "Please ensure the package structure is complete."
    echo "You may need to run: ./update-installer-package.sh"
    exit 1
fi

# Update .VERSION file to ensure it's current
echo ""
if ! create_version_file "$VERSION"; then
    print_error "Failed to create .VERSION file"
    exit 1
fi

# Create archives using library function
echo ""
if ! create_archives "$VERSION" "$ARCHIVE_NAME"; then
    print_error "Archive creation failed"
    exit 1
fi

echo ""
print_status "Installer archives created successfully!"
echo ""
print_info "Distribution Instructions:"
echo "  1. Upload midiplay-installer-v${VERSION}.tar.gz or .zip to your distribution platform"
echo "  2. Users should download and extract the archive"
echo "  3. Users run: cd midiplay-installer && ./install.sh"
echo ""
print_info "Testing:"
echo "  To test the installer locally:"
echo "    tar -xzf midiplay-installer-v${VERSION}.tar.gz"
echo "    cd midiplay-installer"
echo "    ./install.sh"