/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#ifndef CpuWrappedBenchmark_DEFINED
#define CpuWrappedBenchmark_DEFINED

#include "Benchmark.h"
#include "SkSurface.h"

class CpuWrappedBenchmark : public Benchmark {
public:
    // Takes ownership of caller's ref on `bench`.
    explicit CpuWrappedBenchmark(Benchmark* bench) : fBench(bench) {}

    const char* onGetName()       override { return fBench->getName(); }
    const char* onGetUniqueName() override { return fBench->getUniqueName(); }

    void onDraw(int loops, SkCanvas* canvas) override {
        // TODO: use onPreDraw() to move offscreen allocation/deallocation out of timing.
        SkAutoTUnref<SkSurface> offscreen(SkSurface::NewRaster(canvas->imageInfo()));

        fBench->draw(loops, offscreen->getCanvas());
        SkAutoTUnref<SkImage> image(offscreen->newImageSnapshot());
        canvas->drawImage(image, 0,0);
    }

private:
    SkAutoTUnref<Benchmark> fBench;
};

#endif//CpuWrappedBenchmark_DEFINED
