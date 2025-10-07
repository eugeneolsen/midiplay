#pragma once

#include <libintl.h>
#include <locale.h>
#include <string>
#include <cstdio>
#include <iostream>

namespace MidiPlay {

/**
 * @brief Internationalization (i18n) utilities and macros
 * 
 * This header provides centralized internationalization support for the midiplay application.
 * It includes macros for both singular and plural string translations, and functions
 * for initializing the localization system.
 */

// Package and locale directory definitions
#define PACKAGE "midiplay"

// Use local locale directory if running from source, otherwise system directory
#ifdef DEBUG
#define LOCALEDIR "/home/eugene/src/midiplay/po"
#else
#define LOCALEDIR "/usr/share/locale"
#endif

// Standard translation macro for singular strings
#define _(String) gettext(String)

// Plural translation macro using ngettext
// Usage: N_("singular form", "plural form", count)
#define N_(singular, plural, count) ngettext(singular, plural, count)

// Context-specific translation macro (for disambiguation)
#define C_(context, String) pgettext(context, String)

// Context-specific plural translation macro
#define CN_(context, singular, plural, count) npgettext(context, singular, plural, count)

/**
 * @brief Initialize the internationalization system
 *
 * This function sets up the locale and text domain for internationalization.
 * It should be called once at the beginning of the program.
 */
inline void initializeI18n() {
    // Set the current locale based on environment variables
    const char* locale = setlocale(LC_ALL, "");
    
    // Handle locale name normalization (e.g., es_ES.UTF-8 -> es_ES.utf8)
    if (locale) {
        std::string localeStr(locale);
        // Replace UTF-8 with utf8 if needed
        size_t pos = localeStr.find("UTF-8");
        if (pos != std::string::npos) {
            localeStr.replace(pos, 5, "utf8");
            setlocale(LC_ALL, localeStr.c_str());
        }
    }
    
    // Set the text domain for translations
    bindtextdomain(PACKAGE, LOCALEDIR);
    
    // Set the current text domain
    textdomain(PACKAGE);
    
    // Debug output
    #ifdef DEBUG
    std::cerr << "DEBUG: Locale set to: " << (locale ? locale : "NULL") << std::endl;
    std::cerr << "DEBUG: Text domain: " << PACKAGE << std::endl;
    std::cerr << "DEBUG: Locale dir: " << LOCALEDIR << std::endl;
    std::cerr << "DEBUG: Test translation: " << gettext("verse") << std::endl;
    #endif
}

/**
 * @brief Get the current locale
 * 
 * @return std::string The current locale string
 */
inline std::string getCurrentLocale() {
    const char* locale = setlocale(LC_ALL, nullptr);
    return locale ? locale : "";
}

/**
 * @brief Check if a specific locale is available
 * 
 * @param locale The locale to check (e.g., "es", "fr_FR", "pt_BR")
 * @return true if the locale is available, false otherwise
 */
inline bool isLocaleAvailable(const std::string& locale) {
    std::string currentLocale = getCurrentLocale();
    
    // Try to set the locale temporarily
    if (setlocale(LC_ALL, locale.c_str()) != nullptr) {
        // Restore the original locale
        setlocale(LC_ALL, currentLocale.c_str());
        return true;
    }
    
    return false;
}

/**
 * @brief Format a plural string with proper internationalization
 *
 * This is a convenience function for formatting strings that include numbers
 * with proper pluralization. It handles the common case of "N items" patterns.
 *
 * @param count The number to format
 * @param singular The singular form (e.g., "verse")
 * @param plural The plural form (e.g., "verses")
 * @return std::string The formatted string with proper pluralization
 */
inline std::string formatPlural(int count, const char* singular, const char* plural) {
    // Use a large enough buffer for the formatted string
    char buffer[256];
    
    // Get the properly pluralized word
    const char* word = N_(singular, plural, count);
    
    // Format the string with the count and the translated word
    snprintf(buffer, sizeof(buffer), "%d %s", count, word);
    
    return std::string(buffer);
}

} // namespace MidiPlay