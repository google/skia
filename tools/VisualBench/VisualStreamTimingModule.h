/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef VisualStreamTimingModule_DEFINED
#define VisualStreamTimingModule_DEFINED

#include "VisualModule.h"

#include "TimingStateMachine.h"
#include "VisualBench.h"
#include "VisualBenchmarkStream.h"

class SkCanvas;

/*
 * VisualStreamTimingModule is the base class for modules which want to time a stream of Benchmarks.
 *
 * Subclasses should implement renderFrame, which is called for each frame, and timingFinished,
 * which is called when a sample has finished timing.
 */
class VisualStreamTimingModule : public VisualModule {
public:
    VisualStreamTimingModule(VisualBench* owner);
    void draw(SkCanvas* canvas) override;

private:
    virtual void renderFrame(SkCanvas*, Benchmark*, int loops)=0;

    // subclasses should return true to advance the stream
    virtual bool timingFinished(Benchmark*, int loops, double measurement)=0;

    inline void handleInitState(SkCanvas*);
    inline void handleTimingEvent(SkCanvas*, TimingStateMachine::ParentEvents);

    TimingStateMachine fTSM;
    SkAutoTDelete<VisualBenchmarkStream> fBenchmarkStream;
    enum InitState {
        kNone_InitState,
        kReset_InitState,
        kNewBenchmark_InitState,
    };
    InitState fInitState;

    // support framework
    VisualBench* fOwner;

    typedef VisualModule INHERITED;
};

#endif
