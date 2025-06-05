#include "SystemInfo.hpp"
#include "WaylandScreen.hpp"
#include "util/Utils.hpp"
#include <qclipboard.h>
#include <array>
#include <qcontainerfwd.h>
#include <qfiledevice.h>
#include <qimage.h>
#include <qobject.h>
#include <qscreen.h>
#include <qfile.h>
#include <qstringliteral.h>
#include <string>
#include <format>

#include <qguiapplication.h>
#include <qtenvironmentvariables.h>
#include <qwindowdefs.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <hyprutils/string/VarList.hpp>
#include <hyprutils/string/String.hpp>
#include <cstdlib>

using namespace Hyprutils::String;

CSystemInternals::CSystemInternals(QObject* parent) : QObject(parent) {
    setenv("LC_ALL", "en_US.UTF-8", true);

    // gather data from os-release
    if (auto data = readFile("/etc/os-release")) {
        CVarList lines(data.value(), 0, '\n');

        for (auto& line : lines) {
            CVarList    param(line, 2, '=');

            const auto& key   = param[0];
            auto        value = param[1];

            if (value.length() >= 2 && value.at(0) == '\"')
                value = value.substr(1, value.length() - 2);

            if (key == "PRETTY_NAME") {
                systemName = QString::fromLocal8Bit(value);
                continue;
            }

            if (key == "HOME_URL") {
                systemURL = QString::fromLocal8Bit(value);
                continue;
            }

            if (key == "LOGO") {
                systemLogoName = QString::fromLocal8Bit(value);
                continue;
            }
        }
    }

    // get kernel ver
    utsname unamebuf;
    if (uname(&unamebuf) == 0) {
        systemKernel = unamebuf.release;
    }

    // get hyprland info
    if (getenv("HYPRLAND_INSTANCE_SIGNATURE")) {
        hlSystemVersion = execAndGet("hyprctl", {"version"});
        auto DATA       = hlSystemVersion.toStdString();

        if (DATA.contains("Tag: ")) {
            auto temp           = DATA.substr(DATA.find("Tag: ") + 5);
            temp                = temp.substr(0, temp.find(","));
            hyprlandVersionLong = QString::fromLocal8Bit(temp);
            hyprlandVersion     = QString::fromLocal8Bit(temp.substr(0, temp.find("-")));
        }

        if (hyprlandVersionLong.length() <= 0 && DATA.contains("at commit")) {
            auto temp           = DATA.substr(DATA.find("at commit") + 10);
            temp                = temp.substr(0, temp.find(" "));
            hyprlandVersionLong = QString::fromLocal8Bit(temp.substr(0, 7));
            hyprlandVersion     = QString::fromLocal8Bit(temp.substr(0, 7));
        }

        if (hyprlandVersionLong.isEmpty()) {
            hyprlandVersionLong = QStringLiteral("unknown");
            hyprlandVersion     = QStringLiteral("unknown");
        }

        hlSystemInfo = execAndGet("hyprctl", {"systeminfo"});
        if (!hlSystemInfo.contains("Hyprland"))
            hlSystemInfo = "";
    }

    // get cpu info
    {
        const auto DATA = execAndGet("lscpu").toStdString();
        if (DATA.contains("odel name")) {
            std::string arch, model, ghz, nproc;

            CVarList    data(DATA, 0, '\n');
            for (auto& line : data) {
                std::string left, right;
                left  = trim(line.substr(0, line.find(":")));
                right = trim(line.substr(line.find(":") + 1));

                if (left == "Architecture") {
                    arch = right;
                    continue;
                }
                if (left == "Model name") {
                    model = right;
                    continue;
                }
                if (left == "CPU(s)") {
                    nproc = right;
                    continue;
                }
                if (left == "CPU max MHz") {
                    try {
                        ghz = std::format("{:.02}GHz", std::stof(right) / 1000.F);
                    } catch (...) { ghz = "?GHz"; }
                    continue;
                }
            }

            cpuInfo = QString::fromLocal8Bit(std::format("{} at {}x{} ({})", model, nproc, ghz, arch));
        }
    }

    // get gpu info
    {
        auto       ok   = false;
        const auto DATA = execAndGet("lspci", {"-vnn"}, &ok).toStdString();
        CVarList   lines(DATA, 0, '\n');

        if (ok) {
            for (auto& line : lines) {
                if (!line.contains("VGA"))
                    continue;
                gpuInfo.emplace_back(QString::fromLocal8Bit(std::format("{}", line.substr(line.find(":", line.find("VGA")) + 2))));
            }

            if (gpuInfo.isEmpty())
                gpuInfo.emplaceBack(QStringLiteral("No GPUs found"));
        } else
            gpuInfo.emplaceBack(QStringLiteral("missing dependency: lspci"));
    }

    // get ram info
    {
        const auto DATA = execAndGet("free").toStdString();
        if (DATA.contains("total")) {
            CVarList data(DATA, 0, '\n');

            auto     ramIntToReadable = [](const std::string& datapoint) -> std::string {
                try {
                    auto asInt = std::stoull(datapoint);
                    return std::format("{:.03}GB", asInt / 1000000.0);
                } catch (...) { return "[error]"; }
            };

            CVarList props(data[1], 0, 's', true);

            ramInfo = QString::fromLocal8Bit(std::format("{} / {}", ramIntToReadable(props[2]), ramIntToReadable(props[1])));
        }
    }

    // other, misc
    if (auto current = qEnvironmentVariable("XDG_CURRENT_DESKTOP"); !current.isEmpty())
        DE = current;

    if (auto procUptime = readFile("/proc/uptime")) {
        CVarList data(procUptime.value(), 0, 's', true);

        try {
            int  uptimeSeconds = std::round(std::stof(data[0]));
            int  uptimeDays    = std::floor(uptimeSeconds / 3600.0 / 24.0);
            int  uptimeHours   = std::floor((uptimeSeconds % (3600 * 24)) / 3600.0);
            int  uptimeMinutes = std::floor((uptimeSeconds % (3600)) / 60.0);

            auto upStr = std::format("{}{}{}", (uptimeDays > 0 ? std::format("{} days, ", uptimeDays) : ""), (uptimeHours > 0 ? std::format("{} hours, ", uptimeHours) : ""),
                                     (uptimeMinutes > 0 ? std::format("{} minutes, ", uptimeMinutes) : ""));

            if (!upStr.empty())
                upStr = upStr.substr(0, upStr.length() - 2);

            uptime = QString::fromLocal8Bit(upStr);
        } catch (...) { ; }
    }

    {
        std::string screens;
        for (const auto& s : SWaylandScreenInfo::enumerateScreens()) {
            screens += std::format("{} ({}x{}), ", s.name.toStdString(), s.pixelSize.width(), s.pixelSize.height());
        }

        if (!screens.empty())
            screens = screens.substr(0, screens.length() - 2);

        this->screens = QString::fromLocal8Bit(screens);
    }

    if (auto* username = getlogin()) {
        std::array<char, 128> hostname;
        if (gethostname(hostname.data(), hostname.size()) == 0)
            user = QString::fromLocal8Bit(std::format("{}@{}", username, hostname.data()));
    }

    {
        if (auto productName = readFile("/sys/devices/virtual/dmi/id/product_name"))
            board = QString::fromLocal8Bit(trim(productName.value()));
        else if (auto boardName = readFile("/sys/devices/virtual/dmi/id/board_name"))
            board = QString::fromLocal8Bit(trim(boardName.value()));
    }
}

void CSystemInternals::copySystemInfo() {
    QGuiApplication::clipboard()->setText(hlSystemInfo);
    execAndGet("hyprctl", {"notify", "5", "5000", "0", "Copied system info to the clipboard."});
}

void CSystemInternals::copyVersion() {
    QGuiApplication::clipboard()->setText(hlSystemVersion);
    execAndGet("hyprctl", {"notify", "5", "5000", "0", "Copied version info to the clipboard."});
}
