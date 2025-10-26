#include <algorithm>
#include <fstream>
#include <vector>
#include <map>

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
            {0x416, "ptPT"},  // Portuguese (Portugal)
            {0x419, "ruRU"},  // Russian
            {0x804, "zhCN"},  // Chinese (Simplified)
            {0x809, "enGB"},  // English (UK)
            {0x80A, "esMX"}   // Spanish (Mexico)
    };

    // Create a reverse map for language to locale lookups
    const std::map<std::string, uint16_t> langToLocaleMap = []() {
        std::map<std::string, uint16_t> reverseMap;
        for (const auto& [locale, lang] : localeToLangMap) {
            if (locale != defaultLocale) { // Skip the default locale to avoid duplication
                reverseMap[lang] = locale;
            }
        }
        return reverseMap;
    }();
}

std::string LocaleToLang(uint16_t locale) {
    auto it = localeToLangMap.find(locale);
    return it != localeToLangMap.end() ? it->second : "";
}

LCID LangToLocale(const std::string& lang) {
    auto it = langToLocaleMap.find(lang);
    return it != langToLocaleMap.end() ? it->second : defaultLocale;
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
