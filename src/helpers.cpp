#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>
#include <algorithm>

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
