#include <iostream>
#include <cstdint>

#include <CLI/CLI.hpp>
#include <StormLib.h>

#include "mpq.h"
#include "verify.h"
#include "patch.h"


int main(int argc, char **argv) {
    CLI::App app{"A command line tool to read, extract, search, create and verify MPQ file using the StormLib library"};

    std::string target = "default";
    std::string outputDirectory = "default";
    std::string searchString = "";
    bool searchInsensitive = false;
    bool searchExtract = false;
    bool patchExtractMpq = false;
    bool patchExtractBin = false;

    // Subcommand: About
    CLI::App *about = app.add_subcommand("about", "Prints info about the program");

    // Subcommand: Info
    CLI::App *info = app.add_subcommand("info", "Prints info about the MPQ file");
    info->add_option("target", target, "Target MPQ file")
        ->required()
        ->check(CLI::ExistingFile);

    // Subcommand: Extract
    CLI::App *extract = app.add_subcommand("extract", "Extract files from the MPQ file");
    extract->add_option("target", target, "Target MPQ file")
        ->required()
        ->check(CLI::ExistingFile);
    extract->add_option("-o,--output", outputDirectory, "Output directory");

    // Subcommand: Create
    CLI::App *create = app.add_subcommand("create", "Create MPQ file from target directory");
    create->add_option("target", target, "Target directory")
        ->required()
        ->check(CLI::ExistingDirectory);
    
    // Subcommand: Search
    CLI::App *search = app.add_subcommand("search", "Search files from the MPQ file");
    search->add_option("target", target, "Target MPQ file")
        ->required()
        ->check(CLI::ExistingFile);
    search->add_option("-s,--search", searchString, "Search string");
    search->add_flag("-c,--case-insensitive", searchInsensitive, "Case insensitive search");
    search->add_flag("-e,--extract", searchExtract, "Extract any search results");
    search->add_option("-o,--output", outputDirectory, "Output directory");

    // Subcommand: Verify
    CLI::App *verify = app.add_subcommand("verify", "Verify the MPQ file");
    verify->add_option("target", target, "Target MPQ file")
        ->required()
        ->check(CLI::ExistingFile);

    // Subcommand: Patch
    CLI::App *patch = app.add_subcommand("patch", "Various MPQ patch helpers");
    patch->add_option("target", target, "Target file")
        ->required()
        ->check(CLI::ExistingFile);
    patch->add_flag("-e,--extract", patchExtractMpq, "Extract MPQ file from EXE patch");
    patch->add_flag("-b,--bin", patchExtractBin, "Extract BIN file from EXE patch");

    CLI11_PARSE(app, argc, argv);

    // Handle subcommand: About
    if (app.got_subcommand(about)){
        std::cout << "mpqcli" << std::endl;
        std::cout << "\nLibraries used:" << std::endl;
        std::cout << "StormLib - MIT (https://github.com/ladislav-zezula/StormLib)" << std::endl;
        std::cout << "CLI11 - BSD (https://github.com/CLIUtils/CLI11)" << std::endl;
        std::cout << "filesystem - MIT (https://github.com/gulrak/filesystem)" << std::endl;
    }

    // Handle subcommand: Info
    if (app.got_subcommand(info)){
        HANDLE hArchive;
        OpenMpqArchive(target, &hArchive);
        PrintMpqInfo(hArchive);
    }

    // Handle subcommand: Extract
    if (app.got_subcommand(extract)) {
        // If no output directory specified, user MPQ path without extension
        // If output directory specified, create it in pwd
        if (outputDirectory == "default") {
            outputDirectory = target;
            size_t lastDotPos = outputDirectory.rfind('.');
            if (lastDotPos != std::string::npos) {
                outputDirectory = outputDirectory.substr(0, lastDotPos);
            }
        }

        HANDLE hArchive;
        OpenMpqArchive(target, &hArchive);
        ExtractFiles(hArchive, outputDirectory);
    }

    // Handle subcommand: Create
    if (app.got_subcommand(create)) {
        std::cout << "[+] Not yet implemented... Exiting" << std::endl;
        return 0;
    }

    // Handle subcommand: Verify
    if (app.got_subcommand(verify)) {
        HANDLE hArchive;
        OpenMpqArchive(target, &hArchive);
    }

    // Handle subcommand: Search
    if (app.got_subcommand(search)) {
        HANDLE hArchive;
        OpenMpqArchive(target, &hArchive);
        ListFiles(hArchive);
        return 1;
    }

    // Handle subcommand: Patch
    if (app.got_subcommand(patch)) {
        HANDLE hArchive;
        OpenMpqArchive(target, &hArchive);
        if (patchExtractMpq) {
            ExtractMpqFromExe(hArchive);
        }
        if (patchExtractBin) {
            ExtractBinFromExe(hArchive);
        }
    }

    return 0;
}
