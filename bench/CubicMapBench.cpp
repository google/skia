/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
//#include "include/core/SkCubicMap.h"

#include "include/private/SkFixed.h"
#include "include/private/SkNx.h"

#if 0
struct CubicMap {
    using Point = SkNx<4, int>;

    static Point ToPoint(SkPoint p) {
        Point pt = {(int)(p.fX * 65536), (int)(p.fY * 65536), 0, 0};
        return pt;
    }

    CubicMap(SkPoint p1, SkPoint p2) : fP1{ToPoint(p1)}, fP2{ToPoint(p2)} {}
    SkScalar computeYFromX(SkScalar x) {
        Point TargetX = (int)(x * 65536) * 8;

        Point P0 = fP0,
              P1 = fP1,
              P2 = fP2,
              P3 = fP3;

        Point P01121223;
        for (int i = 0; i < 9; i++) {
            Point P01 = (P0 + P1),
                  P12 = (P1 + P2),
                  P23 = (P2 + P3);

            Point P0112 = (P01 + P12),
                  P1223 = (P12 + P23);

            P01121223 = (P0112 + P1223);

            auto cond = P01121223 < TargetX;

            P0 = cond.thenElse(P0, P01121223 >> 3);
            P1 = cond.thenElse(P01 >> 1, P1223 >> 2);
            P2 = cond.thenElse(P0112 >> 2, P23 >> 1);
            P3 = cond.thenElse(P01121223 >> 3, P3);
        }

        return SkFixedToScalar(P01121223[1]);
    }

    const Point fP0 {0, 0, 0, 0};
    const Point fP1;
    const Point fP2;
    const Point fP3 {ToPoint(SkPoint{1, 1})};
};
#endif

#if 1
struct CubicMap {
    using Point = SkNx<4, int>;

    static Point ToPoint(SkPoint p) {
        Point pt = {(int)(p.fX * 65536), (int)(p.fY * 65536), 0, 0};
        return pt;
    }

    CubicMap(SkPoint p1, SkPoint p2) : fP1{ToPoint(p1)}, fP2{ToPoint(p2)} {}
    SkScalar computeYFromX(SkScalar x) {
        int TargetX = (int)(x * 65536) * 8;

        Point P0 = fP0,
              P1 = fP1,
              P2 = fP2,
              P3 = fP3;

        Point P01121223;
        for (int i = 0; i < 10; i++) {
            Point P01 = P0 + P1,
                  P12 = P1 + P2,
                  P23 = P2 + P3;

            Point P0112 = P01 + P12,
                  P1223 = P12 + P23;

            P01 = P01 >> 1;
            P23 = P23 >> 1;

            P01121223 = P0112 + P1223;

            P0112 = P0112 >> 2;
            P1223 = P1223 >> 2;

            int midpointBy8 = P01121223[0];

            P01121223 = P01121223 >> 3;

            if (TargetX < midpointBy8) {
                // P0 = P0;
                P1 = P01;
                P2 = P0112;
                P3 = P01121223;
            } else {
                P0 = P01121223;
                P1 = P1223;
                P2 = P23;
                // P3 = P3;
            }
        }

        return SkFixedToScalar(P01121223[1]);
    }

    const Point fP0 {0, 0, 0, 0};
    const Point fP1;
    const Point fP2;
    const Point fP3 {ToPoint(SkPoint{1, 1})};
};
#endif

#if 0
struct CubicMap {
    using Point = SkNx<4, int>;

    static Point ToPoint(SkPoint p) {
        Point pt = {(int)(p.fX * 65536), (int)(p.fY * 65536), 0, 0};
        return pt;
    }

    CubicMap(SkPoint p1, SkPoint p2) : fP1{ToPoint(p1)}, fP2{ToPoint(p2)} {
        Xs = {0, SkScalarToFixed(p1.fX), SkScalarToFixed(p2.fX), SK_Fixed1};
    }
    SkScalar computeYFromX(SkScalar x) {
        Point TargetX = (int)(x * 65536) * 8;

        Point BezierXs = Xs;

        for (int i = 0; i < 9; i++) {
            Point Lo0 = BezierXs;
            Point Lo1 = Point{0, Lo0[0], Lo0[1], Lo0[2]} + Lo0;
            Point Lo2 = Point{0, 0, Lo1[1], Lo1[2]} + Lo1;
            Point Lo3 = Point{0, 0, 0, Lo2[2]} + Lo2;

            Point Hi3 = Point{Lo3[3], Lo2[3], Lo1[3], Lo0[3]};

            Point cond = TargetX < Point{Lo3[3], Lo3[3], Lo3[3], Lo3[3]};
            BezierXs = cond.thenElse(Lo3, Hi3);
            BezierXs = BezierXs * Point{8, 4, 2, 1};
            BezierXs = BezierXs >> 3;
        }

        return SkFixedToScalar(BezierXs[1]); // This is very wrong
    }

