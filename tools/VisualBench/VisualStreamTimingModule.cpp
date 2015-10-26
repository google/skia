/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "VisualStreamTimingModule.h"

#include "SkCanvas.h"

VisualStreamTimingModule::VisualStreamTimingModule(VisualBench* owner, bool preWarmBeforeSample)
    : fInitState(kReset_InitState)
    , fPreWarmBeforeSample(preWarmBeforeSample)
    , fOwner(owner) {
    fBenchmarkStream.reset(new VisualBenchmarkStream(owner->getSurfaceProps()));
}

inline void VisualStreamTimingModule::handleInitState(SkCanvas* canvas) {
    switch (fInitState) {
        case kNewBenchmark_InitState:
            fBenchmarkStream->current()->delayedSetup();
            // fallthrough
        case kReset_InitState:
            // This will flicker unfortunately, but as we are reseting the GLContext each bench,
            // we unfortunately don't have a choice
            fOwner->clear(canvas, SK_ColorWHITE, 2);
            fBenchmarkStream->current()->preTimingHooks(canvas);
            break;
        case kNone_InitState:
            break;
    }
    fInitState = kNone_InitState;
}

inline void VisualStreamTimingModule::handleTimingEvent(SkCanvas* canvas,
                                                        TimingStateMachine::ParentEvents event) {
    switch (event) {
        case TimingStateMachine::kReset_ParentEvents:
            fBenchmarkStream->current()->postTimingHooks(canvas);
            fOwner->reset();
            fInitState = kReset_InitState;
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
                    fInitState = kNewBenchmark_InitState;
                }
            } else {
                fInitState = kReset_InitState;
            }
            break;
    }
}

void VisualStreamTimingModule::draw(SkCanvas* canvas) {
    if (!fBenchmarkStream->current()) {
        // this should never happen but just to be safe
        // TODO research why this does happen on mac
        return;
    }

    this->handleInitState(canvas);
    this->renderFrame(canvas, fBenchmarkStream->current(), fTSM.loops());
    fOwner->present();
    TimingStateMachine::ParentEvents event = fTSM.nextFrame(fPreWarmBeforeSample);
    this->handleTimingEvent(canvas, event);
}
