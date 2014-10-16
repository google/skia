/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "Sk4x.h"
#include "SkGeometry.h"
#include "SkRandom.h"
#include "SkRect.h"

class GeometryBench : public Benchmark {
public:
    GeometryBench(const char suffix[]) : fVolatileInt(0) {
        fName.printf("geo_%s", suffix);
    }

    virtual const char* onGetName() SK_OVERRIDE {
        return fName.c_str();
    }

    virtual bool isSuitableFor(Backend backend) SK_OVERRIDE {
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
    // void* vptr;
    size_t align_fRects_to_16Bytes[sizeof(void*) == 8 ? 1 : 3];

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
    virtual void onDraw(const int loops, SkCanvas* canvas) SK_OVERRIDE {
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
    virtual void onDraw(const int loops, SkCanvas* canvas) SK_OVERRIDE {
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
    virtual void onDraw(const int loops, SkCanvas* canvas) SK_OVERRIDE {
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
    virtual void onDraw(const int loops, SkCanvas* canvas) SK_OVERRIDE {
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

class GeoRectBench_sort_4f : public GeoRectBench {
public:
    GeoRectBench_sort_4f() : GeoRectBench("rect_sort_4f") { }

protected:
    static SkRect Sort(const SkRect& rect) {
        // To sort:
        //   left, right = minmax(left, right)
        //   top, bottom = minmax(top, bottom)
        Sk4f ltrb(&rect.fLeft),
             rblt = ltrb.zwxy(),
             ltlt = Sk4f::Min(ltrb, rblt),  // Holds (2 copies of) new left and top.
             rbrb = Sk4f::Max(ltrb, rblt),  // Holds (2 copies of) new right and bottom.
             sort = Sk4f::XYAB(ltlt, rbrb);

        SkRect sorted;
        sort.store(&sorted.fLeft);
        return sorted;
    }

    virtual void onDraw(const int loops, SkCanvas* canvas) SK_OVERRIDE {
        for (int outer = 0; outer < loops; ++outer) {
            for (size_t i = 0; i < SK_ARRAY_COUNT(fRects); ++i) {
                fRects[i] = Sort(fRects[i]);
            }
        }
    }
};
DEF_BENCH( return new GeoRectBench_sort_4f; )

class GeoRectBench_Intersects_4f : public GeoRectBench {
public:
    GeoRectBench_Intersects_4f() : GeoRectBench("rect_Intersects_4f") {}

protected:
    static bool Intersects(const SkRect& a, const SkRect& b) {
        Sk4f r1(&a.fLeft),
             r2(&b.fLeft),
             lt = Sk4f::XYAB(r1, r2),  // a.L a.T b.L b.T <
             rb = Sk4f::ZWCD(r2, r1);  // b.R b.B a.R a.B ?
        return lt.lessThan(rb).allTrue();
    }

    virtual void onDraw(const int loops, SkCanvas* canvas) SK_OVERRIDE {
        for (int outer = 0; outer < loops; ++outer) {
            int count = 0;
            for (size_t i = 0; i < SK_ARRAY_COUNT(fRects); ++i) {
                count += Intersects(fRects[0], fRects[i]);
            }
            this->virtualCallToFoilOptimizers(count);
        }
    }
};
DEF_BENCH( return new GeoRectBench_Intersects_4f; )

