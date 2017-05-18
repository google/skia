/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef nanobench_DEFINED
#define nanobench_DEFINED

#include "Benchmark.h"
#include "SkImageInfo.h"
#include "SkSurface.h"
#include "SkTypes.h"

#if SK_SUPPORT_GPU
#include "GrContextFactory.h"
#endif

class ResultsWriter;
class SkBitmap;
class SkCanvas;

struct Config {
    SkString name;
    Benchmark::Backend backend;
    SkColorType color;
    SkAlphaType alpha;
    sk_sp<SkColorSpace> colorSpace;
    int samples;
#if SK_SUPPORT_GPU
    sk_gpu_test::GrContextFactory::ContextType ctxType;
    sk_gpu_test::GrContextFactory::ContextOverrides ctxOverrides;
    bool useDFText;
#else
    int bogusInt;
    int bogusIntOption;
    bool bogusBool;
#endif
};

struct Target {
    explicit Target(const Config& c) : config(c) { }
    virtual ~Target() { }

    const Config config;
    sk_sp<SkSurface> surface;

    /** Called once per target, immediately before any timing or drawing. */
    virtual void setup() { }

    /** Called *after* the clock timer is started, before the benchmark
        is drawn. Most back ends just return the canvas passed in,
        but some may replace it. */
    virtual SkCanvas* beginTiming(SkCanvas* canvas) { return canvas; }

    /** Called *after* a benchmark is drawn, but before the clock timer
        is stopped.  */
    virtual void endTiming() { }

    /** Called between benchmarks (or between calibration and measured
        runs) to make sure all pending work in drivers / threads is
        complete. */
    virtual void fence() { }

    /** CPU-like targets can just be timed, but GPU-like
        targets need to pay attention to frame boundaries
        or other similar details. */
    virtual bool needsFrameTiming(int* frameLag) const { return false; }

    /** Called once per target, during program initialization.
        Returns false if initialization fails. */
    virtual bool init(SkImageInfo info, Benchmark* bench);

    /** Stores any pixels drawn to the screen in the bitmap.
        Returns false on error. */
    virtual bool capturePixels(SkBitmap* bmp);

    /** Writes any config-specific data to the log. */
    virtual void fillOptions(ResultsWriter*) { }

    SkCanvas* getCanvas() const {
        if (!surface.get()) {
            return nullptr;
        }
        return surface->getCanvas();
    }
};

#endif  // nanobench_DEFINED
