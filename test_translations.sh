#!/bin/bash
# Quick test script to verify all translations work

echo "Testing i18n translations for 'Connected to:'"
echo "=============================================="
echo ""

echo "English (default):"
LANG=C ./play --help 2>&1 | head -3
echo ""

echo "Spanish (es_ES):"
echo "  Expected: 'Conectado a:'"
echo ""

echo "Portuguese Brazilian (pt_BR):"
echo "  Expected: 'Conectado a:'"
echo ""

echo "French France (fr_FR):"
echo "  Expected: 'Connecté à :'"
echo ""

echo "French Canada (fr_CA):"
echo "  Expected: 'Connecté à :'"
echo ""

echo "To test translations properly, run with a MIDI file:"
echo "  LANG=es_ES.UTF-8 ./play -V your_file.mid"
echo "  LANG=pt_BR.UTF-8 ./play -V your_file.mid"
echo "  LANG=fr_FR.UTF-8 ./play -V your_file.mid"
echo "  LANG=fr_CA.UTF-8 ./play -V your_file.mid"