#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <StormLib.h>

#include "patch.h"
#include "mpq.h"
#include "helpers.h"


int ExtractMpqFromExe(HANDLE hArchive) {
    std::string fileName = GetMpqFileName(hArchive);
    std::cout << "[+] Archive name: " << fileName << std::endl;

    int64_t archiveOffset = GetMpqArchiveOffset(hArchive);
    std::cout << "[+] Archive offset: " << archiveOffset << std::endl;
    
    int32_t archiveSize = GetMpqArchiveSize(hArchive);
    std::cout << "[+] Archive size: " << archiveSize << std::endl;

    // Check the segment is aligned to 512 bytes
    // MPQ archives embedded in an exe must be sector (512) aligned
    if (archiveOffset % 512 != 0) {
        std::cerr << "[+] Error: Archive offset not aligned to 512 bytes" << std::endl;
        return 1;
    }

    // Create filename for BIN output (EXE file with BIN extension)
    std::string outputBinFilePath = replaceFileExtension(fileName, ".bin");
    std::cout << "[+] outputBinFilePath: " << outputBinFilePath << std::endl;
    std::ifstream file_bin(fileName, std::ios::binary);
    std::ofstream output_bin(outputBinFilePath, std::ios::binary);
    file_bin.seekg(0, std::ios::beg);
    std::vector<char> buffer_bin(archiveOffset);
    file_bin.read(buffer_bin.data(), buffer_bin.size());
    output_bin.write(buffer_bin.data(), buffer_bin.size());

    // Create filename for MPQ output (EXE file with MPQ extension)
    std::string outputMpqFilePath = replaceFileExtension(fileName, ".mpq");
    std::cout << "[+] outputMpqFilePath: " << outputMpqFilePath << std::endl;
    std::ifstream file_mpq(fileName, std::ios::binary);
    std::ofstream output_mpq(outputMpqFilePath, std::ios::binary);
    file_mpq.seekg(archiveOffset, std::ios::beg);
    std::vector<char> buffer(archiveSize);
    file_mpq.read(buffer.data(), buffer.size());
    output_mpq.write(buffer.data(), buffer.size());

    return 0;
}
