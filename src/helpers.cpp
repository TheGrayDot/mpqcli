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
    localtime_r(&unixTime, &tm_buf);
    strftime(buf, sizeof(buf), "%b %e %Y %H:%M", &tm_buf);
    return std::string(buf);
}

std::string LocaleToLang(uint16_t locale) {
    switch (locale) {
        case 0:  return "enUS";             // English (US/GB)
        case 1:  return "koKR";             // Korean
        case 2:  return "frFR";             // French
        case 3:  return "deDE";             // German
        case 4:  return "zhCN";             // Chinese (Simplified)
        case 5:  return "zhTW";             // Chinese (Taiwan)
        case 6:  return "esES";             // Spanish (Spain)
        case 7:  return "esMX";             // Spanish (Mexico)
        case 8:  return "ruRU";             // Russian
        case 9:  return "jaJP";             // Japanese
        case 10: return "ptPT";             // Portuguese (Portugal)
        case 11: return "itIT";             // Italian
        default: return "NULL";             // Unknown locale
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

int CountFilesInDirectory(const std::string &directory) {
    int fileCount = 0;
    for (const auto &entry : fs::directory_iterator(directory)) {
        if (fs::is_regular_file(entry.path())) {
            ++fileCount;
        }
    }
    return fileCount;
}

bool IsPrintable(const char* buffer, uint32_t size) {
    for (uint32_t i = 0; i < size; ++i) {
        unsigned char c = static_cast<unsigned char>(buffer[i]);
        if (!std::isprint(c) && !std::isspace(c)) {
            return false;
        }
    }
    return true;
}

void PrintAsText(const char* buffer, uint32_t size) {
    std::cout.write(buffer, size);
}

void PrintAsHex(const char* buffer, uint32_t size) {
    for (uint32_t i = 0; i < size; ++i) {
        std::cout << "\\x" << std::hex << std::setw(2) << std::setfill('0')
                  << (static_cast<unsigned int>(
                         static_cast<unsigned char>(buffer[i])));
    }
}
