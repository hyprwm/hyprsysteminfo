#include "SystemInfo.hpp"

#include <optional>
#include <format>
#include <algorithm>

#include <hyprutils/os/File.hpp>
#include <hyprutils/string/VarList2.hpp>
#include <hyprutils/string/String.hpp>

#include "HyprlandIPC.hpp"
#include "../helpers/Logger.hpp"
#include "../icons/Icons.hpp"

#include <sys/utsname.h>
#include <glaze/glaze.hpp>
#include <pwd.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <linux/sysinfo.h>
#include <unistd.h>
#include <glob.h>
#include <pci/pci.h>

using namespace Hyprutils::File;
using namespace Hyprutils::String;

std::optional<std::string> Info::getFromEtcOsRelease(const std::string_view& sv) {
    static std::string content = "";
    static bool        once    = true;

    if (once) {
        once = false;

        auto read = readFileAsString("/etc/os-release");
        content   = read.value_or("");
    }

    static CVarList2 vars(std::move(content), 0, '\n', true);

    for (const auto& v : vars) {
        if (v.starts_with(sv) && v.contains('=')) {
            // found
            auto value = trim(v.substr(v.find('=') + 1));

            if (value.back() == value.front() && value.back() == '"')
                value = value.substr(1, value.size() - 2);

            return std::string{value};
        }
    }

    return std::nullopt;
}

std::expected<std::string, std::string> Info::getDistroLogoName() {
    auto v = getFromEtcOsRelease("LOGO");
    if (!v)
        return std::unexpected("Prop missing");
    return *v;
}

std::string Info::kernel() {
    struct utsname un;

    if (::uname(&un) != 0)
        return "";

    return std::string(un.release);
}

std::string Info::desktop() {
    static std::string x = [] -> std::string {
        auto ENV = getenv("XDG_CURRENT_DESKTOP");

        if (!ENV)
            return "Unknown";

        if (!std::string_view{ENV}.contains(':'))
            return ENV;

        CVarList2 vl(ENV, 0, ':', true);

        return std::string{vl[0]};
    }();
    return x;
}

std::string Info::desktopHome() {
    static std::string x = [] -> std::string {
        auto D = desktop();
        std::ranges::transform(D, D.begin(), ::tolower);

        if (D == "hyprland")
            return "https://hypr.land/";
        if (D == "sway")
            return "https://swaywm.org/";
        if (D == "wayfire")
            return "https://wayfire.org/";
        if (D == "kde")
            return "https://kde.org/";
        if (D == "gnome")
            return "https://gnome.org/";

        return "";
    }();
    return x;
}

std::string Info::desktopVersion() {
    static std::string x = [] -> std::string {
        auto D = desktop();
        std::ranges::transform(D, D.begin(), ::tolower);

        if (D == "hyprland") {
            auto res = HyprlandIPC::getFromSocket("j/version");

            if (res) {
                auto json = glz::read_json<glz::generic>(*res);
                if (json && json->contains("tag"))
                    return (*json)["tag"].get_string();
            }
        }

        return "";
    }();
    return x;
}

std::string Info::user() {
    std::string   username, host;

    uid_t         uid = geteuid();
    struct passwd pwd, *result = NULL;
    char          buf[16384];

    if (getpwuid_r(uid, &pwd, buf, sizeof(buf), &result) == 0 && result)
        username = pwd.pw_name;

    if (gethostname(buf, sizeof(buf)) == 0) {
        buf[HOST_NAME_MAX] = '\0';
        host               = buf;
    }

    return std::format("{}@{}", username, host);
}

std::string Info::model() {
    auto manu  = cat("/sys/class/dmi/id/sys_vendor");
    auto model = cat("/sys/class/dmi/id/product_name");

    if (manu && model) {
        if (model->starts_with(*manu))
            return *model;
        return std::format("{} {}", *manu, *model);
    }

    if (manu)
        return *manu;
    if (model)
        return *model;
    return "";
}

std::optional<std::string> Info::cat(const std::filesystem::path& p) {
    auto f = readFileAsString(p.string());

    if (!f)
        return std::nullopt;

    return trim(*f);
}

static std::string formatKHz(const uint64_t khz) {
    if (khz >= 1000000)
        return std::format("{:.1f}GHz", khz / 1000000.F);
    return std::format("{:.1f}MHz", khz / 1000.F);
}

static std::optional<std::string> getFromCpuinfo(const std::string_view& sv) {
    static std::string content = "";
    static bool        once    = true;

    if (once) {
        once = false;

        auto read = readFileAsString("/proc/cpuinfo");
        content   = read.value_or("");
    }

    static CVarList2 vars(std::move(content), 0, '\n', true);

    for (const auto& v : vars) {
        if (v.starts_with(sv) && v.contains(':')) {
            // found
            auto value = trim(v.substr(v.find(':') + 2));

            if (value.back() == value.front() && value.back() == '"')
                value = value.substr(1, value.size() - 2);

            return std::string{value};
        }
    }

    return std::nullopt;
}

