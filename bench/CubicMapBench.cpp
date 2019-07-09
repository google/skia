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

int overallSum0;
int overallSum1;

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

#if 0
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
        for (int i = 0; i < 9; i++) {
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

struct Stats{

    ~Stats() {
        overallSum0 += sum0;
        overallSum1 += sum1;
        overallMax = std::max(overallMax, max);
        SkDebugf("max: %d avg: %g overall max: %d, overall avg: %g\n",
                max, (float)sum1/sum0, overallMax, (float)overallSum1/overallSum0);
    }

    void sample(int64_t s) {
        sum0 += 1;
        sum1 += s;
        max = std::max(max, s);
    }

    int64_t max = 0;
    int64_t sum0 = 0, sum1 = 0;
    static int64_t overallSum0, overallSum1, overallMax;
};

int64_t Stats::overallSum0 = 0;
int64_t Stats::overallSum1 = 0;
int64_t Stats::overallMax = 0;

struct StatsError {

    void sample(SkScalar a, SkScalar b) {
        SkScalar e = std::abs(a - b);
        sum0 += 1;
        sum1 += e;
        maxError = std::max(maxError, e);
    }

    ~StatsError() {
        SkDebugf("maxError %g avg %g\n", maxError, sum1/sum0);
    }

    SkScalar sum0 = 0;
    SkScalar sum1 = 0;
    SkScalar maxError = 0;
};

struct CubicMapGood {
    using Co = SkNx<4, float>;

    CubicMapGood(SkPoint P1, SkPoint P2) {

        float p1 = P1.fX,
              p2 = P2.fX;

        // 3 * p1 * t + (-6 * p1 + 3 * p2) * t^2 + (1 + 3 * p1 - 3 * p2) * t^3
        float c = 3 * p1;
        float b = 3 * p2;
        float a = 1 + c - b;
        b = b - c - c;

        A = a;
        B = b;
        C = c;

        // 3 * p1 + (-12 * p1 + 6 * p2) * t + ( 3 + 9 * p1 - 9 * p2) * t^2
        DA = A * 3;
        DB = B * 2;
        DC = C * 1;

        p1 = P1.fY;
        p2 = P2.fY;
        c = 3 * p1;
        b = 3 * p2;
        a = 1 + c - b;
        b = b - c - c;

        ay = a;
        by = b;
        cy = c;
    }

    Co eval(Co t) {
        // D + t * (C + t * (B + A * t))
        return D + t * (C + t * ( B + A * t));
    }

    Co derivative(Co t) {
        return DC + t * (DB + DA * t);
    }

    static constexpr float kWindow = 0.39f;

    SkScalar computeYFromX(SkScalar x) {
        if (x == 0) {return 0;}
        if (x == 1) {return 1;}

        Co T = x;
        D = -x;

        SkScalar leftWing = std::min(x, kWindow);
        SkScalar rightWing = std::min(1-x, kWindow);

        T = T + Co{leftWing, leftWing, rightWing, rightWing} * Co{-3.0f/5, -1.0f/5, 1.0f/5, 3.0f/5};

        Co guess = eval(T);
        Co howClose = guess.abs();

        SkScalar minimum = howClose.min();

        SkScalar bestGuess = 0;
        // Make T the best guess.
        if (howClose[0] == minimum) {
            bestGuess = guess[0];
            T = T[0];
        }
        if (howClose[1] == minimum) {
            bestGuess = guess[1];
            T = T[1];
        }
        if (howClose[2] == minimum) {
            bestGuess = guess[2];
            T = T[2];
        }
        if (howClose[3] == minimum) {
            bestGuess = guess[3];
            T = T[3];
        }

        Co spread{0};
        if (bestGuess < 0) {
            // answer is higher than t.
            SkScalar interval = std::min(kWindow * 1.0f/5, 1.0f - T[0]);
            spread = Co{0.25f, 0.5f, 0.75f, 1.0f} * interval;
        } else {
            // answer is lower than t.
            SkScalar interval = std::min(kWindow * 1.0f/5, T[0]);
            spread = Co{-1.0f, -0.75f, -0.5f, -0.25f} * interval;
        }
        T += spread;

        Co epsilon = Co{0.00005f};
        int i;
        for (i = 0; i < 15; i++) {
            guess = eval(T);
            howClose = guess.abs();
            Co closeEnough = howClose < epsilon;
            if (closeEnough.anyTrue()) { break; }
            T = T - guess * derivative(T).invert();
        }
        stats.sample(i + 1);

        SkScalar t = T[0];
        SkScalar distance = howClose[0];
        if (howClose[1] < distance) {
            t = T[1];
        }
        if (howClose[2] < distance) {
            t = T[2];
        }
        if (howClose[3] < distance) {
            t = T[3];
        }
        return ((ay * t) + by) * t + cy;
    }

    Co A, B, C, D;
    Co DA, DB, DC;
    SkScalar ay, by, cy;
    Stats stats;
};


struct CubicMapNext {
    using Co = SkNx<4, float>;

    CubicMapNext(SkPoint P1, SkPoint P2) {

        float p1 = P1.fX,
                p2 = P2.fX;

        // 3 * p1 * t + (-6 * p1 + 3 * p2) * t^2 + (1 + 3 * p1 - 3 * p2) * t^3
        float c = 3 * p1;
        float b = 3 * p2;
        float a = 1 + c - b;
        b = b - c - c;

        A = a;
        B = b;
        C = c;

        // 3 * p1 + (-12 * p1 + 6 * p2) * t + ( 3 + 9 * p1 - 9 * p2) * t^2
        DA = A * 3;
        DB = B * 2;
        DC = C * 1;

        p1 = P1.fY;
        p2 = P2.fY;
        c = 3 * p1;
        b = 3 * p2;
        a = 1 + c - b;
        b = b - c - c;

        ay = a;
        by = b;
        cy = c;
    }

    Co eval(Co t) {
        // D + t * (C + t * (B + A * t))
        return D + t * (C + t * ( B + A * t));
    }

    Co derivative(Co t) {
        return DC + t * (DB + DA * t);
    }

    static constexpr float kWindow = 0.39f;

    SkScalar computeYFromX(SkScalar x) {
        if (x == 0) {return 0;}
        if (x == 1) {return 1;}

        Co T = x;
        D = -x;

        SkScalar leftWing = std::min(x, kWindow);
        SkScalar rightWing = std::min(1-x, kWindow);

        T = T + Co{leftWing, leftWing, rightWing, rightWing} * Co{-3.0f/5, -1.0f/5, 1.0f/5, 3.0f/5};

        Co guess = eval(T);
        Co howClose = guess.abs();

        SkScalar minimum = howClose.min();

        SkScalar bestGuess = 0;
        // Make T the best guess.
        if (howClose[0] == minimum) {
            bestGuess = guess[0];
            T = T[0];
        }
        if (howClose[1] == minimum) {
            bestGuess = guess[1];
            T = T[1];
        }
        if (howClose[2] == minimum) {
            bestGuess = guess[2];
            T = T[2];
        }
        if (howClose[3] == minimum) {
            bestGuess = guess[3];
            T = T[3];
        }

        Co spread{0};
        if (bestGuess < 0) {
            // answer is higher than t.
            SkScalar interval = std::min(kWindow * 1.0f/10, 1.0f - T[0]);
            spread = Co{0.25f, 0.5f, 0.75f, 1.0f} * interval;
        } else {
            // answer is lower than t.
            SkScalar interval = std::min(kWindow * 1.0f/10, T[0]);
            spread = Co{-1.0f, -0.75f, -0.5f, -0.25f} * interval;
        }
        T += spread;

        Co epsilon = Co{0.00005f};
        int i;
        for (i = 0; i < 15; i++) {
            guess = eval(T);
            howClose = guess.abs();
            Co closeEnough = howClose < epsilon;
            if (closeEnough.anyTrue()) { break; }
            T = T - guess * derivative(T).invert();
        }
        stats.sample(i + 1);

        SkScalar t = T[0];
        SkScalar distance = howClose[0];
        if (howClose[1] < distance) {
            t = T[1];
        }
        if (howClose[2] < distance) {
            t = T[2];
        }
        if (howClose[3] < distance) {
            t = T[3];
        }

        SkASSERT(0 <= t && t <= 1.0);

        return ((ay * t) + by) * t + cy;
    }

    Co A, B, C, D;
    Co DA, DB, DC;
    SkScalar ay, by, cy;
    Stats stats;
};

const float CubicMapNext::kWindow;

struct CubicMapNext2 {
    using Co = SkNx<8, float>;

    CubicMapNext2(SkPoint P1, SkPoint P2) {

        float p1 = P1.fX,
                p2 = P2.fX;

        // 3 * p1 * t + (-6 * p1 + 3 * p2) * t^2 + (1 + 3 * p1 - 3 * p2) * t^3
        float c = 3 * p1;
        float b = 3 * p2;
        float a = 1 + c - b;
        b = b - c - c;

        A = a;
        B = b;
        C = c;

        // 3 * p1 + (-12 * p1 + 6 * p2) * t + ( 3 + 9 * p1 - 9 * p2) * t^2
        DA = A * 3;
        DB = B * 2;
        DC = C * 1;

        p1 = P1.fY;
        p2 = P2.fY;
        c = 3 * p1;
        b = 3 * p2;
        a = 1 + c - b;
        b = b - c - c;

        ay = a;
        by = b;
        cy = c;
    }

    Co eval(Co t) {
        // D + t * (C + t * (B + A * t))
        return D + t * (C + t * ( B + A * t));
    }

    Co derivative(Co t) {
        return DC + t * (DB + DA * t);
    }

    static constexpr float kWindow = 0.39f;

    SkScalar computeYFromX(SkScalar x) {
        if (x == 0) {return 0;}
        if (x == 1) {return 1;}

        Co T = x;
        D = -x;

        float g = 9.0f;

        SkScalar leftWing = std::min(x, kWindow);
        SkScalar rightWing = std::min(1-x, kWindow);

        T = T + Co{leftWing, leftWing, leftWing, leftWing, rightWing, rightWing, rightWing, rightWing}
                * Co{-7/g, -5/g, -3/g, -1/g, 1/g, 3/g, 5/g, 7/g};

        Co guess = eval(T);
        Co howClose = guess.abs();

        SkScalar minimum = howClose.min();

        SkScalar t = T[0];
        SkScalar bestGuess = guess[0];
        // Make T the best guess.
        if (howClose[1] == minimum) {
            bestGuess = guess[1];
            t = T[1];
        }
        if (howClose[2] == minimum) {
            bestGuess = guess[2];
            t = T[2];
        }
        if (howClose[3] == minimum) {
            bestGuess = guess[3];
            t = T[3];
        }
        if (howClose[4] == minimum) {
            bestGuess = guess[4];
            t = T[4];
        }
        if (howClose[5] == minimum) {
            bestGuess = guess[5];
            t = T[5];
        }
        if (howClose[6] == minimum) {
            bestGuess = guess[6];
            t = T[6];
        }
        if (howClose[7] == minimum) {
            bestGuess = guess[7];
            t = T[7];
        }

        T = t;

        float k = 8.0f;

        Co spread = Co{1/k, 2/k, 3/k, 4/k, 5/k, 6/k, 7/k, 8/k};
        if (bestGuess < 0) {
            // answer is higher than t.
            spread *= std::min(kWindow * 1/(k), 1.0f - t);
        } else {
            // answer is lower than t.
            spread *= -std::min(kWindow * 1/(k), t);
        }
        T += spread;

        Co epsilon = Co{0.00005f};
        int i;
        for (i = 0; i < 15; i++) {
            guess = eval(T);
            howClose = guess.abs();
            Co closeEnough = howClose < epsilon;
            if (closeEnough.anyTrue()) { break; }
            T = T - guess * derivative(T).invert();
        }
        stats.sample(i + 1);

        t = T[0];
        SkScalar distance = howClose[0];
        if (howClose[1] < distance) {
            t = T[1];
        }
        if (howClose[2] < distance) {
            t = T[2];
        }
        if (howClose[3] < distance) {
            t = T[3];
        }
        if (howClose[4] < distance) {
            t = T[4];
        }
        if (howClose[5] < distance) {
            t = T[5];
        }
        if (howClose[6] < distance) {
            t = T[6];
        }
        if (howClose[7] < distance) {
            t = T[7];
        }

        SkASSERT(0 <= t && t <= 1.0);

        return ((ay * t) + by) * t + cy;
    }

    Co A, B, C, D;
    Co DA, DB, DC;
    SkScalar ay, by, cy;
    Stats stats;
};

const float CubicMapNext2::kWindow;

static float eval_poly(float t, float b) {
    return b;
}

template <typename... Rest>
static float eval_poly(float t, float m, float b, Rest... rest) {
    return eval_poly(t, sk_fmaf(m,t,b), rest...);
}

struct CubicMapNext3 {
    CubicMapNext3(SkPoint P1, SkPoint P2) {

        // 3 * p1 * t + (-6 * p1 + 3 * p2) * t^2 + (1 + 3 * p1 - 3 * p2) * t^3
        SkPoint P1x3 = P1 * 3,
                P2x3 = P2 * 3;

        // A, B, C for A * t^3 + B * t^2 + C * t
        fA = P1x3 - P2x3 + SkPoint{1, 1};
        fB = P1x3 * -2 + P2x3;
        fC = P1x3;

        // Derivative of x only.
        fDA = 3 * fA.fX;
        fDB = 2 * fB.fX;
        fDC = 1 * fC.fX;

        fSwitchPoint = eval_poly(0.5f, fA.fX, fB.fX, fC.fX, 0);
        SkScalar half = fSwitchPoint;
        SkScalar quarter = eval_poly(0.25f, fA.fX, fB.fX, fC.fX, 0);
        SkScalar threeQuarters = eval_poly(0.75f, fA.fX, fB.fX, fC.fX, 0);

        // (-b + sqrt(b^2 - 4a(c - x))) / 2a
        // -b/(2a) + 1/(2a) * sqrt(b^2 - 4a(c-x))
        // T + U * sqrt(V - W(X-x))
        {
            // a = 8*(h-2*q), b = -2*(h-4*q), c = 0
            float a = 8 * (half - 2 * quarter);
            float b = -2 * (half - 4 * quarter);
            float c = 0;
            fT[0] = -b / (2 * a);
            fU[0] = 1 / (2 * a);
            fV[0] = b * b;
            fW[0] = 4 * a;
            fX[0] = c;
        }
        {
            // a = 8*(1 + h - 2*qqq), b = -2*(5 + 7*h - 12*qqq), c = 3 + 6*h - 8*qqq
            float a = 8 * (1 + half - 2 * threeQuarters);
            float b = -2 * (5 + 7 * half - 12 * threeQuarters);
            float c = 3 + 6 * half - 8 * threeQuarters;
            fT[1] = -b / (2 * a);
            fU[1] = 1 / (2 * a);
            fV[1] = b * b;
            fW[1] = 4 * a;
            fX[1] = c;
        }
    }

    SkScalar computeYFromX(SkScalar x) {
        if (x == 0) {return 0;}
        if (x == 1) {return 1;}

        int i = x <= fSwitchPoint ? 0 : 1;

        // T + U * sqrt(V - W(X-x))
        SkScalar t = fT[i] + fU[i] * std::sqrtf(fV[i] - fW[i] * (fX[i] - x));


        t = t - eval_poly(t, fA.fX, fB.fX, fC.fX, -x) / eval_poly(t, fDA, fDB, fDC);
        SkScalar v = eval_poly(t, fA.fX, fB.fX, fC.fX, -x);
        if (v > 0.00005) {
            t = t - v / eval_poly(t, fDA, fDB, fDC);
        }
        stats.sample(x, eval_poly(t, fA.fX, fB.fX, fC.fX, 0));

        return eval_poly(t, fA.fY, fB.fY, fC.fY, 0);
    }

    SkPoint fA, fB, fC;
    SkScalar fDA, fDB, fDC;

    SkScalar fSwitchPoint;

    SkScalar fT[2], fU[2], fV[2], fW[2], fX[2];
    StatsError stats;
};


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
                for (SkScalar x = 0; x <= 1; x += 1.0f / 512.0f) {
                    accum += fCMap.computeYFromX(x);
                }
            }
        }
        volatile float fred = accum;
    }

private:
    CubicMapNext3  fCMap;
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
DEF_BENCH( return new CubicMapBench({0.25f, 0}, {0.5f, 0}); )

