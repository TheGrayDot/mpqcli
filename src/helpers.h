#ifndef HELPERS_H
#define HELPERS_H

#include <string>
#include <filesystem>

namespace fs = std::filesystem;

std::string FileTimeToLsTime(int64_t fileTime);
std::string LocaleToLang(uint16_t locale);
std::string NormalizeFilePath(const fs::path &path);
std::string WindowsifyFilePath(const fs::path &path);
int32_t CalculateMpqMaxFileValue(const std::string &directory);
int32_t NextPowerOfTwo(int32_t n);
void PrintAsBinary(const char* buffer, uint32_t size);

#endif
