/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
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

    ParentEvents nextFrame(SkCanvas* canvas, Benchmark* benchmark);

    /*
     * The caller should call this when they are ready to move to the next benchmark.  The caller
     * must call this with the *last* benchmark so post draw hooks can be invoked
     */
    void nextBenchmark(SkCanvas*, Benchmark*);


    /*
     * When TimingStateMachine returns kTimingFinished_ParentEvents, then the owner can call
     * lastMeasurement() to get the time
     */
    double lastMeasurement() const { return fLastMeasurement; }

    int loops() const { return fLoops; }

private:
    /*
     * The heart of the timing state machine is an event driven timing loop.
     * kPreWarmLoopsPerCanvasPreDraw_State:  Before we begin timing, Benchmarks have a hook to
     *                                       access the canvas.  Then we prewarm before the autotune
     *                                       loops step.
     * kPreWarmLoops_State:                  We prewarm the gpu before auto tuning to enter a steady
     *                                       work state
     * kTuneLoops_State:                     Then we tune the loops of the benchmark to ensure we
     *                                       are doing a measurable amount of work
     * kPreWarmTimingPerCanvasPreDraw_State: Because reset the context after tuning loops to ensure
     *                                       coherent state, we need to give the benchmark
     *                                       another hook
     * kPreWarmTiming_State:                 We prewarm the gpu again to enter a steady state
     * kTiming_State:                        Finally we time the benchmark.  When finished timing
     *                                       if we have enough samples then we'll start the next
     *                                       benchmark in the kPreWarmLoopsPerCanvasPreDraw_State.
     *                                       otherwise, we enter the
     *                                       kPreWarmTimingPerCanvasPreDraw_State for another sample
     *                                       In either case we reset the context.
     */
    enum State {
        kPreWarmLoopsPerCanvasPreDraw_State,
        kPreWarmLoops_State,
        kTuneLoops_State,
        kPreWarmTimingPerCanvasPreDraw_State,
        kPreWarmTiming_State,
        kTiming_State,
    };

    inline void nextState(State);
    ParentEvents perCanvasPreDraw(SkCanvas*, Benchmark*, State);
    ParentEvents preWarm(State nextState);
    inline ParentEvents tuneLoops();
    inline ParentEvents timing(SkCanvas*, Benchmark*);
    inline double elapsed();
    void resetTimingState();
    void postDraw(SkCanvas*, Benchmark*);
    void recordMeasurement();

    int fCurrentFrame;
    int fLoops;
    double fLastMeasurement;
    WallTimer fTimer;
    State fState;
};

#endif
