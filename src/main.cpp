#include <hyprtoolkit/core/Backend.hpp>
#include <hyprtoolkit/window/Window.hpp>
#include <hyprtoolkit/element/Rectangle.hpp>
#include <hyprtoolkit/element/RowLayout.hpp>
#include <hyprtoolkit/element/ColumnLayout.hpp>
#include <hyprtoolkit/element/Text.hpp>
#include <hyprtoolkit/element/Image.hpp>
#include <hyprtoolkit/element/Button.hpp>
#include <hyprtoolkit/element/Null.hpp>
#include <hyprtoolkit/system/Icons.hpp>
#include <hyprtoolkit/core/Output.hpp>

#include <hyprutils/memory/SharedPtr.hpp>
#include <hyprutils/memory/UniquePtr.hpp>
#include <hyprutils/os/Process.hpp>

#include "utils/SystemInfo.hpp"
#include "utils/HyprlandIPC.hpp"

#include <cstring>

using namespace Hyprutils::Memory;
using namespace Hyprutils::Math;
using namespace Hyprutils::OS;
using namespace Hyprtoolkit;

#define SP CSharedPointer
#define WP CWeakPointer
#define UP CUniquePointer

static SP<IBackend> backend;

//
static SP<IElement> space(SP<IElement> e) {
    auto n = CNullBuilder::begin()->size({CDynamicSize::HT_SIZE_PERCENT, CDynamicSize::HT_SIZE_AUTO, {1, 1}})->commence();
    n->addChild(e);
    e->setPositionMode(Hyprtoolkit::IElement::HT_POSITION_ABSOLUTE);
    e->setPositionFlag(Hyprtoolkit::IElement::HT_POSITION_FLAG_LEFT, true);
    e->setPositionFlag(Hyprtoolkit::IElement::HT_POSITION_FLAG_VCENTER, true);
    return n;
}

