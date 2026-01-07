#include "gamerules.h"
#include <algorithm>
#include <cctype>
#include <map>

// Constructor
GameRules::GameRules(GameProfile gameProfile) : profile(gameProfile) {
    InitializeRules();
}

// Helper function to convert string to lowercase
static std::string ToLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

// Helper function to match wildcards (* and ?)
bool GameRules::MatchFileMask(const std::string& filename, const std::string& mask) {
    // Convert both to lowercase for case-insensitive matching
    std::string lowerFilename = ToLower(filename);
    std::string lowerMask = ToLower(mask);

    // Replace backslashes with forward slashes for consistent path handling
    std::replace(lowerFilename.begin(), lowerFilename.end(), '\\', '/');
    std::replace(lowerMask.begin(), lowerMask.end(), '\\', '/');

    // Simple wildcard matching
    size_t maskPos = 0;
    size_t filePos = 0;
    size_t starPos = std::string::npos;
    size_t matchPos = 0;

    while (filePos < lowerFilename.length()) {
        if (maskPos < lowerMask.length() && (lowerMask[maskPos] == '?' || lowerMask[maskPos] == lowerFilename[filePos])) {
            maskPos++;
            filePos++;
        } else if (maskPos < lowerMask.length() && lowerMask[maskPos] == '*') {
            starPos = maskPos;
            matchPos = filePos;
            maskPos++;
        } else if (starPos != std::string::npos) {
            maskPos = starPos + 1;
            matchPos++;
            filePos = matchPos;
        } else {
            return false;
        }
    }

    while (maskPos < lowerMask.length() && lowerMask[maskPos] == '*') {
        maskPos++;
    }

    return maskPos == lowerMask.length();
}

void GameRules::AddRuleByFileMask(const std::string& fileMask, DWORD mpqFlags, DWORD compressionFirst, DWORD compressionNext) {
    rules.emplace_back(fileMask, mpqFlags, compressionFirst, compressionNext);
}

// Use UINT32_MAX for sizeMax to indicate "no upper limit"
// Examples:
//   AddRuleByFileSize(0, 0, ...)                - Match files with exactly 0 bytes
//   AddRuleByFileSize(0, 0x4000, ...)           - Match files from 0 to 16KB
//   AddRuleByFileSize(0x4000, UINT32_MAX, ...)  - Match files from 16KB onwards
// ReSharper disable all CppDFAConstantParameter
void GameRules::AddRuleByFileSize(DWORD sizeMin, DWORD sizeMax, DWORD mpqFlags, DWORD compressionFirst, DWORD compressionNext) {
    rules.emplace_back(sizeMin, sizeMax, mpqFlags, compressionFirst, compressionNext);
}

// ReSharper disable once CppDFAConstantParameter
void GameRules::AddRuleDefault(DWORD mpqFlags, DWORD compressionFirst, DWORD compressionNext) {
    rules.emplace_back(mpqFlags, compressionFirst, compressionNext);
}

// Get compression settings for a specific file
CompressionSettings GameRules::GetCompressionSettings(const std::string& filename, const DWORD fileSize) const {
    // Iterate through rules in order (first match wins)
    for (const auto& rule : rules) {
        switch (rule.type) {
            case RuleType::FILE_MASK:
                if (MatchFileMask(filename, rule.fileMask)) {
                    return {rule.mpqFlags, rule.compressionFirst, rule.compressionNext};
                }
                break;

            case RuleType::FILE_SIZE: {
                // Use UINT32_MAX to indicate "no upper limit"
                bool hasUpperLimit = (rule.sizeMax != UINT32_MAX);
                bool inRange = fileSize >= rule.sizeMin && (!hasUpperLimit || fileSize <= rule.sizeMax);

                if (inRange) {
                    return {rule.mpqFlags, rule.compressionFirst, rule.compressionNext};
                }
                break;
            }

            case RuleType::DEFAULT:
                return {rule.mpqFlags, rule.compressionFirst, rule.compressionNext};
        }
    }

    // Fallback if no rules match (shouldn't happen if DEFAULT rule is present)
    return {MPQ_FILE_COMPRESS | MPQ_FILE_ENCRYPTED, MPQ_COMPRESSION_PKWARE, MPQ_COMPRESSION_NEXT_SAME};
}

