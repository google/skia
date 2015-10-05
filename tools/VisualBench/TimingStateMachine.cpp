/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "TimingStateMachine.h"

#include "SkCanvas.h"
#include "SkCommandLineFlags.h"

DEFINE_int32(gpuFrameLag, 5, "Overestimate of maximum number of frames GPU is allowed to lag.");
DEFINE_int32(frames, 5, "Number of frames of each skp to render per sample.");
DEFINE_double(loopMs, 5, "Each benchmark will be tuned until it takes loopsMs millseconds.");

TimingStateMachine::TimingStateMachine()
    : fCurrentFrame(0)
    , fLoops(1)
    , fLastMeasurement(0.)
    , fState(kPreWarmLoopsPerCanvasPreDraw_State) {
}

TimingStateMachine::ParentEvents TimingStateMachine::nextFrame(SkCanvas* canvas,
                                                               Benchmark* benchmark) {
    switch (fState) {
        case kPreWarmLoopsPerCanvasPreDraw_State:
            return this->perCanvasPreDraw(canvas, benchmark, kPreWarmLoops_State);
        case kPreWarmLoops_State:
            return this->preWarm(kTuneLoops_State);
        case kTuneLoops_State:
            return this->tuneLoops();
        case kPreWarmTimingPerCanvasPreDraw_State:
            return this->perCanvasPreDraw(canvas, benchmark, kPreWarmTiming_State);
        case kPreWarmTiming_State:
            return this->preWarm(kTiming_State);
        case kTiming_State:
            return this->timing(canvas, benchmark);
    }
    SkFAIL("Incomplete switch\n");
    return kTiming_ParentEvents;
}

inline void TimingStateMachine::nextState(State nextState) {
    fState = nextState;
}

TimingStateMachine::ParentEvents TimingStateMachine::perCanvasPreDraw(SkCanvas* canvas,
                                                                      Benchmark* benchmark,
                                                                      State nextState) {
    benchmark->perCanvasPreDraw(canvas);
    benchmark->preDraw(canvas);
    fCurrentFrame = 0;
    this->nextState(nextState);
    return kTiming_ParentEvents;
}

TimingStateMachine::ParentEvents TimingStateMachine::preWarm(State nextState) {
    if (fCurrentFrame >= FLAGS_gpuFrameLag) {
        // we currently time across all frames to make sure we capture all GPU work
        this->nextState(nextState);
        fCurrentFrame = 0;
        fTimer.start();
    } else {
        fCurrentFrame++;
    }
    return kTiming_ParentEvents;
}

inline double TimingStateMachine::elapsed() {
    fTimer.end();
    return fTimer.fWall;
}

void TimingStateMachine::resetTimingState() {
    fCurrentFrame = 0;
    fTimer = WallTimer();
}

inline TimingStateMachine::ParentEvents TimingStateMachine::tuneLoops() {
    if (1 << 30 == fLoops) {
        // We're about to wrap.  Something's wrong with the bench.
        SkDebugf("InnerLoops wrapped\n");
        fLoops = 1;
        return kTiming_ParentEvents;
    } else {
        double elapsedMs = this->elapsed();
        if (elapsedMs > FLAGS_loopMs) {
            this->nextState(kPreWarmTimingPerCanvasPreDraw_State);
        } else {
            fLoops *= 2;
            this->nextState(kPreWarmLoops_State);
        }
        this->resetTimingState();
        return kReset_ParentEvents;
    }
}

void TimingStateMachine::recordMeasurement() {
    fLastMeasurement = this->elapsed() / (FLAGS_frames * fLoops);
}

void TimingStateMachine::nextBenchmark(SkCanvas* canvas, Benchmark* benchmark) {
    benchmark->postDraw(canvas);
    benchmark->perCanvasPostDraw(canvas);
    fLoops = 1;
    this->nextState(kPreWarmLoopsPerCanvasPreDraw_State);
}

inline TimingStateMachine::ParentEvents TimingStateMachine::timing(SkCanvas* canvas,
                                                                   Benchmark* benchmark) {
    if (fCurrentFrame >= FLAGS_frames) {
        this->recordMeasurement();
        this->resetTimingState();
        this->nextState(kPreWarmTimingPerCanvasPreDraw_State);
        return kTimingFinished_ParentEvents;
    } else {
        fCurrentFrame++;
        return kTiming_ParentEvents;
    }
}

