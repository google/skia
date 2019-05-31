/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef Benchmark_DEFINED
#define Benchmark_DEFINED

#include "include/core/SkPoint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkString.h"
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
class SkCanvas;
class SkPaint;

class Benchmark : public SkRefCnt {
public:
    Benchmark();

    const char* getName();
    const char* getUniqueName();
    SkIPoint getSize();

    enum Backend {
        kNonRendering_Backend,
        kRaster_Backend,
        kGPU_Backend,
        kPDF_Backend,
        kHWUI_Backend,
    };

    // Call to determine whether the benchmark is intended for
    // the rendering mode.
    virtual bool isSuitableFor(Backend backend) {
        return backend != kNonRendering_Backend;
    }

    // Allows a benchmark to override options used to construct the GrContext.
    virtual void modifyGrContextOptions(GrContextOptions*) {}

    virtual int calculateLoops(int defaultLoops) const {
        return defaultLoops;
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

    virtual void getGpuStats(SkCanvas*, SkTArray<SkString>* keys, SkTArray<double>* values) {}

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

    virtual SkIPoint onGetSize();

private:
    int fUnits = 1;

    typedef SkRefCnt INHERITED;
};

typedef sk_tools::Registry<Benchmark*(*)(void*)> BenchRegistry;

#endif
