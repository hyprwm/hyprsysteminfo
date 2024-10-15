#pragma once

#include <QIcon>
#include <QObject>
#include <QPixmap>
#include <QQmlApplicationEngine>
#include <QQuickImageProvider>

class CSystemIconProvider : public QQuickImageProvider {
  public:
    CSystemIconProvider() : QQuickImageProvider(QQuickImageProvider::Pixmap) {}

    QPixmap requestPixmap(const QString& id, QSize* size, const QSize& requestedSize) override;
};
