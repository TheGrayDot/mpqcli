#include <iostream>
#include <sys/stat.h>

std::string replaceFileExtension(const std::string& fileName, const std::string& newExtension) {
    size_t lastDotPos = fileName.find_last_of('.');
    if (lastDotPos != std::string::npos) {
        return fileName.substr(0, lastDotPos) + newExtension;
    }
    return fileName + newExtension;
}

void CreateDirectoryIfNotExists(const std::string& path) {
    struct stat info;
    if (stat(path.c_str(), &info) != 0) {
        if (mkdir(path.c_str(), 0777) != 0) {
            std::cerr << "Failed to create directory: " << path << std::endl;
        }
    }
}
