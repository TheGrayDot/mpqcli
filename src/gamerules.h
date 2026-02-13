#ifndef GAMERULES_H
#define GAMERULES_H

#include <string>
#include <vector>
#include <utility>
#include <optional>
#include <StormLib.h>
#include <CLI/CLI.hpp>

enum class GameProfile {
    GENERIC,              // Default/generic MPQ with basic compression
    DIABLO1,              // Diablo I / Hellfire (1997)
    LORDSOFMAGIC,         // Lords of Magic SE (1998)
    STARCRAFT1,           // StarCraft / Brood War (1998)
    WARCRAFT2,            // Warcraft II: Battle.net Edition (1999)
    DIABLO2,              // Diablo II / Lords of Destruction (2000)
    WARCRAFT3,            // Warcraft III / The Frozen Throne (2002)
    WARCRAFT3_MAP,        // Warcraft III Map files (2002)
    WOW_1X,               // World of Warcraft 1 - Vanilla (2004)
    WOW_2X,               // World of Warcraft 2 - The Burning Crusade (2007)
    WOW_3X,               // World of Warcraft 3 - Wrath of the Lich King (2008)
    WOW_4X,               // World of Warcraft 4 - Cataclysm (2010)
    WOW_5X,               // World of Warcraft 5 - Mists of Pandaria (2012)
    STARCRAFT2,           // StarCraft II (2010)
    DIABLO3               // Diablo III (2012)
};

enum class RuleType {
    FILE_MASK,            // Rule based on file pattern (e.g., "*.wav")
    FILE_SIZE,            // Rule based on file size range
    DEFAULT               // Default rule (fallback)
};

// Structure representing a single compression rule
struct CompressionRule {
    RuleType type;
    std::string fileMask;         // For FILE_MASK rules (e.g., "*.wav", "UI\\*.blp")
    DWORD sizeMin;                // For FILE_SIZE rules
    DWORD sizeMax;                // For FILE_SIZE rules
    DWORD mpqFlags;               // MPQ file flags (compression, encryption, etc.)
    DWORD compressionFirst;       // Compression for first sector
    DWORD compressionNext;        // Compression for subsequent sectors

    CompressionRule(std::string mask, const DWORD flags, const DWORD compFirst, const DWORD compNext = MPQ_COMPRESSION_NEXT_SAME)
        : type(RuleType::FILE_MASK), fileMask(std::move(mask)), sizeMin(0), sizeMax(0),
          mpqFlags(flags), compressionFirst(compFirst), compressionNext(compNext) {}

    CompressionRule(const DWORD minSize, const DWORD maxSize, const DWORD flags, const DWORD compFirst, const DWORD compNext = MPQ_COMPRESSION_NEXT_SAME)
        : type(RuleType::FILE_SIZE), fileMask(""), sizeMin(minSize), sizeMax(maxSize),
          mpqFlags(flags), compressionFirst(compFirst), compressionNext(compNext) {}

    CompressionRule(const DWORD flags, const DWORD compFirst, const DWORD compNext = MPQ_COMPRESSION_NEXT_SAME)
        : type(RuleType::DEFAULT), fileMask(""), sizeMin(0), sizeMax(0),
          mpqFlags(flags), compressionFirst(compFirst), compressionNext(compNext) {}
};

// Structure to hold compression settings for a file
struct CompressionSettings {
    DWORD mpqFlags;
    DWORD compressionFirst;
    DWORD compressionNext;
};

// Structure to hold optional override settings for adding files
struct CompressionSettingsOverrides {
    std::optional<DWORD> dwFlags;
    std::optional<DWORD> dwCompression;
    std::optional<DWORD> dwCompressionNext;
};

// Structure to hold MPQ archive creation settings
struct MpqCreateSettings {
    DWORD mpqVersion;         // MPQ format version (1, 2, 3, or 4)
    DWORD streamFlags;        // Stream flags (e.g., STREAM_PROVIDER_FLAT)
    DWORD fileFlags1;         // File flags for (listfile)
    DWORD fileFlags2;         // File flags for (attributes)
    DWORD fileFlags3;         // File flags for (signature)
    DWORD attrFlags;          // Attribute flags (CRC32, FILETIME, MD5, etc.)
    DWORD sectorSize;         // Sector size (typically 0x1000 or 0x4000)
    DWORD rawChunkSize;       // Raw chunk size (for MPQ v4, typically 0x4000)

    // Constructor with defaults
    MpqCreateSettings()
        : mpqVersion(MPQ_FORMAT_VERSION_1),
          streamFlags(STREAM_PROVIDER_FLAT | BASE_PROVIDER_FILE),
          fileFlags1(MPQ_FILE_DEFAULT_INTERNAL),
          fileFlags2(0),
          fileFlags3(MPQ_FILE_DEFAULT_INTERNAL),
          attrFlags(0),
          sectorSize(0x1000),
          rawChunkSize(0) {}
};

// Structure to hold optional override settings for MPQ archive creation
struct MpqCreateSettingsOverrides {
    std::optional<DWORD> mpqVersion;
    std::optional<DWORD> streamFlags;
    std::optional<DWORD> fileFlags1;
    std::optional<DWORD> fileFlags2;
    std::optional<DWORD> fileFlags3;
    std::optional<DWORD> attrFlags;
    std::optional<DWORD> sectorSize;
    std::optional<DWORD> rawChunkSize;
};

// Game rules class that manages compression rules for different games
class GameRules {
private:
    GameProfile profile;
    std::vector<CompressionRule> rules;
    MpqCreateSettings createSettings;

    // Helper function to match file mask pattern
    static bool MatchFileMask(const std::string& filename, const std::string& mask);

    // Add rule by file mask
    void AddRuleByFileMask(const std::string& fileMask, DWORD mpqFlags, DWORD compressionFirst, DWORD compressionNext = MPQ_COMPRESSION_NEXT_SAME);

    // Add rule by file size
    void AddRuleByFileSize(DWORD sizeMin, DWORD sizeMax, DWORD mpqFlags, DWORD compressionFirst, DWORD compressionNext = MPQ_COMPRESSION_NEXT_SAME);

    // Add default rule
    void AddRuleDefault(DWORD mpqFlags, DWORD compressionFirst, DWORD compressionNext = MPQ_COMPRESSION_NEXT_SAME);

    // Initialize rules for the selected game profile
    void InitializeRules();

    // Convert GameProfile enum to string
    static std::string ProfileToString(GameProfile profile);

public:
    // Constructor
    explicit GameRules(GameProfile gameProfile);

    // Get compression settings for a specific file
    [[nodiscard]] CompressionSettings GetCompressionSettings(const std::string& filename, DWORD fileSize) const;

    // Get MPQ creation settings
    [[nodiscard]] const MpqCreateSettings& GetCreateSettings() const { return createSettings; }

    // Override MPQ creation settings
    void OverrideCreateSettings(const MpqCreateSettingsOverrides& overrides);

    // Convert string to GameProfile enum
    static GameProfile StringToProfile(const std::string& profileName);

    // Get list of canonical game profile names (for display purposes)
    static std::vector<std::string> GetCanonicalProfiles();

    // Get available profiles as a comma-separated string
    static std::string GetAvailableProfiles();

    // Get default game profile (GENERIC)
    static GameProfile GetDefaultProfile() { return GameProfile::GENERIC; }
};

// Validator for CLI11 - accepts all profile names but only displays canonical ones
extern const CLI::Validator GameProfileValid;

#endif // GAMERULES_H
