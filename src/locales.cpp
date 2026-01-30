#include <algorithm>
#include <fstream>
#include <vector>
#include <map>
#include <sstream>
#include <iomanip>

#include "locales.h"

namespace {
    // Files in MPQs have locales with which they are associated.
    // Multiple files can have the same file name if they have different locales.
    // This function maps locales to language names.
    //
    // The mappings are from the Windows Language Code Identifier (LCID).
    // They can be found, for example, here:
    // https://winprotocoldoc.z19.web.core.windows.net/MS-LCID/%5bMS-LCID%5d.pdf

    // Define a bidirectional map for locale-language mappings
    const std::map<uint16_t, std::string> localeToLangMap = {
            {0x000, "enUS"},  // Default - English (US)
            {0x404, "zhTW"},  // Chinese (Taiwan)
            {0x405, "csCZ"},  // Czech
            {0x407, "deDE"},  // German
            {0x409, "enUS"},  // English (US)
            {0x40a, "esES"},  // Spanish (Spain)
            {0x40c, "frFR"},  // French
            {0x410, "itIT"},  // Italian
            {0x411, "jaJP"},  // Japanese
            {0x412, "koKR"},  // Korean
            {0x413, "nlNL"},  // Dutch
            {0x415, "plPL"},  // Polish
            {0x416, "ptBR"},  // Portuguese (Brazil)
            {0x419, "ruRU"},  // Russian
            {0x804, "zhCN"},  // Chinese (Simplified)
            {0x809, "enGB"},  // English (UK)
            {0x80A, "esMX"},  // Spanish (Mexico)
            {0x816, "ptPT"},  // Portuguese (Portugal)
    };

    // Create a reverse map for language-to-locale lookups
    const std::map<std::string, uint16_t> langToLocaleMap = []() {
        std::map<std::string, uint16_t> reverseMap;
        for (const auto& [locale, lang] : localeToLangMap) {
            if (locale != defaultLocale) { // Skip the default locale to avoid duplication
                reverseMap[lang] = locale;
            }
        }
        return reverseMap;
    }();

    std::string FormatLocaleAsHex(const LCID locale) {
        std::stringstream ss;
        ss << std::hex << std::uppercase << locale;
        const std::string hexStr = ss.str();
        // Prepend 0s if needed
        return std::string(4 - hexStr.length(), '0') + hexStr;
    }
}

// Check if a string is a 4-character hexadecimal number and parse it
// Returns the parsed LCID if valid, otherwise returns defaultLocale (0)
LCID ParseHexLocale(const std::string& str) {
    if (str.length() != 4) {
        return defaultLocale;
    }

    // Check if all characters are hexadecimal
    for (char c : str) {
        if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))) {
            return defaultLocale;
        }
    }

    // Parse the hexadecimal string
    std::stringstream ss;
    ss << std::hex << str;
    LCID locale;
    ss >> locale;
    return locale;
}

std::string LocaleToLang(uint16_t locale) {
    auto it = localeToLangMap.find(locale);
    return it != localeToLangMap.end() ? it->second : FormatLocaleAsHex(locale);
}

LCID LangToLocale(const std::string& lang) {
    auto it = langToLocaleMap.find(lang);
    if (it != langToLocaleMap.end()) {
        return it->second;
    }

    // Try parsing as a hexadecimal LCID
    LCID hexLocale = ParseHexLocale(lang);
    if (hexLocale != defaultLocale) {
        return hexLocale;
    }

    return defaultLocale;
}


std::vector<std::string> GetAllLocales() {
    std::vector<std::string> locales;
    for (const auto& [locale, lang] : localeToLangMap) {
        if (locale != defaultLocale) { // Skip the default locale to avoid duplication
            locales.push_back(lang);
        }
    }
    // Sort the locales for consistent output
    std::sort(locales.begin(), locales.end());
    return locales;
}

std::string PrettyPrintLocale(const LCID locale, const std::string &prefix, bool alwaysPrint) {
    if (locale == defaultLocale && !alwaysPrint) {
        return "";
    }
    const auto lang = LocaleToLang(locale);
    return prefix + lang;
}
