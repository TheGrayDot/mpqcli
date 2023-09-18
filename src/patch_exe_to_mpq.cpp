#include <iostream>
#include <fstream>
#include <string>

std::string replaceExtension(const std::string& filename, const std::string& newExtension) {
    size_t lastDotPos = filename.find_last_of('.');
    if (lastDotPos != std::string::npos) {
        return filename.substr(0, lastDotPos) + newExtension;
    }
    return filename + newExtension;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "[*] Usage: " << argv[0] << " <input_file>" << std::endl;
        return 1;
    }

    const std::string inputFile = argv[1];
    const std::string outputFile = replaceExtension(inputFile, ".mpq");

    std::ifstream inFile(inputFile, std::ios::binary);
    if (!inFile.is_open()) {
        std::cerr << "[*] Error opening input file... Exiting" << std::endl;
        return 1;
    }

    // Read the file contents into a string
    std::string fileContents((std::istreambuf_iterator<char>(inFile)),
                              std::istreambuf_iterator<char>());

    // Find the first occurrence of "MPQ"
    size_t foundOffset = fileContents.find("MPQ");
    if (foundOffset == std::string::npos) {
        std::cout << "[*] MPQ header not found in the file... Exiting" << std::endl;
        return 0;
    }

    // Print information about the found string and its offset
    std::cout << "[*] MPQ header found at offset: " << foundOffset << std::endl;

    // Modify the content by erasing everything before the found string
    fileContents.erase(0, foundOffset);

    // Close the input file
    inFile.close();

    // Write the modified content to the output file
    std::ofstream outFile(outputFile, std::ios::binary);
    if (!outFile.is_open()) {
        std::cerr << "[*] Error creating output file... Exiting" << std::endl;
        return 1;
    }

    outFile << fileContents;
    outFile.close();

    std::cout << "[*] Modified content saved to " << outputFile << std::endl;

    return 0;
}
