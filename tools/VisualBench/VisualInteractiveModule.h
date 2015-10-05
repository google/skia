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
#include "TimingStateMachine.h"
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
    void setTitle();
    bool setupBackend();
    void setupRenderTarget();
    void drawStats(SkCanvas*);
    bool advanceRecordIfNecessary(SkCanvas*);
    inline void renderFrame(SkCanvas*);

    static const int kMeasurementCount = 64;  // should be power of 2 for fast mod
    double fMeasurements[kMeasurementCount];
    int fCurrentMeasurement;

    SkAutoTDelete<VisualBenchmarkStream> fBenchmarkStream;
    SkAutoTUnref<Benchmark> fBenchmark;
    TimingStateMachine fTSM;
    bool fAdvance;

    // support framework
    SkAutoTUnref<VisualBench> fOwner;

    typedef VisualModule INHERITED;
};

#endif
