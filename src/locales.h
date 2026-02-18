#ifndef LOCALES_H
#define LOCALES_H

#include <string>
#include <filesystem>

#include <StormLib.h>
#include <CLI/CLI.hpp>

const LCID defaultLocale = 0;

std::string LocaleToLang(uint16_t locale);
LCID LangToLocale(const std::string &lang);
LCID ParseHexLocale(const std::string& str);
std::vector<std::string> GetAllLocales();
std::string PrettyPrintLocale(LCID locale, const std::string &prefix = "", bool alwaysPrint = false);

// Validator for CLI11
const inline auto LocaleValid = CLI::Validator(
        [](const std::string &str) {
            if (str == "default") return std::string();

            // Check if it's a 4-character hexadecimal string
            if (ParseHexLocale(str) != defaultLocale) {
                return std::string();
            }

            const LCID locale = LangToLocale(str);
            if (locale == 0) {
                std::string validLocales = "Locale must be nothing, or one of:";
                for (const auto& l : GetAllLocales()) {
                    validLocales += " " + l;
                }
                return validLocales;
            }
            return std::string();
        },
        "",
        "LocaleValidator"
);

#endif //LOCALES_H
