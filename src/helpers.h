#ifndef HELPERS_H
#define HELPERS_H

#include <string>
#include <filesystem>

namespace fs = std::filesystem;

std::string NormalizeFilePath(const fs::path &path);
std::string WindowsifyFilePath(const fs::path &path);
int CountFilesInDirectory(const std::string &directory);
int ExtractMpqAndBinFromExe(HANDLE hArchive, bool extractBin);

#endif
