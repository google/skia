/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef Benchmark_DEFINED
#define Benchmark_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/private/base/SkTArray.h"
#include "tools/Registry.h"

#define DEF_BENCH3(code, N) \
    static BenchRegistry gBench##N([](void*) -> Benchmark* { code; });
#define DEF_BENCH2(code, N) DEF_BENCH3(code, N)
#define DEF_BENCH(code) DEF_BENCH2(code, __COUNTER__)

/*
 *  With the above macros, you can register benches as follows (at the bottom
 *  of your .cpp)
 *
 *  DEF_BENCH(return new MyBenchmark(...))
 *  DEF_BENCH(return new MyBenchmark(...))
 *  DEF_BENCH(return new MyBenchmark(...))
 */

struct GrContextOptions;
class GrRecordingContext;
class SkCanvas;
class SkPaint;

class Benchmark : public SkRefCnt {
public:
    Benchmark();

    const char* getName();
    const char* getUniqueName();
    SkISize getSize();

    enum class Backend {
        kNonRendering,
        kRaster,
        kGanesh,
        kGraphite,
        kPDF,
        kHWUI,
    };

    // Call to determine whether the benchmark is intended for
    // the rendering mode.
    virtual bool isSuitableFor(Backend backend) {
        return backend != Backend::kNonRendering;
    }

    // Allows a benchmark to override options used to construct the GrContext.
    virtual void modifyGrContextOptions(GrContextOptions*) {}

    // Whether or not this benchmark requires multiple samples to get a meaningful result.
    virtual bool shouldLoop() const {
        return true;
    }

    // Call before draw, allows the benchmark to do setup work outside of the
    // timer. When a benchmark is repeatedly drawn, this should be called once
    // before the initial draw.
    void delayedSetup();

    // Called once before and after a series of draw calls to a single canvas.
    // The setup/break down in these calls is not timed.
    void perCanvasPreDraw(SkCanvas*);
    void perCanvasPostDraw(SkCanvas*);

    // Called just before and after each call to draw().  Not timed.
    void preDraw(SkCanvas*);
    void postDraw(SkCanvas*);

    // Bench framework can tune loops to be large enough for stable timing.
    void draw(int loops, SkCanvas*);

    virtual void getGpuStats(SkCanvas*,
                             skia_private::TArray<SkString>* keys,
                             skia_private::TArray<double>* values) {}

    // Replaces the GrRecordingContext's dmsaaStats() with a single frame of this benchmark.
    virtual bool getDMSAAStats(GrRecordingContext*) { return false; }

    // Count of units (pixels, whatever) being exercised, to scale timing by.
    int getUnits() const { return fUnits; }

protected:
    void setUnits(int units) { SkASSERT(units > 0); fUnits = units; }

    virtual void setupPaint(SkPaint* paint);

    virtual const char* onGetName() = 0;
    virtual const char* onGetUniqueName() { return this->onGetName(); }
    virtual void onDelayedSetup() {}
    virtual void onPerCanvasPreDraw(SkCanvas*) {}
    virtual void onPerCanvasPostDraw(SkCanvas*) {}
    virtual void onPreDraw(SkCanvas*) {}
    virtual void onPostDraw(SkCanvas*) {}
    // Each bench should do its main work in a loop like this:
    //   for (int i = 0; i < loops; i++) { <work here> }
    virtual void onDraw(int loops, SkCanvas*) = 0;

    virtual SkISize onGetSize();

private:
    int fUnits = 1;

    using INHERITED = SkRefCnt;
};

typedef sk_tools::Registry<Benchmark*(*)(void*)> BenchRegistry;

#endif
