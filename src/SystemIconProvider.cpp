#include "SystemIconProvider.hpp"
#include <qicon.h>
#include <qlogging.h>
#include <qpixmap.h>

QPixmap CSystemIconProvider::requestPixmap(const QString& id, QSize* size, const QSize& requestedSize) {
    auto icon = QIcon::fromTheme(id);

    if (!requestedSize.isValid()) {
        qCritical() << "Icon requests without an explicit size are not allowed.";
        return QPixmap();
    }

    auto pixmap = icon.pixmap(requestedSize.width(), requestedSize.height());

    if (pixmap.isNull()) {
        qWarning() << "Could not load icon" << id;
        return QPixmap();
    }

    if (size != nullptr)
        *size = pixmap.size();

    return pixmap;
}