// Override MPQ creation settings with user-provided values
void GameRules::OverrideCreateSettings(const MpqCreateSettingsOverrides& overrides) {
    // Track whether user explicitly set fileFlags2 (needed for automatic adjustment logic)
    bool userSetFileFlags2 = false;

    // Step 1: Apply user overrides
    // User-provided values always take priority, even if they might be incorrect.
    // We only apply override if the optional has a value (i.e., user specified it)

    if (overrides.mpqVersion.has_value()) {
        createSettings.mpqVersion = overrides.mpqVersion.value();
    }

    if (overrides.streamFlags.has_value()) {
        createSettings.streamFlags = overrides.streamFlags.value();
    }

    if (overrides.sectorSize.has_value()) {
        createSettings.sectorSize = overrides.sectorSize.value();
    }

    if (overrides.rawChunkSize.has_value()) {
        createSettings.rawChunkSize = overrides.rawChunkSize.value();
    }

    if (overrides.fileFlags1.has_value()) {
        createSettings.fileFlags1 = overrides.fileFlags1.value();
    }

    if (overrides.fileFlags2.has_value()) {
        createSettings.fileFlags2 = overrides.fileFlags2.value();
        userSetFileFlags2 = true;  // User explicitly set this value
    }

    if (overrides.fileFlags3.has_value()) {
        createSettings.fileFlags3 = overrides.fileFlags3.value();
    }

    if (overrides.attrFlags.has_value()) {
        createSettings.attrFlags = overrides.attrFlags.value();
    }

    // Step 2: Apply automatic adjustments based on dependencies
    // These only apply if the user hasn't explicitly overridden the values

    // fileFlags2 controls the (attributes) file, which is only meaningful when
    // attrFlags is also set. According to StormLib's SFileCreateArchive.cpp:
    // - The (attributes) file is created only when BOTH fileFlags2 AND attrFlags are non-zero
    // - If attrFlags is set but fileFlags2 is still 0 (not overridden by user or profile),
    //   we should set fileFlags2 to MPQ_FILE_DEFAULT_INTERNAL to enable the attributes file

    if (!userSetFileFlags2 && createSettings.fileFlags2 == 0 && createSettings.attrFlags != 0) {
        // User wants attributes (attrFlags is set) but hasn't specified how to store
        // the (attributes) file itself. Use the default internal file flags.
        createSettings.fileFlags2 = MPQ_FILE_DEFAULT_INTERNAL;
    }

    // Note: If user explicitly sets fileFlags2 to 0 via override, we respect that choice
    // even if attrFlags is non-zero.
}

// Get the profile name map (single source of truth for all valid profile names)
static const std::map<std::string, GameProfile>& GetProfileMap() {
    static const std::map<std::string, GameProfile> profileMap = {
        {"generic", GameProfile::GENERIC},
        {"diablo1", GameProfile::DIABLO1},
        {"diablo", GameProfile::DIABLO1},
        {"d1", GameProfile::DIABLO1},
        {"lordsofmagic", GameProfile::LORDSOFMAGIC},
        {"lomse", GameProfile::LORDSOFMAGIC},
        {"starcraft", GameProfile::STARCRAFT1},
        {"starcraft1", GameProfile::STARCRAFT1},
        {"sc", GameProfile::STARCRAFT1},
        {"sc1", GameProfile::STARCRAFT1},
        {"warcraft2", GameProfile::WARCRAFT2},
        {"wc2", GameProfile::WARCRAFT2},
        {"war2", GameProfile::WARCRAFT2},
        {"diablo2", GameProfile::DIABLO2},
        {"d2", GameProfile::DIABLO2},
        {"warcraft3", GameProfile::WARCRAFT3},
        {"wc3", GameProfile::WARCRAFT3},
        {"war3", GameProfile::WARCRAFT3},
        {"warcraft3-map", GameProfile::WARCRAFT3_MAP},
        {"wc3-map", GameProfile::WARCRAFT3_MAP},
        {"war3-map", GameProfile::WARCRAFT3_MAP},
        {"wow1", GameProfile::WOW_1X},
        {"wow-vanilla", GameProfile::WOW_1X},
        {"wow2", GameProfile::WOW_2X},
        {"wow-tbc", GameProfile::WOW_2X},
        {"wow3", GameProfile::WOW_3X},
        {"wow-wotlk", GameProfile::WOW_3X},
        {"wow4", GameProfile::WOW_4X},
        {"wow-cataclysm", GameProfile::WOW_4X},
        {"wow5", GameProfile::WOW_5X},
        {"wow-mop", GameProfile::WOW_5X},
        {"starcraft2", GameProfile::STARCRAFT2},
        {"sc2", GameProfile::STARCRAFT2},
        {"diablo3", GameProfile::DIABLO3},
        {"d3", GameProfile::DIABLO3}
    };
    return profileMap;
}

