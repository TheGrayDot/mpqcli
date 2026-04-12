#ifndef VALIDATORS_H
#define VALIDATORS_H

#include <CLI/CLI.hpp>

#include "gamerules.h"
#include "locales.h"

// Defined in gamerules.cpp
extern const CLI::Validator GameProfileValid;

// Inline locale validator
inline const auto LocaleValid = CLI::Validator(
    [](const std::string &str) {
        if (str == "default") return std::string();

        if (ParseHexLocale(str) != defaultLocale) {
            return std::string();
        }

        const LCID locale = LangToLocale(str);
        if (locale == 0) {
            std::string validLocales = "Locale must be nothing, or one of:";
            for (const auto &l : GetAllLocales()) {
                validLocales += " " + l;
            }
            return validLocales;
        }
        return std::string();
    },
    "", "LocaleValidator");

#endif  // VALIDATORS_H
