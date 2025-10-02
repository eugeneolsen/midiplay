#!/bin/bash
# Temporary script for testing i18n setup - Phase 1 Proof of Concept

set -e

echo "=== Phase 1: i18n Proof of Concept ==="
echo ""

# Step 1: Extract translatable strings from source files
echo "1. Extracting strings from source code..."
xgettext --from-code=UTF-8 \
         --keyword=_ \
         --package-name=midiplay \
         --package-version=1.5.6 \
         --msgid-bugs-address="eugene@example.com" \
         --output=po/midiplay.pot \
         --files-from=po/POTFILES.in

echo "   Created: po/midiplay.pot"
echo ""

# Step 2: Create Spanish translation file (if it doesn't exist)
if [ ! -f po/es.po ]; then
    echo "2. Creating Spanish (es) translation file..."
    msginit --input=po/midiplay.pot \
            --locale=es_ES.UTF-8 \
            --output=po/es.po \
            --no-translator
    echo "   Created: po/es.po"
else
    echo "2. Updating Spanish (es) translation file..."
    msgmerge --update po/es.po po/midiplay.pot
    echo "   Updated: po/es.po"
fi
echo ""

# Step 3: Compile .po to .mo
echo "3. Compiling translations to binary format..."
mkdir -p po/es/LC_MESSAGES
msgfmt po/es.po -o po/es/LC_MESSAGES/midiplay.mo
echo "   Created: po/es/LC_MESSAGES/midiplay.mo"
echo ""

echo "=== Translation files ready ==="
echo ""
echo "Next steps:"
echo "1. Edit po/es.po to add Spanish translation"
echo "2. Run this script again to compile"
echo "3. Test with: LANG=es_ES.UTF-8 ./play [args]"