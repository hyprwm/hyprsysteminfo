#include "Utils.hpp"

#include <qdebug.h>
#include <qlogging.h>
#include <qprocess.h>

QString execAndGet(const QString& program, const QStringList& arguments) {
    QProcess process;
    process.setProcessChannelMode(QProcess::SeparateChannels);
    process.start(program, arguments, QIODevice::ReadOnly);

    if (!process.waitForStarted(-1)) {
        qCritical() << "Failed to start process" << program << arguments;
        return "";
    }

    if (!process.waitForFinished(-1)) {
        qCritical() << "Failed to run process" << program << arguments;
        return "";
    }

    return process.readAll();
}

QString substrUntil(const QString& string, char until) {
    auto index = string.indexOf(until);

    if (index == -1)
        return string;
    else
        return string.first(index);
}
