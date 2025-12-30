#pragma once

#include <string>
#include <expected>

namespace HyprlandIPC {
    std::expected<std::string, std::string> getFromSocket(const std::string& cmd);
};
