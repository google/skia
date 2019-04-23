/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GMBench_DEFINED
#define GMBench_DEFINED

#include "bench/Benchmark.h"
#include "gm/gm.h"
#include "include/core/SkCanvas.h"

/**
 * Runs a GM as a benchmark by repeatedly drawing the GM.
 */
class GMBench : public Benchmark {
public:
    // Constructor takes ownership of the GM param.
    GMBench(skiagm::GM* gm);
    ~GMBench() override;

    void modifyGrContextOptions(GrContextOptions* options) override {
        return fGM->modifyGrContextOptions(options);
    }

protected:
    const char* onGetName() override;
    bool isSuitableFor(Backend backend) override;
    void onDraw(int loops, SkCanvas* canvas) override;
    SkIPoint onGetSize() override;

private:
    skiagm::GM* fGM;
    SkString    fName;
    typedef Benchmark INHERITED;
};

#endif
