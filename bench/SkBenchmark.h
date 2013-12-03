
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkBenchmark_DEFINED
#define SkBenchmark_DEFINED

#include "SkRefCnt.h"
#include "SkPoint.h"
#include "SkString.h"
#include "SkTRegistry.h"

#define DEF_BENCH(code) \
static SkBenchmark* SK_MACRO_APPEND_LINE(F_)() { code; } \
static BenchRegistry SK_MACRO_APPEND_LINE(R_)(SK_MACRO_APPEND_LINE(F_));

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

class SkBenchmark : public SkRefCnt {
public:
    SK_DECLARE_INST_COUNT(SkBenchmark)

    SkBenchmark();

    const char* getName();
    SkIPoint getSize();

    enum Backend {
        kNonRendering_Backend,
        kRaster_Backend,
        kGPU_Backend,
        kPDF_Backend,
    };

    // Call to determine whether the benchmark is intended for
    // the rendering mode.
    virtual bool isSuitableFor(Backend backend) {
        return backend != kNonRendering_Backend;
    }

    // Call before draw, allows the benchmark to do setup work outside of the
    // timer. When a benchmark is repeatedly drawn, this should be called once
    // before the initial draw.
    void preDraw();

    // Bench framework can tune loops to be large enough for stable timing.
    void draw(const int loops, SkCanvas*);

    // Call after draw, allows the benchmark to do cleanup work outside of the
    // timer. When a benchmark is repeatedly drawn, this is only called once
    // after the last draw.
    void postDraw();

    void setForceAlpha(int alpha) {
        fForceAlpha = alpha;
    }

    void setForceAA(bool aa) {
        fForceAA = aa;
    }

    void setForceFilter(bool filter) {
        fForceFilter = filter;
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

    static void SetResourcePath(const char* resPath) { gResourcePath.set(resPath); }

    static SkString& GetResourcePath() { return gResourcePath; }

protected:
    virtual void setupPaint(SkPaint* paint);

    virtual const char* onGetName() = 0;
    virtual void onPreDraw() {}
    // Each bench should do its main work in a loop like this:
    //   for (int i = 0; i < loops; i++) { <work here> }
    virtual void onDraw(const int loops, SkCanvas*) = 0;
    virtual void onPostDraw() {}

    virtual SkIPoint onGetSize();

private:
    int     fForceAlpha;
    bool    fForceAA;
    bool    fForceFilter;
    SkTriState::State  fDither;
    uint32_t    fOrMask, fClearMask;
    static  SkString gResourcePath;

    typedef SkRefCnt INHERITED;
};

typedef SkTRegistry<SkBenchmark*(*)()> BenchRegistry;

#endif
