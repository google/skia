/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef Viewer_DEFINED
#define Viewer_DEFINED

#include "../Application.h"
#include "../Window.h"
#include "gm.h"
#include "SkAnimTimer.h"
#include "Slide.h"

class SkCanvas;

class Viewer : public sk_app::Application {
public:
    Viewer(int argc, char** argv, void* platformData);
    ~Viewer() override;

    bool onKey(sk_app::Window::Key key, sk_app::Window::InputState state, uint32_t modifiers);
    bool onChar(SkUnichar, uint32_t modifiers);
    void onPaint(SkCanvas* canvas);
    void onIdle(double ms) override;

private:
    void initSlides();
    void setupCurrentSlide(int previousSlide);

    void drawStats(SkCanvas* canvas);

    void changeZoomLevel(float delta);
    void updateMatrix();

    sk_app::Window*        fWindow;

    static const int kMeasurementCount = 64;  // should be power of 2 for fast mod
    double fMeasurements[kMeasurementCount];
    int fCurrentMeasurement;

    SkAnimTimer            fAnimTimer;
    SkTArray<sk_sp<Slide>> fSlides;
    int                    fCurrentSlide;

    bool                   fDisplayStats;

    // transform data
    SkMatrix               fLocalMatrix;
    SkScalar               fZoomCenterX;
    SkScalar               fZoomCenterY;
    SkScalar               fZoomLevel;
    SkScalar               fZoomScale;

};


#endif
