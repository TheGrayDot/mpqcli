#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>
#include <ctime>

#include <StormLib.h>

#include "helpers.h"
#include "mpq.h"

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

std::string LocaleToLang(uint16_t locale) {
    switch (locale) {
        case 0:  return "enUS";     // English (US/GB)
        case 1:  return "koKR";     // Korean
        case 2:  return "frFR";     // French
        case 3:  return "deDE";     // German
        case 4:  return "zhCN";     // Chinese (Simplified)
        case 5:  return "zhTW";     // Chinese (Taiwan)
        case 6:  return "esES";     // Spanish (Spain)
        case 7:  return "esMX";     // Spanish (Mexico)
        case 8:  return "ruRU";     // Russian
        case 9:  return "jaJP";     // Japanese
        case 10: return "ptPT";     // Portuguese (Portugal)
        case 11: return "itIT";     // Italian
        default: return "";         // Unknown locale
    }
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

    // Determine number of files in target directory
    for (const auto &entry : fs::directory_iterator(directory)) {
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

    fileCount--;
    fileCount |= fileCount >> 1;
    fileCount |= fileCount >> 2;
    fileCount |= fileCount >> 4;
    fileCount |= fileCount >> 8;
    fileCount |= fileCount >> 16;
    fileCount++;

    return fileCount;
}

void PrintAsBinary(const char* buffer, uint32_t size) {
#ifdef _WIN32
    _setmode(_fileno(stdout), _O_BINARY);
#endif
    std::cout.write(buffer, size);
}
