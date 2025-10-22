#!/bin/bash
# Production script to compile all translation files (.po → .mo)
# This script is called during development builds and package creation

set -e

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
LANGUAGES=("es" "pt_BR" "fr_FR" "fr_CA")
PACKAGE_NAME="midiplay"
PACKAGE_VERSION="1.5.7"
PO_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

print_status() {
    echo -e "${GREEN}✅ $1${NC}"
}

print_info() {
    echo -e "${BLUE}ℹ️  $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}⚠️  $1${NC}"
}

print_error() {
    echo -e "${RED}❌ $1${NC}"
}

echo -e "${BLUE}🌍 Compiling Translations for ${PACKAGE_NAME}${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

# Check if msgfmt is available
if ! command -v msgfmt &> /dev/null; then
    print_error "msgfmt not found. Please install gettext:"
    echo "  sudo apt-get install gettext"
    exit 1
fi

# Track compilation results
SUCCESS_COUNT=0
FAILED_COUNT=0
SKIPPED_COUNT=0

# Compile each language
for lang in "${LANGUAGES[@]}"; do
    PO_FILE="${PO_DIR}/${lang}.po"
    MO_DIR="${PO_DIR}/${lang}/LC_MESSAGES"
    MO_FILE="${MO_DIR}/${PACKAGE_NAME}.mo"
    
    if [[ ! -f "$PO_FILE" ]]; then
        print_warning "Translation file not found: ${lang}.po (skipping)"
        SKIPPED_COUNT=$((SKIPPED_COUNT + 1))
        continue
    fi
    
    # Create output directory
    mkdir -p "$MO_DIR"
    
    # Compile .po to .mo
    if msgfmt "$PO_FILE" -o "$MO_FILE" 2>/dev/null; then
        # Get statistics
        STATS=$(msgfmt --statistics "$PO_FILE" 2>&1 | head -1)
        print_status "Compiled ${lang}: ${STATS}"
        SUCCESS_COUNT=$((SUCCESS_COUNT + 1))
    else
        print_error "Failed to compile ${lang}.po"
        FAILED_COUNT=$((FAILED_COUNT + 1))
    fi
done

echo ""
echo -e "${BLUE}Compilation Summary:${NC}"
echo "  ✅ Successful: $SUCCESS_COUNT"
if [[ $FAILED_COUNT -gt 0 ]]; then
    echo "  ❌ Failed: $FAILED_COUNT"
fi
if [[ $SKIPPED_COUNT -gt 0 ]]; then
    echo "  ⏭️  Skipped: $SKIPPED_COUNT"
fi

if [[ $FAILED_COUNT -gt 0 ]]; then
    echo ""
    print_error "Some translations failed to compile!"
    exit 1
fi

echo ""
print_status "All translations compiled successfully!"
print_info "Binary files (.mo) are ready for deployment"