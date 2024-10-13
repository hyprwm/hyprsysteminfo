#include "qmlSources/SystemIconProvider.hpp"
#include "qmlSources/SystemInternals.hpp"
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QScreen>
#include <fstream>
#include <format>
#include "util/Utils.hpp"

#include <hyprutils/string/VarList.hpp>
#include <hyprutils/string/String.hpp>
#include <qtenvironmentvariables.h>
using namespace Hyprutils::String;

static std::string readFile(const std::string& filename) {
    try {
        std::ifstream ifs(filename);
        if (ifs.good()) {
            std::string data(std::istreambuf_iterator<char>{ifs}, {});
            ifs.close();
            return trim(data);
        }
    } catch (...) { return "[error]"; }
    return "[error]";
}

static void getSystemInfo(CSystemInternals* hsi, QGuiApplication* app) {

    // gather data from os-release
    std::ifstream osInfo("/etc/os-release");
    if (osInfo.good()) {
        std::string data(std::istreambuf_iterator<char>{osInfo}, {});

        osInfo.close();

        CVarList lines(data, 0, '\n');

        for (auto& line : lines) {
            CVarList    param(line, 2, '=');

            static auto stripName = [](const std::string& in) -> std::string { return in.length() > 0 && in.at(0) == '\"' ? in.substr(1, in.length() - 2) : in; };

            if (param[0] == "PRETTY_NAME") {
                hsi->systemName = stripName(param[1]).c_str();
                continue;
            }

            if (param[0] == "HOME_URL") {
                hsi->systemURL = stripName(param[1]).c_str();
                continue;
            }

            if (param[0] == "LOGO") {
                hsi->systemLogoName = stripName(param[1]).c_str();
                continue;
            }
        }
    }

    // get kernel ver
    hsi->systemKernel = trim(execAndGet("uname -r")).c_str();

    // get hyprland info
    if (getenv("HYPRLAND_INSTANCE_SIGNATURE")) {
        const auto DATA      = execAndGet("hyprctl version");
        hsi->hlSystemVersion = DATA.c_str();

        if (DATA.contains("Tag:")) {
            auto temp                = DATA.substr(DATA.find("Tag:") + 5);
            temp                     = temp.substr(0, temp.find(","));
            hsi->hyprlandVersionLong = temp.c_str();
            hsi->hyprlandVersion     = temp.substr(0, temp.find("-")).c_str();
        }

        if (hsi->hyprlandVersionLong.length() <= 0 && DATA.contains("at commit")) {
            auto temp                = DATA.substr(DATA.find("at commit") + 10);
            temp                     = temp.substr(0, temp.find(" "));
            hsi->hyprlandVersionLong = temp.substr(0, 7).c_str();
            hsi->hyprlandVersion     = temp.substr(0, 7).c_str();
        }

        if (hsi->hyprlandVersionLong.length() <= 0) {
            hsi->hyprlandVersionLong = "unknown";
            hsi->hyprlandVersion     = "unknown";
        }

        const auto DATA2 = execAndGet("hyprctl systeminfo");
        if (DATA2.contains("Hyprland"))
            hsi->hlSystemInfo = DATA2.c_str();
    }

    // get cpu info
    {
        const auto DATA = execAndGet("lscpu");
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

            hsi->cpuInfo = std::format("{} at {}x{} ({})", model, nproc, ghz, arch).c_str();
        }
    }

    // get gpu info
    {
        const auto DATA = execAndGet("lspci -vnn | grep VGA");
        if (DATA.contains("VGA")) {
            CVarList data(DATA, 0, '\n');
            hsi->gpuInfo.clear();
            for (auto& line : data) {
                if (!line.contains("VGA"))
                    continue;
                hsi->gpuInfo.emplace_back(std::format("{}", line.substr(line.find(":", line.find("VGA")) + 2)).c_str());
            }
        } else {
            hsi->gpuInfo = {"No GPUs found"};
        }
    }

    // get ram info
    {
        const auto DATA = execAndGet("free");
        if (DATA.contains("total")) {
            CVarList data(DATA, 0, '\n');

            auto     ramIntToReadable = [](const std::string& datapoint) -> std::string {
                try {
                    auto asInt = std::stoull(datapoint);
                    return std::format("{:.03}GB", asInt / 1000000.0);
                } catch (...) { return "[error]"; }
            };

            CVarList props(data[1], 0, 's', true);

            hsi->ramInfo = std::format("{} / {}", ramIntToReadable(props[2]), ramIntToReadable(props[1])).c_str();
        }
    }

    // other, misc
    if (const auto DE = getenv("XDG_CURRENT_DESKTOP"); DE)
        hsi->DE = DE;

    if (const auto UPTIME = readFile("/proc/uptime"); UPTIME != "[error]") {
        CVarList data(UPTIME, 0, 's', true);
        try {
            int  uptimeSeconds = std::round(std::stof(data[0]));
            int  uptimeDays    = std::floor(uptimeSeconds / 3600.0 / 24.0);
            int  uptimeHours   = std::floor((uptimeSeconds % (3600 * 24)) / 3600.0);
            int  uptimeMinutes = std::floor((uptimeSeconds % (3600)) / 60.0);

            auto upStr = std::format("{}{}{}", (uptimeDays > 0 ? std::format("{} days, ", uptimeDays) : ""), (uptimeHours > 0 ? std::format("{} hours, ", uptimeHours) : ""),
                                     (uptimeMinutes > 0 ? std::format("{} minutes, ", uptimeMinutes) : ""));

            if (!upStr.empty())
                upStr = upStr.substr(0, upStr.length() - 2);

            hsi->uptime = upStr.c_str();
        } catch (...) { ; }
    }

    {
        std::string screens;

        for (auto& s : app->screens()) {
            screens += std::format("{} ({}x{}), ", s->name().toStdString(), s->geometry().width(), s->geometry().height());
        }

        if (!screens.empty())
            screens = screens.substr(0, screens.length() - 2);

        hsi->screens = screens.c_str();
    }

    hsi->user = std::format("{}@{}", trim(execAndGet("whoami")), readFile("/etc/hostname")).c_str();

    if (const auto PRODUCT = readFile("/sys/devices/virtual/dmi/id/product_name"); PRODUCT != "[error]")
        hsi->board = PRODUCT.c_str();
    else if (const auto BOARD = readFile("/sys/devices/virtual/dmi/id/board_name"); BOARD != "[error]")
        hsi->board = BOARD.c_str();
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    app.setApplicationName("Hyprland System Info");

    if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE"))
        QQuickStyle::setStyle("org.kde.desktop");

    auto systemInternals = new CSystemInternals;

    getSystemInfo(systemInternals, &app);

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("hsi", systemInternals);
    engine.addImageProvider("systemIcons", new CSystemIconProvider);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed, &app, []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);
    engine.load(QUrl{u"qrc:/qt/qml/hsi/qml/main.qml"_qs});

    return app.exec();
}
