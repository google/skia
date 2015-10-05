/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#include "VisualInteractiveModule.h"

#include "ProcStats.h"
#include "SkApplication.h"
#include "SkCanvas.h"
#include "SkCommandLineFlags.h"
#include "SkForceLinking.h"
#include "SkGraphics.h"
#include "SkGr.h"
#include "SkImageDecoder.h"
#include "SkOSFile.h"
#include "SkStream.h"
#include "Stats.h"
#include "gl/GrGLInterface.h"

__SK_FORCE_IMAGE_DECODER_LINKING;

VisualInteractiveModule::VisualInteractiveModule(VisualBench* owner)
    : fCurrentMeasurement(0)
    , fBenchmark(nullptr)
    , fAdvance(false)
    , fHasBeenReset(false)
    , fOwner(SkRef(owner)) {
    fBenchmarkStream.reset(new VisualBenchmarkStream);

    memset(fMeasurements, 0, sizeof(fMeasurements));
}

inline void VisualInteractiveModule::renderFrame(SkCanvas* canvas) {
    fBenchmark->draw(fTSM.loops(), canvas);
    this->drawStats(canvas);
    canvas->flush();
    fOwner->present();
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

bool VisualInteractiveModule::advanceRecordIfNecessary(SkCanvas* canvas) {
    if (fBenchmark) {
        return true;
    }

    fBenchmark.reset(fBenchmarkStream->next());
    if (!fBenchmark) {
        return false;
    }

    // clear both buffers
    fOwner->clear(canvas, SK_ColorWHITE, 2);

    fBenchmark->delayedSetup();
    fBenchmark->preTimingHooks(canvas);
    return true;
}
#include "GrGpu.h"
#include "GrResourceCache.h"
void VisualInteractiveModule::draw(SkCanvas* canvas) {
    if (!this->advanceRecordIfNecessary(canvas)) {
        SkDebugf("Exiting VisualBench successfully\n");
        fOwner->closeWindow();
        return;
    }

    if (fHasBeenReset) {
        fHasBeenReset = false;
        fBenchmark->preTimingHooks(canvas);
    }

    this->renderFrame(canvas);
    TimingStateMachine::ParentEvents event = fTSM.nextFrame(false);
    switch (event) {
        case TimingStateMachine::kReset_ParentEvents:
            fBenchmark->postTimingHooks(canvas);
            fHasBeenReset = true;
            fOwner->reset();
            break;
        case TimingStateMachine::kTiming_ParentEvents:
            break;
        case TimingStateMachine::kTimingFinished_ParentEvents:
            // Record measurements
            fMeasurements[fCurrentMeasurement++] = fTSM.lastMeasurement();
            fCurrentMeasurement &= (kMeasurementCount-1);  // fast mod
            SkASSERT(fCurrentMeasurement < kMeasurementCount);
            this->drawStats(canvas);
            if (fAdvance) {
                fAdvance = false;
                fTSM.nextBenchmark(canvas, fBenchmark);
                fBenchmark->postTimingHooks(canvas);
                fBenchmark.reset(nullptr);
                fOwner->reset();
                fHasBeenReset = true;
            }
            break;
    }
}

bool VisualInteractiveModule::onHandleChar(SkUnichar c) {
    if (' ' == c) {
        fAdvance = true;
    }

    return true;
}
