/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef VulkanViewer_DEFINED
#define VulkanViewer_DEFINED

#include "../Application.h"
#include "../Window.h"
#include "gm.h"
#include "SkAnimTimer.h"
#include "Slide.h"

class SkCanvas;

class VulkanViewer : public Application {
public:
    VulkanViewer(int argc, char** argv, void* platformData);
    ~VulkanViewer() override;

    bool onKey(Window::Key key, Window::InputState state, uint32_t modifiers);
    bool onChar(SkUnichar, uint32_t modifiers);
    void onPaint(SkCanvas* canvas);
    void onIdle(double ms) override;

private:
    void initSlides();
    void setupCurrentSlide(int previousSlide);

    void drawStats(SkCanvas* canvas);

    void changeZoomLevel(float delta);
    void updateMatrix();

    Window*      fWindow;

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
