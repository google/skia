/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef VisualLightweightBenchModule_DEFINED
#define VisualLightweightBenchModule_DEFINED

#include "VisualStreamTimingModule.h"

#include "ResultsWriter.h"
#include "SkPicture.h"
#include "VisualBench.h"

class SkCanvas;

/*
 * This module is designed to be a minimal overhead timing module for VisualBench
 */
class VisualLightweightBenchModule : public VisualStreamTimingModule {
public:
    // TODO get rid of backpointer
    VisualLightweightBenchModule(VisualBench* owner);

    bool onHandleChar(SkUnichar c) override;

private:
    void renderFrame(SkCanvas*, Benchmark*, int loops) override;
    bool timingFinished(Benchmark*, int loops, double measurement) override;
    void printStats(Benchmark*, int loops);

    struct Record {
        SkTArray<double> fMeasurements;
    };
    int fCurrentSample;
    SkTArray<Record> fRecords;

    // support framework
    SkAutoTDelete<ResultsWriter> fResults;

    typedef VisualStreamTimingModule INHERITED;
};

#endif
