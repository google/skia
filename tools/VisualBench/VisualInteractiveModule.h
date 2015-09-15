/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#ifndef VisualInteractiveModule_DEFINED
#define VisualInteractiveModule_DEFINED

#include "VisualModule.h"

#include "ResultsWriter.h"
#include "SkPicture.h"
#include "Timer.h"
#include "VisualBench.h"
#include "VisualBenchmarkStream.h"

class SkCanvas;

/*
 * This module for VisualBench is designed to display stats data dynamically
 */
class VisualInteractiveModule : public VisualModule {
public:
    // TODO get rid of backpointer
    VisualInteractiveModule(VisualBench* owner);

    void draw(SkCanvas* canvas) override;
    bool onHandleChar(SkUnichar unichar) override;

private:
    /*
     * The heart of visual bench is an event driven timing loop.
     * kPreWarmLoopsPerCanvasPreDraw_State:  Before we begin timing, Benchmarks have a hook to
     *                                       access the canvas.  Then we prewarm before the autotune
     *                                       loops step.
     * kPreWarmLoops_State:                  We prewarm the gpu before auto tuning to enter a steady
     *                                       work state
     * kTuneLoops_State:                     Then we tune the loops of the benchmark to ensure we
     *                                       are doing a measurable amount of work
     * kPreTiming_State:                     Because reset the context after tuning loops to ensure
     *                                       coherent state, we need to restart before timing
     * kTiming_State:                        Finally we time the benchmark.  In this case we
     *                                       continue running and displaying benchmark data
     *                                       until we quit or switch to another benchmark
     * kAdvance_State:                       Advance to the next benchmark in the stream
     */
    enum State {
        kPreWarmLoopsPerCanvasPreDraw_State,
        kPreWarmLoops_State,
        kTuneLoops_State,
        kPreTiming_State,
        kTiming_State,
        kAdvance_State,
    };
    void setTitle();
    bool setupBackend();
    void setupRenderTarget();
    void drawStats(SkCanvas*);
    bool advanceRecordIfNecessary(SkCanvas*);
    inline void renderFrame(SkCanvas*);
    inline void nextState(State);
    void perCanvasPreDraw(SkCanvas*, State);
    void preWarm(State nextState);
    void scaleLoops(double elapsedMs);
    inline void tuneLoops(SkCanvas*);
    inline void timing(SkCanvas*);
    inline double elapsed();
    void resetTimingState();
    void postDraw(SkCanvas*);
    void recordMeasurement();

    static const int kMeasurementCount = 64;  // should be power of 2 for fast mod
    double fMeasurements[kMeasurementCount];
    int fCurrentMeasurement;

    int fCurrentFrame;
    int fLoops;
    WallTimer fTimer;
    State fState;
    SkAutoTDelete<VisualBenchmarkStream> fBenchmarkStream;
    SkAutoTUnref<Benchmark> fBenchmark;

    // support framework
    SkAutoTUnref<VisualBench> fOwner;

    typedef VisualModule INHERITED;
};

#endif
