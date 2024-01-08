#ifndef MPQ_H
#define MPQ_H

#include <StormLib.h>

int OpenMpqArchive(const std::string &filename, HANDLE *hArchive);
int ExtractFiles(HANDLE hArchive, const std::string& output);
int ExtractFile(HANDLE hArchive, const std::string& output, const std::string& fileName);
int ListFiles(HANDLE hHandle);
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
