#include "SystemInternals.hpp"
#include "src/util/Utils.hpp"
#include <format>

void CSystemInternals::copySystemInfo() {
    execAndGet(std::format("echo '{}' | wl-copy && hyprctl notify 5 5000 0 'Copied system info to the clipboard.'", hlSystemInfo.toStdString()).c_str());
}

void CSystemInternals::copyVersion() {
    execAndGet(std::format("echo '{}' | wl-copy && hyprctl notify 5 5000 0 'Copied version info to the clipboard.'", hlSystemVersion.toStdString()).c_str());
}