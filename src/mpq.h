#ifndef MPQ_H
#define MPQ_H

#include <vector>
#include <filesystem>

#include <StormLib.h>

namespace fs = std::filesystem;

int OpenMpqArchive(const std::string &filename, HANDLE *hArchive, int32_t flags);
int CloseMpqArchive(HANDLE hArchive);
int SignMpqArchive(HANDLE hArchive);
int ExtractFiles(HANDLE hArchive, const std::string& output, const std::string &listfileName);
int ExtractFile(HANDLE hArchive, const std::string& output, const std::string& fileName, bool keepFolderStructure);
HANDLE CreateMpqArchive(std::string outputArchiveName, int32_t fileCount, int32_t mpqVersion);
int AddFiles(HANDLE hArchive, const std::string& inputPath);
int AddFile(HANDLE hArchive, fs::path localFile, const std::string& archiveFilePath);
int RemoveFile(HANDLE hArchive, const std::string& archiveFilePath);
int ListFiles(HANDLE hHandle, const std::string &listfileName, bool listAll, bool listDetailed);
char* ReadFile(HANDLE hArchive, const char *szFileName, unsigned int *fileSize);
void PrintMpqInfo(HANDLE hArchive, const std::string& infoProperty);
int32_t PrintMpqSignature(HANDLE hArchive, std::string target);

template <typename T>
T GetFileInfo(HANDLE hFile, SFileInfoClass infoClass);

#endif
