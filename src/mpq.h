#ifndef MPQ_H
#define MPQ_H

#include <vector>
#include <StormLib.h>

int OpenMpqArchive(const std::string &filename, HANDLE *hArchive);
int CloseMpqArchive(HANDLE hArchive);
int ExtractFiles(HANDLE hArchive, const std::string& output, const std::string &listfileName);
int ExtractFile(HANDLE hArchive, const std::string& output, const std::string& fileName, bool keepFolderStructure);
HANDLE CreateMpqArchive(std::string outputArchiveName, int32_t fileCount, int32_t mpqVersion);
int AddFiles(HANDLE hArvhive, const std::string& inputPath);
int AddFile(HANDLE hArchive, const std::string& inputFile);
int RemoveFile(HANDLE hArchive, const std::string& fileName);
int ListFiles(HANDLE hHandle, const std::string &listfileName);
char* ReadFile(HANDLE hArchive, const char *szFileName, unsigned int *fileSize);

void PrintMpqInfo(HANDLE hArchive);
std::string GetMpqFileName(HANDLE hArchive);
int32_t GetMpqArchiveSize(HANDLE hArchive);
int64_t GetMpqArchiveHeaderOffset(HANDLE hArchive);
int64_t GetMpqArchiveHeaderSize(HANDLE hArchive);
int32_t GetMpqArchiveSignatureType(HANDLE hArchive);
std::vector<char>  GetMpqArchiveStrongSignature(HANDLE hArchive);
int32_t GetMpqArchiveFileCount(HANDLE hArchive);

int PrintMpqSignature(HANDLE hHandle, int signatureType);

#endif
