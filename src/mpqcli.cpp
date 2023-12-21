#include <iostream>
#include <cstdint>

#include <CLI11.hpp>
#include <StormLib.h>

#include "mpq.h"
#include "info.h"
#include "verify.h"
#include "patch.h"


int main(int argc, char **argv) {
    CLI::App app{"A command line tool to read, extract, search, create and verify MPQ file using the StormLib library"};

    std::string target = "default";
    std::string outputDirectory = "default";
    std::string searchString = "";
    bool searchInsensitive = false;
    bool searchExtract = false;
    bool patchExtract = false;

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
    extract->add_option("target", target, "Target directory")
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
    patch->add_flag("-e,--extract", patchExtract, "Extract MPQ file from EXE patch");

    CLI11_PARSE(app, argc, argv);

    // Handle subcommand: Info
    if (app.got_subcommand(info)){
        HANDLE hArchive;
        OpenMpqArchive(target, &hArchive);

        if (!GetInfo(hArchive)) {
            std::cerr << "[+] Failed..." << std::endl;
            return -1;
        }
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
    if (app.got_subcommand(extract)) {
        std::cout << "Not yet implemented... Exiting" << std::endl;
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
        if (patchExtract) {
            ExtractMpqFromExe(hArchive);
        }
    }

    return 0;
}
