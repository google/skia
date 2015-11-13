/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "VisualInteractiveModule.h"

#include "SkCanvas.h"
#include "SkCommandLineFlags.h"
#include "SkForceLinking.h"
#include "SkImageDecoder.h"

__SK_FORCE_IMAGE_DECODER_LINKING;

VisualInteractiveModule::VisualInteractiveModule(VisualBench* owner)
    : INHERITED(owner)
    , fCurrentMeasurement(0)
    , fAdvance(false) {
    memset(fMeasurements, 0, sizeof(fMeasurements));
}

void VisualInteractiveModule::renderFrame(SkCanvas* canvas, Benchmark* benchmark, int loops) {
    benchmark->draw(loops, canvas);
    this->drawStats(canvas);
    canvas->flush();
}

void VisualInteractiveModule::drawStats(SkCanvas* canvas) {
    static const float kPixelPerMS = 2.0f;
    static const int kDisplayWidth = 130;
    static const int kDisplayHeight = 100;
    static const int kDisplayPadding = 10;
    static const int kGraphPadding = 3;
    static const float kBaseMS = 1000.f / 60.f;  // ms/frame to hit 60 fps

    SkISize canvasSize = canvas->getDeviceSize();
    SkRect rect = SkRect::MakeXYWH(SkIntToScalar(canvasSize.fWidth-kDisplayWidth-kDisplayPadding),
                                   SkIntToScalar(kDisplayPadding),
                                   SkIntToScalar(kDisplayWidth), SkIntToScalar(kDisplayHeight));
    SkPaint paint;
    canvas->clipRect(rect);
    paint.setColor(SK_ColorBLACK);
    canvas->drawRect(rect, paint);
    // draw the 16ms line
    paint.setColor(SK_ColorLTGRAY);
    canvas->drawLine(rect.fLeft, rect.fBottom - kBaseMS*kPixelPerMS, 
                     rect.fRight, rect.fBottom - kBaseMS*kPixelPerMS, paint);
    paint.setColor(SK_ColorRED);
    paint.setStyle(SkPaint::kStroke_Style);
    canvas->drawRect(rect, paint);

    int x = SkScalarTruncToInt(rect.fLeft) + kGraphPadding;
    const int xStep = 2;
    const int startY = SkScalarTruncToInt(rect.fBottom);
    int i = fCurrentMeasurement;
    do {
        int endY = startY - (int)(fMeasurements[i] * kPixelPerMS + 0.5);  // round to nearest value
        canvas->drawLine(SkIntToScalar(x), SkIntToScalar(startY), 
                         SkIntToScalar(x), SkIntToScalar(endY), paint);
        i++;
        i &= (kMeasurementCount - 1);  // fast mod
        x += xStep;
    } while (i != fCurrentMeasurement);

}

bool VisualInteractiveModule::timingFinished(Benchmark* benchmark, int loops, double measurement) {
    // Record measurements
    fMeasurements[fCurrentMeasurement++] = measurement;
    fCurrentMeasurement &= (kMeasurementCount-1);  // fast mod
    SkASSERT(fCurrentMeasurement < kMeasurementCount);
    if (fAdvance) {
        fAdvance = false;
        return true;
    }
    return false;
}

bool VisualInteractiveModule::onHandleChar(SkUnichar c) {
    if (' ' == c) {
        fAdvance = true;
    }

    return true;
}
