/*
* Copyright 2017 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "tools/viewer/StatsLayer.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTime.h"

StatsLayer::StatsLayer()
    : fCurrentMeasurement(-1)
    , fLastTotalBegin(0)
    , fCumulativeMeasurementTime(0)
    , fCumulativeMeasurementCount(0)
    , fDisplayScale(1.0f) {
    memset(fTotalTimes, 0, sizeof(fTotalTimes));
}

void StatsLayer::resetMeasurements() {
    for (int i = 0; i < fTimers.count(); ++i) {
        memset(fTimers[i].fTimes, 0, sizeof(fTimers[i].fTimes));
    }
    memset(fTotalTimes, 0, sizeof(fTotalTimes));
    fCurrentMeasurement = -1;
    fLastTotalBegin = 0;
    fCumulativeMeasurementTime = 0;
    fCumulativeMeasurementCount = 0;
}

StatsLayer::Timer StatsLayer::addTimer(const char* label, SkColor color, SkColor labelColor) {
    Timer newTimer = fTimers.count();
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
    for (int i = 0; i < fTimers.count(); ++i) {
        fTimers[i].fTimes[nextMeasurement] = 0;
    }

#ifdef SK_BUILD_FOR_ANDROID
    // Scale up the stats overlay on Android devices
    static constexpr SkScalar kScale = 1.5;
#else
    SkScalar kScale = fDisplayScale;
#endif

    // Now draw everything
    static const float kPixelPerMS = 2.0f;
    static const int kDisplayWidth = 192;
    static const int kGraphHeight = 100;
    static const int kTextHeight = 60;
    static const int kDisplayHeight = kGraphHeight + kTextHeight;
    static const int kDisplayPadding = 10;
    static const int kGraphPadding = 3;
    static const SkScalar kBaseMS = 1000.f / 60.f;  // ms/frame to hit 60 fps

    auto canvas = surface->getCanvas();
    SkISize canvasSize = canvas->getBaseLayerSize();
    SkRect rect = SkRect::MakeXYWH(SkIntToScalar(canvasSize.fWidth-kDisplayWidth-kDisplayPadding),
                                   SkIntToScalar(kDisplayPadding),
                                   SkIntToScalar(kDisplayWidth), SkIntToScalar(kDisplayHeight));
    SkPaint paint;
    canvas->save();

    // Scale the canvas while keeping the right edge in place.
    canvas->concat(SkMatrix::MakeRectToRect(SkRect::Make(canvasSize),
                                            SkRect::MakeXYWH(canvasSize.width()  * (1 - kScale),
                                                             0,
                                                             canvasSize.width()  * kScale,
                                                             canvasSize.height() * kScale),
                                            SkMatrix::kFill_ScaleToFit));

    paint.setColor(SK_ColorBLACK);
    canvas->drawRect(rect, paint);
    // draw the 16ms line
    paint.setColor(SK_ColorLTGRAY);
    canvas->drawLine(rect.fLeft, rect.fBottom - kBaseMS*kPixelPerMS,
                     rect.fRight, rect.fBottom - kBaseMS*kPixelPerMS, paint);
    paint.setColor(SK_ColorRED);
    paint.setStyle(SkPaint::kStroke_Style);
    canvas->drawRect(rect, paint);
    paint.setStyle(SkPaint::kFill_Style);

    int x = SkScalarTruncToInt(rect.fLeft) + kGraphPadding;
    const int xStep = 3;
    int i = nextMeasurement;
    SkTDArray<double> sumTimes;
    sumTimes.setCount(fTimers.count());
    memset(sumTimes.begin(), 0, sumTimes.count() * sizeof(double));
    int count = 0;
    double totalTime = 0;
    int totalCount = 0;
    do {
        int startY = SkScalarTruncToInt(rect.fBottom);
        double inc = 0;
        for (int timer = 0; timer < fTimers.count(); ++timer) {
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
        height = std::max(0, height - (SkScalarTruncToInt(rect.fBottom) - startY));
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
        x += xStep;
    } while (i != nextMeasurement);

    SkFont font(nullptr, 16);
    paint.setColor(SK_ColorWHITE);
    double time = totalTime / std::max(1, totalCount);
    double measure = fCumulativeMeasurementTime / std::max(1, fCumulativeMeasurementCount);
    canvas->drawString(SkStringPrintf("%4.3f ms -> %4.3f ms", time, measure),
                       rect.fLeft + 3, rect.fTop + 14, font, paint);

    for (int timer = 0; timer < fTimers.count(); ++timer) {
        paint.setColor(fTimers[timer].fLabelColor);
        canvas->drawString(SkStringPrintf("%s: %4.3f ms", fTimers[timer].fLabel.c_str(),
                                          sumTimes[timer] / std::max(1, count)),
                           rect.fLeft + 3, rect.fTop + 28 + (14 * timer), font, paint);
    }

    canvas->restore();
}
