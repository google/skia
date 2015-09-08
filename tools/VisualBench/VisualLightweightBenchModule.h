/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#ifndef VisualLightweightBenchModule_DEFINED
#define VisualLightweightBenchModule_DEFINED

#include "VisualModule.h"

#include "ResultsWriter.h"
#include "SkPicture.h"
#include "Timer.h"
#include "VisualBench.h"
#include "VisualBenchmarkStream.h"

class SkCanvas;

/*
 * This module is designed to be a minimal overhead timing module for VisualBench
 */
class VisualLightweightBenchModule : public VisualModule {
public:
    // TODO get rid of backpointer
    VisualLightweightBenchModule(VisualBench* owner);

    void draw(SkCanvas* canvas) override;

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
    void setTitle();
    bool setupBackend();
    void setupRenderTarget();
    void printStats();
    bool advanceRecordIfNecessary(SkCanvas*);
    inline void renderFrame(SkCanvas*);
    inline void nextState(State);
    void perCanvasPreDraw(SkCanvas*, State);
    void preWarm(State nextState);
    void scaleLoops(double elapsedMs);
    inline void tuneLoops();
    inline void timing(SkCanvas*);
    inline double elapsed();
    void resetTimingState();
    void postDraw(SkCanvas*);
    void recordMeasurement();

    struct Record {
        SkTArray<double> fMeasurements;
    };

    int fCurrentSample;
    int fCurrentFrame;
    int fLoops;
    SkTArray<Record> fRecords;
    WallTimer fTimer;
    State fState;
    SkAutoTDelete<VisualBenchmarkStream> fBenchmarkStream;
    SkAutoTUnref<Benchmark> fBenchmark;

    // support framework
    SkAutoTUnref<VisualBench> fOwner;
    SkAutoTDelete<ResultsWriter> fResults;

    typedef VisualModule INHERITED;
};

#endif
