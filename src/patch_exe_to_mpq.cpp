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

    // Open the file in binary mode
    std::ifstream file(inputFile, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "[*] Error opening input file... Exiting" << std::endl;
        return 1;
    }

    // Define search string for MPQ header
    const std::string searchString = "MPQ";

    // Read the file in 512-byte chunks
    const int chunkSize = 512;
    char buffer[chunkSize];
    std::streampos offset = 0;

    while (file) {
        // Read a chunk of data
        file.read(buffer, chunkSize);

        // Get the number of bytes read
        const std::streamsize bytesRead = file.gcount();

        // Search for the string within the chunk
        std::string chunk(buffer, bytesRead);
        const size_t foundPos = chunk.find(searchString);
        if (foundPos != std::string::npos) {
            // Calculate the offset
            offset += foundPos;

            std::cout << "[*] MPQ header found at offset: " << offset << std::endl;
            break; // Stop searching after the first instance is found
        }

        // Update the offset
        offset += bytesRead;

        // Move the file pointer to the next 512-byte interval
        file.seekg(offset);
    }

    // Seek file back to start
    file.seekg(0);

    // Read the file contents into a string
    std::string fileContents((std::istreambuf_iterator<char>(file)),
                              std::istreambuf_iterator<char>());

    // Modify the content by erasing everything before the found string
    fileContents.erase(0, offset);

    file.close();

    // Write the modified content to the output file
    std::ofstream outFile(outputFile, std::ios::binary);
    if (!outFile.is_open()) {
        std::cerr << "[*] Error creating output file... Exiting" << std::endl;
        return 1;
    }

    outFile << fileContents;
    outFile.close();

    std::cout << "[*] Modified content saved to: " << outputFile << std::endl;

    return 0;
}
