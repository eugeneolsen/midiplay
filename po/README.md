# midiplay Translation Guide

This directory contains all translation files for the midiplay MIDI player. The project uses GNU gettext for internationalization (i18n), enabling automatic language detection based on the user's system locale.

## Current Languages

- **Spanish (es_ES)** - Spain Spanish
- **Brazilian Portuguese (pt_BR)** - Brazilian Portuguese  
- **French - France (fr_FR)** - French (France)
- **French - Canada (fr_CA)** - French (Canada)

## Translation Files

- **`POTFILES.in`** - List of source files containing translatable strings
- **`midiplay.pot`** - Master translation template (auto-generated)
- **`*.po`** - Human-readable translation files (one per language)
- **`*/LC_MESSAGES/*.mo`** - Compiled binary translation files (auto-generated, git-ignored)

## Quick Start for Translators

### Updating an Existing Translation

1. **Edit the `.po` file** for your language:
   ```bash
   # Example: Edit Spanish translation
   nano po/es.po
   ```

2. **Find untranslated strings** marked as `fuzzy` or with empty `msgstr`:
   ```po
   #: play.cpp:100
   msgid "Error: Could not open file"
   msgstr ""  # ← Add your translation here
   ```

3. **Add your translation**:
   ```po
   msgid "Error: Could not open file"
   msgstr "Error: No se pudo abrir el archivo"
   ```

4. **Compile and test**:
   ```bash
   ./po/compile-translations.sh
   LANG=es_ES.UTF-8 ./play --help
   ```

### Adding a New Language

1. **Generate a new `.po` file** from the template:
   ```bash
   # Example: Adding German (de_DE)
   msginit --input=po/midiplay.pot \
           --locale=de_DE.UTF-8 \
           --output=po/de.po
   ```

2. **Update `compile-translations.sh`** to include the new language:
   ```bash
   # Edit po/compile-translations.sh
   # Add "de" to the LANGUAGES array:
   LANGUAGES=("es" "pt_BR" "fr_FR" "fr_CA" "de")
   ```

3. **Translate all strings** in `po/de.po`

4. **Test the new translation**:
   ```bash
   ./po/compile-translations.sh
   LANG=de_DE.UTF-8 ./play --help
   ```

## Translation Workflow

### For Developers: After Modifying Source Code

When you add or modify user-facing strings in the source code:

1. **Wrap strings with `_()`** macro:
   ```cpp
   // Before:
   std::cout << "Hello, world!" << std::endl;
   
   // After:
   std::cout << _("Hello, world!") << std::endl;
   ```

2. **Update the translation template** and all `.po` files:
   ```bash
   # Extract strings from source files
   xgettext --from-code=UTF-8 \
            --keyword=_ \
            --package-name=midiplay \
            --package-version=1.5.0 \
            --output=po/midiplay.pot \
            --files-from=po/POTFILES.in
   
   # Update all .po files with new strings
   for lang in es pt_BR fr_FR fr_CA; do
       msgmerge --update po/$lang.po po/midiplay.pot
   done
   ```

3. **Notify translators** that new strings need translation

4. **Compile translations** (automatic during build):
   ```bash
   ./po/compile-translations.sh
   ```

### For Translators: Updating Translations

1. **Pull latest changes** from version control

2. **Update your language's `.po` file**:
   ```bash
   # Find fuzzy entries (strings that changed)
   grep -n "fuzzy" po/es.po
   
   # Find untranslated entries
   msggrep --untranslated po/es.po
   ```

3. **Translate new/modified strings**

4. **Remove `#, fuzzy` markers** after verifying translations

5. **Test your changes**:
   ```bash
   ./po/compile-translations.sh
   LANG=es_ES.UTF-8 ./play -h
   ```

6. **Commit only `.po` files** (never commit `.mo` files)

## Translation Best Practices

### General Guidelines

1. **Maintain context** - Understand where the string appears in the application
2. **Keep formatting** - Preserve special characters like `\n`, `%s`, etc.
3. **Test thoroughly** - Always test translations in the actual application
4. **Be consistent** - Use the same terms throughout your translation
5. **Cultural adaptation** - Adapt to local conventions, not just literal translation

### Command-Line Options

- **Keep flags in English**: `-h`, `--help`, `-v`, etc. remain unchanged
- **Translate descriptions**: Only translate the help text, not the option names
  ```po
  msgid "  -h, --help           Show this help message"
  msgstr "  -h, --help           Mostrar este mensaje de ayuda"
  ```

### Technical Terms

Some terms should remain in English or be transliterated:
- **MIDI** - Keep as "MIDI" in all languages
- **YAML** - Keep as "YAML"
- File paths and commands - Keep as-is

