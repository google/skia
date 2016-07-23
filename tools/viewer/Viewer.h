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
    void onIdle(double ms) override;
    bool onTouch(intptr_t owner, sk_app::Window::InputState state, float x, float y);
    void onUIStateChanged(const SkString& stateName, const SkString& stateValue);

private:
    void initSlides();
    void updateTitle();
    void setupCurrentSlide(int previousSlide);

    void updateUIState();

    void drawSlide(SkCanvas* canvs, bool inSplitScreen);
    void drawStats(SkCanvas* canvas);

    void changeZoomLevel(float delta);
    SkMatrix computeMatrix();

    sk_app::Window*        fWindow;

    static const int kMeasurementCount = 64;  // should be power of 2 for fast mod
    double fMeasurements[kMeasurementCount];
    int fCurrentMeasurement;

    SkAnimTimer            fAnimTimer;
    SkTArray<sk_sp<Slide>> fSlides;
    int                    fCurrentSlide;

    bool                   fDisplayStats;

    // whether to split the screen and draw two copies of the slide, one with sRGB and one without
    bool                   fSplitScreen;

    sk_app::Window::BackendType fBackendType;

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
