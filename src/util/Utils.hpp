#pragma once

#include <qlist.h>
#include <qstring.h>

QString execAndGet(const QString& program, const QStringList& arguments = {});
QString substrUntil(const QString& string, char until);
