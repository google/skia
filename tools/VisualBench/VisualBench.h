/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#ifndef VisualBench_DEFINED
#define VisualBench_DEFINED

#include "SkWindow.h"

#include "SkPicture.h"
#include "SkString.h"
#include "SkSurface.h"
#include "Timer.h"
#include "VisualBenchmarkStream.h"
#include "gl/SkGLContext.h"

class GrContext;
struct GrGLInterface;
class GrRenderTarget;
class SkCanvas;

/*
 * A Visual benchmarking tool for gpu benchmarking
 */
class VisualBench : public SkOSWindow {
public:
    VisualBench(void* hwnd, int argc, char** argv);
    ~VisualBench() override;

protected:
    SkSurface* createSurface() override;

    void draw(SkCanvas* canvas) override;

    void onSizeChange() override;

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
    void resetContext();
    void setupRenderTarget();
    bool onHandleChar(SkUnichar unichar) override;
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
    int fFlushes;
    int fLoops;
    SkTArray<Record> fRecords;
    WallTimer fTimer;
    State fState;
    SkAutoTDelete<VisualBenchmarkStream> fBenchmarkStream;
    SkAutoTUnref<Benchmark> fBenchmark;

    // support framework
    SkAutoTUnref<SkSurface> fSurface;
    SkAutoTUnref<GrContext> fContext;
    SkAutoTUnref<GrRenderTarget> fRenderTarget;
    AttachmentInfo fAttachmentInfo;
    SkAutoTUnref<const GrGLInterface> fInterface;

    typedef SkOSWindow INHERITED;
};

#endif
