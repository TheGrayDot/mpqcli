#include <iostream>
#include <cstdint>
#include <iomanip>

#include <StormLib.h>

#include "mpq.h"
#include "helpers.h"

int OpenMpqArchive(const std::string &filename, HANDLE *hArchive) {
    if (!SFileOpenArchive(filename.c_str(), 0, MPQ_OPEN_READ_ONLY, hArchive)) {
        std::cerr << "[+] Failed to open: " << filename << std::endl;
        return -1;
    }
    return 1;
}

int ExtractFiles(HANDLE hArchive, const std::string& output) {
    CreateDirectoryIfNotExists(output);

    SFILE_FIND_DATA findData;
    HANDLE findHandle = SFileFindFirstFile(hArchive, "*", &findData, NULL);
    if (findHandle == NULL) {
        std::cerr << "[+] Failed to find first file in MPQ archive." << std::endl;
        SFileCloseArchive(hArchive);
        return -1;
    }

    do {
        std::string outputFileName = output + "/" + findData.cFileName;
        if (SFileExtractFile(hArchive, findData.cFileName, outputFileName.c_str(), 0)) {
            std::cout << "[+] Extracted: " << findData.cFileName << std::endl;
        } else {
            int32_t error = GetLastError();
            std::cerr << "[+] Failed: " << "(" << error << ") " << findData.cFileName << std::endl;
        }
    } while (SFileFindNextFile(findHandle, &findData));

    return 0;
}

int ListFiles(HANDLE hArchive) {
    // Find the first file in MPQ archive to iterate from
    SFILE_FIND_DATA findData;
    HANDLE findHandle = SFileFindFirstFile(hArchive, "*", &findData, NULL);
    if (findHandle == NULL) {
        std::cerr << "[+] Failed to find first file in MPQ archive." << std::endl;
        SFileCloseArchive(hArchive);
        return -1;
    }

    // Loop through all files in MPQ archive
    do {
        std::cout << findData.cFileName << std::endl;
    } while (SFileFindNextFile(findHandle, &findData));

    return 1;
}

char* ReadOneFile(HANDLE hArchive, const char *szFileName, unsigned int *fileSize) {
    if (!SFileHasFile(hArchive, szFileName)) {
        std::cerr << "[+] Failed: File doesn't exist" << std::endl;
        return NULL;
    }

    HANDLE hFile;
    if (!SFileOpenFileEx(hArchive, szFileName, SFILE_OPEN_FROM_MPQ, &hFile)) {
        std::cerr << "[+] Failed: File cannot be opened" << std::endl;
        return NULL;
    }

    *fileSize = SFileGetFileSize(hFile, NULL);

    char* fileContent = new char[*fileSize + 1];
    unsigned int dwBytes;
    if (!SFileReadFile(hFile, fileContent, *fileSize, &dwBytes, NULL)) {
        std::cerr << "[+] Failed: Cannot read file contents..." << std::endl;
        int32_t error = GetLastError();
        std::cout << "[+] Error: " << error << std::endl;
        delete[] fileContent;
        return 0;
    }

    SFileFindClose(hFile);

    fileContent[*fileSize + 1] = '\0';
    return fileContent;
}

void PrintMpqInfo(HANDLE hArchive) {
    std::string fileName = GetMpqFileName(hArchive);
    std::cout << "[+] File name: " << fileName << std::endl;

    int32_t archiveSize = GetMpqArchiveSize(hArchive);
    std::cout << "[+] Archive size: " << archiveSize << std::endl;

    int64_t headerOffset = GetMpqArchiveHeaderOffset(hArchive);
    std::cout << "[+] Header offset: " << headerOffset << std::endl;

    int64_t headerSize = GetMpqArchiveHeaderSize(hArchive);
    std::cout << "[+] Header size: " << headerSize << std::endl;

    int32_t numberOfFiles = GetMpqArchiveFileCount(hArchive);
    std::cout << "[+] File count: " << numberOfFiles << std::endl;

    int32_t signatureType = GetMpqArchiveSignatureType(hArchive);
    std::cout << "[+] Signature type: " << signatureType << std::endl;
}

std::string GetMpqFileName(HANDLE hArchive) {
    char fileName[4096];  // Max path length in Linux, Windows is 256
    if (!SFileGetFileInfo(hArchive, SFileMpqFileName, &fileName, sizeof(fileName), NULL)) {
        std::cerr << "[+] Failed: SFileMpqFileName" << std::endl;
        return "";
    }
    return fileName;
}

int32_t GetMpqArchiveSize(HANDLE hArchive) {
    int32_t archiveSize;
    if (!SFileGetFileInfo(hArchive, SFileMpqArchiveSize, &archiveSize, sizeof(archiveSize), NULL)) {
        std::cerr << "[+] Failed: SFileMpqArchiveSize" << std::endl;
        return -1;
    }
    return archiveSize;
}

int64_t GetMpqArchiveHeaderOffset(HANDLE hArchive) {
    int64_t headerOffset;
    if (!SFileGetFileInfo(hArchive, SFileMpqHeaderOffset, &headerOffset, sizeof(headerOffset), NULL)) {
        std::cerr << "[+] Failed: SFileMpqHeaderOffset" << std::endl;
        return -1;
    }
    return headerOffset;
}

int64_t GetMpqArchiveHeaderSize(HANDLE hArchive) {
    int64_t headerSize;
    if (!SFileGetFileInfo(hArchive, SFileMpqHeaderSize, &headerSize, sizeof(headerSize), NULL)) {
        std::cerr << "[+] Failed: SFileMpqHeaderSize" << std::endl;
        return -1;
    }
    return headerSize;
}

int32_t GetMpqArchiveSignatureType(HANDLE hArchive) {
    int32_t signatureType;
    if (!SFileGetFileInfo(hArchive, SFileMpqSignatures, &signatureType, sizeof(signatureType), NULL)) {
        std::cerr << "[+] Failed: SFileMpqSignatures" << std::endl;
        return -1;
    }
    return signatureType;
}

int32_t GetMpqArchiveFileCount(HANDLE hArchive) {
    int32_t numberOfFiles;
    if (!SFileGetFileInfo(hArchive, SFileMpqNumberOfFiles, &numberOfFiles, sizeof(numberOfFiles), NULL)) {
        std::cerr << "[+] Failed: SFileMpqNumberOfFiles" << std::endl;
        return -1;
    }
    return numberOfFiles;
}