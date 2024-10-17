#pragma once

#include <qcontainerfwd.h>
#include <qsize.h>
#include <qstring.h>
#include <qcontainerfwd.h>
#include <vector>

struct SWaylandScreenInfo {
    QString                                name;
    QSize                                  pixelSize;

    static std::vector<SWaylandScreenInfo> enumerateScreens();
};
