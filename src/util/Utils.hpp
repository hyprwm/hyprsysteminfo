#pragma once

#include <qlist.h>
#include <qstring.h>
#include <optional>

QString                    execAndGet(const QString& program, const QStringList& arguments = {}, bool* ok = nullptr);
std::optional<std::string> readFile(const std::string& filename);