std::string Info::cpu() {
    auto        nproc = sysconf(_SC_NPROCESSORS_ONLN);
    std::string hz    = "?GHz";
    try {
        hz = formatKHz(std::stoull(cat("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq").value()));
    } catch (...) {
        ; // oh well
    }

    struct utsname u;
    std::string    arch;
    if (uname(&u) == 0)
        arch = u.machine;

    // try to read proc names from /proc/cpuinfo
    std::string cpuname = getFromCpuinfo("model name").value_or("Unknown");

    return std::format("{} {}x{} ({})", cpuname, nproc, hz, arch);
}

static std::optional<uint32_t> readU32Hex(const std::string& p) {
    auto str = readFileAsString(p);

    if (!str)
        return std::nullopt;

    auto s = std::move(*str);

    if (s.rfind("0x", 0) == 0)
        s.erase(0, 2);

    try {
        return sc<uint32_t>(std::stoul(s, nullptr, 16));
    } catch (...) { return std::nullopt; }
}

std::vector<std::string> Info::gpus() {
    glob_t g = {0};
    if (glob("/sys/class/drm/card[0-9]*", 0, NULL, &g) != 0)
        return {};

    struct pci_access* pacc = pci_alloc();
    pci_init(pacc);
    pci_scan_bus(pacc);

    std::vector<std::string> gpuNames;

    for (size_t i = 0; i < g.gl_pathc; i++) {
        const char*           cardpath = g.gl_pathv[i];

        std::filesystem::path vendorPath = std::filesystem::path{cardpath} / "device" / "vendor";
        std::filesystem::path devicePath = std::filesystem::path{cardpath} / "device" / "device";
        std::filesystem::path classPath  = std::filesystem::path{cardpath} / "device" / "class";

        uint32_t              vendor = 0, device = 0;

        if (auto ret = readU32Hex(vendorPath.string()); ret)
            vendor = *ret;
        else
            continue;

        if (auto ret = readU32Hex(devicePath.string()); ret)
            device = *ret;
        else
            continue;

        if (auto ret = readU32Hex(classPath.string()); ret) {
            if (((*ret) & 0xFF0000u) != 0x030000u) // only keep PCI class 0x03xxxx (display controller)
                continue;
        }

        static char namebuf[512] = {0};
        pci_lookup_name(pacc, namebuf, sizeof(namebuf), PCI_LOOKUP_VENDOR | PCI_LOOKUP_DEVICE, vendor, device);

        gpuNames.emplace_back(namebuf);
    }

    pci_cleanup(pacc);
    globfree(&g);

    return gpuNames;
}

static uint64_t toBytes(uint64_t v, uint64_t unit) {
    return v * unit;
}

static std::string formatBytes(uint64_t bytes) {
    // If you have more than 1TiB of RAM then holy fucking shit
    if (bytes > 1024 * 1024 * 1024 * 1024ULL)
        return std::format("{:.1f}TiB", bytes / sc<float>(1024 * 1024 * 1024 * 1024ULL));
    if (bytes > 1024 * 1024 * 1024)
        return std::format("{:.1f}GiB", bytes / sc<float>(1024 * 1024 * 1024));
    if (bytes > 1024 * 1024)
        return std::format("{:.1f}MiB", bytes / sc<float>(1024 * 1024));
    if (bytes > 1024)
        return std::format("{:.1f}KiB", bytes / sc<float>(1024));
    return std::format("{}B", bytes);
}

std::string Info::mem() {
    struct sysinfo info{};
    if (sysinfo(&info) != 0)
        return "";

    const auto TOTAL = toBytes(info.totalram, info.mem_unit);
    auto       FREE  = toBytes(info.freeram + info.bufferram, info.mem_unit);

    if (const auto meminfo = cat("/proc/meminfo"); meminfo) {
        CVarList2 vl(std::string{*meminfo}, 0, '\n', true, true);
        for (const auto& l : vl) {
            if (!l.starts_with("MemAvailable"))
                continue;

            CVarList2 ln(std::string{l}, 0, 's', true, false);

            try {
                FREE = toBytes(std::stoull(std::string{ln[1]}), 1000);
            } catch (...) {}
            break;
        }
    }

    return std::format("{} / {} ({:.0f}% used)", formatBytes(TOTAL - FREE), formatBytes(TOTAL), sc<float>((TOTAL - FREE) / sc<float>(TOTAL)) * 100.F);
}

std::string Info::uptime() {
    struct sysinfo info{};
    if (::sysinfo(&info) != 0)
        return "";

    uint64_t   s = sc<uint64_t>(info.uptime);

    const auto DAYS = s / 86400;
    s %= 86400;
    const auto HRS = s / 3600;
    s %= 3600;
    const auto MINS = s / 60;
    s %= 60;

    return std::format("{} days, {} hours, {} minutes", DAYS, HRS, MINS);
}

std::optional<std::span<const uint8_t>> Info::deLogo() {
    const auto DE = desktop();

    if (DE == "Hyprland")
        return std::span<const uint8_t>{Icons::HYPRLAND_LOGO, sizeof(Icons::HYPRLAND_LOGO)};

    return std::nullopt;
}
