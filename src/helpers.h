#ifndef HELPERS_H
#define HELPERS_H

std::string replaceFileExtension(const std::string& fileName, const std::string& newExtension);
void CreateDirectoryIfNotExists(const std::string& path);
int ExtractMpqFromExe(HANDLE hArchive);
int ExtractBinFromExe(HANDLE hArchive);

#endif
