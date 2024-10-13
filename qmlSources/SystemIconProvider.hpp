#ifndef SYSTEMICONPROVIDER_H
#define SYSTEMICONPROVIDER_H

#include <QIcon>
#include <QObject>
#include <QPixmap>
#include <QQmlApplicationEngine>
#include <QQuickImageProvider>
#include <iostream>

class CSystemIconProvider : public QQuickImageProvider {
  public:
    CSystemIconProvider() : QQuickImageProvider(QQuickImageProvider::Pixmap) {}

    QPixmap requestPixmap(const QString& id, QSize* size, const QSize& requestedSize) override {
        return QIcon::fromTheme(id).pixmap({512, 512});
    }
};

#endif // SYSTEMICONPROVIDER_H