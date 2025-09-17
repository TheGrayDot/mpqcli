#include <iostream>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <map>
#include <functional>
#include <vector>

#include <StormLib.h>

#include "mpq.h"
#include "helpers.h"

namespace fs = std::filesystem;

int OpenMpqArchive(const std::string &filename, HANDLE *hArchive, int32_t flags) {
    if (!SFileOpenArchive(filename.c_str(), 0, flags, hArchive)) {
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

int SignMpqArchive(HANDLE hArchive) {
    if (!SFileSignArchive(hArchive, SIGNATURE_TYPE_WEAK)) {
        std::cerr << "[+] Failed to sign MPQ archive." << std::endl;
        return 0;
    }
    return 1;
}

int ExtractFiles(HANDLE hArchive, const std::string& output, const std::string& listfileName) {
    // Check if the user provided a listfile input
    const char *listfile = (listfileName == "default") ? NULL : listfileName.c_str();

    SFILE_FIND_DATA findData;
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
        int32_t error = SErrGetLastError();
        std::cerr << "[+] Failed: " << "(" << error << ") " << szFileName << std::endl;
        return error;
    }

    return 0;
}

HANDLE CreateMpqArchive(std::string outputArchiveName, int32_t fileCount, int32_t mpqVersion) {
    // Check if file already exists
    if (fs::exists(outputArchiveName)) {
        std::cerr << "[+] File already exists: " << outputArchiveName << " Exiting..." << std::endl;
        return NULL;
    }

    // Configure flags for MPQ file, this includes:
    // If we store attributes such as filetime
    // The MPQ archive version
    int32_t dwCreateFlags = 0;

    if (mpqVersion == 1) {
        dwCreateFlags += MPQ_CREATE_ARCHIVE_V1;
    } else if (mpqVersion == 2) {
        dwCreateFlags += MPQ_CREATE_ARCHIVE_V2;
    } else {
        dwCreateFlags += MPQ_CREATE_ARCHIVE_V1;
    };

    // Always include attributes
    // This is needed for filetime, locale, CRC32 and MD5
    dwCreateFlags += MPQ_CREATE_ATTRIBUTES;

    HANDLE hMpq;
    bool result = SFileCreateArchive(
        outputArchiveName.c_str(),
        dwCreateFlags,
        fileCount,
        &hMpq
    );

    if (!result) {
        std::cerr << "[+] Failed to create MPQ archive." << std::endl;
        int32_t error = SErrGetLastError();
        std::cout << "[+] Error: " << error << std::endl;
        return NULL;
    }

    return hMpq;
}

int AddFiles(HANDLE hArchive, const std::string& target) {
    // We need to "clean" the target path to ensure it is a valid directory
    // and to strip any directory structure from the files we add
    fs::path targetPath = fs::path(target);

    for (const auto &entry : fs::recursive_directory_iterator(target)) {
        if (fs::is_regular_file(entry.path())) {
            // Strip the target path from the file name
            fs::path inputFilePath = fs::relative(entry, targetPath);

            // Normalise path for MPQ
            std::string archiveFilePath = WindowsifyFilePath(inputFilePath.u8string());

            AddFile(hArchive, entry.path().u8string(), archiveFilePath);
        }
    }
    return 0;
}

int AddFile(HANDLE hArchive, fs::path localFile, const std::string& archiveFilePath) {
    std::cout << "[+] Adding file: " << archiveFilePath << std::endl;

    // Return if file doesn't exist on disk
    if (!fs::exists(localFile)) {
        std::cerr << "[!] File doesn't exist on disk: " << localFile << std::endl;
        return -1;
    }

    // Check if file exists in MPQ archive
    bool hasFile = SFileHasFile(hArchive, archiveFilePath.c_str());
    if (hasFile) {
        std::cerr << "[!] File already exists in MPQ archive: " << archiveFilePath << " Skipping..." << std::endl;
        return -1;
    }

    // Verify that we are not exceeding maxFile size of the archive, and if we do, increase it
    int32_t numberOfFiles = GetFileInfo<int32_t>(hArchive, SFileMpqNumberOfFiles);
    int32_t maxFiles = GetFileInfo<int32_t>(hArchive, SFileMpqMaxFileCount);

    if (numberOfFiles + 1 > maxFiles)
    {
        int32_t newMaxFiles = NextPowerOfTwo(numberOfFiles + 1);
        bool setMaxFileCount = SFileSetMaxFileCount(hArchive, newMaxFiles);
        if (!setMaxFileCount)
        {
            int32_t error = SErrGetLastError();
            std::cerr << "[!] Error: " << error << " Failed to increase new max file count to: " << newMaxFiles << std::endl;
            return -1;
        }
    }

    // Set file attributes in MPQ archive (compression and encryption)
    DWORD dwFlags = MPQ_FILE_COMPRESS | MPQ_FILE_ENCRYPTED;
    DWORD dwCompression = MPQ_COMPRESSION_ZLIB; 

    bool addedFile = SFileAddFileEx(
        hArchive,
        localFile.u8string().c_str(),
        archiveFilePath.c_str(),
        dwFlags,
        dwCompression,
        MPQ_COMPRESSION_NEXT_SAME
    );

    if (!addedFile) {
        int32_t error = SErrGetLastError();
        std::cerr << "[!] Error: " << error << " Failed to add: " << archiveFilePath << std::endl;
        return -1;
    }

    return 0;
}

int RemoveFile(HANDLE hArchive, const std::string& archiveFilePath) {
    std::cout << "[+] Removing file: " << archiveFilePath << std::endl;

    if (!SFileHasFile(hArchive, archiveFilePath.c_str())) {
        std::cerr << "[+] Failed: File doesn't exist" << std::endl;
        return -1;
    }

    if (!SFileRemoveFile(hArchive, archiveFilePath.c_str(), 0)) {
        std::cerr << "[+] Failed: File cannot be removed" << std::endl;
        return -1;
    }

    return 0;
}

int ListFiles(HANDLE hArchive, const std::string& listfileName, bool listAll, bool listDetailed) {
    // Check if the user provided a listfile input
    const char *listfile = (listfileName == "default") ? NULL : listfileName.c_str();

    SFILE_FIND_DATA findData;
    HANDLE findHandle = SFileFindFirstFile(hArchive, "*", &findData, listfile);
    if (findHandle == NULL) {
        std::cerr << "[+] Failed to find first file in MPQ archive." << std::endl;
        SFileCloseArchive(hArchive);
        return -1;
    }

    // "Special" files are base files used by MPQ file format
    // These are skipped, unless "-a" or "--all" are specified
    std::vector<std::string> specialFiles = {
        "(listfile)",
        "(signature)",
        "(attributes)"
    };

    // Loop through all files in MPQ archive
    do {
        // Skip special files unless user wants to list all (like ls -a)
        if (!listAll && std::find(specialFiles.begin(), specialFiles.end(), findData.cFileName) != specialFiles.end()) {
            continue;
        }

        // Print the detailed (long) file listing (like ls -l)
        if (listDetailed) {
            // We need to open the file to get detailed information
            // Use our custom GetFileInfo function
            HANDLE hFile;
            if (!SFileOpenFileEx(hArchive, findData.cFileName, SFILE_OPEN_FROM_MPQ, &hFile)) {
                std::cerr << "[+] Failed to open file: " << findData.cFileName << std::endl;
                continue; // Skip to the next file
            }

            int32_t fileSize = GetFileInfo<int32_t>(hFile, SFileInfoFileSize);
            int32_t fileLocale = GetFileInfo<int32_t>(hFile, SFileInfoLocale);
            std::string fileLocaleStr = LocaleToLang(fileLocale);
            int64_t fileTime = GetFileInfo<int64_t>(hFile, SFileInfoFileTime);
            std::string fileTimeStr = FileTimeToLsTime(fileTime);

            // Print the file details in a formatted way
            std::cout << std::setw(11) << fileSize << " "  // 4GB max size is 10 characters
                      << std::setw(5) << fileLocaleStr << " "  // Locale is max 4 characters
                      << std::setw(18) << fileTimeStr << " "  // File time is formatted as "MMM DD YYYY HH:MM"
                      << findData.cFileName << std::endl;

            SFileCloseFile(hFile);
        } else {
            // Print just the filename (like default ls command output)
            std::cout << findData.cFileName << std::endl;
        }

    } while (SFileFindNextFile(findHandle, &findData));

    return 0;
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
    if (*fileSize == SFILE_INVALID_SIZE) {
        std::cerr << "[+] Failed: Invalid file size for: " << szFileName << std::endl;
        SFileCloseFile(hFile);
        return NULL;
    }

    char* fileContent = new char[*fileSize];
    DWORD dwBytesRead;
    if (!SFileReadFile(hFile, fileContent, *fileSize, &dwBytesRead, NULL)) {
        std::cerr << "[+] Failed: Cannot read file contents for: " << szFileName << std::endl;
        delete[] fileContent;
        SFileCloseFile(hFile);
        return NULL;
    }

    SFileCloseFile(hFile);
    return fileContent;
}

void PrintMpqInfo(HANDLE hArchive, const std::string& infoProperty) {
    // Map of property names to their corresponding actions
    std::map<std::string, std::function<void(bool)>> propertyActions = {
        {"format-version", [&](bool printName) {
            TMPQHeader header = GetFileInfo<TMPQHeader>(hArchive, SFileMpqHeader);
            uint16_t formatVersion = header.wFormatVersion + 1;  // Add +1 because StormLib starts at 0
            if (printName) {
                std::cout << "Format version: ";
            }
            std::cout << formatVersion << std::endl;
        }},
        {"header-offset", [&](bool printName) {
            int64_t headerOffset = GetFileInfo<int64_t>(hArchive, SFileMpqHeaderOffset);
            if (printName) {
                std::cout << "Header offset: ";
            }
            std::cout << headerOffset << std::endl;
        }},
        {"header-size", [&](bool printName) {
            int64_t headerSize = GetFileInfo<int64_t>(hArchive, SFileMpqHeaderSize);
            if (printName) {
                std::cout << "Header size: ";
            }
            std::cout << headerSize << std::endl;
        }},
        {"archive-size", [&](bool printName) {
            int32_t archiveSize = GetFileInfo<int32_t>(hArchive, SFileMpqArchiveSize);
            if (printName) {
                std::cout << "Archive size: ";
            }
            std::cout << archiveSize << std::endl;
        }},
        {"file-count", [&](bool printName) {
            int32_t numberOfFiles = GetFileInfo<int32_t>(hArchive, SFileMpqNumberOfFiles);
            if (printName) {
                std::cout << "File count: ";
            }
            std::cout << numberOfFiles << std::endl;
        }},
        {"max-files", [&](bool printName) {
            int32_t maxFiles = GetFileInfo<int32_t>(hArchive, SFileMpqMaxFileCount);
            if (printName) {
                std::cout << "Max files: ";
            }
            std::cout << maxFiles << std::endl;
        }},
        {"signature-type", [&](bool printName) {
            int32_t signatureType = GetFileInfo<int32_t>(hArchive, SFileMpqSignatures);
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

    // If infoProperty is "default", print all properties with their names (key)
    // Otherwise, print only the property value
    if (infoProperty == "default") {
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
T GetFileInfo(HANDLE hFile, SFileInfoClass infoClass) {
    T value{};
    if (!SFileGetFileInfo(hFile, infoClass, &value, sizeof(T), NULL)) {
        int32_t error = SErrGetLastError();
        // std::cerr << "[+] GetFileInfo failed (Error: " << error << ")" << std::endl;
        return T{}; // Return default value for the type
    }
    return value;
}

int32_t PrintMpqSignature(HANDLE hArchive, std::string target) {
    // Determine if we have a strong or weak digital signature
    int32_t signatureType =
        GetFileInfo<int32_t>(hArchive, SFileMpqSignatures);

    std::vector<char> signatureContent;

    if (signatureType == SIGNATURE_TYPE_NONE) {
        return 1;
    } else if (signatureType == SIGNATURE_TYPE_WEAK) {
        const char* szFileName = "(signature)";
        uint32_t fileSize;
        char* fileContent = ReadFile(hArchive, szFileName, &fileSize);

        if (fileContent == NULL) {
            std::cerr << "[+] Failed to read weak signature file." << std::endl;
            return -1;
        }
        signatureContent.resize(fileSize);
        std::copy(fileContent, fileContent + fileSize,
                  signatureContent.begin());

        PrintAsBinary(fileContent, fileSize);
        delete[] fileContent;

    } else if (signatureType == SIGNATURE_TYPE_STRONG) {
        signatureContent = GetFileInfo<std::vector<char>>(
            hArchive, SFileMpqStrongSignature);
        if (signatureContent.empty()) {
            int64_t archiveSize =
                GetFileInfo<int64_t>(hArchive, SFileMpqArchiveSize64);
            int64_t archiveOffset =
                GetFileInfo<int64_t>(hArchive, SFileMpqHeaderOffset);

            const fs::path archivePath = fs::canonical(target);
            std::uintmax_t fileSize = fs::file_size(archivePath);
            int64_t signatureLength = fileSize - archiveOffset - archiveSize;

            std::ifstream file_mpq(archivePath, std::ios::binary);
            file_mpq.seekg(archiveOffset + archiveSize, std::ios::beg);
            signatureContent.resize(signatureLength);
            file_mpq.read(signatureContent.data(), signatureContent.size());
            file_mpq.close();

            char* fileContent = new char[signatureContent.size()];
            std::copy(signatureContent.begin(), signatureContent.end(),
                      fileContent);

            PrintAsBinary(fileContent, static_cast<uint32_t>(signatureContent.size()));
            delete[] fileContent;
        }
    }

    return 0;
}
