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
#include "Slide.h"

class SkCanvas;

class Viewer : public sk_app::Application {
public:
    Viewer(int argc, char** argv, void* platformData);
    ~Viewer() override;

    void onPaint(SkCanvas* canvas);
    void onIdle(double ms) override;
    bool onTouch(int owner, sk_app::Window::InputState state, float x, float y);

private:
    void initSlides();
    void updateTitle();
    void setupCurrentSlide(int previousSlide);

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

    // transform data
    SkScalar               fZoomCenterX;
    SkScalar               fZoomCenterY;
    SkScalar               fZoomLevel;
    SkScalar               fZoomScale;

    sk_app::CommandSet     fCommands;

    SkTouchGesture         fGesture;
};


#endif
