/*
* Copyright 2017 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "StatsLayer.h"

#include "SkCanvas.h"
#include "SkString.h"
#include "SkTime.h"

StatsLayer::StatsLayer()
    : fCurrentMeasurement(0)
    , fCumulativeMeasurementTime(0)
    , fCumulativeMeasurementCount(0) {}

void StatsLayer::resetMeasurements() {
    for (int i = 0; i < fTimers.count(); ++i) {
        memset(fTimers[i].fTimes, 0, sizeof(fTimers[i].fTimes));
    }
    fCurrentMeasurement = 0;
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
    fTimers[timer].fTimes[fCurrentMeasurement] -= SkTime::GetMSecs();
}

void StatsLayer::endTiming(Timer timer) {
    fTimers[timer].fTimes[fCurrentMeasurement] += SkTime::GetMSecs();
}

double StatsLayer::getLastTime(Timer timer) {
    int idx = (fCurrentMeasurement + (kMeasurementCount - 1)) & (kMeasurementCount - 1);
    return fTimers[timer].fTimes[idx];
}

void StatsLayer::onPaint(SkCanvas* canvas) {
    // Advance our timing bookkeeping
    for (int i = 0; i < fTimers.count(); ++i) {
        fCumulativeMeasurementTime += fTimers[i].fTimes[fCurrentMeasurement];
    }
    fCumulativeMeasurementCount++;
    fCurrentMeasurement = (fCurrentMeasurement + 1) & (kMeasurementCount - 1);
    SkASSERT(fCurrentMeasurement < kMeasurementCount);
    for (int i = 0; i < fTimers.count(); ++i) {
        fTimers[i].fTimes[fCurrentMeasurement] = 0;
    }

    // Now draw everything
    static const float kPixelPerMS = 2.0f;
    static const int kDisplayWidth = 192;
    static const int kGraphHeight = 100;
    static const int kTextHeight = 60;
    static const int kDisplayHeight = kGraphHeight + kTextHeight;
    static const int kDisplayPadding = 10;
    static const int kGraphPadding = 3;
    static const SkScalar kBaseMS = 1000.f / 60.f;  // ms/frame to hit 60 fps

    SkISize canvasSize = canvas->getBaseLayerSize();
    SkRect rect = SkRect::MakeXYWH(SkIntToScalar(canvasSize.fWidth-kDisplayWidth-kDisplayPadding),
                                   SkIntToScalar(kDisplayPadding),
                                   SkIntToScalar(kDisplayWidth), SkIntToScalar(kDisplayHeight));
    SkPaint paint;
    canvas->save();

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
    int i = fCurrentMeasurement;
    double ms = 0;
    SkTDArray<double> sumTimes;
    sumTimes.setCount(fTimers.count());
    memset(sumTimes.begin(), 0, sumTimes.count() * sizeof(double));
    int count = 0;
    do {
        int startY = SkScalarTruncToInt(rect.fBottom);
        double inc = 0;
        for (int timer = 0; timer < fTimers.count(); ++timer) {
            int height = (int)(fTimers[timer].fTimes[i] * kPixelPerMS + 0.5);
            int endY = SkTMax(startY - height, kDisplayPadding + kTextHeight);
            paint.setColor(fTimers[timer].fColor);
            canvas->drawLine(SkIntToScalar(x), SkIntToScalar(startY),
                             SkIntToScalar(x), SkIntToScalar(endY), paint);
            startY = endY;
            inc += fTimers[timer].fTimes[i];
            sumTimes[timer] += fTimers[timer].fTimes[i];
        }

        if (inc > 0) {
            ms += inc;
            ++count;
        }

        i++;
        i &= (kMeasurementCount - 1);  // fast mod
        x += xStep;
    } while (i != fCurrentMeasurement);

    paint.setTextSize(16);
    SkString mainString;
    mainString.appendf("%4.3f ms -> %4.3f ms", ms / SkTMax(1, count),
                  fCumulativeMeasurementTime / SkTMax(1, fCumulativeMeasurementCount));
    paint.setColor(SK_ColorWHITE);
    canvas->drawString(mainString.c_str(), rect.fLeft + 3, rect.fTop + 14, paint);

    for (int timer = 0; timer < fTimers.count(); ++timer) {
        SkString str;
        str.appendf("%s: %4.3f ms", fTimers[timer].fLabel.c_str(),
                    sumTimes[timer] / SkTMax(1, count));
        paint.setColor(fTimers[timer].fLabelColor);
        canvas->drawString(str, rect.fLeft + 3, rect.fTop + 28 + (14 * timer), paint);
    }

    canvas->restore();
}
