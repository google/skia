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
    void setTitle();
    bool setupBackend();
    void resetContext();
    void setupRenderTarget();
    bool onHandleChar(SkUnichar unichar) override;
    void printStats();
    bool advanceRecordIfNecessary(SkCanvas*);
    inline void renderFrame(SkCanvas*);

    struct Record {
        SkTArray<double> fMeasurements;
    };

    enum State {
        kPreWarmLoops_State,
        kTuneLoops_State,
        kPreWarmTiming_State,
        kTiming_State,
    };
    void preWarm(State nextState);

    int fCurrentSample;
    int fCurrentFrame;
    int fFlushes;
    int fLoops;
    SkTArray<Record> fRecords;
    WallTimer fTimer;
    State fState;
    SkAutoTDelete<VisualBenchmarkStream> fBenchmarkStream;
    Benchmark* fBenchmark;

    // support framework
    SkAutoTUnref<SkSurface> fSurface;
    SkAutoTUnref<GrContext> fContext;
    SkAutoTUnref<GrRenderTarget> fRenderTarget;
    AttachmentInfo fAttachmentInfo;
    SkAutoTUnref<const GrGLInterface> fInterface;

    typedef SkOSWindow INHERITED;
};

#endif
