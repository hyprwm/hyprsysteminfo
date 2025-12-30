#pragma once

#include <expected>
#include <string>
#include <optional>
#include <filesystem>
#include <vector>

namespace Info {
    std::expected<std::string, std::string> getDistroLogoName();
    std::optional<std::string>              getFromEtcOsRelease(const std::string_view& sv);
    std::string                             kernel();
    std::string                             desktop();
    std::string                             desktopHome();
    std::string                             desktopVersion();
    std::string                             user();
    std::optional<std::string>              cat(const std::filesystem::path& p);
    std::string                             model();
    std::string                             cpu();
    std::vector<std::string>                gpus();
    std::string                             mem();
    std::string                             uptime();
    std::optional<std::span<const uint8_t>> deLogo();
};