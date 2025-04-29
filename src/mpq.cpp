#include <iostream>
#include <filesystem>
#include <fstream>
#include <algorithm>

#include <StormLib.h>

#include "mpq.h"
#include "helpers.h"

namespace fs = std::filesystem;

int OpenMpqArchive(const std::string &filename, HANDLE *hArchive) {
    if (!SFileOpenArchive(filename.c_str(), 0, MPQ_OPEN_READ_ONLY, hArchive)) {
        std::cerr << "[+] Failed to open: " << filename << std::endl;
        return -1;
    }
    return 1;
}

int CloseMpqArchive(HANDLE hArchive) {
    if (!SFileCloseArchive(hArchive)) {
        std::cerr << "[+] Failed to close MPQ archive." << std::endl;
        return -1;
    }
    return 0;
}

int ExtractFiles(HANDLE hArchive, const std::string& output, const std::string& listfileName) {
    SFILE_FIND_DATA findData;
    const char *listfile = listfileName.empty() ? NULL : listfileName.c_str();
    HANDLE findHandle = SFileFindFirstFile(hArchive, "*", &findData, listfile);
    if (findHandle == NULL) {
        std::cerr << "[+] Failed to find first file in MPQ archive." << std::endl;
        SFileCloseArchive(hArchive);
        return -1;
    }

    do {
        int32_t result = ExtractFile(
            hArchive,
            output,
            findData.cFileName,
            true  // Keep folder structure
        );
        if (result != 0) {
            return result;
        }
    } while (SFileFindNextFile(
        findHandle,
        &findData));

    return 0;
}

int ExtractFile(HANDLE hArchive, const std::string& output, const std::string& fileName, bool keepFolderStructure) {
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
    std::string fileNameString = NormalizeFilePath(fileNamePath);

    // Remove folder structure if keepFolderStructure is false
    if (!keepFolderStructure) {
        fileNamePath = fs::path(fileNameString);
        fileNameString = fileNamePath.filename().u8string();
    }

    // Create output directory
    fs::path outputPathAbsolute = fs::canonical(output);
    fs::path outputPathBase = outputPathAbsolute.parent_path() / outputPathAbsolute.filename();
    std::filesystem::create_directories(fs::path(outputPathBase).parent_path());

    // Ensure sub-directories for folder-nested files exist
    fs::path outputFilePathName = outputPathBase / fileNameString;
    std::string outputFileName{outputFilePathName.u8string()};
    std::filesystem::create_directories(outputFilePathName.parent_path());

    if (SFileExtractFile(hArchive, szFileName, outputFileName.c_str(), 0)) {
        std::cout << "[+] Extracted: " << fileNameString << std::endl;
    } else {
        int32_t error = GetLastError();
        std::cerr << "[+] Failed: " << "(" << error << ") " << szFileName << std::endl;
        return error;
    }

    return 0;
}

HANDLE CreateMpqArchive(std::string outputArchiveName, int32_t fileCount, int32_t mpqVersion) {
    // Determine MPQ archive version we are creating
    int32_t targetMpqVersion = MPQ_CREATE_ARCHIVE_V1;
    if (mpqVersion == 1) {
        targetMpqVersion = MPQ_CREATE_ARCHIVE_V1;
    } else if (mpqVersion == 2) {
        targetMpqVersion = MPQ_CREATE_ARCHIVE_V2;
    } else {
        std::cerr << "[+] Invalid MPQ version specified. Exiting..." << std::endl;
        return NULL;
    };

    // Check if file already exists
    if (fs::exists(outputArchiveName)) {
        std::cerr << "[+] File already exists: " << outputArchiveName << " Exiting..." << std::endl;
        return NULL;
    }

    HANDLE hMpq;
    bool result = SFileCreateArchive(
        outputArchiveName.c_str(),
        targetMpqVersion,
        fileCount,
        &hMpq
    );

    if (!result) {
        std::cerr << "[+] Failed to create MPQ archive." << std::endl;
        int32_t error = GetLastError();
        std::cout << "[+] Error: " << error << std::endl;
        return NULL;
    }

    return hMpq;
}

int AddFiles(HANDLE hArvhive, const std::string& inputPath) {
    for (const auto &entry : fs::recursive_directory_iterator(inputPath)) {
        if (fs::is_regular_file(entry.path())) {
            AddFile(hArvhive, entry.path().u8string());
        }
    }
    return 0;
}

