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
#include "TimingStateMachine.h"
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

    bool onHandleChar(SkUnichar c) override;

private:
    void setTitle();
    bool setupBackend();
    void setupRenderTarget();
    void printStats();
    bool advanceRecordIfNecessary(SkCanvas*);
    inline void renderFrame(SkCanvas*);

    struct Record {
        SkTArray<double> fMeasurements;
    };
    int fCurrentSample;
    SkTArray<Record> fRecords;
    SkAutoTDelete<VisualBenchmarkStream> fBenchmarkStream;
    SkAutoTUnref<Benchmark> fBenchmark;
    TimingStateMachine fTSM;
    bool fHasBeenReset;

    // support framework
    SkAutoTUnref<VisualBench> fOwner;
    SkAutoTDelete<ResultsWriter> fResults;

    typedef VisualModule INHERITED;
};

#endif
