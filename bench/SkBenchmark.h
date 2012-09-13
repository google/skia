
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

protected:
    void setupPaint(SkPaint* paint);

    virtual const char* onGetName() = 0;
    virtual void onPreDraw() {}
    virtual void onDraw(SkCanvas*) = 0;
    virtual void onPostDraw() {}

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

    typedef SkRefCnt INHERITED;
};

typedef SkTRegistry<SkBenchmark*, void*> BenchRegistry;

#endif
