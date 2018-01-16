/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef Viewer_DEFINED
#define Viewer_DEFINED

#include "sk_app/Application.h"
#include "sk_app/CommandSet.h"
#include "sk_app/Window.h"
#include "gm.h"
#include "ImGuiLayer.h"
#include "SkAnimTimer.h"
#include "SkExecutor.h"
#include "SkJSONCPP.h"
#include "SkTouchGesture.h"
#include "Slide.h"
#include "StatsLayer.h"

class SkCanvas;

class Viewer : public sk_app::Application, sk_app::Window::Layer {
public:
    Viewer(int argc, char** argv, void* platformData);
    ~Viewer() override;

    void onIdle() override;

    void onBackendCreated() override;
    void onPaint(SkCanvas* canvas) override;
    bool onTouch(intptr_t owner, sk_app::Window::InputState state, float x, float y) override;
    bool onMouse(int x, int y, sk_app::Window::InputState state, uint32_t modifiers) override;
    void onUIStateChanged(const SkString& stateName, const SkString& stateValue) override;
    bool onKey(sk_app::Window::Key key, sk_app::Window::InputState state, uint32_t modifiers) override;
    bool onChar(SkUnichar c, uint32_t modifiers) override;

private:
    enum class ColorMode {
        kLegacy,                                 // N32, no color management
        kColorManagedSRGB8888_NonLinearBlending, // N32, sRGB transfer function, nonlinear blending
        kColorManagedSRGB8888,                   // N32, sRGB transfer function, linear blending
        kColorManagedLinearF16,                  // F16, linear transfer function, linear blending
    };

    void initSlides();
    void updateTitle();
    void setBackend(sk_app::Window::BackendType);
    void setColorMode(ColorMode);
    int startupSlide() const;
    void setCurrentSlide(int);
    void setupCurrentSlide();
    void listNames() const;

    void updateUIState();

    void drawSlide(SkCanvas* canvs);
    void drawImGui();

    void changeZoomLevel(float delta);
    SkMatrix computeMatrix();

    void resetExecutor() {
        fExecutor = SkExecutor::MakeFIFOThreadPool(fThreadCnt == 0 ? fTileCnt : fThreadCnt);
    }

    sk_app::Window*        fWindow;

    StatsLayer             fStatsLayer;
    StatsLayer::Timer      fPaintTimer;
    StatsLayer::Timer      fFlushTimer;
    StatsLayer::Timer      fAnimateTimer;

    SkAnimTimer            fAnimTimer;
    SkTArray<sk_sp<Slide>> fSlides;
    int                    fCurrentSlide;

    bool                   fRefresh; // whether to continuously refresh for measuring render time

    bool                   fSaveToSKP;

    ImGuiLayer             fImGuiLayer;
    SkPaint                fImGuiGamutPaint;
    bool                   fShowImGuiDebugWindow;
    bool                   fShowSlidePicker;
    bool                   fShowImGuiTestWindow;

    bool                   fShowZoomWindow;
    sk_sp<SkImage>         fLastImage;

    sk_app::Window::BackendType fBackendType;

    // Color properties for slide rendering
    ColorMode              fColorMode;
    SkColorSpacePrimaries  fColorSpacePrimaries;
    SkColorSpaceTransferFn fColorSpaceTransferFn;

    // transform data
    SkScalar               fZoomLevel;

    sk_app::CommandSet     fCommands;

    enum class GestureDevice {
        kNone,
        kTouch,
        kMouse,
    };

    SkTouchGesture         fGesture;
    GestureDevice          fGestureDevice;

    // identity unless the window initially scales the content to fit the screen.
    SkMatrix               fDefaultMatrix;

    SkTArray<std::function<void(void)>> fDeferredActions;

    Json::Value            fAllSlideNames; // cache all slide names for fast updateUIState

    int fTileCnt;
    int fThreadCnt;
    std::unique_ptr<SkExecutor> fExecutor;
};


#endif
