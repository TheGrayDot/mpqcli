#include <iostream>
#include <filesystem>
#include <fstream>
#include <algorithm>

#include <StormLib.h>

#include "mpq.h"

namespace fs = std::filesystem;

int OpenMpqArchive(const std::string &filename, HANDLE *hArchive) {
    if (!SFileOpenArchive(filename.c_str(), 0, MPQ_OPEN_READ_ONLY, hArchive)) {
        std::cerr << "[+] Failed to open: " << filename << std::endl;
        return -1;
    }
    return 1;
}

int ExtractFiles(HANDLE hArchive, const std::string& output) {
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

int ExtractFile(HANDLE hArchive, const std::string& output, const std::string& fileName) {
    const char *szFileName = fileName.c_str();
    if (!SFileHasFile(hArchive, szFileName)) {
        std::cerr << "[+] Failed: File doesn't exist" << std::endl;
        return -1;
    }

    HANDLE hFile;
    if (!SFileOpenFileEx(hArchive, szFileName, SFILE_OPEN_FROM_MPQ, &hFile)) {
        std::cerr << "[+] Failed: File cannot be opened" << std::endl;
        return -1;
    }

    // Change forward slashes on non-Windows systems
    fs::path fileNamePath(fileName);
    std::string fileNameString;
    #ifndef _WIN32
        fileNameString = fileNamePath.string();
        std::replace(fileNameString.begin(), fileNameString.end(), '\\', '/');
    #else
        fileNameString = fileNamePath.string();
    #endif

    fs::path outputPathAbsolute = fs::canonical(output);
    fs::path outputPathBase = outputPathAbsolute.parent_path() / outputPathAbsolute.filename();
    std::filesystem::create_directories(fs::path(outputPathBase).parent_path());

    fs::path outputFilePathName = outputPathBase / szFileName;
    std::string outputFileName{outputFilePathName.u8string()};

    if (SFileExtractFile(hArchive, szFileName, outputFileName.c_str(), 0)) {
        std::cout << "[+] Extracted: " << szFileName << std::endl;
    } else {
        int32_t error = GetLastError();
        std::cerr << "[+] Failed: " << "(" << error << ") " << szFileName << std::endl;
        return error;
    }
 
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

char* ReadFile(HANDLE hArchive, const char *szFileName, unsigned int *fileSize) {
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
    DWORD dwBytes;
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
        int32_t error = GetLastError();
        std::cerr << "[+] Failed: " << "(" << error << ") " << std::endl;
        return "";
    }
    return fileName;
}

TMPQHeader GetMpqHeader(HANDLE hArchive) {
    TMPQHeader header;
    if (!SFileGetFileInfo(hArchive, SFileMpqHeader, &header, sizeof(header), NULL)) {
        std::cerr << "[+] Failed: SFileMpqHeader" << std::endl;
        int32_t error = GetLastError();
        std::cerr << "[+] Failed: " << "(" << error << ") " << std::endl;
        return header;
    }

    unsigned short formatVersion = header.wFormatVersion;
    std::cout << "[+] Format version: " << formatVersion << std::endl;
    return header;
}

int32_t GetMpqArchiveSize(HANDLE hArchive) {
    int32_t archiveSize;
    if (!SFileGetFileInfo(hArchive, SFileMpqArchiveSize, &archiveSize, sizeof(archiveSize), NULL)) {
        std::cerr << "[+] Failed: SFileMpqArchiveSize" << std::endl;
        int32_t error = GetLastError();
        std::cerr << "[+] Failed: " << "(" << error << ") " << std::endl;
        return error;
    }
    return archiveSize;
}

int64_t GetMpqArchiveHeaderOffset(HANDLE hArchive) {
    int64_t headerOffset;
    if (!SFileGetFileInfo(hArchive, SFileMpqHeaderOffset, &headerOffset, sizeof(headerOffset), NULL)) {
        std::cerr << "[+] Failed: SFileMpqHeaderOffset" << std::endl;
        int32_t error = GetLastError();
        std::cerr << "[+] Failed: " << "(" << error << ") " << std::endl;
        return error;
    }
    return headerOffset;
}

int64_t GetMpqArchiveHeaderSize(HANDLE hArchive) {
    int64_t headerSize;
    if (!SFileGetFileInfo(hArchive, SFileMpqHeaderSize, &headerSize, sizeof(headerSize), NULL)) {
        std::cerr << "[+] Failed: SFileMpqHeaderSize" << std::endl;
        int32_t error = GetLastError();
        std::cerr << "[+] Failed: " << "(" << error << ") " << std::endl;
        return error;
    }
    return headerSize;
}

int32_t GetMpqArchiveSignatureType(HANDLE hArchive) {
    int32_t signatureType;
    if (!SFileGetFileInfo(hArchive, SFileMpqSignatures, &signatureType, sizeof(signatureType), NULL)) {
        std::cerr << "[+] Failed: SFileMpqSignatures" << std::endl;
        int32_t error = GetLastError();
        std::cerr << "[+] Failed: " << "(" << error << ") " << std::endl;
        return error;
    }
    return signatureType;
}

std::vector<char> GetMpqArchiveStrongSignature(HANDLE hArchive) {
    std::vector<char> strongSignature;
    if (!SFileGetFileInfo(hArchive, SFileMpqStrongSignature, &strongSignature, sizeof(strongSignature), NULL)) {
        std::cerr << "[+] Failed: SFileMpqStrongSignature" << std::endl;
        int32_t error = GetLastError();
        std::cerr << "[+] Failed: " << "(" << error << ") " << std::endl;
        return strongSignature;
    }
    return strongSignature;
}

int32_t GetMpqArchiveFileCount(HANDLE hArchive) {
    int32_t numberOfFiles;
    if (!SFileGetFileInfo(hArchive, SFileMpqNumberOfFiles, &numberOfFiles, sizeof(numberOfFiles), NULL)) {
        std::cerr << "[+] Failed: SFileMpqNumberOfFiles" << std::endl;
        int32_t error = GetLastError();
        std::cerr << "[+] Failed: " << "(" << error << ") " << std::endl;
        return error;
    }
    return numberOfFiles;
}

int PrintMpqSignature(HANDLE hArchive, int signatureType) {
    std::vector<char> signatureContent;

    // Verify archive using Stormlib function
    int32_t verifiedArchive = SFileVerifyArchive(hArchive);
    std::string verifiyMessage;
    switch (verifiedArchive) {
        case 0:
            verifiyMessage = "ERROR_NO_SIGNATURE";
            break;
        case 1:
            verifiyMessage = "ERROR_VERIFY_FAILED";
            break;
        case 2:
            verifiyMessage = "WEAK_SIGNATURE_OK";
            break;
        case 3:
            verifiyMessage = "WEAK_SIGNATURE_ERROR";
            break;
        case 4:
            verifiyMessage = "STRONG_SIGNATURE_OK";
            break;
        case 5:
            verifiyMessage = "STRONG_SIGNATURE_ERROR";
            break;
        default:
            verifiyMessage = "UNKNOWN_ERROR";
    }

    std::cout << "[+] StormLib verify status: " << verifiyMessage << std::endl;

    // Select signature file location based on signature type
    if (signatureType == 1) {
        std::cout << "[+] Attempting weak signature extraction..." << std::endl;
        const char *szFileName = "(signature)";
        unsigned int fileSize;
        char* fileContent = ReadFile(hArchive, szFileName, &fileSize);
        signatureContent.resize(fileSize);
        std::copy(fileContent, fileContent + fileSize, signatureContent.begin());
    } else {
        std::cout << "[+] Attempting strong signature extraction..." << std::endl;
        signatureContent = GetMpqArchiveStrongSignature(hArchive);

        if (signatureContent.empty()) {
            std::string archiveName = GetMpqFileName(hArchive);
            const fs::path archivePath = fs::canonical(archiveName);
            int32_t archiveSize = GetMpqArchiveSize(hArchive);
            int64_t archiveOffset = GetMpqArchiveHeaderOffset(hArchive);
            std::uintmax_t fileSize = fs::file_size(archivePath);
            int64_t signatureLength = fileSize - archiveOffset - archiveSize;

            std::ifstream file_mpq(archivePath, std::ios::binary);
            file_mpq.seekg(archiveOffset + archiveSize, std::ios::beg);
            signatureContent.resize(signatureLength);
            file_mpq.read(signatureContent.data(), signatureContent.size());
            std::cout << "[+] Buffer size: " << signatureContent.size() << std::endl;
        }
    }

    std::cout << "[+] Signature content: " << std::endl;
    for (char c : signatureContent) {
        std::cout << std::hex << std::setw(2) << "\\x" << std::setfill('0') << (0xff & static_cast<unsigned int>(c));
    }
    std::cout << std::endl;

    return 1;
}
