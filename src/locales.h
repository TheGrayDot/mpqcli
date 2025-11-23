#ifndef LOCALES_H
#define LOCALES_H

#include <string>
#include <filesystem>

#include <StormLib.h>
#include <CLI/CLI.hpp>

const LCID defaultLocale = 0;

std::string LocaleToLang(uint16_t locale);
LCID LangToLocale(const std::string &lang);
std::vector<std::string> GetAllLocales();

// Validator for CLI11
const inline auto LocaleValid = CLI::Validator(
        [](const std::string &str) {
            if (str == "default") return std::string();

            LCID locale = LangToLocale(str);
            if (locale == 0) {
                std::string validLocales = "Locale must be nothing, or one of:";
                for (const auto& l : GetAllLocales()) {
                    validLocales += " " + l;
                }
                return validLocales;
            }
            return std::string();
        },
        "Validates locales and outputs valid locales",
        "LocaleValidator"
);

#endif //LOCALES_H
