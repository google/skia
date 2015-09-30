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

static const int kGpuFrameLag = 5;
static const int kFrames = 5;
static const double kLoopMs = 5;

VisualInteractiveModule::VisualInteractiveModule(VisualBench* owner)
    : fCurrentMeasurement(0)
    , fCurrentFrame(0)
    , fLoops(1)
    , fState(kPreWarmLoops_State)
    , fBenchmark(nullptr)
    , fOwner(SkRef(owner)) {
    fBenchmarkStream.reset(new VisualBenchmarkStream);

    memset(fMeasurements, 0, sizeof(fMeasurements));
}

inline void VisualInteractiveModule::renderFrame(SkCanvas* canvas) {
    fBenchmark->draw(fLoops, canvas);
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

    return true;
}

void VisualInteractiveModule::draw(SkCanvas* canvas) {
    if (!this->advanceRecordIfNecessary(canvas)) {
        SkDebugf("Exiting VisualBench successfully\n");
        fOwner->closeWindow();
        return;
    }
    this->renderFrame(canvas);
    switch (fState) {
        case kPreWarmLoopsPerCanvasPreDraw_State: {
            this->perCanvasPreDraw(canvas, kPreWarmLoops_State);
            break;
        }
        case kPreWarmLoops_State: {
            this->preWarm(kTuneLoops_State);
            break;
        }
        case kTuneLoops_State: {
            this->tuneLoops(canvas);
            break;
        }
        case kPreTiming_State: {
            fBenchmark->perCanvasPreDraw(canvas);
            fBenchmark->preDraw(canvas);
            fCurrentFrame = 0;
            fTimer.start();
            fState = kTiming_State;
            // fall to next state
        }
        case kTiming_State: {
            this->timing(canvas);
            break;
        }
        case kAdvance_State: {
            this->postDraw(canvas);
            this->nextState(kPreWarmLoopsPerCanvasPreDraw_State);
            break;
        }
    }
}

inline void VisualInteractiveModule::nextState(State nextState) {
    fState = nextState;
}

void VisualInteractiveModule::perCanvasPreDraw(SkCanvas* canvas, State nextState) {
    fBenchmark->perCanvasPreDraw(canvas);
    fBenchmark->preDraw(canvas);
    fCurrentFrame = 0;
    this->nextState(nextState);
}

void VisualInteractiveModule::preWarm(State nextState) {
    if (fCurrentFrame >= kGpuFrameLag) {
        // we currently time across all frames to make sure we capture all GPU work
        this->nextState(nextState);
        fCurrentFrame = 0;
        fTimer.start();
    } else {
        fCurrentFrame++;
    }
}

inline double VisualInteractiveModule::elapsed() {
    fTimer.end();
    return fTimer.fWall;
}

void VisualInteractiveModule::resetTimingState() {
    fCurrentFrame = 0;
    fTimer = WallTimer();
    fOwner->reset();
}

void VisualInteractiveModule::scaleLoops(double elapsedMs) {
    // Scale back the number of loops
    fLoops = (int)ceil(fLoops * kLoopMs / elapsedMs);
}

inline void VisualInteractiveModule::tuneLoops(SkCanvas* canvas) {
    if (1 << 30 == fLoops) {
        // We're about to wrap.  Something's wrong with the bench.
        SkDebugf("InnerLoops wrapped\n");
        fLoops = 0;
    } else {
        double elapsedMs = this->elapsed();
        if (elapsedMs > kLoopMs) {
            this->scaleLoops(elapsedMs);
            fBenchmark->perCanvasPostDraw(canvas);
            this->nextState(kPreTiming_State);
        } else {
            fLoops *= 2;
            this->nextState(kPreWarmLoops_State);
        }
        this->resetTimingState();
    }
}

void VisualInteractiveModule::recordMeasurement() {
    double measurement = this->elapsed() / (kFrames * fLoops);
    fMeasurements[fCurrentMeasurement++] = measurement;
    fCurrentMeasurement &= (kMeasurementCount-1);  // fast mod
    SkASSERT(fCurrentMeasurement < kMeasurementCount);
}

void VisualInteractiveModule::postDraw(SkCanvas* canvas) {
    fBenchmark->postDraw(canvas);
    fBenchmark->perCanvasPostDraw(canvas);
    fBenchmark.reset(nullptr);
    fLoops = 1;
}

inline void VisualInteractiveModule::timing(SkCanvas* canvas) {
    if (fCurrentFrame >= kFrames) {
        this->recordMeasurement();
        fTimer.start();
        fCurrentFrame = 0;
    } else {
        fCurrentFrame++;
    }
}

bool VisualInteractiveModule::onHandleChar(SkUnichar c) {
    if (' ' == c) {
        this->nextState(kAdvance_State);
    }

    return true;
}
