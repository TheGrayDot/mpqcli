#ifndef MPQ_H
#define MPQ_H

#include <vector>
#include <StormLib.h>

int OpenMpqArchive(const std::string &filename, HANDLE *hArchive);
int CloseMpqArchive(HANDLE hArchive);
int SignMpqArchive(HANDLE hArchive);
int ExtractFiles(HANDLE hArchive, const std::string& output, const std::string &listfileName);
int ExtractFile(HANDLE hArchive, const std::string& output, const std::string& fileName, bool keepFolderStructure);
HANDLE CreateMpqArchive(std::string outputArchiveName, int32_t fileCount, int32_t mpqVersion);
int AddFiles(HANDLE hArvhive, const std::string& inputPath);
int AddFile(HANDLE hArchive, const std::string& entry, const std::string& target);
int RemoveFile(HANDLE hArchive, const std::string& fileName);
int ListFiles(HANDLE hHandle, const std::string &listfileName);
char* ReadFile(HANDLE hArchive, const char *szFileName, unsigned int *fileSize);
void PrintMpqInfo(HANDLE hArchive, const std::string& infoProperty);
bool VerifyMpqArchive(HANDLE hArchive);
int32_t PrintMpqSignature(HANDLE hArchive, std::string target);

template <typename T>
T GetMpqArchiveInfo(HANDLE hArchive, SFileInfoClass infoClass);

#endif
