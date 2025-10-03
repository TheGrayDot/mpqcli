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
    CLI::App app{
        "A command line tool to create, add, remove, list, extract, read, and verify MPQ archives "
        "using the StormLib library"
    };

    // CLI: base
    // These are reused in multiple subcommands
    std::string baseTarget = "default";  // all subcommands
    std::string baseFile = "default";  // add, remove, extract, read
    std::string basePath = "default"; // add
    std::string baseOutput = "default";  // create, extract
    std::string baseListfileName = "default";  // list, extract
    // CLI: info
    std::string infoProperty = "default";    
    // CLI: extract
    bool extractKeepFolderStructure = false;
    // CLI: create
    bool createSignArchive = false;
    int32_t createMpqVersion = 1;
    // CLI: list
    bool listDetailed = false;
    bool listAll = false;
    // CLI: verify
    bool verifyPrintSignature = false;

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
    info->add_option("target", baseTarget, "Target MPQ archive")
        ->required()
        ->check(CLI::ExistingFile);
    info->add_option("-p,--property", infoProperty, "Prints only a specific property value")
        ->check(CLI::IsMember(validInfoProperties));

    // Subcommand: Create
    CLI::App *create = app.add_subcommand("create", "Create an MPQ archive from target directory");
    create->add_option("target", baseTarget, "Target directory")
        ->required()
        ->check(CLI::ExistingDirectory);
    create->add_option("-o,--output", baseOutput, "Output MPQ archive");
    create->add_flag("-s,--sign", createSignArchive, "Sign the MPQ archive (default false)");
    create->add_option("-v,--version", createMpqVersion, "Set the MPQ archive version (default 1)")
        ->check(CLI::Range(1, 2));

    // Subcommand: Add
    CLI::App *add = app.add_subcommand("add", "Add a file to an existing MPQ archive");
    add->add_option("file", baseFile, "File to add")
        ->required()
        ->check(CLI::ExistingFile);
    add->add_option("target", baseTarget, "Target MPQ archive")
        ->required()
        ->check(CLI::ExistingFile);
    add->add_option("-p,--path", basePath, "Path within MPQ archive");

    // Subcommand: Remove
    CLI::App *remove = app.add_subcommand("remove", "Remove file from an existing MPQ archive");
    remove->add_option("file", baseFile, "File to remove")
        ->required();
    remove->add_option("target", baseTarget, "Target MPQ archive")
        ->required()
        ->check(CLI::ExistingFile);

    // Subcommand: List
    CLI::App *list = app.add_subcommand("list", "List files from the MPQ archive");
    list->add_option("target", baseTarget, "Target MPQ archive")
        ->required()
        ->check(CLI::ExistingFile);
    list->add_option("-l,--listfile", baseListfileName, "File listing content of an MPQ archive")
        ->check(CLI::ExistingFile);
    list->add_flag("-d,--detailed", listDetailed, "File listing with additional columns (default false)");
    list->add_flag("-a,--all", listAll, "File listing including hidden files (default true)");

    // Subcommand: Extract
    CLI::App *extract = app.add_subcommand("extract", "Extract files from the MPQ archive");
    extract->add_option("target", baseTarget, "Target MPQ archive")
        ->required()
        ->check(CLI::ExistingFile);
    extract->add_option("-o,--output", baseOutput, "Output directory");
    extract->add_option("-f,--file", baseFile, "Target file to extract");
    extract->add_flag("-k,--keep", extractKeepFolderStructure, "Keep folder structure (default false)");
    extract->add_option("-l,--listfile", baseListfileName, "File listing content of an MPQ archive")
        ->check(CLI::ExistingFile);

    // Subcommand: Read
    CLI::App* read = app.add_subcommand("read", "Read a file from an MPQ archive");
    read->add_option("file", baseFile, "File to read")
        ->required();
    read->add_option("target", baseTarget, "Target MPQ archive")
        ->required()
        ->check(CLI::ExistingFile);

    // Subcommand: Verify
    CLI::App *verify = app.add_subcommand("verify", "Verify the MPQ archive");
    verify->add_option("target", baseTarget, "Target MPQ archive")
        ->required()
        ->check(CLI::ExistingFile);
    verify->add_flag("-p,--print", verifyPrintSignature, "Print the digital signature (in hex)");

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
        if (!OpenMpqArchive(baseTarget, &hArchive, MPQ_OPEN_READ_ONLY)) {
            std::cerr << "[!] Failed to open MPQ archive." << std::endl;
            return 1;
        }
        PrintMpqInfo(hArchive, infoProperty);
    }

    // Handle subcommand: Create
    if (app.got_subcommand(create)) {
        fs::path outputFilePath;
        if (baseOutput != "default") {
            outputFilePath = fs::absolute(baseOutput);
        } else {
            outputFilePath = fs::path(baseTarget);
            outputFilePath.replace_extension(".mpq");
        }
        std::string outputFile = outputFilePath.u8string();
        
        std::cout << "[+] Output file: " << outputFile << std::endl;

        // Determine number of files we are going to add
        int32_t fileCount = CalculateMpqMaxFileValue(baseTarget);
    
        // Create the MPQ archive and add files
        HANDLE hArchive = CreateMpqArchive(outputFile, fileCount, createMpqVersion);
        if (hArchive) {
            AddFiles(hArchive, baseTarget);
            if (createSignArchive) {
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
        HANDLE hArchive;
        // Open the MPQ archive for writing (this is why we set flag as 0)
        if (!OpenMpqArchive(baseTarget, &hArchive, 0)) {
            std::cerr << "[!] Failed to open MPQ archive." << std::endl;
            return 1;
        }

        // Path to file on disk
        fs::path filePath = fs::path(baseFile);

        // Default: use the filename as path, saves file to root of MPQ
        std::string archivePath = filePath.filename().u8string();

        // Optional: specified path inside archive
        if (basePath != "default") {
            fs::path archiveFullPath = fs::path(basePath) / filePath.filename();

            // Normalise path for MPQ
            archivePath = WindowsifyFilePath(archiveFullPath.u8string());
        }

        AddFile(hArchive, baseFile, archivePath);
        CloseMpqArchive(hArchive);
    }

    // Handle subcommand: Remove
    if (app.got_subcommand(remove)) {
        HANDLE hArchive;
        // Open the MPQ archive for writing (this is why we set flag as 0)
        if (!OpenMpqArchive(baseTarget, &hArchive, 0)) {
            std::cerr << "[!] Failed to open MPQ archive." << std::endl;
            return 1;
        }

        RemoveFile(hArchive, baseFile);
        CloseMpqArchive(hArchive);
    }

    // Handle subcommand: List
    if (app.got_subcommand(list)) {
        HANDLE hArchive;
        if (!OpenMpqArchive(baseTarget, &hArchive, MPQ_OPEN_READ_ONLY)) {
            std::cerr << "[!] Failed to open MPQ archive." << std::endl;
            return 1;
        }
        ListFiles(hArchive, baseListfileName, listAll, listDetailed);
    }

    // Handle subcommand: Extract
    if (app.got_subcommand(extract)) {
        // If no output directory specified, use MPQ path without extension
        // If output directory specified, create it if it doesn't exist
        if (baseOutput == "default") {
            fs::path outputPathAbsolute = fs::canonical(baseTarget);
            fs::path outputPath = outputPathAbsolute.parent_path() / outputPathAbsolute.stem();
            std::string outputString{outputPath.u8string()};
            baseOutput = outputString;
        }
        fs::create_directory(baseOutput);

        HANDLE hArchive;
        if (!OpenMpqArchive(baseTarget, &hArchive, MPQ_OPEN_READ_ONLY)) {
            std::cerr << "[!] Failed to open MPQ archive." << std::endl;
            return 1;
        }

        if (baseFile != "default") {
            ExtractFile(hArchive, baseOutput, baseFile, extractKeepFolderStructure);
        } else {
            ExtractFiles(hArchive, baseOutput, baseListfileName);
        }
    }

    // Handle subcommand: Read
    if (app.got_subcommand(read)) {
        HANDLE hArchive;
        if (!OpenMpqArchive(baseTarget, &hArchive, MPQ_OPEN_READ_ONLY)) {
            std::cerr << "[!] Failed to open MPQ archive." << std::endl;
            return 1;
        }

        uint32_t fileSize;
        char* fileContent = ReadFile(hArchive, baseFile.c_str(), &fileSize);
        if (fileContent == NULL) {
            return 1;
        }

        PrintAsBinary(fileContent, fileSize);

        delete[] fileContent;
        CloseMpqArchive(hArchive);
        return 0;
    }

    // Handle subcommand: Verify
    if (app.got_subcommand(verify)) {
        HANDLE hArchive;
        if (!OpenMpqArchive(baseTarget, &hArchive, MPQ_OPEN_READ_ONLY)) {
            std::cerr << "[!] Failed to open MPQ archive." << std::endl;
            return 1;
        }

        uint32_t verifyResult = SFileVerifyArchive(hArchive);
        if (verifyResult == ERROR_WEAK_SIGNATURE_OK || verifyResult == ERROR_STRONG_SIGNATURE_OK) {
            if (verifyPrintSignature) {
                // If printing the signature, don't print success message
                // because the user might want to pipe/redirect the signature data
                PrintMpqSignature(hArchive, baseTarget);
            } else {
                // Just print verification success
                std::cout << "[+] Verify success" << std::endl;
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
