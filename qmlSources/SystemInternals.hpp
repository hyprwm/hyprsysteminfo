#ifndef SYSTEMINTERNALS_H
#define SYSTEMINTERNALS_H

#include <QObject>
#include <QQmlApplicationEngine>
#include <QPixmap>
#include <QIcon>

class CSystemInternals : public QObject {
    Q_OBJECT;

  public:
    explicit CSystemInternals(QObject* parent = nullptr) : QObject(parent) {
        ;
    }

    QString              systemLogoName, systemName = "Linux", systemURL = "https://kernel.org/", systemKernel = "unknown";
    QString              hyprlandVersion, hyprlandVersionLong;

    QString              cpuInfo = "missing dependency: lscpu";
    std::vector<QString> gpuInfo = {"missing dependency: lspci"};
    QString              ramInfo = "?";

    QString              hlSystemInfo = "[error]", hlSystemVersion = "[error]";

    Q_INVOKABLE bool     hasSystemLogoName() {
        return systemLogoName.size() > 0;
    }

    Q_INVOKABLE QString getSystemLogoName() {
        return systemLogoName;
    }

    Q_INVOKABLE QString getSystemName() {
        return systemName;
    }

    Q_INVOKABLE QString getSystemURL() {
        return systemURL;
    }

    Q_INVOKABLE QString getSystemKernel() {
        return systemKernel;
    }

    Q_INVOKABLE bool hasHyprland() {
        return hyprlandVersionLong.size() > 0;
    }

    Q_INVOKABLE QString getHyprlandVersion() {
        return hyprlandVersion;
    }

    Q_INVOKABLE QString getHyprlandVersionLong() {
        return hyprlandVersionLong;
    }

    Q_INVOKABLE QString getCPUInfo() {
        return cpuInfo;
    }

    Q_INVOKABLE int getGPUInfoCount() {
        return gpuInfo.size();
    }

    Q_INVOKABLE QString getGPUInfo(int idx) {
        if (idx >= gpuInfo.size())
            return "[error]";
        return gpuInfo.at(idx);
    }

    Q_INVOKABLE QString getRAMInfo() {
        return ramInfo;
    }

    Q_INVOKABLE void copySystemInfo();
    Q_INVOKABLE void copyVersion();
};

#endif // SYSTEMINTERNALS_H
