/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkRect.h"
#include "include/utils/SkRandom.h"
#include "src/core/SkGeometry.h"

class GeometryBench : public Benchmark {
public:
    GeometryBench(const char suffix[]) : fVolatileInt(0) {
        fName.printf("geo_%s", suffix);
    }

    const char* onGetName() override {
        return fName.c_str();
    }

    bool isSuitableFor(Backend backend) override {
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

    virtual void onDelayedSetup() {
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
    void onDraw(int loops, SkCanvas* canvas) override {
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
    void onDraw(int loops, SkCanvas* canvas) override {
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
    void onDraw(int loops, SkCanvas* canvas) override {
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
    void onDraw(int loops, SkCanvas* canvas) override {
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

///////////////////////////////////////////////////////////////////////////////////////////////////

class QuadBenchBase : public GeometryBench {
protected:
    SkPoint fPts[4];
public:
    QuadBenchBase(const char name[]) : GeometryBench(name) {
        SkRandom rand;
        for (int i = 0; i < 4; ++i) {
            fPts[i].set(rand.nextUScalar1(), rand.nextUScalar1());
        }
    }
};

class EvalQuadAt0 : public QuadBenchBase {
public:
    EvalQuadAt0() : QuadBenchBase("evalquadat0") {}
protected:
    void onDraw(int loops, SkCanvas* canvas) override {
        SkPoint result;
        for (int outer = 0; outer < loops; ++outer) {
            SkEvalQuadAt(fPts, 0.5f, &result);
            SkEvalQuadAt(fPts, 0.5f, &result);
            SkEvalQuadAt(fPts, 0.5f, &result);
            SkEvalQuadAt(fPts, 0.5f, &result);
        }
    }
};
DEF_BENCH( return new EvalQuadAt0; )

class EvalQuadAt1 : public QuadBenchBase {
public:
    EvalQuadAt1() : QuadBenchBase("evalquadat1") {}
protected:
    void onDraw(int loops, SkCanvas* canvas) override {
        SkPoint result;
        for (int outer = 0; outer < loops; ++outer) {
            result = SkEvalQuadAt(fPts, 0.5f);
            result = SkEvalQuadAt(fPts, 0.5f);
            result = SkEvalQuadAt(fPts, 0.5f);
            result = SkEvalQuadAt(fPts, 0.5f);
        }
    }
};
DEF_BENCH( return new EvalQuadAt1; )

////////

class EvalQuadTangentAt0 : public QuadBenchBase {
public:
    EvalQuadTangentAt0() : QuadBenchBase("evalquadtangentat0") {}
protected:
    void onDraw(int loops, SkCanvas* canvas) override {
        SkPoint result;
        for (int outer = 0; outer < loops; ++outer) {
            SkEvalQuadAt(fPts, 0.5f, nullptr, &result);
            SkEvalQuadAt(fPts, 0.5f, nullptr, &result);
            SkEvalQuadAt(fPts, 0.5f, nullptr, &result);
            SkEvalQuadAt(fPts, 0.5f, nullptr, &result);
        }
    }
};
DEF_BENCH( return new EvalQuadTangentAt0; )

class EvalQuadTangentAt1 : public QuadBenchBase {
public:
    EvalQuadTangentAt1() : QuadBenchBase("evalquadtangentat1") {}
protected:
    void onDraw(int loops, SkCanvas* canvas) override {
        SkPoint result;
        for (int outer = 0; outer < loops; ++outer) {
            result = SkEvalQuadTangentAt(fPts, 0.5f);
            result = SkEvalQuadTangentAt(fPts, 0.5f);
            result = SkEvalQuadTangentAt(fPts, 0.5f);
            result = SkEvalQuadTangentAt(fPts, 0.5f);
        }
    }
};
DEF_BENCH( return new EvalQuadTangentAt1; )

////////

class ChopQuadAt : public QuadBenchBase {
public:
    ChopQuadAt() : QuadBenchBase("chopquadat") {}
protected:
    void onDraw(int loops, SkCanvas* canvas) override {
        SkPoint dst[5];
        for (int outer = 0; outer < loops; ++outer) {
            SkChopQuadAt(fPts, dst, 0.5f);
            SkChopQuadAt(fPts, dst, 0.5f);
            SkChopQuadAt(fPts, dst, 0.5f);
            SkChopQuadAt(fPts, dst, 0.5f);
        }
    }
};
DEF_BENCH( return new ChopQuadAt; )

class ChopCubicAt : public QuadBenchBase {
public:
    ChopCubicAt() : QuadBenchBase("chopcubicat0") {}
protected:
    void onDraw(int loops, SkCanvas* canvas) override {
        SkPoint dst[7];
        for (int outer = 0; outer < loops; ++outer) {
            SkChopCubicAt(fPts, dst, 0.5f);
            SkChopCubicAt(fPts, dst, 0.5f);
            SkChopCubicAt(fPts, dst, 0.5f);
            SkChopCubicAt(fPts, dst, 0.5f);
        }
    }
};
DEF_BENCH( return new ChopCubicAt; )

#include "include/core/SkPath.h"

class ConvexityBench : public Benchmark {
    SkPath fPath;

public:
    ConvexityBench(const char suffix[]) {
        fName.printf("convexity_%s", suffix);
    }

    const char* onGetName() override {
        return fName.c_str();
    }

    bool isSuitableFor(Backend backend) override {
        return kNonRendering_Backend == backend;
    }

    virtual void preparePath(SkPath*) = 0;

protected:
    void onPreDraw(SkCanvas*) override {
        this->preparePath(&fPath);
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        for (int i = 0; i < loops; ++i) {
            fPath.setConvexity(SkPathConvexityType::kUnknown);
            (void)fPath.isConvex();
        }
    }

private:
    SkString fName;
};

class RRectConvexityBench : public ConvexityBench {
public:
    RRectConvexityBench() : ConvexityBench("rrect") {}

    void preparePath(SkPath* path) override {
        SkRRect rr;
        rr.setRectXY({0, 0, 100, 100}, 20, 30);
        path->addRRect(rr);
    }
};
DEF_BENCH( return new RRectConvexityBench; )

