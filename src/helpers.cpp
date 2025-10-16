#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>
#include <map>
#include <ctime>

#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#endif

#include <StormLib.h>

#include "helpers.h"

namespace fs = std::filesystem;

std::string FileTimeToLsTime(int64_t fileTime) {
    if (fileTime == 0) {
        return "";
    }
    constexpr int64_t EPOCH_DIFF = 11644473600LL;
    int64_t unixTime = (fileTime / 10000000) - EPOCH_DIFF;
    char buf[20];
    struct tm tm_buf;
#ifdef _WIN32
    localtime_s(&tm_buf, &unixTime);
#else
    localtime_r(&unixTime, &tm_buf);
#endif
    strftime(buf, sizeof(buf), "%b %e %Y %H:%M", &tm_buf);
    return std::string(buf);
}

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
            {0x409, "enUS"},  // English (US)
            {0x404, "zhTW"},  // Chinese (Taiwan)
            {0x405, "csCZ"},  // Czech
            {0x407, "deDE"},  // German
            {0x40a, "esES"},  // Spanish (Spain)
            {0x40c, "frFR"},  // French
            {0x410, "itIT"},  // Italian
            {0x411, "jaJP"},  // Japanese
            {0x412, "koKR"},  // Korean
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
            if (locale != 0x000) { // Skip the default entry to avoid duplication
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
    return it != langToLocaleMap.end() ? it->second : 0;
}

std::string NormalizeFilePath(const fs::path &path) {
    std::string filePath = path.u8string();
    #ifndef _WIN32
        std::replace(filePath.begin(), filePath.end(), '\\', '/');
        return filePath;
    #else
        return filePath;
    #endif
}

std::string WindowsifyFilePath(const fs::path &path) {
    std::string filePath = path.u8string();
    std::replace(filePath.begin(), filePath.end(), '/', '\\');
    return filePath;
}

int32_t CalculateMpqMaxFileValue(const std::string &directory) {
    int32_t fileCount = 0;

    // Determine number of files in target directory, recusively
    for (const auto &entry : fs::recursive_directory_iterator(directory)) {
        if (fs::is_regular_file(entry.path())) {
            ++fileCount;
        }
    }

    // Always add 3 for "special" files
    fileCount += 3;

    // Based on file count, determine max number of files an MPQ archive can hold
    // We always have a minimum of 32
    // Anything over is rounded up to the closest power of 2
    // For example: 64, 128, 256
    // This is examples behavior of WoW MPQ archives (patches and installs)
    if (fileCount <= 32) {
        return 32;
    }

    return NextPowerOfTwo(fileCount);
}

int32_t NextPowerOfTwo(int32_t n)
{
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    return n + 1;
}

void PrintAsBinary(const char* buffer, uint32_t size) {
#ifdef _WIN32
    _setmode(_fileno(stdout), _O_BINARY);
#endif
    std::cout.write(buffer, size);
}
