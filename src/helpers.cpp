#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

#include <StormLib.h>

#include "helpers.h"
#include "mpq.h"

namespace fs = std::filesystem;


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
