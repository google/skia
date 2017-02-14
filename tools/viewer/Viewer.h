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

    void onPaint(SkCanvas* canvas);
    void onIdle() override;
    bool onTouch(intptr_t owner, sk_app::Window::InputState state, float x, float y);
    void onUIStateChanged(const SkString& stateName, const SkString& stateValue);
    bool onKey(sk_app::Window::Key key, sk_app::Window::InputState state, uint32_t modifiers);
    bool onChar(SkUnichar c, uint32_t modifiers);

private:
    void initSlides();
    void updateTitle();
    void setColorMode(SkColorType, sk_sp<SkColorSpace>);
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
    bool                   fSetupFirstFrame;

    bool                   fDisplayStats;
    bool                   fRefresh; // whether to continuously refresh for measuring render time

    SkPaint                fImGuiFontPaint;
    bool                   fShowImGuiDebugWindow;
    bool                   fShowImGuiTestWindow;

    bool                   fShowZoomWindow;
    sk_sp<SkImage>         fLastImage;

    sk_app::Window::BackendType fBackendType;

    // Color properties for slide rendering
    SkColorType            fColorType;
    sk_sp<SkColorSpace>    fColorSpace;

    // transform data
    SkScalar               fZoomCenterX;
    SkScalar               fZoomCenterY;
    SkScalar               fZoomLevel;
    SkScalar               fZoomScale;

    sk_app::CommandSet     fCommands;

    SkTouchGesture         fGesture;

    // identity unless the window initially scales the content to fit the screen.
    SkMatrix               fDefaultMatrix;
    SkMatrix               fDefaultMatrixInv;

    Json::Value            fAllSlideNames; // cache all slide names for fast updateUIState
};


#endif