    Point Xs;

    const Point fP0 {0, 0, 0, 0};
    const Point fP1;
    const Point fP2;
    const Point fP3 {ToPoint(SkPoint{1, 1})};
};
#endif

#if 0
struct CubicMap {
    using Point = SkNx<4, int>;

    static Point ToPoint(SkPoint p) {
        Point pt = {(int)(p.fX * 65536), (int)(p.fY * 65536), 0, 0};
        return pt;
    }

    CubicMap(SkPoint p1, SkPoint p2) : fP1{ToPoint(p1)}, fP2{ToPoint(p2)} {
        Xs = {0, SkScalarToFixed(p1.fX), SkScalarToFixed(p2.fX), SK_Fixed1};
    }
    SkScalar computeYFromX(SkScalar x) {
        int TargetX = (int)(x * 65536) * 8;

        Point BezierXs = Xs;

        for (int i = 0; i < 9; i++) {
            Point acc0 = BezierXs;
            Point acc1 = Point{0, acc0[0], acc0[1], acc0[2]} + acc0;
            Point acc2 = Point{0, acc1[0], acc1[1], acc1[2]} + acc1;
            Point acc3 = Point{0, acc2[0], acc2[1], acc2[2]} + acc2;

            if (TargetX <= acc3[3]) {
                BezierXs = Point{acc0[0], acc1[1], acc2[2], acc3[3]};
            } else {
                BezierXs = Point{acc3[3], acc2[3], acc1[3], acc0[3]};
            }
            BezierXs = BezierXs *
                    Point{SK_Fixed1, SK_FixedHalf, SK_FixedQuarter, SK_FixedQuarter * SK_FixedHalf};
        }

        return SkFixedToScalar(BezierXs[1]); // This is very wrong
    }

    Point Xs;

    const Point fP0 {0, 0, 0, 0};
    const Point fP1;
    const Point fP2;
    const Point fP3 {ToPoint(SkPoint{1, 1})};
};
#endif

class CubicMapBench : public Benchmark {
public:
    CubicMapBench(SkPoint p1, SkPoint p2) : fCMap(p1, p2) {
        fName.printf("cubicmap_%g_%g_%g_%g", p1.fX, p1.fY, p2.fX, p2.fY);
    }

    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

    const char* onGetName() override {
        return fName.c_str();
    }

    void onDraw(int loops, SkCanvas*) override {
        SkScalar accum = 0;
        for (int outer = 0; outer < 100; ++outer) {
            for (int i = 0; i < loops; ++i) {
                for (SkScalar x = 0; x <= 1; x += 1.0f / 512) {
                    accum += fCMap.computeYFromX(x);
                }
            }
        }
        volatile float fred = accum;
    }

private:
    CubicMap  fCMap;
    SkString    fName;

    typedef Benchmark INHERITED;
};

DEF_BENCH( return new CubicMapBench({1, 0}, {0,0}); )
DEF_BENCH( return new CubicMapBench({1, 0}, {0,1}); )
DEF_BENCH( return new CubicMapBench({1, 0}, {1,0}); )
DEF_BENCH( return new CubicMapBench({1, 0}, {1,1}); )

DEF_BENCH( return new CubicMapBench({0, 1}, {0,0}); )
DEF_BENCH( return new CubicMapBench({0, 1}, {0,1}); )
DEF_BENCH( return new CubicMapBench({0, 1}, {1,0}); )
DEF_BENCH( return new CubicMapBench({0, 1}, {1,1}); )

DEF_BENCH( return new CubicMapBench({0, 0}, {1,1}); )
DEF_BENCH( return new CubicMapBench({1, 1}, {0,0}); )