### Placeholders

Preserve special formatting:
```po
# Correct:
msgid "Playing: \"%s\""
msgstr "Tocando: \"%s\""

# Wrong (missing quotes):
msgid "Playing: \"%s\""
msgstr "Tocando: %s"
```

### Plural Forms

Gettext supports proper plural handling for different languages. Each language has its own plural rules.

#### In C++ Source Code

Use `ngettext()` for plural strings:
```cpp
#include <libintl.h>
#define _(String) gettext(String)
#define N_(Singular, Plural, Count) ngettext(Singular, Plural, Count)

// Example usage:
int count = get_file_count();
printf(N_("%d file processed", "%d files processed", count), count);
```

#### In Translation Files

When you extract strings with `xgettext`, plural forms appear as `msgid_plural`:

**English source** (`midiplay.pot`):
```po
#: file_processor.cpp:42
#, c-format
msgid "%d file processed"
msgid_plural "%d files processed"
msgstr[0] ""
msgstr[1] ""
```

**Spanish translation** (`es.po`):
```po
# Spanish has 2 plural forms: n != 1
#: file_processor.cpp:42
#, c-format
msgid "%d file processed"
msgid_plural "%d files processed"
msgstr[0] "%d archivo procesado"
msgstr[1] "%d archivos procesados"
```

**Polish translation** (if added) (`pl.po`):
```po
# Polish has 3 plural forms:
# 0: (n==1)
# 1: (n%10>=2 && n%10<=4 && (n%100<10 || n%100>=20))
# 2: other
#: file_processor.cpp:42
#, c-format
msgid "%d file processed"
msgid_plural "%d files processed"
msgstr[0] "%d plik przetworzony"      # 1 file
msgstr[1] "%d pliki przetworzone"     # 2-4 files
msgstr[2] "%d plików przetworzonych"  # 0, 5+ files
```

#### Plural Rules by Language

Each `.po` file includes a `Plural-Forms` header:

**Spanish** (es, pt_BR, fr_FR, fr_CA):
```po
"Plural-Forms: nplurals=2; plural=(n != 1);\n"
```
- `msgstr[0]`: n == 1 (singular: "1 archivo")
- `msgstr[1]`: n != 1 (plural: "0 archivos", "2 archivos", etc.)

**English**:
```po
"Plural-Forms: nplurals=2; plural=(n != 1);\n"
```
- `msgstr[0]`: n == 1 (singular: "1 file")
- `msgstr[1]`: n != 1 (plural: "0 files", "2 files", etc.)

**Russian** (if added):
```po
"Plural-Forms: nplurals=3; plural=(n%10==1 && n%100!=11 ? 0 : n%10>=2 && n%10<=4 && (n%100<10 || n%100>=20) ? 1 : 2);\n"
```
- `msgstr[0]`: 1, 21, 31, ... (один файл)
- `msgstr[1]`: 2-4, 22-24, ... (два файла)
- `msgstr[2]`: 0, 5-20, 25-30, ... (пять файлов)

**Arabic** (if added):
```po
"Plural-Forms: nplurals=6; plural=(n==0 ? 0 : n==1 ? 1 : n==2 ? 2 : n%100>=3 && n%100<=10 ? 3 : n%100>=11 ? 4 : 5);\n"
```
- Has 6 different plural forms!

#### Common Plural Examples

**Simple counting**:
```cpp
// C++ code:
printf(N_("%d MIDI port detected", "%d MIDI ports detected", count), count);

// Spanish translation:
msgstr[0] "%d puerto MIDI detectado"
msgstr[1] "%d puertos MIDI detectados"
```

**Files/items**:
```cpp
// C++ code:
printf(N_("Found %d error", "Found %d errors", error_count), error_count);

// French translation:
msgstr[0] "%d erreur trouvée"
msgstr[1] "%d erreurs trouvées"
```

**Time units**:
```cpp
// C++ code:
printf(N_("%d second remaining", "%d seconds remaining", seconds), seconds);

// Portuguese translation:
msgstr[0] "%d segundo restante"
msgstr[1] "%d segundos restantes"
```

#### Testing Plural Forms

Always test with different counts to verify all plural forms work correctly:

```bash
# Test n=0 (should use plural form in Romance languages)
LANG=es_ES.UTF-8 ./program_with_0_items

# Test n=1 (singular)
LANG=es_ES.UTF-8 ./program_with_1_item

# Test n=2 (plural)
LANG=es_ES.UTF-8 ./program_with_2_items

# Test n=5 (plural)
LANG=es_ES.UTF-8 ./program_with_5_items
```

For languages with more than 2 plural forms (like Polish, Russian, Arabic), test additional edge cases based on the plural formula.

