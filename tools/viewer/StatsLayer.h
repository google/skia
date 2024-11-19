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
#include "include/private/base/SkTArray.h"
#include "tools/sk_app/Window.h"

class SkSurface;

class StatsLayer : public sk_app::Window::Layer {
public:
    StatsLayer();
    void resetMeasurements();

    typedef int Timer;

    Timer addTimer(const char* label, SkColor color, SkColor labelColor = 0);
    void beginTiming(Timer);
    void endTiming(Timer);

    void enableGpuTimer(SkColor color);
    void disableGpuTimer();
    bool isGpuTimerEnabled() const { return fGpuTimerEnabled; }
    std::function<void(uint64_t ns)> issueGpuTimer();

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
    skia_private::TArray<TimerData> fTimers;
    double fTotalTimes[kMeasurementCount];

    TimerData fGpuTimer;
    bool fGpuTimerEnabled = false;

    int fCurrentMeasurement;
    double fLastTotalBegin;
    double fCumulativeMeasurementTime;
    int fCumulativeMeasurementCount;
    float fDisplayScale;
};

#endif
