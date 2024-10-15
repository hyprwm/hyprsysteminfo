#include "Utils.hpp"

#include <fstream>
#include <qdebug.h>
#include <qlogging.h>
#include <qprocess.h>
#include <optional>

QString execAndGet(const QString& program, const QStringList& arguments, bool* ok) {
    QProcess process;
    process.setProcessChannelMode(QProcess::SeparateChannels);
    process.start(program, arguments, QIODevice::ReadOnly);

    if (!process.waitForStarted(-1)) {
        qCritical() << "Failed to start process" << program << arguments;
        if (ok)
            *ok = false;

        return "";
    }

    if (!process.waitForFinished(-1)) {
        qCritical() << "Failed to run process" << program << arguments;
        if (ok)
            *ok = false;

        return "";
    }

    if (ok)
        *ok = true;
    return process.readAll();
}

std::optional<std::string> readFile(const std::string& filename) {
    try {
        std::ifstream ifs(filename);
        if (ifs.good()) {
            std::string data(std::istreambuf_iterator<char>{ifs}, {});
            ifs.close();
            return data;
        }
    } catch (...) {}

    return {};
}
