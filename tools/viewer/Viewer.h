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
#include "SkAnimTimer.h"
#include "SkTouchGesture.h"
#include "Slide.h"

class SkCanvas;

class Viewer : public sk_app::Application {
public:
    Viewer(int argc, char** argv, void* platformData);
    ~Viewer() override;

    void onBackendCreated();
    void onPaint(SkCanvas* canvas);
    void onIdle() override;
    bool onTouch(intptr_t owner, sk_app::Window::InputState state, float x, float y);
    bool onMouse(float x, float y, sk_app::Window::InputState state, uint32_t modifiers);
    void onUIStateChanged(const SkString& stateName, const SkString& stateValue);
    bool onKey(sk_app::Window::Key key, sk_app::Window::InputState state, uint32_t modifiers);
    bool onChar(SkUnichar c, uint32_t modifiers);

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
    void setStartupSlide();
    void setupCurrentSlide(int previousSlide);
    void listNames();

    void updateUIState();

    void drawSlide(SkCanvas* canvs);
    void drawStats(SkCanvas* canvas);
    void drawImGui(SkCanvas* canvas);

    void changeZoomLevel(float delta);
    SkMatrix computeMatrix();

    sk_app::Window*        fWindow;

    static const int kMeasurementCount = 64;  // should be power of 2 for fast mod
    double fPaintTimes[kMeasurementCount];
    double fFlushTimes[kMeasurementCount];
    double fAnimateTimes[kMeasurementCount];
    int fCurrentMeasurement;

    SkAnimTimer            fAnimTimer;
    SkTArray<sk_sp<Slide>> fSlides;
    int                    fCurrentSlide;

    bool                   fDisplayStats;
    bool                   fRefresh; // whether to continuously refresh for measuring render time

    SkPaint                fImGuiFontPaint;
    SkPaint                fImGuiGamutPaint;
    bool                   fShowImGuiDebugWindow;
    bool                   fShowImGuiTestWindow;

    bool                   fShowZoomWindow;
    sk_sp<SkImage>         fLastImage;

    sk_app::Window::BackendType fBackendType;

    // Color properties for slide rendering
    ColorMode              fColorMode;
    SkColorSpacePrimaries  fColorSpacePrimaries;

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
};


#endif
