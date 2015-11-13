/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef TimingStateMachine_DEFINED
#define TimingStateMachine_DEFINED

#include "Benchmark.h"
#include "SkTArray.h"
#include "Timer.h"

class SkCanvas;

/*
 * Manages a timer via a state machine.  Can be used by modules to time benchmarks
 *
 * Clients call nextFrame, and must handle any requests from the timing state machine, specifically
 * to reset.  When kTimingFinished_ParentEvents is returned, then lastMeasurement() will return the
 * timing and loops() will return the number of loops used to time.
 *
 * A client may continue timing the same benchmark indefinitely.  To advance to the next
 * benchmark, the client should call nextBenchmark.
 */
class TimingStateMachine {
public:
    TimingStateMachine();

    enum ParentEvents {
        kReset_ParentEvents,
        kTiming_ParentEvents,
        kTimingFinished_ParentEvents,// This implies parent can read lastMeasurement() and must
                                     // reset
    };

    ParentEvents nextFrame(bool preWarmBetweenSamples);

    /*
     * The caller should call this when they are ready to move to the next benchmark.
     */
    void nextBenchmark();

    /*
     * When TimingStateMachine returns kTimingFinished_ParentEvents, then the owner can call
     * lastMeasurement() to get the time
     */
    double lastMeasurement() const { return fLastMeasurement; }

    int loops() const { return fLoops; }

private:
    enum State {
        kPreWarm_State,
        kTiming_State,
    };
    enum InnerState {
        kTuning_InnerState,
        kTiming_InnerState,
    };

    int fCurrentFrame;
    int fLoops;
    double fLastMeasurement;
    double fStartTime;
    State fState;
    InnerState fInnerState;
};

#endif
