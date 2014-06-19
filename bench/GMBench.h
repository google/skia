/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

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
    virtual const char* onGetName() SK_OVERRIDE;
    virtual bool isSuitableFor(Backend backend) SK_OVERRIDE;
    virtual void onDraw(const int loops, SkCanvas* canvas) SK_OVERRIDE;
    virtual SkIPoint onGetSize() SK_OVERRIDE;

private:
    skiagm::GM* fGM;
    SkString    fName;
    typedef Benchmark INHERITED;
};