int main(int argc, char** argv, char** envp) {
    setenv("HT_QUIET", "1", true);
    backend = IBackend::create();

    const auto FONT_SIZE   = CFontSize{CFontSize::HT_FONT_TEXT}.ptSize();
    const auto WINDOW_SIZE = Vector2D{FONT_SIZE * 100.F, FONT_SIZE * 60.F};

    auto       window =
        CWindowBuilder::begin()->preferredSize(WINDOW_SIZE)->minSize(WINDOW_SIZE)->maxSize(WINDOW_SIZE)->appTitle("System Information")->appClass("hyprsysteminfo")->commence();

    window->m_rootElement->addChild(CRectangleBuilder::begin()->color([] { return backend->getPalette()->m_colors.background; })->commence());

    auto mainLayout = CColumnLayoutBuilder::begin()->size({CDynamicSize::HT_SIZE_PERCENT, CDynamicSize::HT_SIZE_PERCENT, {1, 1}})->gap(2)->commence();
    mainLayout->setMargin(4);

    window->m_rootElement->addChild(mainLayout);

    auto topLayout = CRowLayoutBuilder::begin()->size({CDynamicSize::HT_SIZE_PERCENT, CDynamicSize::HT_SIZE_AUTO, {1, 1}})->commence();

    // Left: Distro
    {
        auto distroNull   = CNullBuilder::begin()->size({CDynamicSize::HT_SIZE_PERCENT, CDynamicSize::HT_SIZE_ABSOLUTE, {0.5F, FONT_SIZE * 10.F}})->commence();
        auto distroLayout = CRowLayoutBuilder::begin()->size({CDynamicSize::HT_SIZE_PERCENT, CDynamicSize::HT_SIZE_PERCENT, {0.9F, 1.F}})->commence();
        distroLayout->setPositionMode(Hyprtoolkit::IElement::HT_POSITION_ABSOLUTE);
        distroLayout->setPositionFlag(Hyprtoolkit::IElement::HT_POSITION_FLAG_CENTER, true);

        auto spacer1 = CNullBuilder::begin()->size({CDynamicSize::HT_SIZE_ABSOLUTE, CDynamicSize::HT_SIZE_ABSOLUTE, {1, 1}})->commence();
        spacer1->setGrow(true);

        distroLayout->addChild(spacer1);

        auto distroLogoName = Info::getDistroLogoName();
        if (distroLogoName) {
            auto icon = backend->systemIcons()->lookupIcon(*distroLogoName);

            if (icon->exists()) {
                auto img = CImageBuilder::begin()->size({CDynamicSize::HT_SIZE_AUTO, CDynamicSize::HT_SIZE_PERCENT, {1.F, 0.9F}})->icon(icon)->commence();
                distroLayout->addChild(img);
            }
        }

        auto distroTextLayout = CColumnLayoutBuilder::begin()->size({CDynamicSize::HT_SIZE_PERCENT, CDynamicSize::HT_SIZE_AUTO, {0.5F, 1.F}})->commence();

        auto distroTextA = CTextBuilder::begin()
                               ->text(Info::getFromEtcOsRelease("NAME").value_or("Unknown"))
                               ->fontSize({CFontSize::HT_FONT_TEXT, 1.F})
                               ->color([] { return backend->getPalette()->m_colors.text; })
                               ->commence();
        auto distroTextB = CTextBuilder::begin()
                               ->text(std::format("<a href=\"{}\">{}</a>", Info::getFromEtcOsRelease("HOME_URL").value_or(""), Info::getFromEtcOsRelease("HOME_URL").value_or("")))
                               ->fontSize({CFontSize::HT_FONT_TEXT, 1.F})
                               ->color([] { return backend->getPalette()->m_colors.text; })
                               ->commence();
        auto distroTextC =
            CTextBuilder::begin()->text(Info::kernel())->fontSize({CFontSize::HT_FONT_TEXT, 1.F})->color([] { return backend->getPalette()->m_colors.text; })->commence();

        distroTextLayout->addChild(distroTextA);
        distroTextLayout->addChild(distroTextB);
        distroTextLayout->addChild(distroTextC);

        distroLayout->addChild(distroTextLayout);

        distroNull->addChild(distroLayout);

        topLayout->addChild(distroNull);
    }

    // Right: Hyprland

    {
        auto hlNull   = CNullBuilder::begin()->size({CDynamicSize::HT_SIZE_PERCENT, CDynamicSize::HT_SIZE_ABSOLUTE, {0.5F, FONT_SIZE * 10.F}})->commence();
        auto hlLayout = CRowLayoutBuilder::begin()->size({CDynamicSize::HT_SIZE_PERCENT, CDynamicSize::HT_SIZE_PERCENT, {0.9F, 1.F}})->commence();
        hlLayout->setPositionMode(Hyprtoolkit::IElement::HT_POSITION_ABSOLUTE);
        hlLayout->setPositionFlag(Hyprtoolkit::IElement::HT_POSITION_FLAG_CENTER, true);

        auto spacer1 = CNullBuilder::begin()->size({CDynamicSize::HT_SIZE_ABSOLUTE, CDynamicSize::HT_SIZE_ABSOLUTE, {1, 1}})->commence();
        spacer1->setGrow(true);

        auto logoData = Info::deLogo();
        if (logoData) {
            std::vector<uint8_t> data;
            data.resize(logoData->size());
            std::memcpy(data.data(), logoData->data(), logoData->size());

            auto img = CImageBuilder::begin()->size({CDynamicSize::HT_SIZE_AUTO, CDynamicSize::HT_SIZE_PERCENT, {1.F, 0.9F}})->data(std::move(data))->commence();
            hlLayout->addChild(img);
        }

        auto hlTextLayout = CColumnLayoutBuilder::begin()->size({CDynamicSize::HT_SIZE_PERCENT, CDynamicSize::HT_SIZE_AUTO, {0.5F, 1.F}})->commence();

        auto hlTextA =
            CTextBuilder::begin()->text(Info::desktop())->fontSize({CFontSize::HT_FONT_TEXT, 1.F})->color([] { return backend->getPalette()->m_colors.text; })->commence();
        auto hlTextB = CTextBuilder::begin()
                           ->text(std::format("<a href=\"{}\">{}</a>", Info::desktopHome(), Info::desktopHome()))
                           ->fontSize({CFontSize::HT_FONT_TEXT, 1.F})
                           ->color([] { return backend->getPalette()->m_colors.text; })
                           ->commence();
        auto hlTextC =
            CTextBuilder::begin()->text(Info::desktopVersion())->fontSize({CFontSize::HT_FONT_TEXT, 1.F})->color([] { return backend->getPalette()->m_colors.text; })->commence();

        hlTextLayout->addChild(hlTextA);
        hlTextLayout->addChild(hlTextB);
        hlTextLayout->addChild(hlTextC);

        hlLayout->addChild(hlTextLayout);

        hlLayout->addChild(spacer1);

        hlNull->addChild(hlLayout);

        topLayout->addChild(hlNull);
    }

    mainLayout->addChild(topLayout);

    // Spacer, then system props

    auto hr = Hyprtoolkit::CRectangleBuilder::begin() //
                  ->color([] { return Hyprtoolkit::CHyprColor{backend->getPalette()->m_colors.text.darken(0.65)}; })
                  ->size({Hyprtoolkit::CDynamicSize::HT_SIZE_PERCENT, Hyprtoolkit::CDynamicSize::HT_SIZE_ABSOLUTE, {1.F, 9.F}})
                  ->commence();
    hr->setMargin(4);

    mainLayout->addChild(hr);

    auto container = CColumnLayoutBuilder::begin()->size({CDynamicSize::HT_SIZE_PERCENT, CDynamicSize::HT_SIZE_AUTO, {0.95F, 1.F}})->gap(2)->commence();

    container->setMargin(10);

    container->setPositionMode(Hyprtoolkit::IElement::HT_POSITION_ABSOLUTE);
    container->setPositionFlag(Hyprtoolkit::IElement::HT_POSITION_FLAG_HCENTER, true);

    container->addChild(space(CTextBuilder::begin()
                                  ->text(std::format("User: {}", Info::user()))
                                  ->fontSize({CFontSize::HT_FONT_TEXT, 1.F})
                                  ->color([] { return backend->getPalette()->m_colors.text; })
                                  ->commence()));

    container->addChild(space(CTextBuilder::begin()
                                  ->text(std::format("Model: {}", Info::model()))
                                  ->fontSize({CFontSize::HT_FONT_TEXT, 1.F})
                                  ->color([] { return backend->getPalette()->m_colors.text; })
                                  ->commence()));

    container->addChild(space(CTextBuilder::begin()
                                  ->text(std::format("CPU: {}", Info::cpu()))
                                  ->fontSize({CFontSize::HT_FONT_TEXT, 1.F})
                                  ->color([] { return backend->getPalette()->m_colors.text; })
                                  ->commence()));

    for (const auto& g : Info::gpus()) {
        container->addChild(space(CTextBuilder::begin()
                                      ->text(std::format("GPU: {}", g))
                                      ->fontSize({CFontSize::HT_FONT_TEXT, 1.F})
                                      ->color([] { return backend->getPalette()->m_colors.text; })
                                      ->commence()));
    }

    container->addChild(space(CTextBuilder::begin()
                                  ->text(std::format("RAM: {}", Info::mem()))
                                  ->fontSize({CFontSize::HT_FONT_TEXT, 1.F})
                                  ->color([] { return backend->getPalette()->m_colors.text; })
                                  ->commence()));

    container->addChild(space(CTextBuilder::begin()
                                  ->text(std::format("Uptime: {}", Info::uptime()))
                                  ->fontSize({CFontSize::HT_FONT_TEXT, 1.F})
                                  ->color([] { return backend->getPalette()->m_colors.text; })
                                  ->commence()));

    mainLayout->addChild(container);

    // bottom layout

    auto bottomNull = CNullBuilder::begin()->size({CDynamicSize::HT_SIZE_PERCENT, CDynamicSize::HT_SIZE_AUTO, {1, 1}})->commence();
    bottomNull->setMargin(6);
    bottomNull->setPositionMode(Hyprtoolkit::IElement::HT_POSITION_ABSOLUTE);
    bottomNull->setPositionFlag(Hyprtoolkit::IElement::HT_POSITION_FLAG_BOTTOM, true);

    auto bottomLayout = CRowLayoutBuilder::begin()->size({CDynamicSize::HT_SIZE_PERCENT, CDynamicSize::HT_SIZE_AUTO, {1, 1}})->gap(10)->commence();

    auto spacer = CNullBuilder::begin()->size({CDynamicSize::HT_SIZE_ABSOLUTE, CDynamicSize::HT_SIZE_ABSOLUTE, {1, 1}})->commence();
    spacer->setGrow(true, false);

    bottomLayout->addChild(spacer);

    bottomLayout->addChild( //
        CButtonBuilder::begin()
            ->label("Copy Hyprland System Info")
            ->onMainClick([](SP<CButtonElement> e) {
                // FIXME: toolkit needs to provide clipboard stuff

                auto info = HyprlandIPC::getFromSocket("/systeminfo");

                if (!info) {
                    e->rebuild()->label("Failed copying")->commence();
                    return;
                }

                CProcess proc("wl-copy", {*info});
                if (!proc.runAsync()) { // FIXME: why does runSync hang?!
                    e->rebuild()->label("Failed copying")->commence();
                    return;
                }

                e->rebuild()->label("Copied!")->commence();
            })
            ->commence() //
    );

    bottomLayout->addChild( //
        CButtonBuilder::begin()
            ->label("Exit")
            ->onMainClick([w = WP<IWindow>{window}](SP<CButtonElement> e) {
                w->close();
                backend->destroy();
            })
            ->commence() //
    );

    //

    bottomNull->addChild(bottomLayout);
    window->m_rootElement->addChild(bottomNull);

    window->m_events.closeRequest.listenStatic([w = WP<IWindow>{window}] {
        w->close();
        backend->destroy();
    });

    window->open();

    backend->enterLoop();

    return 0;
}