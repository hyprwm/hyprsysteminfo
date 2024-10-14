#pragma once

#include <QObject>
#include <QQmlApplicationEngine>
#include <QPixmap>
#include <QIcon>
#include <qcontainerfwd.h>
#include <qqmlintegration.h>
#include <qtmetamacros.h>

class CSystemInternals : public QObject {
    Q_OBJECT;
    QML_NAMED_ELEMENT(SystemInfo);
    QML_SINGLETON;
    Q_PROPERTY(QString systemLogoName MEMBER systemLogoName CONSTANT);
    Q_PROPERTY(QString systemName MEMBER systemName CONSTANT);
    Q_PROPERTY(QString systemUrl MEMBER systemURL CONSTANT);
    Q_PROPERTY(QString systemKernel MEMBER systemKernel CONSTANT);
    Q_PROPERTY(QString hyprlandVersion MEMBER hyprlandVersion CONSTANT);
    Q_PROPERTY(QString hyprlandVersionLong MEMBER hyprlandVersionLong CONSTANT);
    Q_PROPERTY(QString cpuInfo MEMBER cpuInfo CONSTANT);
    Q_PROPERTY(QVector<QString> gpuInfo MEMBER gpuInfo CONSTANT);
    Q_PROPERTY(QString ramInfo MEMBER ramInfo CONSTANT);
    Q_PROPERTY(QString uptime MEMBER uptime CONSTANT);
    Q_PROPERTY(QString de MEMBER DE CONSTANT);
    Q_PROPERTY(QString screens MEMBER screens CONSTANT);
    Q_PROPERTY(QString model MEMBER board CONSTANT);
    Q_PROPERTY(QString user MEMBER user CONSTANT);

  public:
    explicit CSystemInternals(QObject* parent = nullptr);

    QString          systemLogoName, systemName = "Linux", systemURL = "https://kernel.org/", systemKernel = "unknown";
    QString          hyprlandVersion, hyprlandVersionLong;

    QString          cpuInfo = "missing dependency: lscpu";
    QVector<QString> gpuInfo = {"missing dependency: lspci"};
    QString          ramInfo = "?";

    QString          hlSystemInfo = "[error]", hlSystemVersion = "[error]";

    QString          uptime = "unknown", DE = "Unknown", screens = "unknown", board = "", user = "";

    Q_INVOKABLE void copySystemInfo();
    Q_INVOKABLE void copyVersion();
};