int AddFile(HANDLE hArchive, const std::string& inputFile) {
    // Return if file doesn't exist on disk
    if (!fs::exists(inputFile)) {
        std::cerr << "[!] File doesn't exist on disk: " << inputFile << std::endl;
        return -1;
    }

    // Normalise path for MPQ
    fs::path inputFilePath = fs::relative(inputFile);
    std::string archiveFileName = WindowsifyFilePath(inputFilePath.u8string());
    std::cout << "[+] Adding file: " << archiveFileName << std::endl;

    // Check if file exists in MPQ archive
    bool hasFile = SFileHasFile(hArchive, archiveFileName.c_str());
    if (hasFile) {
        std::cerr << "[!] File already exists in MPQ archive: " << archiveFileName << " Skipping..." << std::endl;
        return -1;
    }

    // Set file attributes in MPQ archive (compression and encryption)
    DWORD dwFlags = MPQ_FILE_COMPRESS | MPQ_FILE_ENCRYPTED;
    DWORD dwCompression = MPQ_COMPRESSION_ZLIB;

    bool addedFile = SFileAddFileEx(
        hArchive,
        inputFile.c_str(),
        archiveFileName.c_str(),
        dwFlags,
        dwCompression,
        MPQ_COMPRESSION_NEXT_SAME
    );

    if (!addedFile) {
        int32_t error = GetLastError();
        std::cerr << "[!] Error: " << error << " Failed to add: " << archiveFileName << std::endl;
        return -1;
    }

    return 0;
}

int RemoveFile(HANDLE hArchive, const std::string& fileName) {
    if (!SFileHasFile(hArchive, fileName.c_str())) {
        std::cerr << "[+] Failed: File doesn't exist" << std::endl;
        return -1;
    }

    if (!SFileRemoveFile(hArchive, fileName.c_str(), 0)) {
        std::cerr << "[+] Failed: File cannot be removed" << std::endl;
        return -1;
    }

    return 0;
}

int ListFiles(HANDLE hArchive, const std::string& listfileName) {
    // Find the first file in MPQ archive to iterate from
    SFILE_FIND_DATA findData;
    const char *listfile = listfileName.empty() ? NULL : listfileName.c_str();
    HANDLE findHandle = SFileFindFirstFile(hArchive, "*", &findData, listfile);
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
    uint16_t formatVersion = GetMpqFormatVersion(hArchive);
    std::cout << "Format version: " << formatVersion << std::endl;

    int32_t archiveSize = GetMpqArchiveInfo<int32_t>(hArchive, SFileMpqArchiveSize);
    std::cout << "Archive size: " << archiveSize << std::endl;

    int64_t headerOffset = GetMpqArchiveInfo<int64_t>(hArchive, SFileMpqHeaderOffset);
    std::cout << "Header offset: " << headerOffset << std::endl;

    int64_t headerSize = GetMpqArchiveInfo<int64_t>(hArchive, SFileMpqHeaderSize);
    std::cout << "Header size: " << headerSize << std::endl;

    int32_t numberOfFiles = GetMpqArchiveInfo<int32_t>(hArchive, SFileMpqNumberOfFiles);
    std::cout << "File count: " << numberOfFiles << std::endl;

    int32_t signatureType = GetMpqArchiveInfo<int32_t>(hArchive, SFileMpqSignatures);
    std::cout << "Signature type: " << signatureType << std::endl;
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

uint16_t GetMpqFormatVersion(HANDLE hArchive) {
    TMPQHeader header;
    if (!SFileGetFileInfo(hArchive, SFileMpqHeader, &header, sizeof(header), NULL)) {
        std::cerr << "[+] Failed: SFileMpqHeader" << std::endl;
        int32_t error = GetLastError();
        std::cerr << "[+] Failed: " << "(" << error << ") " << std::endl;
        return 0;
    }
    // Add +1 because StormLib starts at 0
    return header.wFormatVersion + 1;
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
        signatureContent = GetMpqArchiveInfo<std::vector<char>>(hArchive, SFileMpqStrongSignature);

        if (signatureContent.empty()) {
            std::string archiveName = GetMpqFileName(hArchive);
            const fs::path archivePath = fs::canonical(archiveName);
            int32_t archiveSize = GetMpqArchiveInfo<int32_t>(hArchive, SFileMpqArchiveSize); 
            int64_t archiveOffset = GetMpqArchiveInfo<int64_t>(hArchive, SFileMpqHeaderOffset);
            std::cout << "[+] Archive offset: " << archiveOffset << std::endl;

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
