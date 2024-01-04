#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>

#include <StormLib.h>

#include "helpers.h"
#include "mpq.h"

namespace fs = std::filesystem;

int ExtractMpqAndBinFromExe(HANDLE hArchive, bool extractBin) {
    // Fetch details from MPQ archive metadata
    std::string archiveName = GetMpqFileName(hArchive);
    const fs::path archivePath = fs::canonical(archiveName);
    std::cout << "[+] Archive name: " << archiveName << std::endl;
    int64_t archiveOffset = GetMpqArchiveHeaderOffset(hArchive);
    std::cout << "[+] Archive offset: " << archiveOffset << std::endl;
    int32_t archiveSize = GetMpqArchiveSize(hArchive);
    std::cout << "[+] Archive size: " << archiveSize << std::endl;

    // Determine if the file is padded (at end of file)
    // MPQ files with padding at end of file may have strong signature
    std::uintmax_t fileSize = fs::file_size(archivePath);
    int32_t paddingSize = fileSize - archiveOffset - archiveSize;
    int32_t extractSize;
    if (paddingSize > 0) {
        std::cout << "[+] Archive appears to be padded..." << std::endl;
        extractSize = fileSize - archiveOffset;
    } else {
        extractSize = archiveSize;
    }
    std::cout << "[+] Extract size: " << extractSize << std::endl;

    // Check the segment is aligned to 512 bytes
    // MPQ archives embedded in an exe must be sector (512) aligned
    if (archiveOffset % 512 != 0) {
        std::cerr << "[+] Error: Archive offset not aligned to 512 bytes." << std::endl;
        return 1;
    }

    if (extractBin) {
        std::string outputBinFile = archivePath.parent_path() / archivePath.stem();
        outputBinFile = outputBinFile + ".bin";
        std::cout << "[+] Output location: " << outputBinFile << std::endl;
        std::ifstream file_bin(archiveName, std::ios::binary);
        std::ofstream output_bin(outputBinFile, std::ios::binary);
        file_bin.seekg(0, std::ios::beg);
        std::vector<char> buffer_bin(archiveOffset);
        file_bin.read(buffer_bin.data(), buffer_bin.size());
        output_bin.write(buffer_bin.data(), buffer_bin.size());
    }
 
    std::string outputMpqFile = archivePath.parent_path() / archivePath.stem();
    outputMpqFile = outputMpqFile + ".mpq";
    std::cout << "[+] Output location: " << outputMpqFile << std::endl;
    std::ifstream file_mpq(archiveName, std::ios::binary);
    std::ofstream output_mpq(outputMpqFile, std::ios::binary);
    file_mpq.seekg(archiveOffset, std::ios::beg);
    std::vector<char> buffer(extractSize);
    file_mpq.read(buffer.data(), buffer.size());
    output_mpq.write(buffer.data(), buffer.size());

    return 0;
}
