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

static double now_ms() { return SkTime::GetNSecs() * 1e-6; }

TimingStateMachine::TimingStateMachine()
    : fCurrentFrame(0)
    , fLoops(1)
    , fLastMeasurement(0.)
    , fState(kPreWarm_State)
    , fInnerState(kTuning_InnerState) {
}

TimingStateMachine::ParentEvents TimingStateMachine::nextFrame(bool preWarmBetweenSamples) {
    ParentEvents parentEvent = kTiming_ParentEvents;
    switch (fState) {
        case kPreWarm_State: {
            if (fCurrentFrame >= FLAGS_gpuFrameLag) {
                fCurrentFrame = 0;
                fStartTime = now_ms();
                fState = kTiming_State;
            } else {
                fCurrentFrame++;
            }
            break;
        }
        case kTiming_State: {
            switch (fInnerState) {
                case kTuning_InnerState: {
                    if (1 << 30 == fLoops) {
                        // We're about to wrap.  Something's wrong with the bench.
                        SkDebugf("InnerLoops wrapped\n");
                        fLoops = 1;
                    } else {
                        double elapsedMs = this->elapsed();
                        if (elapsedMs < FLAGS_loopMs) {
                            fLoops *= 2;
                        } else {
                            fInnerState = kTiming_InnerState;
                        }
                        fState = kPreWarm_State;
                        this->resetTimingState();
                        parentEvent = kReset_ParentEvents;
                    }
                    break;
                }
                case kTiming_InnerState: {
                    if (fCurrentFrame >= FLAGS_frames) {
                        this->recordMeasurement();
                        this->resetTimingState();
                        parentEvent = kTimingFinished_ParentEvents;
                        if (preWarmBetweenSamples) {
                            fState = kPreWarm_State;
                        } else {
                            fStartTime = now_ms();
                        }
                    } else {
                        fCurrentFrame++;
                    }
                    break;
                }
            }
        }
        break;
    }
    return parentEvent;
}

inline double TimingStateMachine::elapsed() {
    return now_ms() - fStartTime;
}

void TimingStateMachine::resetTimingState() {
    fCurrentFrame = 0;
}

void TimingStateMachine::recordMeasurement() {
    fLastMeasurement = this->elapsed() / (FLAGS_frames * fLoops);
}

void TimingStateMachine::nextBenchmark() {
    fLoops = 1;
    fInnerState = kTuning_InnerState;
    fState = kPreWarm_State;
}
