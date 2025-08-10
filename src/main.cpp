#include <iostream>
#include <cstdint>
#include <filesystem>

#include <CLI/CLI.hpp>
#include <StormLib.h>

#include "mpq.h"
#include "helpers.h"
#include "mpqcli.h"

namespace fs = std::filesystem;

int main(int argc, char **argv) {
    CLI::App app{"A command line tool to read, extract, search, create and verify MPQ archives using the StormLib library"};

    // CLI args: base, reused in multiple subcommands
    std::string target = "default";
    std::string file = "default";  // add, remove, extract, read
    std::string output = "default";  // create, extract
    std::string fileName = "default";  // read, extract
    std::string listfileName = "default";  // list, extract
    bool signArchive = false; // create, add
    // CLI: extract
    bool keepFolderStructure = false;
    // CLI: create
    int32_t mpqVersion = 1;
    std::string baseFolder = "";
    std::string infoProperty = "";
    // CLI: list
    bool listDetailed = false;
    bool listAll = false;
    // CLI: sign
    bool printSignature = false;

    std::set<std::string> validInfoProperties = {
        "format-version",
        "header-offset",
        "header-size",
        "archive-size",
        "file-count",
        "max-files",
        "signature-type"
    };

    // Subcommand: Version
    CLI::App *version = app.add_subcommand("version", "Prints program version");

    // Subcommand: About
    CLI::App *about = app.add_subcommand("about", "Prints program information");

    // Subcommand: Info
    CLI::App *info = app.add_subcommand("info", "Prints info about an MPQ archive");
    info->add_option("target", target, "Target MPQ archive")
        ->required()
        ->check(CLI::ExistingFile);
    info->add_option("-p,--property", infoProperty, "Prints only a specific property value")
        ->check(CLI::IsMember(validInfoProperties));

    // Subcommand: Create
    CLI::App *create = app.add_subcommand("create", "Create an MPQ archive from target directory");
    create->add_option("target", target, "Target directory")
        ->required()
        ->check(CLI::ExistingDirectory);
    create->add_option("-o,--output", output, "Output MPQ archive");
    create->add_flag("-s,--sign", signArchive, "Sign the MPQ archive (default false)");
    create->add_option("-v,--version", mpqVersion, "Set the MPQ archive version (default 1)")
        ->check(CLI::Range(1, 2));

    // Subcommand: Add
    CLI::App *add = app.add_subcommand("add", "Add a file to an existing MPQ archive");
    add->add_option("file", file, "File to add")
        ->required()
        ->check(CLI::ExistingFile);
    add->add_option("target", target, "Target MPQ archive")
        ->required()
        ->check(CLI::ExistingFile);

    // Subcommand: Remove
    CLI::App *remove = app.add_subcommand("remove", "Remove file from an existing MPQ archive");
    remove->add_option("file", target, "File to remove")
        ->required();
    remove->add_option("target", target, "Target MPQ archive")
        ->required()
        ->check(CLI::ExistingFile);

    // Subcommand: List
    CLI::App *list = app.add_subcommand("list", "List files from the MPQ archive");
    list->add_option("target", target, "Target MPQ archive")
        ->required()
        ->check(CLI::ExistingFile);
    list->add_option("-l,--listfile", listfileName, "File listing content of an MPQ archive")
        ->check(CLI::ExistingFile);
    list->add_flag("-d,--detailed", listDetailed, "File listing with additional columns (default false)");
    list->add_flag("-a,--all", listAll, "File listing including hidden files (default true)");

    // Subcommand: Extract
    CLI::App *extract = app.add_subcommand("extract", "Extract files from the MPQ archive");
    extract->add_option("target", target, "Target MPQ archive")
        ->required()
        ->check(CLI::ExistingFile);
    extract->add_option("-o,--output", output, "Output directory");
    extract->add_option("-f,--file", fileName, "Target file to extract");
    extract->add_flag("-k,--keep", keepFolderStructure, "Keep folder structure (default false)");
    extract->add_option("-l,--listfile", listfileName, "File listing content of an MPQ archive")
        ->check(CLI::ExistingFile);

    // Subcommand: Read
    CLI::App* read =
        app.add_subcommand("read", "Read a file from an MPQ archive");
    read->add_option("target", target, "Target MPQ archive")
        ->required()
        ->check(CLI::ExistingFile);
    read->add_option("-f,--file", fileName, "Target file to read")
        ->required();

    // Subcommand: Verify
    CLI::App *verify = app.add_subcommand("verify", "Verify the MPQ archive");
    verify->add_option("target", target, "Target MPQ archive")
        ->required()
        ->check(CLI::ExistingFile);
    verify->add_flag("-p,--print", printSignature, "Print the digital signature (in hex)");

    CLI11_PARSE(app, argc, argv);

    // Handle subcommand: Version
    if (app.got_subcommand(version)){
        std::cout << MPQCLI_VERSION << "-" << GIT_COMMIT_HASH << std::endl;
    }

    // Handle subcommand: About
    if (app.got_subcommand(about)){
        std::cout << "Name: mpqcli" << std::endl;
        std::cout << "Version: " << MPQCLI_VERSION << "-" << GIT_COMMIT_HASH << std::endl;
        std::cout << "Author: Thomas Laurenson" << std::endl;
        std::cout << "License: MIT" << std::endl;
        std::cout << "GitHub: https://github.com/TheGrayDot/mpqcli" << std::endl;
        std::cout << "Dependencies:" << std::endl;
        std::cout << " - StormLib (https://github.com/ladislav-zezula/StormLib)" << std::endl;
        std::cout << " - CLI11 (https://github.com/CLIUtils/CLI11)" << std::endl;
    };

    // Handle subcommand: Info
    if (app.got_subcommand(info)){
        HANDLE hArchive;
        if (!OpenMpqArchive(target, &hArchive)) {
            std::cerr << "[!] Failed to open MPQ archive." << std::endl;
            return 1;
        }
        PrintMpqInfo(hArchive, infoProperty);
    }

    // Handle subcommand: Create
    if (app.got_subcommand(create)) {
        fs::path outputFilePath;
        if (output != "default") {
            outputFilePath = fs::absolute(output);
        } else {
            outputFilePath = fs::path(target);
            outputFilePath.replace_extension(".mpq");
        }
        std::string outputFile = outputFilePath.u8string();
        
        std::cout << "[+] Output file: " << outputFile << std::endl;

        // Determine number of files we are going to add
        int32_t fileCount = CountFilesInDirectory(target);
        fileCount += 3;  // Add 3 for listfile, attributes and signature (if needed)
    
        // Create the MPQ archive and add files
        HANDLE hArchive = CreateMpqArchive(outputFile, fileCount, mpqVersion);
        if (hArchive) {
            AddFiles(hArchive, target);
            if (signArchive) {
                SignMpqArchive(hArchive);
            }
            CloseMpqArchive(hArchive);
        } else {
            std::cerr << "[!] Failed to create MPQ archive." << std::endl;
            return 1;
        }
    }

    // Handle subcommand: Add
    if (app.got_subcommand(add)) {
        std::cout << "[!] add not implemented..." << std::endl;
    }

    // Handle subcommand: Remove
    if (app.got_subcommand(remove)) {
        std::cout << "[!] remove not implemented..." << std::endl;
    }

    // Handle subcommand: List
    if (app.got_subcommand(list)) {
        HANDLE hArchive;
        OpenMpqArchive(target, &hArchive);
        ListFiles(hArchive, listfileName, listAll, listDetailed);
    }

    // Handle subcommand: Extract
    if (app.got_subcommand(extract)) {
        // If no output directory specified, use MPQ path without extension
        // If output directory specified, create it if it doesn't exist
        if (output == "default") {
            fs::path outputPathAbsolute = fs::canonical(target);
            fs::path outputPath = outputPathAbsolute.parent_path() / outputPathAbsolute.stem();
            std::string outputString{outputPath.u8string()};
            output = outputString;
        }
        fs::create_directory(output);

        HANDLE hArchive;
        if (!OpenMpqArchive(target, &hArchive)) {
            std::cerr << "[!] Failed to open MPQ archive." << std::endl;
            return 1;
        }

        if (fileName != "default") {
            ExtractFile(hArchive, output, fileName, keepFolderStructure);
        } else {
            ExtractFiles(hArchive, output, listfileName);
        }
    }

    // Handle subcommand: Read
    if (app.got_subcommand(read)) {
        HANDLE hArchive;
        if (!OpenMpqArchive(target, &hArchive)) {
            std::cerr << "[!] Failed to open MPQ archive." << std::endl;
            return 1;
        }

        uint32_t fileSize;
        char* fileContent =
            ReadFile(hArchive, fileName.c_str(), &fileSize);
        if (fileContent == NULL) {
            return 1;
        }

        if (IsPrintable(fileContent, fileSize)) {
            PrintAsText(fileContent, fileSize);
        } else {
            PrintAsHex(fileContent, fileSize);
        }

        delete[] fileContent;
        CloseMpqArchive(hArchive);
        return 0;
    }

    // Handle subcommand: Verify
    if (app.got_subcommand(verify)) {
        HANDLE hArchive;
        if (!OpenMpqArchive(target, &hArchive)) {
            std::cerr << "[!] Failed to open MPQ archive." << std::endl;
            return 1;
        }

        uint32_t verifyResult = SFileVerifyArchive(hArchive);
        if (verifyResult == ERROR_WEAK_SIGNATURE_OK || verifyResult == ERROR_STRONG_SIGNATURE_OK) {
            // Print verification success
            std::cout << "[+] Verify success" << std::endl;
            // If verification passed, print signature if user requested it
            if (printSignature) {
                PrintMpqSignature(hArchive, target);
            }

            // Return 0, because verification passed
            return 0;
        }

        // Any other verify result is no signature, or error verifying
        std::cout << "[!] Verify failed" << std::endl;
        return 1;        
    }

    return 0;
}
