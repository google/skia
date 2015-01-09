/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkGeometry.h"
#include "SkRandom.h"
#include "SkRect.h"

class GeometryBench : public Benchmark {
public:
    GeometryBench(const char suffix[]) : fVolatileInt(0) {
        fName.printf("geo_%s", suffix);
    }

    const char* onGetName() SK_OVERRIDE {
        return fName.c_str();
    }

    bool isSuitableFor(Backend backend) SK_OVERRIDE {
        return kNonRendering_Backend == backend;
    }

protected:
    volatile int fVolatileInt;

    /**
     *  Subclasses can call this to try to defeat the optimizer (with some result of their
     *  inner loop), since it will fool the compiler into assuming that "n" is actually
     *  needed somewhere, and since this method is not const, the member fields cannot
     *  be assumed to be const before and after the call.
     */
    virtual void virtualCallToFoilOptimizers(int n) { fVolatileInt += n; }

private:
    SkString fName;
};

class GeoRectBench : public GeometryBench {
public:
    GeoRectBench(const char suffix[]) : GeometryBench(suffix) {}

protected:
    SkRect fRects[2048];

    virtual void onPreDraw() {
        const SkScalar min = -100;
        const SkScalar max = 100;
        SkRandom rand;
        for (size_t i = 0; i < SK_ARRAY_COUNT(fRects); ++i) {
            SkScalar x = rand.nextRangeScalar(min, max);
            SkScalar y = rand.nextRangeScalar(min, max);
            SkScalar w = rand.nextRangeScalar(min, max);
            SkScalar h = rand.nextRangeScalar(min, max);
            fRects[i].setXYWH(x, y, w, h);
        }
    }
};

class GeoRectBench_intersect : public GeoRectBench {
public:
    GeoRectBench_intersect() : GeoRectBench("rect_intersect") {}

protected:
    void onDraw(const int loops, SkCanvas* canvas) SK_OVERRIDE {
        for (int outer = 0; outer < loops; ++outer) {
            int count = 0;
            for (size_t i = 0; i < SK_ARRAY_COUNT(fRects); ++i) {
                SkRect r = fRects[0];
                count += r.intersect(fRects[i]);
            }
            this->virtualCallToFoilOptimizers(count);
        }
    }
};

class GeoRectBench_intersect_rect : public GeoRectBench {
public:
    GeoRectBench_intersect_rect() : GeoRectBench("rect_intersect_rect") {}

protected:
    void onDraw(const int loops, SkCanvas* canvas) SK_OVERRIDE {
        for (int outer = 0; outer < loops; ++outer) {
            int count = 0;
            SkRect r;
            for (size_t i = 0; i < SK_ARRAY_COUNT(fRects); ++i) {
                count += r.intersect(fRects[0], fRects[i]);
            }
            this->virtualCallToFoilOptimizers(count);
        }
    }
};

class GeoRectBench_Intersects : public GeoRectBench {
public:
    GeoRectBench_Intersects() : GeoRectBench("rect_Intersects") {}
    
protected:
    void onDraw(const int loops, SkCanvas* canvas) SK_OVERRIDE {
        for (int outer = 0; outer < loops; ++outer) {
            int count = 0;
            for (size_t i = 0; i < SK_ARRAY_COUNT(fRects); ++i) {
                count += SkRect::Intersects(fRects[0], fRects[i]);
            }
            this->virtualCallToFoilOptimizers(count);
        }
    }
};

class GeoRectBench_sort : public GeoRectBench {
public:
    GeoRectBench_sort() : GeoRectBench("rect_sort") {}
    
protected:
    void onDraw(const int loops, SkCanvas* canvas) SK_OVERRIDE {
        for (int outer = 0; outer < loops; ++outer) {
            for (size_t i = 0; i < SK_ARRAY_COUNT(fRects); ++i) {
                fRects[i].sort();
            }
        }
    }
};

DEF_BENCH( return new GeoRectBench_intersect; )
DEF_BENCH( return new GeoRectBench_intersect_rect; )
DEF_BENCH( return new GeoRectBench_Intersects; )

DEF_BENCH( return new GeoRectBench_sort; )
