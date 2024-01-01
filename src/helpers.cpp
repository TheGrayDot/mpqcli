#include <iostream>
#include <fstream>
#include <vector>

#include <StormLib.h>
#include <ghc/filesystem.hpp>

#include "helpers.h"
#include "mpq.h"

int ExtractMpqAndBinFromExe(HANDLE hArchive, bool extractMpq, bool extractBin) {
    std::string fileName = GetMpqFileName(hArchive);
    std::cout << "[+] Archive name: " << fileName << std::endl;

    int64_t archiveOffset = GetMpqArchiveHeaderOffset(hArchive);
    std::cout << "[+] Archive offset: " << archiveOffset << std::endl;
    
    int32_t archiveSize = GetMpqArchiveSize(hArchive);
    std::cout << "[+] Archive size: " << archiveSize << std::endl;

    // Check the segment is aligned to 512 bytes
    // MPQ archives embedded in an exe must be sector (512) aligned
    if (archiveOffset % 512 != 0) {
        std::cerr << "[+] Error: Archive offset not aligned to 512 bytes" << std::endl;
        return 1;
    }

    const ghc::filesystem::path outputPath = ghc::filesystem::canonical(fileName);
    std::cout << "[+] Output path: " << outputPath << std::endl;

    if (extractBin) {
        std::string outputBinFile = outputPath.parent_path() / outputPath.stem();
        outputBinFile = outputBinFile + ".bin";
        std::ifstream file_bin(fileName, std::ios::binary);
        std::ofstream output_bin(outputBinFile, std::ios::binary);
        file_bin.seekg(0, std::ios::beg);
        std::vector<char> buffer_bin(archiveOffset);
        file_bin.read(buffer_bin.data(), buffer_bin.size());
        output_bin.write(buffer_bin.data(), buffer_bin.size());
    }
 
    if (extractMpq) {
        std::string outputMpqFile = outputPath.parent_path() / outputPath.stem();
        outputMpqFile = outputMpqFile + ".mpq";
        std::ifstream file_mpq(fileName, std::ios::binary);
        std::ofstream output_mpq(outputMpqFile, std::ios::binary);
        file_mpq.seekg(archiveOffset, std::ios::beg);
        std::vector<char> buffer(archiveSize);
        file_mpq.read(buffer.data(), buffer.size());
        output_mpq.write(buffer.data(), buffer.size());
    }

    return 0;
}
