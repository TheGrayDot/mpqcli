#ifndef HELPERS_H
#define HELPERS_H

#include <filesystem>

namespace fs = std::filesystem;

std::string NormalizeFilePath(const fs::path &path);
int ExtractMpqAndBinFromExe(HANDLE hArchive, bool extractBin);

#endif
