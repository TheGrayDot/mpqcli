#ifndef HELPERS_H
#define HELPERS_H

#include <filesystem>
#include <string>

namespace fs = std::filesystem;

std::string FileTimeToLsTime(int64_t fileTime);
std::string NormalizeFilePath(const fs::path &path);
std::string WindowsifyFilePath(const fs::path &path);
uint32_t CalculateMpqMaxFileValue(const std::string &path);
uint32_t NextPowerOfTwo(uint32_t n);
void PrintAsBinary(const char *buffer, uint32_t size);

#endif
