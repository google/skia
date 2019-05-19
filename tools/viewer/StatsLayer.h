/*
* Copyright 2017 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef StatsLayer_DEFINED
#define StatsLayer_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkString.h"
#include "tools/sk_app/Window.h"

class StatsLayer : public sk_app::Window::Layer {
public:
    StatsLayer();
    void resetMeasurements();

    typedef int Timer;

    Timer addTimer(const char* label, SkColor color, SkColor labelColor = 0);
    void beginTiming(Timer);
    void endTiming(Timer);

    void onPrePaint() override;
    void onPaint(SkSurface*) override;

    void setDisplayScale(float scale) { fDisplayScale = scale; }

private:
    static const int kMeasurementCount = 1 << 6;  // should be power of 2 for fast mod
    struct TimerData {
        double fTimes[kMeasurementCount];
        SkString fLabel;
        SkColor fColor;
        SkColor fLabelColor;
    };
    SkTArray<TimerData> fTimers;
    double fTotalTimes[kMeasurementCount];
    int fCurrentMeasurement;
    double fLastTotalBegin;
    double fCumulativeMeasurementTime;
    int fCumulativeMeasurementCount;
    float fDisplayScale;
};

#endif
