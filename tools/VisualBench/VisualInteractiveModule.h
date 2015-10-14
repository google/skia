/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef VisualInteractiveModule_DEFINED
#define VisualInteractiveModule_DEFINED

#include "VisualStreamTimingModule.h"

class SkCanvas;

/*
 * This module for VisualBench is designed to display stats data dynamically
 */
class VisualInteractiveModule : public VisualStreamTimingModule {
public:
    // TODO get rid of backpointer
    VisualInteractiveModule(VisualBench* owner);

    bool onHandleChar(SkUnichar c) override;

private:
    void drawStats(SkCanvas*);
    void renderFrame(SkCanvas*, Benchmark*, int loops) override;
    bool timingFinished(Benchmark*, int loops, double measurement) override;

    static const int kMeasurementCount = 64;  // should be power of 2 for fast mod
    double fMeasurements[kMeasurementCount];
    int fCurrentMeasurement;
    bool fAdvance;

    typedef VisualStreamTimingModule INHERITED;
};

#endif
