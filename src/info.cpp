#include <iostream>
#include <fstream>

#include <StormLib.h>

#include "info.h"

int GetInfo(HANDLE hArchive) {
    TCHAR fileName[1000];
    // int32_t fileSize;
    int32_t archiveSize;
    int64_t headerOffset;
    int32_t headerSize;
    int32_t numberOfFiles;
    int32_t signatureType;

    if (!SFileGetFileInfo(hArchive, SFileMpqFileName, &fileName, sizeof(fileName), NULL)) {
        std::cerr << "[+] Failed: GetInfo.SFileMpqFileName" << std::endl;
        return -1;
    }

    // SFileInfoFileSize fails, so implemeted temp fix
    // if (!SFileGetFileInfo(hArchive, SFileInfoFileSize, &fileSize, sizeof(fileSize), NULL)) {
    //     std::cerr << "[+] Failed to get: SFileInfoFileSize" << std::endl;
    //     return -1;
    // }
    std::ifstream file(fileName, std::ios::binary | std::ios::ate);
    std::streamsize fileSize = file.tellg();
    file.close();

    if (!SFileGetFileInfo(hArchive, SFileMpqNumberOfFiles, &numberOfFiles, sizeof(numberOfFiles), NULL)) {
        std::cerr << "[+] Failed to get: SFileMpqNumberOfFiles" << std::endl;
        return -1;
    }

    if (!SFileGetFileInfo(hArchive, SFileMpqArchiveSize, &archiveSize, sizeof(archiveSize), NULL)) {
        std::cerr << "[+] Failed to get: SFileMpqArchiveSize" << std::endl;
        return -1;
    }

    if (!SFileGetFileInfo(hArchive, SFileMpqHeaderOffset, &headerOffset, sizeof(headerOffset), NULL)) {
        std::cerr << "[+] Failed to get: SFileMpqHeaderOffset" << std::endl;
        return -1;
    }

    if (!SFileGetFileInfo(hArchive, SFileMpqHeaderSize, &headerSize, sizeof(headerSize), NULL)) {
        std::cerr << "[+] Failed to get: SFileMpqHeaderSize" << std::endl;
        return -1;
    }

    if (!SFileGetFileInfo(hArchive, SFileMpqSignatures, &signatureType, sizeof(signatureType), NULL)) {
        std::cerr << "[+] Failed: SFileMpqSignatures" << std::endl;
        return -1;
    }

    std::cout << "[+] File name: " << fileName << std::endl;
    std::cout << "[+] File size: " << fileSize << std::endl;
    std::cout << "[+] Archive size: " << archiveSize << std::endl;
    std::cout << "[+] Header offset: " << headerOffset << std::endl;
    std::cout << "[+] Header size: " << headerSize << std::endl;
    std::cout << "[+] File count: " << numberOfFiles << std::endl;
    std::cout << "[+] Signature type: " << signatureType << std::endl;

    return 1;
}
