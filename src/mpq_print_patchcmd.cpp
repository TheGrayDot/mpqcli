#include <StormLib.h>
#include <iostream>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cout << "Usage: " << argv[0] << " <mpq_filename>" << std::endl;
    return 1;
  }

  const char *mpqFileName = argv[1];

  // Open the MPQ archive
  HANDLE mpqHandle = NULL;
  if (!SFileOpenArchive(mpqFileName, 0, MPQ_OPEN_READ_ONLY, &mpqHandle)) {
    std::cerr << "Failed to open MPQ archive." << std::endl;
    return 1;
  }

  // Enumerate files in the MPQ archive
  SFILE_FIND_DATA findData;
  HANDLE findHandle = SFileFindFirstFile(mpqHandle, "*", &findData, NULL);
  if (findHandle == NULL) {
    std::cerr << "Failed to find first file in MPQ archive." << std::endl;
    SFileCloseArchive(mpqHandle);
    return 1;
  }

  bool foundPatchCmd = false;

  // Loop through and find "patch.cmd" file
  do {
    if (std::string(findData.cFileName) == "patch.cmd") {
      foundPatchCmd = true;

      // Open and read the contents of "patch.cmd"
      HANDLE fileHandle;
      if (SFileOpenFileEx(mpqHandle, findData.cFileName, 0, &fileHandle)) {
        DWORD fileSize = SFileGetFileSize(fileHandle, NULL);
        char *fileContent = new char[fileSize + 1];
        SFileReadFile(fileHandle, fileContent, fileSize, NULL, NULL);
        fileContent[fileSize] = '\0'; // Null-terminate the content
        std::cout << fileContent << std::endl;
        delete[] fileContent;
        SFileCloseFile(fileHandle);
      } else {
        std::cerr << "Failed to open patch.cmd file." << std::endl;
      }
    }
  } while (!foundPatchCmd && SFileFindNextFile(findHandle, &findData));

  // Clean up
  SFileFindClose(findHandle);
  SFileCloseArchive(mpqHandle);

  return 0;
}
