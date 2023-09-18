#include <iostream>
#include <StormLib.h>
#include <sys/stat.h>

void CreateDirectoryIfNotExists(const std::string& path) {
    struct stat info;
    if (stat(path.c_str(), &info) != 0) {
        if (mkdir(path.c_str(), 0777) != 0) {
            std::cerr << "Failed to create directory: " << path << std::endl;
        }
    }
}

void ExtractFilesFromMPQ(const std::string& mpqFileName, const std::string& outputFolder) {
    // Open the MPQ archive
    HANDLE mpqHandle = NULL;
    if (!SFileOpenArchive(mpqFileName.c_str(), 0, MPQ_OPEN_READ_ONLY, &mpqHandle)) {
        std::cerr << "Failed to open MPQ archive." << std::endl;
        return;
    }

    // Enumerate files in the MPQ archive
    SFILE_FIND_DATA findData;
    HANDLE findHandle = SFileFindFirstFile(mpqHandle, "*", &findData, NULL);
    if (findHandle == NULL) {
        std::cerr << "Failed to find first file in MPQ archive." << std::endl;
        SFileCloseArchive(mpqHandle);
        return;
    }

    // Create output directory with the same name as input MPQ file (without extension)
    std::string outputDirName = outputFolder + "/";
    outputDirName += mpqFileName;
    size_t lastDotPos = outputDirName.rfind('.');
    if (lastDotPos != std::string::npos) {
        outputDirName = outputDirName.substr(0, lastDotPos);
    }
    CreateDirectoryIfNotExists(outputDirName);

    // Loop through and extract files
    do {
        std::string outputFile = outputDirName + "/" + findData.cFileName;
        HANDLE outFileHandle;
        if (SFileExtractFile(mpqHandle, findData.cFileName, outputFile.c_str(), 0)) {
            std::cout << "Extracted: " << findData.cFileName << std::endl;
        } else {
            std::cerr << "Failed to extract: " << findData.cFileName << std::endl;
        }
    } while (SFileFindNextFile(findHandle, &findData));

    // Clean up
    SFileFindClose(findHandle);
    SFileCloseArchive(mpqHandle);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <mpq_filename>" << std::endl;
        return 1;
    }

    std::string mpqFileName = argv[1];
    std::string outputFolder = "."; // Current directory

    CreateDirectoryIfNotExists(outputFolder);
    ExtractFilesFromMPQ(mpqFileName, outputFolder);

    return 0;
}
