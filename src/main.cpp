#include <iostream>
#include <cstdint>
#include <filesystem>

#include <CLI/CLI.hpp>
#include <StormLib.h>

#include "mpq.h"
#include "helpers.h"
#include "locales.h"
#include "mpqcli.h"
#include "gamerules.h"

int main(int argc, char **argv) {
    CLI::App app{
        "A command line tool to create, add, remove, list, extract, read, and verify MPQ archives "
        "using the StormLib library"
    };

    // Require at least one subcommand
    app.require_subcommand(1);

    // CLI: base
    // These are reused in multiple subcommands
    std::string baseTarget = "default";  // all subcommands
    std::string baseFile = "default";  // add, remove, extract, read
    std::string basePath = "default"; // add
    std::string baseLocale = "default"; // create, add, remove, extract, read
    std::string baseOutput = "default";  // create, extract
    std::string baseListfileName = "default";  // list, extract
    std::string baseGameProfile = "default";  // create, add
    // CLI: info
    std::string infoProperty = "default";
    // CLI: list
    std::vector<std::string> listProperties;
    // CLI: extract
    bool extractKeepFolderStructure = false;
    // CLI: create
    bool createSignArchive = false;
    int32_t createMpqVersion = -1;
    int64_t createStreamFlags = -1;
    int64_t createSectorSize = -1;
    int64_t createRawChunkSize = -1;
    int64_t createFileFlags1 = -1;
    int64_t createFileFlags2 = -1;
    int64_t createFileFlags3 = -1;
    int64_t createAttrFlags = -1;
    // CLI: add and create (compression overrides for files being added)
    int64_t fileDwFlags = -1;
    int64_t fileDwCompression = -1;
    int64_t fileDwCompressionNext = -1;
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
            "signature-type",
    };
    std::set<std::string> validFileListProperties = {
            "hash-index",
            "name-hash1",
            "name-hash2",
            "name-hash3",
            "locale",
            "file-index",
            "byte-offset",
            "file-time",
            "file-size",
            "compressed-size",
            "flags",
            "encryption-key",
            "encryption-key-raw",
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
    create->add_option("--locale", baseLocale, "Locale to use for added files")
        ->check(LocaleValid);
    create->add_option("-g,--game", baseGameProfile, "Game profile for MPQ creation. Valid options:\n" + GameRules::GetAvailableProfiles())
        ->check(GameProfileValid);
    // MPQ creation settings overrides
    create->add_option("--version", createMpqVersion, "Override the MPQ archive version")->check(CLI::Range(1, 4))->group("Game setting overrides");
    create->add_option("--stream-flags", createStreamFlags, "Override stream flags")->group("Game setting overrides");
    create->add_option("--sector-size", createSectorSize, "Override sector size")->group("Game setting overrides");
    create->add_option("--raw-chunk-size", createRawChunkSize, "Override raw chunk size for MPQ v4")->group("Game setting overrides");
    create->add_option("--file-flags1", createFileFlags1, "Override file flags for (listfile)")->group("Game setting overrides");
    create->add_option("--file-flags2", createFileFlags2, "Override file flags for (attributes)")->group("Game setting overrides");
    create->add_option("--file-flags3", createFileFlags3, "Override file flags for (signature)")->group("Game setting overrides");
    create->add_option("--attr-flags", createAttrFlags, "Override attribute flags (CRC32, FILETIME, MD5)")->group("Game setting overrides");
    // Compression settings overrides for files being added
    create->add_option("--flags", fileDwFlags, "Override MPQ file flags for added files")->group("Game setting overrides");
    create->add_option("--compression", fileDwCompression, "Override compression for first sector of added files")->group("Game setting overrides");
    create->add_option("--compression-next", fileDwCompressionNext, "Override compression for subsequent sectors of added files")->group("Game setting overrides");

    // Subcommand: Add
    CLI::App *add = app.add_subcommand("add", "Add a file to an existing MPQ archive");
    add->add_option("file", baseFile, "File to add")
        ->required()
        ->check(CLI::ExistingFile);
    add->add_option("target", baseTarget, "Target MPQ archive")
        ->required()
        ->check(CLI::ExistingFile);
    add->add_option("-p,--path", basePath, "Path within MPQ archive");
    add->add_option("--locale", baseLocale, "Locale to use for added file")
        ->check(LocaleValid);
    add->add_option("-g,--game", baseGameProfile, "Game profile for compression rules. Valid options:\n" + GameRules::GetAvailableProfiles())
        ->check(GameProfileValid);
    // Compression settings overrides
    add->add_option("--flags", fileDwFlags, "Override MPQ file flags")->group("Game setting overrides");
    add->add_option("--compression", fileDwCompression, "Override compression for first sector")->group("Game setting overrides");
    add->add_option("--compression-next", fileDwCompressionNext, "Override compression for subsequent sectors")->group("Game setting overrides");

    // Subcommand: Remove
    CLI::App *remove = app.add_subcommand("remove", "Remove file from an existing MPQ archive");
    remove->add_option("file", baseFile, "File to remove")
        ->required();
    remove->add_option("target", baseTarget, "Target MPQ archive")
        ->required()
        ->check(CLI::ExistingFile);
    remove->add_option("--locale", baseLocale, "Locale of file to remove")
        ->check(LocaleValid);

    // Subcommand: List
    CLI::App *list = app.add_subcommand("list", "List files from the MPQ archive");
    list->add_option("target", baseTarget, "Target MPQ archive")
        ->required()
        ->check(CLI::ExistingFile);
    list->add_option("-l,--listfile", baseListfileName, "File listing content of an MPQ archive")
        ->check(CLI::ExistingFile);
    list->add_flag("-d,--detailed", listDetailed, "File listing with additional columns (default false)");
    list->add_flag("-a,--all", listAll, "File listing including hidden files (default true)");
    list->add_option("-p,--property", listProperties, "Prints only specific property values")
        ->check(CLI::IsMember(validFileListProperties));

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
    extract->add_option("--locale", baseLocale, "Preferred locale for extracted file");

    // Subcommand: Read
    CLI::App* read = app.add_subcommand("read", "Read a file from an MPQ archive");
    read->add_option("file", baseFile, "File to read")
        ->required();
    read->add_option("target", baseTarget, "Target MPQ archive")
        ->required()
        ->check(CLI::ExistingFile);
    read->add_option("--locale", baseLocale, "Preferred locale for read file");

    // Subcommand: Verify
    CLI::App *verify = app.add_subcommand("verify", "Verify the MPQ archive");
    verify->add_option("target", baseTarget, "Target MPQ archive")
        ->required()
        ->check(CLI::ExistingFile);
    verify->add_flag("-p,--print", verifyPrintSignature, "Print the digital signature (in hex)");

    // Parse command line arguments and handle errors
    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError &e) {
        // If we get a "subcommand required" error, print help message
        if (e.get_exit_code() == static_cast<int>(CLI::ExitCodes::RequiredError)) {
            std::cout << app.help() << std::endl;
            return 0;
        }
        // For other errors, use the default error handling
        return app.exit(e);
    }

    // Handle subcommand: Version
    if (app.got_subcommand(version)){
        std::cout << MPQCLI_VERSION << "-" << GIT_COMMIT_HASH << std::endl;
    }

    // Handle subcommand: About
    if (app.got_subcommand(about)) {
        std::cout << "Name: mpqcli" << std::endl;
        std::cout << "Version: " << MPQCLI_VERSION << "-" << GIT_COMMIT_HASH << std::endl;
        std::cout << "Author: Thomas Laurenson" << std::endl;
        std::cout << "License: MIT" << std::endl;
        std::cout << "GitHub: https://github.com/TheGrayDot/mpqcli" << std::endl;
        std::cout << "Dependencies:" << std::endl;
        std::cout << " - StormLib (https://github.com/ladislav-zezula/StormLib)" << std::endl;
        std::cout << " - CLI11 (https://github.com/CLIUtils/CLI11)" << std::endl;
    }

    // Handle subcommand: Info
    if (app.got_subcommand(info)) {
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

        GameProfile profile;
        if (baseGameProfile != "default") {
            profile = GameRules::StringToProfile(baseGameProfile);
        } else {
            profile = GameRules::GetDefaultProfile();
        }
        GameRules gameRules(profile);

        std::cout << "[*] Game profile: " << baseGameProfile << ", Output file: " << outputFile << std::endl;

        if (createMpqVersion > 0) {
            createMpqVersion--; // We label versions 1-4, but StormLib uses 0-3
        }
        // Apply MpqCreateSettings overrides if provided
        MpqCreateSettingsOverrides overrides;
        if (createMpqVersion >= 0) {
            overrides.mpqVersion = static_cast<DWORD>(createMpqVersion);
        }
        if (createStreamFlags >= 0) {
            overrides.streamFlags = static_cast<DWORD>(createStreamFlags);
        }
        if (createFileFlags1 >= 0) {
            overrides.fileFlags1 = static_cast<DWORD>(createFileFlags1);
        }
        if (createFileFlags2 >= 0) {
            overrides.fileFlags2 = static_cast<DWORD>(createFileFlags2);
        }
        if (createFileFlags3 >= 0) {
            overrides.fileFlags3 = static_cast<DWORD>(createFileFlags3);
        }
        if (createAttrFlags >= 0) {
            overrides.attrFlags = static_cast<DWORD>(createAttrFlags);
        }
        if (createSectorSize >= 0) {
            overrides.sectorSize = static_cast<DWORD>(createSectorSize);
        }
        if (createRawChunkSize >= 0) {
            overrides.rawChunkSize = static_cast<DWORD>(createRawChunkSize);
        }
        gameRules.OverrideCreateSettings(overrides);

        // Determine the number of files we are going to add
        int32_t fileCount = CalculateMpqMaxFileValue(baseTarget);

        // Create the MPQ archive and add files
        HANDLE hArchive = CreateMpqArchive(outputFile, fileCount, gameRules);
        if (hArchive) {
            LCID locale = LangToLocale(baseLocale);

            // Apply AddFileSettings overrides if provided
            CompressionSettingsOverrides addOverrides;
            if (fileDwFlags >= 0) addOverrides.dwFlags = static_cast<DWORD>(fileDwFlags);
            if (fileDwCompression >= 0) addOverrides.dwCompression = static_cast<DWORD>(fileDwCompression);
            if (fileDwCompressionNext >= 0) addOverrides.dwCompressionNext = static_cast<DWORD>(fileDwCompressionNext);

            AddFiles(hArchive, baseTarget, locale, gameRules, addOverrides);

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

        LCID locale = LangToLocale(baseLocale);

        GameProfile profile;
        if (baseGameProfile != "default") {
            profile = GameRules::StringToProfile(baseGameProfile);
            std::cout << "[*] Using game profile: " << baseGameProfile << std::endl;
        } else {
            profile = GameRules::GetDefaultProfile();
        }
        GameRules gameRules(profile);

        // Apply AddFileSettings overrides if provided
        CompressionSettingsOverrides addOverrides;
        if (fileDwFlags >= 0) addOverrides.dwFlags = static_cast<DWORD>(fileDwFlags);
        if (fileDwCompression >= 0) addOverrides.dwCompression = static_cast<DWORD>(fileDwCompression);
        if (fileDwCompressionNext >= 0) addOverrides.dwCompressionNext = static_cast<DWORD>(fileDwCompressionNext);

        AddFile(hArchive, baseFile, archivePath, locale, gameRules, addOverrides);
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

        LCID locale = LangToLocale(baseLocale);
        RemoveFile(hArchive, baseFile, locale);
        CloseMpqArchive(hArchive);
    }

    // Handle subcommand: List
    if (app.got_subcommand(list)) {
        HANDLE hArchive;
        if (!OpenMpqArchive(baseTarget, &hArchive, MPQ_OPEN_READ_ONLY)) {
            std::cerr << "[!] Failed to open MPQ archive." << std::endl;
            return 1;
        }
        ListFiles(hArchive, baseListfileName, listAll, listDetailed, listProperties);
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

        LCID locale = LangToLocale(baseLocale);
        if (baseLocale != "default" && locale == defaultLocale) {
            std::cout << "[!] Warning: The locale '" << baseLocale << "' is unknown. Will use default locale instead." << std::endl;
        }

        if (baseFile != "default") {
            ExtractFile(hArchive, baseOutput, baseFile, extractKeepFolderStructure, locale);
        } else {
            ExtractFiles(hArchive, baseOutput, baseListfileName, locale);
        }
    }

    // Handle subcommand: Read
    if (app.got_subcommand(read)) {
        HANDLE hArchive;
        if (!OpenMpqArchive(baseTarget, &hArchive, MPQ_OPEN_READ_ONLY)) {
            std::cerr << "[!] Failed to open MPQ archive." << std::endl;
            return 1;
        }

        LCID locale = LangToLocale(baseLocale);
        if (baseLocale != "default" && locale == defaultLocale) {
            std::cout << "[!] Warning: The locale '" << baseLocale << "' is unknown. Will use default locale instead." << std::endl;
        }

        uint32_t fileSize;
        char* fileContent = ReadFile(hArchive, baseFile.c_str(), &fileSize, locale);
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

        uint32_t verifyResult = VerifyMpqArchive(hArchive);
        if (verifyResult == ERROR_WEAK_SIGNATURE_OK ||
            verifyResult == ERROR_STRONG_SIGNATURE_OK ||
            verifyResult == ERROR_WEAK_SIGNATURE_ERROR ||
            verifyResult == ERROR_STRONG_SIGNATURE_ERROR) {
            if (verifyPrintSignature) {
                // If printing the signature, don't print success message
                // because the user might want to pipe/redirect the signature data
                PrintMpqSignature(hArchive, baseTarget);
            } else {
                // Just print verification success
                std::cout << "[*] Verify success" << std::endl;
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
