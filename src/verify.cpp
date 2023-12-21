#include <iostream>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <sys/stat.h>

#include <StormLib.h>

#include "verify.h"
#include "mpq.h"


int VerifyArchive(HANDLE hArchive)
{
    int32_t signatureType;

    if (!SFileGetFileInfo(hArchive, SFileMpqSignatures, &signatureType, sizeof(signatureType), NULL)) {
        std::cerr << "[+] Failed: SFileMpqSignatures" << std::endl;
        return -1;
    }

    if (signatureType == SIGNATURE_TYPE_WEAK) {
        std::cout << "[+] Signature type: Weak" << std::endl;
    } 
    else if (signatureType == SIGNATURE_TYPE_STRONG) {
        std::cout << "[+] Signature type: Strong" << std::endl;
        return 0;
    } 
    else {
        std::cout << "[+] Signature type: None" << std::endl;
        return 1;
    }
}

int VerifyMpqSignature(HANDLE hArchive)
{
    std::cout << "VerifyMpqSignature" << std::endl;

    const char *szFileName = "(signature)";
    unsigned int fileSize;
    char* fileContent = ReadOneFile(hArchive, szFileName, &fileSize);

    for (DWORD i = 0; i < fileSize; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(static_cast<unsigned char>(fileContent[i])) << " ";
    }
    std::cout << std::endl;

    return 1;
}
