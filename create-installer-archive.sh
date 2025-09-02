#!/bin/bash
# Script to create a distributable archive of the midiplay installer

set -e

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

ARCHIVE_NAME="midiplay-installer"
VERSION="1.4.7"

echo -e "${BLUE}📦 Creating Organ Pi MIDI File Player Installer Archive${NC}"
echo -e "${BLUE}====================================================${NC}"
echo ""

# Check if the installer directory exists
if [[ ! -d "midiplay-installer" ]]; then
    echo "❌ Error: midiplay-installer directory not found!"
    echo "Please run this script from the project root directory."
    exit 1
fi

# Check if the play binary exists in the installer
if [[ ! -f "midiplay-installer/debian-package/usr/local/bin/play" ]]; then
    echo "❌ Error: play binary not found in installer package!"
    echo "The installer structure is incomplete."
    exit 1
fi

# Verify all required files are present
echo "Verifying installer package contents..."
required_files=(
    "midiplay-installer/install.sh"
    "midiplay-installer/uninstall.sh"
    "midiplay-installer/README.md"
    "midiplay-installer/debian-package/DEBIAN/control"
    "midiplay-installer/debian-package/DEBIAN/postinst"
    "midiplay-installer/debian-package/DEBIAN/prerm"
    "midiplay-installer/debian-package/usr/local/bin/play"
)

for file in "${required_files[@]}"; do
    if [[ ! -f "$file" ]]; then
        echo "❌ Missing required file: $file"
        exit 1
    fi
done

echo -e "${GREEN}✅ All required files present${NC}"

# Create tar.gz archive
echo ""
echo "Creating compressed archive..."
ARCHIVE_FILE="${ARCHIVE_NAME}-v${VERSION}.tar.gz"

if tar -czf "$ARCHIVE_FILE" midiplay-installer/; then
    echo -e "${GREEN}✅ Archive created: $ARCHIVE_FILE${NC}"
else
    echo "❌ Failed to create archive"
    exit 1
fi

# Create zip archive as alternative
echo "Creating zip archive..."
ZIP_FILE="${ARCHIVE_NAME}-v${VERSION}.zip"

if zip -r "$ZIP_FILE" midiplay-installer/ > /dev/null; then
    echo -e "${GREEN}✅ Zip archive created: $ZIP_FILE${NC}"
else
    echo "❌ Failed to create zip archive"
    exit 1
fi

# Display archive information
echo ""
echo -e "${BLUE}📊 Archive Information:${NC}"
echo "----------------------------------------"
echo "Tar.gz file: $ARCHIVE_FILE"
echo "Size: $(du -h "$ARCHIVE_FILE" | cut -f1)"
echo ""
echo "Zip file: $ZIP_FILE"
echo "Size: $(du -h "$ZIP_FILE" | cut -f1)"
echo ""

# Display contents
echo -e "${BLUE}📋 Archive Contents:${NC}"
echo "----------------------------------------"
tar -tzf "$ARCHIVE_FILE" | head -20
if [[ $(tar -tzf "$ARCHIVE_FILE" | wc -l) -gt 20 ]]; then
    echo "... and more files"
fi

echo ""
echo -e "${GREEN}🎉 Installer archives created successfully!${NC}"
echo ""
echo -e "${BLUE}Distribution Instructions:${NC}"
echo "1. Upload $ARCHIVE_FILE or $ZIP_FILE to your distribution platform"
echo "2. Users should download and extract the archive"
echo "3. Users run: cd midiplay-installer && ./install.sh"
echo ""
echo -e "${BLUE}Testing:${NC}"
echo "To test the installer locally:"
echo "  tar -xzf $ARCHIVE_FILE"
echo "  cd midiplay-installer"
echo "  ./install.sh"