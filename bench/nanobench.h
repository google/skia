/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef nanobench_DEFINED
#define nanobench_DEFINED

#include "bench/Benchmark.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "tools/gpu/GrContextFactory.h"
#if defined(SK_GRAPHITE)
#include "tools/graphite/ContextFactory.h"
#endif

class SkBitmap;
class SkCanvas;
class NanoJSONResultsWriter;

struct Config {
    SkString name;
    Benchmark::Backend backend;
    SkColorType color;
    SkAlphaType alpha;
    sk_sp<SkColorSpace> colorSpace;
    int samples;
    sk_gpu_test::GrContextFactory::ContextType ctxType;
    sk_gpu_test::GrContextFactory::ContextOverrides ctxOverrides;
    uint32_t surfaceFlags;
};

struct Target {
    explicit Target(const Config& c) : config(c) { }
    virtual ~Target() { }

    const Config config;
    sk_sp<SkSurface> surface;

    /** Called once per target, immediately before any timing or drawing. */
    void setup() {
        this->onSetup();
        // Make sure we're done with setup work before we start timing.
        this->submitWorkAndSyncCPU();
    }
    virtual void onSetup() { }

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
    virtual void submitWorkAndSyncCPU() { }

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

    /** Writes gathered stats using SkDebugf. */
    virtual void dumpStats() {}

    SkCanvas* getCanvas() const {
        if (!surface) {
            return nullptr;
        }
        return surface->getCanvas();
    }
};

#endif  // nanobench_DEFINED
