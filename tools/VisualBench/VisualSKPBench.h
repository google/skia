/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#ifndef VisualSKPBench_DEFINED
#define VisualSKPBench_DEFINED

#include "Benchmark.h"
#include "SkCanvas.h"
#include "SkPicture.h"

/**
 * Runs an SkPicture as a benchmark by repeatedly drawing it
 */
class VisualSKPBench : public Benchmark {
public:
    VisualSKPBench(const char* name, const SkPicture*);

protected:
    const char* onGetName() override;
    const char* onGetUniqueName() override;
    bool isSuitableFor(Backend backend) override;
    SkIPoint onGetSize() override;
    void onDraw(int loops, SkCanvas* canvas) override;

private:
    SkAutoTUnref<const SkPicture> fPic;
    SkIRect fCullRect;
    SkString fName;
    SkString fUniqueName;

    typedef Benchmark INHERITED;
};

#endif
