/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef Benchmark_DEFINED
#define Benchmark_DEFINED

#include "SkPoint.h"
#include "SkRefCnt.h"
#include "SkString.h"
#include "SkTRegistry.h"

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


class SkCanvas;
class SkPaint;

class SkTriState {
public:
    enum State {
        kDefault,
        kTrue,
        kFalse
    };
    static const char* Name[];
};

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

    virtual int calculateLoops(int defaultLoops) const {
        return defaultLoops;
    }

    // Call before draw, allows the benchmark to do setup work outside of the
    // timer. When a benchmark is repeatedly drawn, this should be called once
    // before the initial draw.
    void preDraw();

    // Called once before and after a series of draw calls to a single canvas.
    // The setup/break down in these calls is not timed.
    void perCanvasPreDraw(SkCanvas*);
    void perCanvasPostDraw(SkCanvas*);

    // Bench framework can tune loops to be large enough for stable timing.
    void draw(const int loops, SkCanvas*);

    void setForceAlpha(int alpha) {
        fForceAlpha = alpha;
    }

    void setDither(SkTriState::State state) {
        fDither = state;
    }

    /** Assign masks for paint-flags. These will be applied when setupPaint()
     *  is called.
     *
     *  Performs the following on the paint:
     *      uint32_t flags = paint.getFlags();
     *      flags &= ~clearMask;
     *      flags |= orMask;
     *      paint.setFlags(flags);
     */
    void setPaintMasks(uint32_t orMask, uint32_t clearMask) {
        fOrMask = orMask;
        fClearMask = clearMask;
    }

    /*
     * Benches which support running in a visual mode can advertise this functionality
     */
    virtual bool isVisual() { return false; }

protected:
    virtual void setupPaint(SkPaint* paint);

    virtual const char* onGetName() = 0;
    virtual const char* onGetUniqueName() { return this->onGetName(); }
    virtual void onPreDraw() {}
    virtual void onPerCanvasPreDraw(SkCanvas*) {}
    virtual void onPerCanvasPostDraw(SkCanvas*) {}
    // Each bench should do its main work in a loop like this:
    //   for (int i = 0; i < loops; i++) { <work here> }
    virtual void onDraw(const int loops, SkCanvas*) = 0;

    virtual SkIPoint onGetSize();

private:
    int     fForceAlpha;
    SkTriState::State  fDither;
    uint32_t    fOrMask, fClearMask;

    typedef SkRefCnt INHERITED;
};

typedef SkTRegistry<Benchmark*(*)(void*)> BenchRegistry;

#endif
