#include "SystemInfo.hpp"
#include "util/Utils.hpp"
#include <qclipboard.h>
#include <array>
#include <qfiledevice.h>
#include <qimage.h>
#include <qobject.h>
#include <qscreen.h>
#include <qfile.h>

#include <qguiapplication.h>
#include <qtenvironmentvariables.h>
#include <qwindowdefs.h>
#include <unistd.h>
#include <sys/utsname.h>

CSystemInternals::CSystemInternals(QObject* parent) : QObject(parent) {
    // gather data from os-release
    {
        QFile osInfo("/etc/os-release");
        if (osInfo.open(QIODevice::ReadOnly | QIODevice::Text)) {
            auto stream = QTextStream(&osInfo);

            while (!stream.atEnd()) {
                auto line  = stream.readLine();
                auto split = line.split('=');
                if (split.length() != 2)
                    continue;

                const auto& key   = split.at(0);
                auto        value = split.at(1);

                if (value.length() >= 2 && value.at(0) == '\"')
                    value = value.sliced(1).chopped(1);

                if (key == "PRETTY_NAME") {
                    systemName = value;
                    continue;
                }

                if (key == "HOME_URL") {
                    systemURL = value;
                    continue;
                }

                if (key == "LOGO") {
                    systemLogoName = value;
                    continue;
                }
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
        auto tagIndex   = hlSystemVersion.indexOf("Tag: ");

        if (tagIndex != -1) {
            hyprlandVersionLong = hlSystemVersion.sliced(tagIndex + 5);
            auto commaIndex     = hyprlandVersionLong.indexOf(',');
            if (commaIndex != -1)
                hyprlandVersionLong = hyprlandVersionLong.first(commaIndex);

            auto dashIndex = hyprlandVersionLong.indexOf('-');
            if (dashIndex == -1)
                hyprlandVersion = hyprlandVersionLong;
            else
                hyprlandVersion = hyprlandVersionLong.first(dashIndex);
        }

        if (hyprlandVersionLong.isEmpty() && hlSystemVersion.contains("at commit")) {
            auto commitIndex = hlSystemVersion.indexOf("at commit ");
            if (commitIndex != -1) {
                hyprlandVersionLong = hlSystemVersion.sliced(commitIndex + 10);

                hyprlandVersionLong = substrUntil(hyprlandVersionLong, ' ').left(7);
                hyprlandVersion     = hyprlandVersionLong;
            }
        }

        if (hyprlandVersionLong.isEmpty()) {
            hyprlandVersionLong = "unknown";
            hyprlandVersion     = "unknown";
        }

        hlSystemInfo = execAndGet("hyprctl", {"systeminfo"});
        if (!hlSystemInfo.contains("Hyprland"))
            hlSystemInfo = "";
    }

    // get cpu info
    {
        auto DATA = execAndGet("lscpu");
        if (DATA.contains("odel name")) {
            QString arch, model, ghz, nproc;
            auto    textStream = QTextStream(&DATA);

            while (!textStream.atEnd()) {
                auto line         = textStream.readLine();
                auto indexOfColon = line.indexOf(':');
                if (indexOfColon == -1)
                    continue;

                auto left  = line.sliced(0, indexOfColon).trimmed();
                auto right = line.sliced(indexOfColon + 1).trimmed();

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
                    auto ok       = false;
                    auto ghzFloat = right.toFloat(&ok);

                    if (ok)
                        ghz = QString("%1Ghz").arg(ghzFloat / 1000.F, 2, 'g', 2);
                    else
                        ghz = "?Ghz";

                    continue;
                }
            }

            cpuInfo = QString("%1 at %2x%3 (%4)").arg(model).arg(nproc).arg(ghz).arg(arch);
        }
    }

    // get gpu info
    {
        auto DATA       = execAndGet("lspci", {"-vnn"});
        auto textStream = QTextStream(&DATA);

        if (!DATA.isEmpty())
            gpuInfo.clear();

        while (!textStream.atEnd()) {
            auto line = textStream.readLine();

            auto indexOfVga = line.indexOf("VGA");
            if (indexOfVga == -1)
                continue;

            line = line.sliced(indexOfVga);

            auto indexOfColon = line.indexOf(": ");
            if (indexOfColon == -1)
                continue;

            line = line.sliced(indexOfColon + 2);

            gpuInfo.emplaceBack(line);
        }

        if (gpuInfo.isEmpty()) {
            gpuInfo.emplaceBack("No GPUs found");
        }
    }

    // get ram info
    {
        auto DATA       = execAndGet("free");
        auto textStream = QTextStream(&DATA);

        if (textStream.readLine().contains("total")) {
            auto ramIntToReadable = [](QStringView datapoint) -> QString {
                auto ok       = false;
                auto ghzFloat = datapoint.toULongLong(&ok);

                if (ok)
                    return QString("%1GB").arg(ghzFloat / 1000000.0, 3, 'g', 3);
                else
                    return "[error]";
            };

            auto props = textStream.readLine().simplified().split(' ');
            ramInfo    = QString("%1 / %2").arg(ramIntToReadable(props[2])).arg(ramIntToReadable(props[1]));
        }
    }

    // other, misc
    if (auto DE = qEnvironmentVariable("XDG_CURRENT_DESKTOP"); !DE.isEmpty())
        this->DE = DE;

    {
        QFile uptimeFile("/proc/uptime");
        if (uptimeFile.open(QFile::ReadOnly | QFile::Text)) {
            auto ok     = false;
            auto uptime = substrUntil(uptimeFile.readAll(), ' ').toFloat(&ok);

            if (ok) {
                int     uptimeSeconds = std::round(uptime);
                int     uptimeDays    = std::floor(uptimeSeconds / 3600.0 / 24.0);
                int     uptimeHours   = std::floor((uptimeSeconds % (3600 * 24)) / 3600.0);
                int     uptimeMinutes = std::floor((uptimeSeconds % (3600)) / 60.0);

                QString upStr;
                auto    textStream = QTextStream(&upStr);

                if (uptimeDays > 0)
                    textStream << uptimeDays << " days, ";

                if (uptimeHours > 0)
                    textStream << uptimeHours << " hours, ";

                if (uptimeMinutes > 0)
                    textStream << uptimeMinutes << " minutes, ";

                if (!upStr.isEmpty())
                    upStr.chop(2);

                this->uptime = upStr;
            }
        }
    }

    {
        QString     screens;
        QTextStream textStream(&screens);
        for (auto* s : QGuiApplication::screens()) {
            textStream << s->name() << ' ' << '(' << s->geometry().width() << 'x' << s->geometry().height() << "), ";
        }

        if (!screens.isEmpty())
            screens.chop(2);

        this->screens = screens;
    }

    if (auto* username = getlogin()) {
        std::array<char, 128> hostname;
        if (gethostname(hostname.data(), hostname.size()) == 0)
            user = QString("%1@%2").arg(QLatin1StringView(username)).arg(QLatin1StringView(hostname.data()));
    }

    {
        QFile productFile("/sys/devices/virtual/dmi/id/product_name");
        if (productFile.open(QFile::ReadOnly | QFile::Text))
            board = productFile.readAll().trimmed();
        else {
            productFile.setFileName("/sys/devices/virtual/dmi/id/board_name");
            if (productFile.open(QFile::ReadOnly | QFile::Text))
                board = productFile.readAll().trimmed();
        }
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
