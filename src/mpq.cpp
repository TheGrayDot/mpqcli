#include <iostream>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <map>
#include <functional>

#include <StormLib.h>

#include "mpq.h"
#include "helpers.h"

namespace fs = std::filesystem;

int OpenMpqArchive(const std::string &filename, HANDLE *hArchive) {
    if (!SFileOpenArchive(filename.c_str(), 0, MPQ_OPEN_READ_ONLY, hArchive)) {
        std::cerr << "[+] Failed to open: " << filename << std::endl;
        return 0;
    }
    return 1;
}

int CloseMpqArchive(HANDLE hArchive) {
    if (!SFileCloseArchive(hArchive)) {
        std::cerr << "[+] Failed to close MPQ archive." << std::endl;
        return 0;
    }
    return 1;
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

void PrintMpqInfo(HANDLE hArchive, const std::string& infoProperty) {
    // Map of property names to their corresponding actions
    std::map<std::string, std::function<void(bool)>> propertyActions = {
        {"format-version", [&](bool printName) {
            TMPQHeader header = GetMpqArchiveInfo<TMPQHeader>(hArchive, SFileMpqHeader);
            uint16_t formatVersion = header.wFormatVersion + 1;  // Add +1 because StormLib starts at 0
            if (printName) {
                std::cout << "Format version: ";
            }
            std::cout << formatVersion << std::endl;
        }},
        {"header-offset", [&](bool printName) {
            int64_t headerOffset = GetMpqArchiveInfo<int64_t>(hArchive, SFileMpqHeaderOffset);
            if (printName) {
                std::cout << "Header offset: ";
            }
            std::cout << headerOffset << std::endl;
        }},
        {"header-size", [&](bool printName) {
            int64_t headerSize = GetMpqArchiveInfo<int64_t>(hArchive, SFileMpqHeaderSize);
            if (printName) {
                std::cout << "Header size: ";
            }
            std::cout << headerSize << std::endl;
        }},
        {"archive-size", [&](bool printName) {
            int32_t archiveSize = GetMpqArchiveInfo<int32_t>(hArchive, SFileMpqArchiveSize);
            if (printName) {
                std::cout << "Archive size: ";
            }
            std::cout << archiveSize << std::endl;
        }},
        {"file-count", [&](bool printName) {
            int32_t numberOfFiles = GetMpqArchiveInfo<int32_t>(hArchive, SFileMpqNumberOfFiles);
            if (printName) {
                std::cout << "File count: ";
            }
            std::cout << numberOfFiles << std::endl;
        }},
        {"signature-type", [&](bool printName) {
            int32_t signatureType = GetMpqArchiveInfo<int32_t>(hArchive, SFileMpqSignatures);
            if (printName) {
                std::cout << "Signature type: ";
            }
            if (signatureType == SIGNATURE_TYPE_NONE) {
                std::cout << "None" << std::endl;
            } else if (signatureType == SIGNATURE_TYPE_WEAK) {
                std::cout << "Weak" << std::endl;
            } else if (signatureType == SIGNATURE_TYPE_STRONG) {
                std::cout << "Strong" << std::endl;
            }
        }}
    };

    // If infoProperty is empty, print all properties with their names (key)
    // Otherwise, print only the property value
    if (infoProperty.empty()) {
        for (const auto& [key, action] : propertyActions) {
            action(true);  // Print property name and value
        }
    } else {
        auto it = propertyActions.find(infoProperty);
        if (it != propertyActions.end()) {
            it->second(false);  // Print only the value
        }
    }
}

template <typename T>
T GetMpqArchiveInfo(HANDLE hArchive, SFileInfoClass infoClass) {
    T value{};
    if (!SFileGetFileInfo(hArchive, infoClass, &value, sizeof(T), NULL)) {
        int32_t error = GetLastError();
        std::cerr << "[+] Failed to retrieve info (Error: " << error << ")" << std::endl;
        return T{}; // Return default value for the type
    }
    return value;
}
