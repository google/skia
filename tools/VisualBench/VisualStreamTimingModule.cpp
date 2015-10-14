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

bool VisualStreamTimingModule::nextBenchmarkIfNecessary(SkCanvas* canvas) {
    if (fBenchmark) {
        return true;
    }

    fBenchmark.reset(fBenchmarkStream->next());
    if (!fBenchmark) {
        return false;
    }

    fOwner->clear(canvas, SK_ColorWHITE, 2);

    fBenchmark->delayedSetup();
    fBenchmark->preTimingHooks(canvas);
    return true;
}

void VisualStreamTimingModule::draw(SkCanvas* canvas) {
    if (!this->nextBenchmarkIfNecessary(canvas)) {
        SkDebugf("Exiting VisualBench successfully\n");
        fOwner->closeWindow();
        return;
    }

    if (fReinitializeBenchmark) {
        fReinitializeBenchmark = false;
        fBenchmark->preTimingHooks(canvas);
    }

    this->renderFrame(canvas, fBenchmark, fTSM.loops());
    fOwner->present();
    TimingStateMachine::ParentEvents event = fTSM.nextFrame(fPreWarmBeforeSample);
    switch (event) {
        case TimingStateMachine::kReset_ParentEvents:
            fBenchmark->postTimingHooks(canvas);
            fOwner->reset();
            fReinitializeBenchmark = true;
            break;
        case TimingStateMachine::kTiming_ParentEvents:
            break;
        case TimingStateMachine::kTimingFinished_ParentEvents:
            fBenchmark->postTimingHooks(canvas);
            fOwner->reset();
            if (this->timingFinished(fBenchmark, fTSM.loops(), fTSM.lastMeasurement())) {
                fTSM.nextBenchmark(canvas, fBenchmark);
                fBenchmark.reset(nullptr);
            } else {
                fReinitializeBenchmark = true;
            }
            break;
    }
}
