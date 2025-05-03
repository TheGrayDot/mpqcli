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
    CLI::App app{"A command line tool to read, extract, search, create and verify MPQ file using the StormLib library"};

    std::string target = "default";
    std::string file = "default";
    std::string output = "default";
    bool keepFolderStructure = false;
    bool patchExtractBin = false;
    std::string extractFileName = "default";
    std::string listfileName = "";
    int32_t mpqVersion = 1;
    bool signArchive = false;
    std::string baseFolder = "";
    std::string infoProperty = "";
    bool printSignature = false;

    std::set<std::string> validInfoProperties = {
        "format-version",
        "header-offset",
        "header-size",
        "archive-size",
        "file-count",
        "signature-type"
    };

    // Subcommand: Version
    CLI::App *version = app.add_subcommand("version", "Prints program version");

    // Subcommand: About
    CLI::App *about = app.add_subcommand("about", "Prints program information");

    // Subcommand: Info
    CLI::App *info = app.add_subcommand("info", "Prints info about the MPQ file");
    info->add_option("target", target, "Target MPQ file")
        ->required()
        ->check(CLI::ExistingFile);
    info->add_option("-p,--property", infoProperty, "Print a specific property value only")
        ->check(CLI::IsMember(validInfoProperties));

    // Subcommand: Create
    CLI::App *create = app.add_subcommand("create", "Create MPQ file from target directory");
    create->add_option("target", target, "Target directory")
        ->required()
        ->check(CLI::ExistingDirectory);
    create->add_option("-o,--output", output, "Output file");
    create->add_option("-b,--base", baseFolder, "Base folder to use for file names");
    create->add_flag("-s,--sign", signArchive, "Sign the MPQ archive (default false)");
    create->add_option("-v,--version", mpqVersion, "MPQ version (default 1)")
        ->check(CLI::Range(1, 2));

    // Subcommand: Add
    CLI::App *add = app.add_subcommand("add", "Add file to an existing MPQ file");
    add->add_option("file", file, "File to add")
        ->required()
        ->check(CLI::ExistingFile);
    add->add_option("target", target, "Target MPQ file")
        ->required()
        ->check(CLI::ExistingFile);

    // Subcommand: Remove
    CLI::App *remove = app.add_subcommand("remove", "Remove file from an existing MPQ file");
    remove->add_option("file", target, "File to remove")
        ->required();
    remove->add_option("target", target, "Target MPQ file")
        ->required()
        ->check(CLI::ExistingFile);

    // Subcommand: List
    CLI::App *list = app.add_subcommand("list", "List files from the MPQ file");
    list->add_option("target", target, "Target MPQ file")
        ->required()
        ->check(CLI::ExistingFile);
    list->add_option("-l,--listfile", listfileName, "File listing content of MPQ")
        ->check(CLI::ExistingFile);

    // Subcommand: Extract
    CLI::App *extract = app.add_subcommand("extract", "Extract files from the MPQ file");
    extract->add_option("target", target, "Target MPQ file")
        ->required()
        ->check(CLI::ExistingFile);
    extract->add_option("-o,--output", output, "Output directory");
    extract->add_option("-f,--file", extractFileName, "Target file to extract");
    extract->add_flag("-k,--keep", keepFolderStructure, "Keep folder structure (default false)");
    extract->add_option("-l,--listfile", listfileName, "File listing content of MPQ")
        ->check(CLI::ExistingFile);

    // Subcommand: Patch
    CLI::App *patch = app.add_subcommand("patch", "Various MPQ patch helpers");
    patch->add_option("target", target, "Target file")
        ->required()
        ->check(CLI::ExistingFile);
    patch->add_flag("-b,--bin", patchExtractBin, "Extract BIN file from EXE patch (default false)");

    // Subcommand: Verify
    CLI::App *verify = app.add_subcommand("verify", "Verify the MPQ file");
    verify->add_option("target", target, "Target MPQ file")
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
        fileCount += 2;  // Add 2 for listfile and attributes
    
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

        if (extractFileName != "default") {
            ExtractFile(hArchive, output, extractFileName, keepFolderStructure);
        } else {
            ExtractFiles(hArchive, output, listfileName);
        }
    }

    // Handle subcommand: List
    if (app.got_subcommand(list)) {
        HANDLE hArchive;
        OpenMpqArchive(target, &hArchive);
        ListFiles(hArchive, listfileName);
    }

    // Handle subcommand: Patch
    if (app.got_subcommand(patch)) {
        std::cout << "[!] patch not implemented..." << std::endl;
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