## File Structure

```
po/
├── README.md                    # This file
├── POTFILES.in                  # List of source files to scan
├── midiplay.pot                 # Translation template (auto-generated)
├── compile-translations.sh      # Compilation script
├── es.po                        # Spanish translations
├── pt_BR.po                     # Brazilian Portuguese translations
├── fr_FR.po                     # French (France) translations
├── fr_CA.po                     # French (Canada) translations
├── es/LC_MESSAGES/
│   └── midiplay.mo              # Compiled Spanish (auto-generated)
├── pt_BR/LC_MESSAGES/
│   └── midiplay.mo              # Compiled Portuguese (auto-generated)
├── fr_FR/LC_MESSAGES/
│   └── midiplay.mo              # Compiled French (auto-generated)
└── fr_CA/LC_MESSAGES/
    └── midiplay.mo              # Compiled French-Canada (auto-generated)
```

## Common gettext Commands

### Extracting Strings
```bash
xgettext --keyword=_ --from-code=UTF-8 \
         --output=po/midiplay.pot \
         --files-from=po/POTFILES.in
```

### Creating a New Translation
```bash
msginit --input=po/midiplay.pot \
        --locale=de_DE.UTF-8 \
        --output=po/de.po
```

### Updating Existing Translation
```bash
msgmerge --update po/es.po po/midiplay.pot
```

### Compiling Translation
```bash
msgfmt po/es.po -o po/es/LC_MESSAGES/midiplay.mo
```

### Checking Translation Statistics
```bash
msgfmt --statistics po/es.po
```

### Finding Untranslated Strings
```bash
msgattrib --untranslated po/es.po
```

## Testing Translations

### Test Specific Language
```bash
# Spanish
LANG=es_ES.UTF-8 ./play --help

# Portuguese
LANG=pt_BR.UTF-8 ./play your_file.mid

# French (France)
LANG=fr_FR.UTF-8 ./play -h

# French (Canada)
LANG=fr_CA.UTF-8 ./play -h
```

### Test System Default
```bash
# Will use your system's configured language
./play --help
```

### Enable All Locales (if needed)
```bash
# Edit locale configuration
sudo nano /etc/locale.gen

# Uncomment desired locales:
# es_ES.UTF-8 UTF-8
# pt_BR.UTF-8 UTF-8
# fr_FR.UTF-8 UTF-8
# fr_CA.UTF-8 UTF-8

# Generate locales
sudo locale-gen
```

## Build Integration

Translations are automatically compiled during:

1. **Development builds** - VSCode build task compiles translations first
2. **Package creation** - `update-installer-package.sh` compiles and includes translations
3. **Manual compilation** - Run `./po/compile-translations.sh` anytime

The compiled `.mo` files are:
- ✅ **Included in installer packages** (`/usr/share/locale/*/LC_MESSAGES/`)
- ❌ **Never committed to git** (listed in `.gitignore`)
- ✅ **Automatically distributed** with the application

## Troubleshooting

### Translation Not Showing

1. **Check locale is installed**:
   ```bash
   locale -a | grep es_ES
   ```

2. **Verify `.mo` file exists**:
   ```bash
   ls -la po/es/LC_MESSAGES/midiplay.mo
   ```

3. **Check file permissions**:
   ```bash
   # Development
   ls -la po/es/LC_MESSAGES/midiplay.mo
   
   # Production
   ls -la /usr/share/locale/es/LC_MESSAGES/midiplay.mo
   ```

4. **Test with explicit LANG**:
   ```bash
   LANG=es_ES.UTF-8 ./play --help
   ```

### Compilation Errors

- **"msgfmt: found X fatal errors"** - Check `.po` file syntax
- **Missing translations** - Look for empty `msgstr ""` entries
- **Fuzzy entries** - Review and remove `#, fuzzy` after verification

## Resources

- **GNU gettext manual**: https://www.gnu.org/software/gettext/manual/
- **PO file format**: https://www.gnu.org/software/gettext/manual/html_node/PO-Files.html
- **Translation tools**: Poedit (https://poedit.net/), Lokalize, gtranslator

## Contributing

1. Fork the repository
2. Update or add translations in `po/*.po` files
3. Test your translations thoroughly
4. Commit **only** `.po` files (never `.mo` files)
5. Submit a pull request

## Questions?

For translation questions or issues, please:
- Open an issue on the project repository
- Contact the maintainers
- Check the GNU gettext documentation

---

**Last Updated**: Phase 3 Implementation (Internationalization Complete)  
**Supported Languages**: 4 (es, pt_BR, fr_FR, fr_CA)  
**Total Translatable Strings**: 63