// Convert string to GameProfile enum
GameProfile GameRules::StringToProfile(const std::string& profileName) {
    const auto& profileMap = GetProfileMap();
    std::string lower = ToLower(profileName);
    auto it = profileMap.find(lower);
    if (it != profileMap.end()) {
        return it->second;
    }
    return GameProfile::GENERIC;
}

// Convert GameProfile enum to string
std::string GameRules::ProfileToString(GameProfile profile) {
    switch (profile) {
        case GameProfile::GENERIC: return "generic";
        case GameProfile::DIABLO1: return "diablo1";
        case GameProfile::LORDSOFMAGIC: return "lordsofmagic";
        case GameProfile::WARCRAFT2: return "warcraft2";
        case GameProfile::STARCRAFT1: return "starcraft1";
        case GameProfile::DIABLO2: return "diablo2";
        case GameProfile::WARCRAFT3: return "warcraft3";
        case GameProfile::WARCRAFT3_MAP: return "warcraft3-map";
        case GameProfile::WOW_1X: return "wow-vanilla";
        case GameProfile::WOW_2X: return "wow-tbc";
        case GameProfile::WOW_3X: return "wow-wotlk";
        case GameProfile::WOW_4X: return "wow-cataclysm";
        case GameProfile::WOW_5X: return "wow-mop";
        case GameProfile::STARCRAFT2: return "starcraft2";
        case GameProfile::DIABLO3: return "diablo3";
        default: return "generic";
    }
}

// Get list of canonical game profile names (for display purposes)
std::vector<std::string> GameRules::GetCanonicalProfiles() {
    // Iterate through all GameProfile enum values and get their canonical names
    std::vector<std::string> profiles;

    for (int i = static_cast<int>(GameProfile::GENERIC); i <= static_cast<int>(GameProfile::DIABLO3); ++i) {
        profiles.push_back(ProfileToString(static_cast<GameProfile>(i)));
    }

    return profiles;
}

// Get available profiles as a comma-separated string
std::string GameRules::GetAvailableProfiles() {
    auto profiles = GetCanonicalProfiles();
    std::string result;

    for (size_t i = 0; i < profiles.size(); ++i) {
        result += profiles[i];
        if (i < profiles.size() - 1) {
            result += ", ";
        }
    }

    return result;
}

// Validator for CLI11 - accepts all profile names but only displays canonical ones
const CLI::Validator GameProfileValid = CLI::Validator(
    [](const std::string &str) {
        if (str == "default") return std::string();

        // Try to convert the string to a profile
        GameProfile profile = GameRules::StringToProfile(str);

        // If it's GENERIC and the input wasn't "generic", it means the profile wasn't found
        if (profile == GameProfile::GENERIC && str != "generic") {
            std::string validProfiles = "Game profile must be one of:";
            for (const auto& p : GameRules::GetCanonicalProfiles()) {
                validProfiles += " " + p;
            }
            return validProfiles;
        }
        return std::string();
    },
    "",
    "GameProfileValidator"
);

