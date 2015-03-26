/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GMBench_DEFINED
#define GMBench_DEFINED

#include "Benchmark.h"
#include "SkCanvas.h"
#include "gm.h"

/**
 * Runs a GM as a benchmark by repeatedly drawing the GM.
 */
class GMBench : public Benchmark {
public:
    // Constructor takes ownership of the GM param.
    GMBench(skiagm::GM* gm);
    virtual ~GMBench();

protected:
    const char* onGetName() override;
    bool isSuitableFor(Backend backend) override;
    void onDraw(const int loops, SkCanvas* canvas) override;
    SkIPoint onGetSize() override;

private:
    skiagm::GM* fGM;
    SkString    fName;
    typedef Benchmark INHERITED;
};

#endif
