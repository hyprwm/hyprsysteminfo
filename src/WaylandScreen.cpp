#include "WaylandScreen.hpp"

#include <QtWaylandClient/private/qwayland-wayland.h>
#include <QtWaylandClient/private/qwaylanddisplay_p.h>
#include <QtWaylandClient/private/qwaylandintegration_p.h>
#include <cstdint>
#include <qcontainerfwd.h>
#include <qsize.h>
#include <qstringliteral.h>
#include <qtclasshelpermacros.h>
#include <vector>

using namespace QtWaylandClient;

class CWaylandScreen : public QtWayland::wl_output {
  public:
    explicit CWaylandScreen(QWaylandDisplay* display, uint32_t version, uint32_t id);
    ~CWaylandScreen() override;
    Q_DISABLE_COPY_MOVE(CWaylandScreen);
    SWaylandScreenInfo info;

  protected:
    void output_mode(uint32_t flags, int32_t width, int32_t height, int32_t refresh) override;
    void output_name(const QString& name) override;
};

std::vector<SWaylandScreenInfo> SWaylandScreenInfo::enumerateScreens() {
    auto*                        display = QWaylandIntegration::instance()->display();

    std::vector<CWaylandScreen*> screens;
    for (const auto& global : display->globals()) {
        if (global.interface == QStringLiteral("wl_output")) {
            screens.emplace_back(new CWaylandScreen(display, global.version, global.id));
        }
    }

    display->forceRoundTrip();

    std::vector<SWaylandScreenInfo> info;
    for (auto* screen : screens) {
        info.push_back(screen->info);
        delete screen;
    }

    return info;
}

CWaylandScreen::CWaylandScreen(QWaylandDisplay* display, uint32_t version, uint32_t id) : QtWayland::wl_output(display->wl_registry(), id, 4u) {}

CWaylandScreen::~CWaylandScreen() {
    release();
}

void CWaylandScreen::output_mode(uint32_t flags, int width, int height, int refresh) {
    info.pixelSize = QSize(width, height);
}

void CWaylandScreen::output_name(const QString& name) {
    info.name = name;
}
