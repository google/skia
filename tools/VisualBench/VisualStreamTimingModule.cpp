/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "VisualStreamTimingModule.h"

#include "SkCanvas.h"

VisualStreamTimingModule::VisualStreamTimingModule(VisualBench* owner, bool preWarmBeforeSample)
    : fReinitializeBenchmark(false)
    , fPreWarmBeforeSample(preWarmBeforeSample)
    , fOwner(owner) {
    fBenchmarkStream.reset(new VisualBenchmarkStream);
}

void VisualStreamTimingModule::draw(SkCanvas* canvas) {
    if (!fBenchmarkStream->current()) {
        // this should never happen but just to be safe
        return;
    }

    if (fReinitializeBenchmark) {
        fReinitializeBenchmark = false;
        fBenchmarkStream->current()->preTimingHooks(canvas);
    }

    this->renderFrame(canvas, fBenchmarkStream->current(), fTSM.loops());
    fOwner->present();
    TimingStateMachine::ParentEvents event = fTSM.nextFrame(fPreWarmBeforeSample);
    switch (event) {
        case TimingStateMachine::kReset_ParentEvents:
            fBenchmarkStream->current()->postTimingHooks(canvas);
            fOwner->reset();
            fReinitializeBenchmark = true;
            break;
        case TimingStateMachine::kTiming_ParentEvents:
            break;
        case TimingStateMachine::kTimingFinished_ParentEvents:
            fBenchmarkStream->current()->postTimingHooks(canvas);
            fOwner->reset();
            if (this->timingFinished(fBenchmarkStream->current(), fTSM.loops(),
                                     fTSM.lastMeasurement())) {
                fTSM.nextBenchmark();
                if (!fBenchmarkStream->next()) {
                    SkDebugf("Exiting VisualBench successfully\n");
                    fOwner->closeWindow();
                } else {
                    fOwner->clear(canvas, SK_ColorWHITE, 2);

                    fBenchmarkStream->current()->delayedSetup();
                    fBenchmarkStream->current()->preTimingHooks(canvas);
                }
            } else {
                fReinitializeBenchmark = true;
            }
            break;
    }
}
