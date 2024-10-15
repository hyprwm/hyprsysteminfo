#include "SystemIconProvider.hpp"
#include <qapplication.h>
#include <qqmlapplicationengine.h>
#include <qquickstyle.h>
#include <qtenvironmentvariables.h>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    app.setApplicationName("Hyprland System Info");

    if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE"))
        QQuickStyle::setStyle("org.kde.desktop");

    QQmlApplicationEngine engine;
    engine.addImageProvider("systemIcons", new CSystemIconProvider);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed, &app, []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);
    engine.load("qrc:/qt/qml/org/hyprland/systeminfo/main.qml");

    return app.exec();
}
