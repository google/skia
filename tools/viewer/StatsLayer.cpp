/*
* Copyright 2017 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "tools/viewer/StatsLayer.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypeface.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkTDArray.h"
#include "src/base/SkTime.h"
#include "tools/fonts/FontToolUtils.h"

#include <algorithm>
#include <string>

StatsLayer::StatsLayer()
    : fCurrentMeasurement(-1)
    , fLastTotalBegin(0)
    , fCumulativeMeasurementTime(0)
    , fCumulativeMeasurementCount(0)
    , fDisplayScale(1.0f) {
    memset(fTotalTimes, 0, sizeof(fTotalTimes));
}

void StatsLayer::resetMeasurements() {
    for (int i = 0; i < fTimers.size(); ++i) {
        memset(fTimers[i].fTimes, 0, sizeof(fTimers[i].fTimes));
    }
    memset(fTotalTimes, 0, sizeof(fTotalTimes));
    fCurrentMeasurement = -1;
    fLastTotalBegin = 0;
    fCumulativeMeasurementTime = 0;
    fCumulativeMeasurementCount = 0;
}

StatsLayer::Timer StatsLayer::addTimer(const char* label, SkColor color, SkColor labelColor) {
    Timer newTimer = fTimers.size();
    TimerData& newData = fTimers.push_back();
    memset(newData.fTimes, 0, sizeof(newData.fTimes));
    newData.fLabel = label;
    newData.fColor = color;
    newData.fLabelColor = labelColor ? labelColor : color;
    return newTimer;
}

void StatsLayer::beginTiming(Timer timer) {
    if (fCurrentMeasurement >= 0) {
        fTimers[timer].fTimes[fCurrentMeasurement] -= SkTime::GetMSecs();
    }
}

void StatsLayer::endTiming(Timer timer) {
    if (fCurrentMeasurement >= 0) {
        fTimers[timer].fTimes[fCurrentMeasurement] += SkTime::GetMSecs();
    }
}

void StatsLayer::enableGpuTimer(SkColor color) {
    fGpuTimer.fColor = color;
    std::fill_n(fGpuTimer.fTimes, std::size(fGpuTimer.fTimes), 0);
    fGpuTimerEnabled = true;
}

void StatsLayer::disableGpuTimer() { fGpuTimerEnabled = false; }

std::function<void(uint64_t ns)> StatsLayer::issueGpuTimer() {
    if (fCurrentMeasurement < 0 || !fGpuTimerEnabled) {
        return {};
    }
    // The -1 indicates to the rendering code that we are still awaiting the result. Unlike the CPU
    // timers, there may be a multi-frame latency.
    fGpuTimer.fTimes[fCurrentMeasurement] = -1;
    return [index = fCurrentMeasurement, layer = this](uint64_t ns) {
        layer->fGpuTimer.fTimes[index] = static_cast<double>(ns) / 1000000.0;
    };
}

void StatsLayer::onPrePaint() {
    if (fCurrentMeasurement >= 0) {
        fTotalTimes[fCurrentMeasurement] = SkTime::GetMSecs() - fLastTotalBegin;
        fCumulativeMeasurementTime += fTotalTimes[fCurrentMeasurement];
        fCumulativeMeasurementCount++;
    }
    fCurrentMeasurement = (fCurrentMeasurement + 1) & (kMeasurementCount - 1);
    SkASSERT(fCurrentMeasurement >= 0 && fCurrentMeasurement < kMeasurementCount);
    fLastTotalBegin = SkTime::GetMSecs();
}

void StatsLayer::onPaint(SkSurface* surface) {
    int nextMeasurement = (fCurrentMeasurement + 1) & (kMeasurementCount - 1);
    for (int i = 0; i < fTimers.size(); ++i) {
        fTimers[i].fTimes[nextMeasurement] = 0;
    }

#ifdef SK_BUILD_FOR_ANDROID
    // Scale up the stats overlay on Android devices
    static constexpr SkScalar kScale = 1.5;
#else
    SkScalar kScale = fDisplayScale;
#endif

    // Now draw everything

    // Vertical height corresponding to 1 ms in the graph
    static const float kPixelPerMS = 2.0f;
    // Horizontal spacing between measurements
    static const int kPixelPerMeasurement = 3;
    // We add one extra spacing on the left, hence the + 1
    static const int kDisplayWidth = (kMeasurementCount + 1) * kPixelPerMeasurement;
    static const int kGraphHeight = 100;
    static const int kTextHeight = 60;
    // The GPU graph is only shown if supported by the backend.
    const int gpuGraphHeight = fGpuTimerEnabled ? kGraphHeight : 0;
    const int displayHeight = gpuGraphHeight + kGraphHeight + kTextHeight;
    // Padding between the graph and top/right edges of the canvas.
    static const int kDisplayPadding = 10;
    // ms/frame to hit 60 fps
    static const SkScalar kBaseMS = 1000.f / 60.f;

    auto canvas = surface->getCanvas();
    SkISize canvasSize = canvas->getBaseLayerSize();
    SkRect rect = SkRect::MakeXYWH(SkIntToScalar(canvasSize.fWidth-kDisplayWidth-kDisplayPadding),
                                   SkIntToScalar(kDisplayPadding),
                                   SkIntToScalar(kDisplayWidth), SkIntToScalar(displayHeight));
    SkPaint paint;
    SkAutoCanvasRestore acr(canvas, /*doSave=*/true);

    // Scale the canvas while keeping the right edge in place.
    canvas->concat(*SkMatrix::Rect2Rect(SkRect::Make(canvasSize),
                                        SkRect::MakeXYWH(canvasSize.width()  * (1 - kScale),
                                                         0,
                                                         canvasSize.width()  * kScale,
                                                         canvasSize.height() * kScale)));

    paint.setColor(SK_ColorBLACK);
    canvas->drawRect(rect, paint);

    float cpuGraphBottom = rect.fBottom - gpuGraphHeight;

    // draw the 16ms line
    paint.setColor(SK_ColorLTGRAY);
    canvas->drawLine(rect.fLeft, cpuGraphBottom - kBaseMS*kPixelPerMS,
                     rect.fRight, cpuGraphBottom - kBaseMS*kPixelPerMS, paint);
    paint.setColor(SK_ColorRED);
    paint.setStyle(SkPaint::kStroke_Style);
    canvas->drawRect(SkRect::MakeLTRB(rect.fLeft, rect.fTop, rect.fRight, cpuGraphBottom), paint);
    paint.setStyle(SkPaint::kFill_Style);

    int x = SkScalarTruncToInt(rect.fLeft) + kPixelPerMeasurement;
    int i = nextMeasurement;
    SkTDArray<double> sumTimes;
    sumTimes.resize(fTimers.size());
    memset(sumTimes.begin(), 0, sumTimes.size() * sizeof(double));
    int count = 0;
    double totalTime = 0;
    int totalCount = 0;
    do {
        int startY = SkScalarTruncToInt(cpuGraphBottom);
        double inc = 0;
        for (int timer = 0; timer < fTimers.size(); ++timer) {
            int height = (int)(fTimers[timer].fTimes[i] * kPixelPerMS + 0.5);
            int endY = std::max(startY - height, kDisplayPadding + kTextHeight);
            paint.setColor(fTimers[timer].fColor);
            canvas->drawLine(SkIntToScalar(x), SkIntToScalar(startY),
                             SkIntToScalar(x), SkIntToScalar(endY), paint);
            startY = endY;
            inc += fTimers[timer].fTimes[i];
            sumTimes[timer] += fTimers[timer].fTimes[i];
        }

        int height = (int)(fTotalTimes[i] * kPixelPerMS + 0.5);
        height = std::max(0, height - (SkScalarTruncToInt(cpuGraphBottom) - startY));
        int endY = std::max(startY - height, kDisplayPadding + kTextHeight);
        paint.setColor(SK_ColorWHITE);
        canvas->drawLine(SkIntToScalar(x), SkIntToScalar(startY),
                         SkIntToScalar(x), SkIntToScalar(endY), paint);
        totalTime += fTotalTimes[i];
        if (fTotalTimes[i] > 0) {
            ++totalCount;
        }

        if (inc > 0) {
            ++count;
        }

        i++;
        i &= (kMeasurementCount - 1);  // fast mod
        x += kPixelPerMeasurement;
    } while (i != nextMeasurement);

    SkFont font(ToolUtils::CreatePortableTypeface("sans-serif", SkFontStyle()), 14);
    paint.setColor(SK_ColorWHITE);
    double time = totalTime / std::max(1, totalCount);
    double measure = fCumulativeMeasurementTime / std::max(1, fCumulativeMeasurementCount);
    canvas->drawString(SkStringPrintf("C: %4.3f ms -> %4.3f ms", time, measure),
                       rect.fLeft + 3, rect.fTop + 14, font, paint);

    for (int timer = 0; timer < fTimers.size(); ++timer) {
        paint.setColor(fTimers[timer].fLabelColor);
        canvas->drawString(SkStringPrintf("%s: %4.3f ms", fTimers[timer].fLabel.c_str(),
                                          sumTimes[timer] / std::max(1, count)),
                           rect.fLeft + 3, rect.fTop + 28 + (14 * timer), font, paint);
    }

    if (!fGpuTimerEnabled) {
        return;
    }

    float gpuGraphBottom = rect.bottom();
    // draw the 16ms line
    paint.setColor(SK_ColorLTGRAY);
    canvas->drawLine(rect.fLeft, gpuGraphBottom - kBaseMS*kPixelPerMS,
                     rect.fRight, gpuGraphBottom - kBaseMS*kPixelPerMS,
                     paint);
    paint.setColor(SK_ColorRED);
    paint.setStyle(SkPaint::kStroke_Style);
    canvas->drawRect(SkRect::MakeLTRB(rect.fLeft, cpuGraphBottom, rect.fRight, gpuGraphBottom),
                     paint);
    paint.setStyle(SkPaint::kFill_Style);

    x = SkScalarTruncToInt(rect.fLeft) + kPixelPerMeasurement;
    i = nextMeasurement;
    totalCount = 0;
    totalTime = 0;
    do {
        int endY;
        if (fGpuTimer.fTimes[i] < 0) {
            // Draw a full height line with the color inverted to indicate a measurement
            // that is still pending.
            auto alpha = SkColorSetARGB(SkColorGetA(fGpuTimer.fColor), 0, 0, 0);
            auto inv = (0x00FFFFF & ~fGpuTimer.fColor);
            paint.setColor(alpha | inv);
            endY = cpuGraphBottom;
        } else {
            paint.setColor(fGpuTimer.fColor);
            ++totalCount;
            totalTime += fGpuTimer.fTimes[i];
            float height = fGpuTimer.fTimes[i] * kPixelPerMS + 0.5f;
            endY = std::max(gpuGraphBottom - height, cpuGraphBottom);
        }
        canvas->drawLine(x, gpuGraphBottom, x, endY, paint);

        i++;
        i &= (kMeasurementCount - 1);  // fast mod
        x += kPixelPerMeasurement;
    } while (i != nextMeasurement);
    paint.setColor(SK_ColorWHITE);
    time = totalTime / std::max(1, totalCount);
    canvas->drawString(SkStringPrintf("G: %4.3f ms", time),
                       rect.fLeft + 3,
                       cpuGraphBottom + 14,
                       font,
                       paint);
}