// Initialize rules for the selected game profile
void GameRules::InitializeRules() {
    rules.clear();

    switch (profile) {
        case GameProfile::DIABLO1:
        case GameProfile::LORDSOFMAGIC:
            // File rules when adding files to archive:
            AddRuleByFileMask("*.wav", MPQ_FILE_ENCRYPTED, 0x00, 0x00);
            AddRuleByFileMask("*.smk", 0x00000000, 0x00, 0x00);
            AddRuleByFileMask("*.bik", 0x00000000, 0x00, 0x00);
            AddRuleByFileMask("*.mpq", MPQ_FILE_ENCRYPTED, 0x00, 0x00);
            AddRuleByFileMask("game", MPQ_FILE_IMPLODE, 0x00, 0x00);
            AddRuleByFileMask("hero", MPQ_FILE_IMPLODE, 0x00, 0x00);
            AddRuleDefault(MPQ_FILE_IMPLODE | MPQ_FILE_ENCRYPTED, MPQ_COMPRESSION_PKWARE);

            // Settings for archive creation:
            createSettings.mpqVersion = MPQ_FORMAT_VERSION_1;
            createSettings.sectorSize = 0x1000;
            break;

        case GameProfile::WARCRAFT2:
        case GameProfile::STARCRAFT1:
            // File rules when adding files to archive:
            AddRuleByFileMask("*.wav", MPQ_FILE_COMPRESS | MPQ_FILE_ENCRYPTED | MPQ_FILE_KEY_V2,
                              MPQ_COMPRESSION_PKWARE, MPQ_COMPRESSION_HUFFMANN | MPQ_COMPRESSION_ADPCM_STEREO);
            AddRuleByFileMask("*.smk", 0x00000000, 0x00, 0x00);
            AddRuleByFileMask("*.bik", 0x00000000, 0x00, 0x00);
            AddRuleByFileMask("*.mpq", 0x00000000, 0x00, 0x00);
            AddRuleDefault(MPQ_FILE_COMPRESS | MPQ_FILE_ENCRYPTED | MPQ_FILE_KEY_V2, MPQ_COMPRESSION_PKWARE);

            // Settings for archive creation:
            createSettings.mpqVersion = MPQ_FORMAT_VERSION_1;
            createSettings.fileFlags1 = MPQ_FILE_EXISTS | MPQ_FILE_COMPRESS | MPQ_FILE_SECTOR_CRC;
            createSettings.fileFlags2 = MPQ_FILE_EXISTS | MPQ_FILE_COMPRESS | MPQ_FILE_SECTOR_CRC;
            createSettings.sectorSize = 0x1000;
            break;

        case GameProfile::DIABLO2:
            // File rules when adding files to archive:
            AddRuleByFileMask("*.wav", MPQ_FILE_COMPRESS | MPQ_FILE_ENCRYPTED | MPQ_FILE_KEY_V2,
                              MPQ_COMPRESSION_PKWARE, MPQ_COMPRESSION_HUFFMANN | MPQ_COMPRESSION_ADPCM_STEREO);
            AddRuleByFileMask("*.d2", MPQ_FILE_COMPRESS, MPQ_COMPRESSION_PKWARE);
            AddRuleByFileMask("*.txt", MPQ_FILE_COMPRESS, MPQ_COMPRESSION_PKWARE);
            AddRuleByFileMask("*.dc6", MPQ_FILE_COMPRESS, MPQ_COMPRESSION_PKWARE);
            AddRuleByFileMask("*.tbl", MPQ_FILE_COMPRESS, MPQ_COMPRESSION_PKWARE);
            AddRuleByFileMask("*.map", MPQ_FILE_COMPRESS, MPQ_COMPRESSION_PKWARE);
            AddRuleByFileMask("*.key", MPQ_FILE_COMPRESS, MPQ_COMPRESSION_PKWARE);
            AddRuleByFileMask("*.dat", MPQ_FILE_COMPRESS, MPQ_COMPRESSION_PKWARE);
            AddRuleByFileMask("*.ds1", MPQ_FILE_COMPRESS, MPQ_COMPRESSION_PKWARE);
            AddRuleByFileMask("*.dcc", MPQ_FILE_COMPRESS, MPQ_COMPRESSION_PKWARE);
            AddRuleByFileMask("*.cof", MPQ_FILE_COMPRESS, MPQ_COMPRESSION_PKWARE);
            AddRuleByFileMask("*.dt1", MPQ_FILE_COMPRESS, MPQ_COMPRESSION_PKWARE);
            AddRuleByFileMask("*.pl2", MPQ_FILE_COMPRESS, MPQ_COMPRESSION_PKWARE);
            AddRuleByFileMask("*.dn1", MPQ_FILE_COMPRESS, MPQ_COMPRESSION_PKWARE);
            AddRuleByFileMask("*.ico", MPQ_FILE_COMPRESS, MPQ_COMPRESSION_PKWARE);
            AddRuleDefault(MPQ_FILE_COMPRESS | MPQ_FILE_ENCRYPTED | MPQ_FILE_KEY_V2, MPQ_COMPRESSION_PKWARE);

            // Settings for archive creation:
            createSettings.mpqVersion = MPQ_FORMAT_VERSION_1;
            createSettings.fileFlags1 = MPQ_FILE_EXISTS | MPQ_FILE_COMPRESS;
            createSettings.fileFlags2 = MPQ_FILE_EXISTS | MPQ_FILE_COMPRESS;
            createSettings.sectorSize = 0x1000;
            break;

        case GameProfile::WARCRAFT3:
            // File rules when adding files to archive:
            AddRuleByFileMask("Abilities\\*.wav", MPQ_FILE_COMPRESS, MPQ_COMPRESSION_ZLIB,
                              MPQ_COMPRESSION_HUFFMANN | MPQ_COMPRESSION_ADPCM_MONO);
            AddRuleByFileMask("Buildings\\*.wav", MPQ_FILE_COMPRESS, MPQ_COMPRESSION_ZLIB,
                              MPQ_COMPRESSION_HUFFMANN | MPQ_COMPRESSION_ADPCM_MONO);
            AddRuleByFileMask("*.wav", MPQ_FILE_COMPRESS | MPQ_FILE_ENCRYPTED | MPQ_FILE_KEY_V2,
                              MPQ_COMPRESSION_ZLIB, MPQ_COMPRESSION_HUFFMANN | MPQ_COMPRESSION_ADPCM_MONO);

            AddRuleByFileMask("ReplaceableTextures\\WorldEditUI\\*.blp", MPQ_FILE_COMPRESS, MPQ_COMPRESSION_ZLIB);
            AddRuleByFileMask("ReplaceableTextures\\Selection\\*.blp", MPQ_FILE_COMPRESS, MPQ_COMPRESSION_ZLIB);
            AddRuleByFileMask("ReplaceableTextures\\Shadows\\*.blp", MPQ_FILE_COMPRESS, MPQ_COMPRESSION_ZLIB);
            AddRuleByFileMask("UI\\Glues\\Loading\\Backgrounds\\*.blp", 0, 0);
            AddRuleByFileMask("UI\\Glues\\Loading\\Multiplayer\\*.blp", 0, 0);
            AddRuleByFileMask("UI\\*.blp", MPQ_FILE_COMPRESS, MPQ_COMPRESSION_ZLIB);
            AddRuleByFileMask("*.blp", 0, 0);

            AddRuleByFileMask("Maps\\Campaign\\*.w3m", 0, 0);
            AddRuleByFileMask("*.w3m", MPQ_FILE_COMPRESS | MPQ_FILE_ENCRYPTED | MPQ_FILE_KEY_V2, MPQ_COMPRESSION_PKWARE);

            AddRuleByFileMask("*.toc", MPQ_FILE_COMPRESS, MPQ_COMPRESSION_ZLIB);
            AddRuleByFileMask("*.ifl", MPQ_FILE_COMPRESS, MPQ_COMPRESSION_ZLIB);
            AddRuleByFileMask("*.mdx", MPQ_FILE_COMPRESS, MPQ_COMPRESSION_ZLIB);
            AddRuleByFileMask("*.tga", MPQ_FILE_COMPRESS, MPQ_COMPRESSION_ZLIB);
            AddRuleByFileMask("*.slk", MPQ_FILE_COMPRESS, MPQ_COMPRESSION_ZLIB);
            AddRuleByFileMask("*.ai", MPQ_FILE_COMPRESS, MPQ_COMPRESSION_ZLIB);
            AddRuleByFileMask("*.j", MPQ_FILE_COMPRESS, MPQ_COMPRESSION_ZLIB);
            AddRuleByFileMask("*.txt", MPQ_FILE_COMPRESS | MPQ_FILE_ENCRYPTED | MPQ_FILE_KEY_V2, MPQ_COMPRESSION_ZLIB);
            AddRuleByFileMask("*.fdf", MPQ_FILE_COMPRESS | MPQ_FILE_ENCRYPTED | MPQ_FILE_KEY_V2, MPQ_COMPRESSION_ZLIB);
            AddRuleByFileMask("*.pld", MPQ_FILE_COMPRESS | MPQ_FILE_ENCRYPTED | MPQ_FILE_KEY_V2, MPQ_COMPRESSION_ZLIB);
            AddRuleByFileMask("*.mid", MPQ_FILE_COMPRESS | MPQ_FILE_ENCRYPTED | MPQ_FILE_KEY_V2, MPQ_COMPRESSION_ZLIB);
            AddRuleByFileMask("*.dls", MPQ_FILE_COMPRESS | MPQ_FILE_ENCRYPTED | MPQ_FILE_KEY_V2, MPQ_COMPRESSION_ZLIB);
            AddRuleByFileMask("*.mpq", 0, 0);
            AddRuleByFileMask("*.mp3", 0, 0);

            AddRuleDefault(MPQ_FILE_COMPRESS | MPQ_FILE_ENCRYPTED | MPQ_FILE_KEY_V2, MPQ_COMPRESSION_PKWARE);

            // Settings for archive creation:
            createSettings.mpqVersion = MPQ_FORMAT_VERSION_1;
            createSettings.sectorSize = 0x1000;
            createSettings.fileFlags1 = MPQ_FILE_EXISTS | MPQ_FILE_COMPRESS;
            createSettings.fileFlags2 = MPQ_FILE_EXISTS | MPQ_FILE_COMPRESS;
            createSettings.attrFlags = MPQ_ATTRIBUTE_FILETIME | MPQ_ATTRIBUTE_CRC32;
            break;

        case GameProfile::WARCRAFT3_MAP: // Warcraft III Map files
            // File rules when adding files to archive:
            AddRuleDefault(MPQ_FILE_COMPRESS, MPQ_COMPRESSION_ZLIB);

            // Settings for archive creation:
            createSettings.mpqVersion = MPQ_FORMAT_VERSION_1;
            createSettings.sectorSize = 0x1000;
            createSettings.fileFlags1 = MPQ_FILE_EXISTS | MPQ_FILE_COMPRESS;
            createSettings.fileFlags2 = MPQ_FILE_EXISTS | MPQ_FILE_COMPRESS;
            createSettings.attrFlags = MPQ_ATTRIBUTE_FILETIME | MPQ_ATTRIBUTE_CRC32;
            break;

        case GameProfile::WOW_1X:
            // File rules when adding files to archive:
            AddRuleByFileMask("*.mp3", 0, 0);
            AddRuleDefault(MPQ_FILE_COMPRESS, MPQ_COMPRESSION_ZLIB);

            // Settings for archive creation:
            createSettings.mpqVersion = MPQ_FORMAT_VERSION_1;
            createSettings.sectorSize = 0x1000;
            createSettings.fileFlags1 = MPQ_FILE_EXISTS | MPQ_FILE_COMPRESS;
            createSettings.fileFlags2 = MPQ_FILE_EXISTS | MPQ_FILE_COMPRESS;
            createSettings.attrFlags = MPQ_ATTRIBUTE_FILETIME | MPQ_ATTRIBUTE_CRC32 | MPQ_ATTRIBUTE_MD5;
            break;

        case GameProfile::WOW_2X:
        case GameProfile::WOW_3X:
            // File rules when adding files to archive:
            AddRuleByFileMask("*.mp3", 0, 0);
            AddRuleDefault(MPQ_FILE_COMPRESS | MPQ_FILE_SECTOR_CRC, MPQ_COMPRESSION_ZLIB);

            // Settings for archive creation:
            createSettings.mpqVersion = MPQ_FORMAT_VERSION_2;
            createSettings.sectorSize = 0x1000;
            createSettings.fileFlags1 = MPQ_FILE_EXISTS | MPQ_FILE_COMPRESS;
            createSettings.fileFlags2 = MPQ_FILE_EXISTS | MPQ_FILE_COMPRESS;
            createSettings.attrFlags = MPQ_ATTRIBUTE_FILETIME | MPQ_ATTRIBUTE_CRC32 | MPQ_ATTRIBUTE_MD5;
            break;

        case GameProfile::WOW_4X:
        case GameProfile::WOW_5X:
            // File rules when adding files to archive:
            AddRuleByFileSize(0, 0, MPQ_FILE_DELETE_MARKER, 0);
            AddRuleByFileMask("*.mp3", 0, 0);
            AddRuleByFileMask("*.ogg", 0, 0);
            AddRuleByFileMask("*.ogv", 0, 0);
            AddRuleByFileSize(0, 0x4000, MPQ_FILE_COMPRESS | MPQ_FILE_SINGLE_UNIT, MPQ_COMPRESSION_ZLIB);
            AddRuleDefault(MPQ_FILE_COMPRESS | MPQ_FILE_SECTOR_CRC, MPQ_COMPRESSION_ZLIB);

            // Settings for archive creation:
            createSettings.mpqVersion = MPQ_FORMAT_VERSION_4;
            createSettings.rawChunkSize = 0x4000;
            createSettings.sectorSize = 0x4000;
            createSettings.fileFlags1 = MPQ_FILE_EXISTS | MPQ_FILE_COMPRESS;
            createSettings.fileFlags2 = MPQ_FILE_EXISTS | MPQ_FILE_COMPRESS;
            createSettings.attrFlags = MPQ_ATTRIBUTE_CRC32 | MPQ_ATTRIBUTE_MD5;
            break;

        case GameProfile::STARCRAFT2:
            // File rules when adding files to archive:
            AddRuleByFileSize(0, 0, MPQ_FILE_DELETE_MARKER, 0);
            AddRuleByFileMask("*.mp3", 0, 0);
            AddRuleByFileMask("*.ogg", 0, 0);
            AddRuleByFileMask("*.ogv", 0, 0);
            AddRuleByFileSize(0, 0x4000, MPQ_FILE_COMPRESS | MPQ_FILE_SINGLE_UNIT, MPQ_COMPRESSION_ZLIB);
            AddRuleByFileMask("*.wav", MPQ_FILE_COMPRESS, MPQ_COMPRESSION_ZLIB);
            AddRuleDefault(MPQ_FILE_COMPRESS | MPQ_FILE_SECTOR_CRC, MPQ_COMPRESSION_ZLIB);

            // Settings for archive creation:
            createSettings.mpqVersion = MPQ_FORMAT_VERSION_2;
            createSettings.sectorSize = 0x4000;
            createSettings.fileFlags1 = MPQ_FILE_EXISTS | MPQ_FILE_COMPRESS;
            createSettings.fileFlags2 = MPQ_FILE_EXISTS | MPQ_FILE_COMPRESS;
            createSettings.attrFlags = MPQ_ATTRIBUTE_CRC32 | MPQ_ATTRIBUTE_MD5;
            break;

        case GameProfile::DIABLO3:
            // File rules when adding files to archive:
            AddRuleByFileSize(0, 0, MPQ_FILE_DELETE_MARKER, 0);
            AddRuleByFileMask("*.mp3", 0, 0);
            AddRuleByFileMask("*.ogg", 0, 0);
            AddRuleByFileMask("*.ogv", 0, 0);
            AddRuleByFileSize(0, 0x4000, MPQ_FILE_COMPRESS | MPQ_FILE_SINGLE_UNIT, MPQ_COMPRESSION_ZLIB);
            AddRuleDefault(MPQ_FILE_COMPRESS, MPQ_COMPRESSION_ZLIB);

            // Settings for archive creation:
            createSettings.mpqVersion = MPQ_FORMAT_VERSION_4;
            createSettings.rawChunkSize = 0x4000;
            createSettings.sectorSize = 0x4000;
            createSettings.fileFlags1 = MPQ_FILE_EXISTS | MPQ_FILE_COMPRESS;
            createSettings.fileFlags2 = MPQ_FILE_EXISTS | MPQ_FILE_COMPRESS;
            createSettings.attrFlags = MPQ_ATTRIBUTE_CRC32 | MPQ_ATTRIBUTE_MD5;
            break;

        case GameProfile::GENERIC:
        default:
            // File rules when adding files to archive:
            AddRuleDefault(MPQ_FILE_COMPRESS | MPQ_FILE_ENCRYPTED, MPQ_COMPRESSION_PKWARE);

            // For settings for archive creation, use defaults from MpqCreateSettings constructor
            break;
    }
}
