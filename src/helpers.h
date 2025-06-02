#ifndef HELPERS_H
#define HELPERS_H

#include <string>
#include <filesystem>

namespace fs = std::filesystem;

std::string NormalizeFilePath(const fs::path &path);
std::string WindowsifyFilePath(const fs::path &path);
int CountFilesInDirectory(const std::string &directory);
bool IsPrintable(const char* buffer, uint32_t size);
void PrintAsText(const char* buffer, uint32_t size);
void PrintAsHex(const char* buffer, uint32_t size);

#endif
