
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
#include "SkTDict.h"
#include "SkTRegistry.h"

#define DEF_BENCH(code) \
static SkBenchmark* SK_MACRO_APPEND_LINE(F_)(void* p) { code; } \
static BenchRegistry SK_MACRO_APPEND_LINE(R_)(SK_MACRO_APPEND_LINE(F_));

/*
 *  With the above macros, you can register benches as follows (at the bottom
 *  of your .cpp)
 *
 *  DEF_BENCH(new MyBenchmark(p, ...))
 *  DEF_BENCH(new MyBenchmark(p, ...))
 *  DEF_BENCH(new MyBenchmark(p, ...))
 */


#ifdef SK_DEBUG
    #define SkBENCHLOOP(n) 1
#else
    #define SkBENCHLOOP(n) n
#endif

class SkCanvas;
class SkPaint;

class SkTriState {
public:
    enum State {
        kDefault,
        kTrue,
        kFalse
    };
};

class SkBenchmark : public SkRefCnt {
public:
    SK_DECLARE_INST_COUNT(SkBenchmark)

    SkBenchmark(void* defineDict);

    const char* getName();
    SkIPoint getSize();

    // Call before draw, allows the benchmark to do setup work outside of the
    // timer. When a benchmark is repeatedly drawn, this should be called once
    // before the initial draw.
    void preDraw();

    void draw(SkCanvas*);

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

    void setStrokeWidth(SkScalar width) {
      strokeWidth = width;
      fHasStrokeWidth = true;
    }

    SkScalar getStrokeWidth() {
      return strokeWidth;
    }

    bool hasStrokeWidth() {
      return fHasStrokeWidth;
    }

    /** If true; the benchmark does rendering; if false, the benchmark
        doesn't, and so need not be re-run in every different rendering
        mode. */
    bool isRendering() {
        return fIsRendering;
    }

    const char* findDefine(const char* key) const;
    bool findDefine32(const char* key, int32_t* value) const;
    bool findDefineScalar(const char* key, SkScalar* value) const;

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

    float getDurationScale() { return this->onGetDurationScale(); }

protected:
    virtual void setupPaint(SkPaint* paint);

    virtual const char* onGetName() = 0;
    virtual void onPreDraw() {}
    virtual void onDraw(SkCanvas*) = 0;
    virtual void onPostDraw() {}
    // the caller will scale the computed duration by this value. It allows a
    // slow bench to run fewer inner loops, but return the corresponding scale
    // so that its reported duration can be compared against other benches.
    // e.g.
    //      if I run 10x slower, I can run 1/10 the number of inner-loops, but
    //      return 10.0 for my durationScale, so I "report" the honest duration.
    virtual float onGetDurationScale() { return 1; }

    virtual SkIPoint onGetSize();
    /// Defaults to true.
    bool    fIsRendering;

private:
    const SkTDict<const char*>* fDict;
    int     fForceAlpha;
    bool    fForceAA;
    bool    fForceFilter;
    SkTriState::State  fDither;
    bool    fHasStrokeWidth;
    SkScalar strokeWidth;
    uint32_t    fOrMask, fClearMask;

    typedef SkRefCnt INHERITED;
};

typedef SkTRegistry<SkBenchmark*, void*> BenchRegistry;

#endif
