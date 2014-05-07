/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkLineParameters.h"
#include "SkPathOpsCubic.h"
#include "SkPathOpsLine.h"
#include "SkPathOpsQuad.h"
#include "SkPathOpsRect.h"
#include "SkTSort.h"

const int SkDCubic::gPrecisionUnit = 256;  // FIXME: test different values in test framework

// give up when changing t no longer moves point
// also, copy point rather than recompute it when it does change
double SkDCubic::binarySearch(double min, double max, double axisIntercept,
        SearchAxis xAxis) const {
    double t = (min + max) / 2;
    double step = (t - min) / 2;
    SkDPoint cubicAtT = ptAtT(t);
    double calcPos = (&cubicAtT.fX)[xAxis];
    double calcDist = calcPos - axisIntercept;
    do {
        double priorT = t - step;
        SkASSERT(priorT >= min);
        SkDPoint lessPt = ptAtT(priorT);
        if (approximately_equal(lessPt.fX, cubicAtT.fX)
                && approximately_equal(lessPt.fY, cubicAtT.fY)) {
            return -1;  // binary search found no point at this axis intercept
        }
        double lessDist = (&lessPt.fX)[xAxis] - axisIntercept;
#if DEBUG_CUBIC_BINARY_SEARCH
        SkDebugf("t=%1.9g calc=%1.9g dist=%1.9g step=%1.9g less=%1.9g\n", t, calcPos, calcDist,
                step, lessDist);
#endif
        double lastStep = step;
        step /= 2;
        if (calcDist > 0 ? calcDist > lessDist : calcDist < lessDist) {
            t = priorT;
        } else {
            double nextT = t + lastStep;
            SkASSERT(nextT <= max);
            SkDPoint morePt = ptAtT(nextT);
            if (approximately_equal(morePt.fX, cubicAtT.fX)
                    && approximately_equal(morePt.fY, cubicAtT.fY)) {
                return -1;  // binary search found no point at this axis intercept
            }
            double moreDist = (&morePt.fX)[xAxis] - axisIntercept;
            if (calcDist > 0 ? calcDist <= moreDist : calcDist >= moreDist) {
                continue;
            }
            t = nextT;
        }
        SkDPoint testAtT = ptAtT(t);
        cubicAtT = testAtT;
        calcPos = (&cubicAtT.fX)[xAxis];
        calcDist = calcPos - axisIntercept;
    } while (!approximately_equal(calcPos, axisIntercept));
    return t;
}

// FIXME: cache keep the bounds and/or precision with the caller?
double SkDCubic::calcPrecision() const {
    SkDRect dRect;
    dRect.setBounds(*this);  // OPTIMIZATION: just use setRawBounds ?
    double width = dRect.fRight - dRect.fLeft;
    double height = dRect.fBottom - dRect.fTop;
    return (width > height ? width : height) / gPrecisionUnit;
}

bool SkDCubic::clockwise() const {
    double sum = (fPts[0].fX - fPts[3].fX) * (fPts[0].fY + fPts[3].fY);
    for (int idx = 0; idx < 3; ++idx) {
        sum += (fPts[idx + 1].fX - fPts[idx].fX) * (fPts[idx + 1].fY + fPts[idx].fY);
    }
    return sum <= 0;
}

void SkDCubic::Coefficients(const double* src, double* A, double* B, double* C, double* D) {
    *A = src[6];  // d
    *B = src[4] * 3;  // 3*c
    *C = src[2] * 3;  // 3*b
    *D = src[0];  // a
    *A -= *D - *C + *B;     // A =   -a + 3*b - 3*c + d
    *B += 3 * *D - 2 * *C;  // B =  3*a - 6*b + 3*c
    *C -= 3 * *D;           // C = -3*a + 3*b
}

bool SkDCubic::controlsContainedByEnds() const {
    SkDVector startTan = fPts[1] - fPts[0];
    if (startTan.fX == 0 && startTan.fY == 0) {
        startTan = fPts[2] - fPts[0];
    }
    SkDVector endTan = fPts[2] - fPts[3];
    if (endTan.fX == 0 && endTan.fY == 0) {
        endTan = fPts[1] - fPts[3];
    }
    if (startTan.dot(endTan) >= 0) {
        return false;
    }
    SkDLine startEdge = {{fPts[0], fPts[0]}};
    startEdge[1].fX -= startTan.fY;
    startEdge[1].fY += startTan.fX;
    SkDLine endEdge = {{fPts[3], fPts[3]}};
    endEdge[1].fX -= endTan.fY;
    endEdge[1].fY += endTan.fX;
    double leftStart1 = startEdge.isLeft(fPts[1]);
    if (leftStart1 * startEdge.isLeft(fPts[2]) < 0) {
        return false;
    }
    double leftEnd1 = endEdge.isLeft(fPts[1]);
    if (leftEnd1 * endEdge.isLeft(fPts[2]) < 0) {
        return false;
    }
    return leftStart1 * leftEnd1 >= 0;
}

bool SkDCubic::endsAreExtremaInXOrY() const {
    return (between(fPts[0].fX, fPts[1].fX, fPts[3].fX)
            && between(fPts[0].fX, fPts[2].fX, fPts[3].fX))
            || (between(fPts[0].fY, fPts[1].fY, fPts[3].fY)
            && between(fPts[0].fY, fPts[2].fY, fPts[3].fY));
}

bool SkDCubic::isLinear(int startIndex, int endIndex) const {
    SkLineParameters lineParameters;
    lineParameters.cubicEndPoints(*this, startIndex, endIndex);
    // FIXME: maybe it's possible to avoid this and compare non-normalized
    lineParameters.normalize();
    double distance = lineParameters.controlPtDistance(*this, 1);
    if (!approximately_zero(distance)) {
        return false;
    }
    distance = lineParameters.controlPtDistance(*this, 2);
    return approximately_zero(distance);
}

bool SkDCubic::monotonicInY() const {
    return between(fPts[0].fY, fPts[1].fY, fPts[3].fY)
            && between(fPts[0].fY, fPts[2].fY, fPts[3].fY);
}

int SkDCubic::searchRoots(double extremeTs[6], int extrema, double axisIntercept,
        SearchAxis xAxis, double* validRoots) const {
    extrema += findInflections(&extremeTs[extrema]);
    extremeTs[extrema++] = 0;
    extremeTs[extrema] = 1;
    SkTQSort(extremeTs, extremeTs + extrema);
    int validCount = 0;
    for (int index = 0; index < extrema; ) {
        double min = extremeTs[index];
        double max = extremeTs[++index];
        if (min == max) {
            continue;
        }
        double newT = binarySearch(min, max, axisIntercept, xAxis);
        if (newT >= 0) {
            validRoots[validCount++] = newT;
        }
    }
    return validCount;
}

bool SkDCubic::serpentine() const {
#if 0  // FIXME: enabling this fixes cubicOp114 but breaks cubicOp58d and cubicOp53d
    double tValues[2];
    // OPTIMIZATION : another case where caching the present of cubic inflections would be useful
    return findInflections(tValues) > 1;
#endif
    if (!controlsContainedByEnds()) {
        return false;
    }
    double wiggle = (fPts[0].fX - fPts[2].fX) * (fPts[0].fY + fPts[2].fY);
    for (int idx = 0; idx < 2; ++idx) {
        wiggle += (fPts[idx + 1].fX - fPts[idx].fX) * (fPts[idx + 1].fY + fPts[idx].fY);
    }
    double waggle = (fPts[1].fX - fPts[3].fX) * (fPts[1].fY + fPts[3].fY);
    for (int idx = 1; idx < 3; ++idx) {
        waggle += (fPts[idx + 1].fX - fPts[idx].fX) * (fPts[idx + 1].fY + fPts[idx].fY);
    }
    return wiggle * waggle < 0;
}

// cubic roots

static const double PI = 3.141592653589793;

// from SkGeometry.cpp (and Numeric Solutions, 5.6)
int SkDCubic::RootsValidT(double A, double B, double C, double D, double t[3]) {
    double s[3];
    int realRoots = RootsReal(A, B, C, D, s);
    int foundRoots = SkDQuad::AddValidTs(s, realRoots, t);
    return foundRoots;
}

int SkDCubic::RootsReal(double A, double B, double C, double D, double s[3]) {
#ifdef SK_DEBUG
    // create a string mathematica understands
    // GDB set print repe 15 # if repeated digits is a bother
    //     set print elements 400 # if line doesn't fit
    char str[1024];
    sk_bzero(str, sizeof(str));
    SK_SNPRINTF(str, sizeof(str), "Solve[%1.19g x^3 + %1.19g x^2 + %1.19g x + %1.19g == 0, x]",
            A, B, C, D);
    SkPathOpsDebug::MathematicaIze(str, sizeof(str));
#if ONE_OFF_DEBUG && ONE_OFF_DEBUG_MATHEMATICA
    SkDebugf("%s\n", str);
#endif
#endif
    if (approximately_zero(A)
            && approximately_zero_when_compared_to(A, B)
            && approximately_zero_when_compared_to(A, C)
            && approximately_zero_when_compared_to(A, D)) {  // we're just a quadratic
        return SkDQuad::RootsReal(B, C, D, s);
    }
    if (approximately_zero_when_compared_to(D, A)
            && approximately_zero_when_compared_to(D, B)
            && approximately_zero_when_compared_to(D, C)) {  // 0 is one root
        int num = SkDQuad::RootsReal(A, B, C, s);
        for (int i = 0; i < num; ++i) {
            if (approximately_zero(s[i])) {
                return num;
            }
        }
        s[num++] = 0;
        return num;
    }
    if (approximately_zero(A + B + C + D)) {  // 1 is one root
        int num = SkDQuad::RootsReal(A, A + B, -D, s);
        for (int i = 0; i < num; ++i) {
            if (AlmostDequalUlps(s[i], 1)) {
                return num;
            }
        }
        s[num++] = 1;
        return num;
    }
    double a, b, c;
    {
        double invA = 1 / A;
        a = B * invA;
        b = C * invA;
        c = D * invA;
    }
    double a2 = a * a;
    double Q = (a2 - b * 3) / 9;
    double R = (2 * a2 * a - 9 * a * b + 27 * c) / 54;
    double R2 = R * R;
    double Q3 = Q * Q * Q;
    double R2MinusQ3 = R2 - Q3;
    double adiv3 = a / 3;
    double r;
    double* roots = s;
    if (R2MinusQ3 < 0) {   // we have 3 real roots
        double theta = acos(R / sqrt(Q3));
        double neg2RootQ = -2 * sqrt(Q);

        r = neg2RootQ * cos(theta / 3) - adiv3;
        *roots++ = r;

        r = neg2RootQ * cos((theta + 2 * PI) / 3) - adiv3;
        if (!AlmostDequalUlps(s[0], r)) {
            *roots++ = r;
        }
        r = neg2RootQ * cos((theta - 2 * PI) / 3) - adiv3;
        if (!AlmostDequalUlps(s[0], r) && (roots - s == 1 || !AlmostDequalUlps(s[1], r))) {
            *roots++ = r;
        }
    } else {  // we have 1 real root
        double sqrtR2MinusQ3 = sqrt(R2MinusQ3);
        double A = fabs(R) + sqrtR2MinusQ3;
        A = SkDCubeRoot(A);
        if (R > 0) {
            A = -A;
        }
        if (A != 0) {
            A += Q / A;
        }
        r = A - adiv3;
        *roots++ = r;
        if (AlmostDequalUlps((double) R2, (double) Q3)) {
            r = -A / 2 - adiv3;
            if (!AlmostDequalUlps(s[0], r)) {
                *roots++ = r;
            }
        }
    }
    return static_cast<int>(roots - s);
}

// from http://www.cs.sunysb.edu/~qin/courses/geometry/4.pdf
// c(t)  = a(1-t)^3 + 3bt(1-t)^2 + 3c(1-t)t^2 + dt^3
// c'(t) = -3a(1-t)^2 + 3b((1-t)^2 - 2t(1-t)) + 3c(2t(1-t) - t^2) + 3dt^2
//       = 3(b-a)(1-t)^2 + 6(c-b)t(1-t) + 3(d-c)t^2
static double derivative_at_t(const double* src, double t) {
    double one_t = 1 - t;
    double a = src[0];
    double b = src[2];
    double c = src[4];
    double d = src[6];
    return 3 * ((b - a) * one_t * one_t + 2 * (c - b) * t * one_t + (d - c) * t * t);
}

// OPTIMIZE? compute t^2, t(1-t), and (1-t)^2 and pass them to another version of derivative at t?
SkDVector SkDCubic::dxdyAtT(double t) const {
    SkDVector result = { derivative_at_t(&fPts[0].fX, t), derivative_at_t(&fPts[0].fY, t) };
    return result;
}

// OPTIMIZE? share code with formulate_F1DotF2
int SkDCubic::findInflections(double tValues[]) const {
    double Ax = fPts[1].fX - fPts[0].fX;
    double Ay = fPts[1].fY - fPts[0].fY;
    double Bx = fPts[2].fX - 2 * fPts[1].fX + fPts[0].fX;
    double By = fPts[2].fY - 2 * fPts[1].fY + fPts[0].fY;
    double Cx = fPts[3].fX + 3 * (fPts[1].fX - fPts[2].fX) - fPts[0].fX;
    double Cy = fPts[3].fY + 3 * (fPts[1].fY - fPts[2].fY) - fPts[0].fY;
    return SkDQuad::RootsValidT(Bx * Cy - By * Cx, Ax * Cy - Ay * Cx, Ax * By - Ay * Bx, tValues);
}

static void formulate_F1DotF2(const double src[], double coeff[4]) {
    double a = src[2] - src[0];
    double b = src[4] - 2 * src[2] + src[0];
    double c = src[6] + 3 * (src[2] - src[4]) - src[0];
    coeff[0] = c * c;
    coeff[1] = 3 * b * c;
    coeff[2] = 2 * b * b + c * a;
    coeff[3] = a * b;
}

/** SkDCubic'(t) = At^2 + Bt + C, where
    A = 3(-a + 3(b - c) + d)
    B = 6(a - 2b + c)
    C = 3(b - a)
    Solve for t, keeping only those that fit between 0 < t < 1
*/
int SkDCubic::FindExtrema(double a, double b, double c, double d, double tValues[2]) {
    // we divide A,B,C by 3 to simplify
    double A = d - a + 3*(b - c);
    double B = 2*(a - b - b + c);
    double C = b - a;

    return SkDQuad::RootsValidT(A, B, C, tValues);
}

/*  from SkGeometry.cpp
    Looking for F' dot F'' == 0

    A = b - a
    B = c - 2b + a
    C = d - 3c + 3b - a

    F' = 3Ct^2 + 6Bt + 3A
    F'' = 6Ct + 6B

    F' dot F'' -> CCt^3 + 3BCt^2 + (2BB + CA)t + AB
*/
int SkDCubic::findMaxCurvature(double tValues[]) const {
    double coeffX[4], coeffY[4];
    int i;
    formulate_F1DotF2(&fPts[0].fX, coeffX);
    formulate_F1DotF2(&fPts[0].fY, coeffY);
    for (i = 0; i < 4; i++) {
        coeffX[i] = coeffX[i] + coeffY[i];
    }
    return RootsValidT(coeffX[0], coeffX[1], coeffX[2], coeffX[3], tValues);
}

SkDPoint SkDCubic::top(double startT, double endT) const {
    SkDCubic sub = subDivide(startT, endT);
    SkDPoint topPt = sub[0];
    if (topPt.fY > sub[3].fY || (topPt.fY == sub[3].fY && topPt.fX > sub[3].fX)) {
        topPt = sub[3];
    }
    double extremeTs[2];
    if (!sub.monotonicInY()) {
        int roots = FindExtrema(sub[0].fY, sub[1].fY, sub[2].fY, sub[3].fY, extremeTs);
        for (int index = 0; index < roots; ++index) {
            double t = startT + (endT - startT) * extremeTs[index];
            SkDPoint mid = ptAtT(t);
            if (topPt.fY > mid.fY || (topPt.fY == mid.fY && topPt.fX > mid.fX)) {
                topPt = mid;
            }
        }
    }
    return topPt;
}

SkDPoint SkDCubic::ptAtT(double t) const {
    if (0 == t) {
        return fPts[0];
    }
    if (1 == t) {
        return fPts[3];
    }
    double one_t = 1 - t;
    double one_t2 = one_t * one_t;
    double a = one_t2 * one_t;
    double b = 3 * one_t2 * t;
    double t2 = t * t;
    double c = 3 * one_t * t2;
    double d = t2 * t;
    SkDPoint result = {a * fPts[0].fX + b * fPts[1].fX + c * fPts[2].fX + d * fPts[3].fX,
            a * fPts[0].fY + b * fPts[1].fY + c * fPts[2].fY + d * fPts[3].fY};
    return result;
}

/*
 Given a cubic c, t1, and t2, find a small cubic segment.

 The new cubic is defined as points A, B, C, and D, where
 s1 = 1 - t1
 s2 = 1 - t2
 A = c[0]*s1*s1*s1 + 3*c[1]*s1*s1*t1 + 3*c[2]*s1*t1*t1 + c[3]*t1*t1*t1
 D = c[0]*s2*s2*s2 + 3*c[1]*s2*s2*t2 + 3*c[2]*s2*t2*t2 + c[3]*t2*t2*t2

 We don't have B or C. So We define two equations to isolate them.
 First, compute two reference T values 1/3 and 2/3 from t1 to t2:

 c(at (2*t1 + t2)/3) == E
 c(at (t1 + 2*t2)/3) == F

 Next, compute where those values must be if we know the values of B and C:

 _12   =  A*2/3 + B*1/3
 12_   =  A*1/3 + B*2/3
 _23   =  B*2/3 + C*1/3
 23_   =  B*1/3 + C*2/3
 _34   =  C*2/3 + D*1/3
 34_   =  C*1/3 + D*2/3
 _123  = (A*2/3 + B*1/3)*2/3 + (B*2/3 + C*1/3)*1/3 = A*4/9 + B*4/9 + C*1/9
 123_  = (A*1/3 + B*2/3)*1/3 + (B*1/3 + C*2/3)*2/3 = A*1/9 + B*4/9 + C*4/9
 _234  = (B*2/3 + C*1/3)*2/3 + (C*2/3 + D*1/3)*1/3 = B*4/9 + C*4/9 + D*1/9
 234_  = (B*1/3 + C*2/3)*1/3 + (C*1/3 + D*2/3)*2/3 = B*1/9 + C*4/9 + D*4/9
 _1234 = (A*4/9 + B*4/9 + C*1/9)*2/3 + (B*4/9 + C*4/9 + D*1/9)*1/3
       =  A*8/27 + B*12/27 + C*6/27 + D*1/27
       =  E
 1234_ = (A*1/9 + B*4/9 + C*4/9)*1/3 + (B*1/9 + C*4/9 + D*4/9)*2/3
       =  A*1/27 + B*6/27 + C*12/27 + D*8/27
       =  F
 E*27  =  A*8    + B*12   + C*6     + D
 F*27  =  A      + B*6    + C*12    + D*8

Group the known values on one side:

 M       = E*27 - A*8 - D     = B*12 + C* 6
 N       = F*27 - A   - D*8   = B* 6 + C*12
 M*2 - N = B*18
 N*2 - M = C*18
 B       = (M*2 - N)/18
 C       = (N*2 - M)/18
 */

static double interp_cubic_coords(const double* src, double t) {
    double ab = SkDInterp(src[0], src[2], t);
    double bc = SkDInterp(src[2], src[4], t);
    double cd = SkDInterp(src[4], src[6], t);
    double abc = SkDInterp(ab, bc, t);
    double bcd = SkDInterp(bc, cd, t);
    double abcd = SkDInterp(abc, bcd, t);
    return abcd;
}

SkDCubic SkDCubic::subDivide(double t1, double t2) const {
    if (t1 == 0 || t2 == 1) {
        if (t1 == 0 && t2 == 1) {
            return *this;
        }
        SkDCubicPair pair = chopAt(t1 == 0 ? t2 : t1);
        SkDCubic dst = t1 == 0 ? pair.first() : pair.second();
        return dst;
    }
    SkDCubic dst;
    double ax = dst[0].fX = interp_cubic_coords(&fPts[0].fX, t1);
    double ay = dst[0].fY = interp_cubic_coords(&fPts[0].fY, t1);
    double ex = interp_cubic_coords(&fPts[0].fX, (t1*2+t2)/3);
    double ey = interp_cubic_coords(&fPts[0].fY, (t1*2+t2)/3);
    double fx = interp_cubic_coords(&fPts[0].fX, (t1+t2*2)/3);
    double fy = interp_cubic_coords(&fPts[0].fY, (t1+t2*2)/3);
    double dx = dst[3].fX = interp_cubic_coords(&fPts[0].fX, t2);
    double dy = dst[3].fY = interp_cubic_coords(&fPts[0].fY, t2);
    double mx = ex * 27 - ax * 8 - dx;
    double my = ey * 27 - ay * 8 - dy;
    double nx = fx * 27 - ax - dx * 8;
    double ny = fy * 27 - ay - dy * 8;
    /* bx = */ dst[1].fX = (mx * 2 - nx) / 18;
    /* by = */ dst[1].fY = (my * 2 - ny) / 18;
    /* cx = */ dst[2].fX = (nx * 2 - mx) / 18;
    /* cy = */ dst[2].fY = (ny * 2 - my) / 18;
    // FIXME: call align() ?
    return dst;
}

void SkDCubic::align(int endIndex, int ctrlIndex, SkDPoint* dstPt) const {
    if (fPts[endIndex].fX == fPts[ctrlIndex].fX) {
        dstPt->fX = fPts[endIndex].fX;
    }
    if (fPts[endIndex].fY == fPts[ctrlIndex].fY) {
        dstPt->fY = fPts[endIndex].fY;
    }
}

void SkDCubic::subDivide(const SkDPoint& a, const SkDPoint& d,
                         double t1, double t2, SkDPoint dst[2]) const {
    SkASSERT(t1 != t2);
#if 0
    double ex = interp_cubic_coords(&fPts[0].fX, (t1 * 2 + t2) / 3);
    double ey = interp_cubic_coords(&fPts[0].fY, (t1 * 2 + t2) / 3);
    double fx = interp_cubic_coords(&fPts[0].fX, (t1 + t2 * 2) / 3);
    double fy = interp_cubic_coords(&fPts[0].fY, (t1 + t2 * 2) / 3);
    double mx = ex * 27 - a.fX * 8 - d.fX;
    double my = ey * 27 - a.fY * 8 - d.fY;
    double nx = fx * 27 - a.fX - d.fX * 8;
    double ny = fy * 27 - a.fY - d.fY * 8;
    /* bx = */ dst[0].fX = (mx * 2 - nx) / 18;
    /* by = */ dst[0].fY = (my * 2 - ny) / 18;
    /* cx = */ dst[1].fX = (nx * 2 - mx) / 18;
    /* cy = */ dst[1].fY = (ny * 2 - my) / 18;
#else
    // this approach assumes that the control points computed directly are accurate enough
    SkDCubic sub = subDivide(t1, t2);
    dst[0] = sub[1] + (a - sub[0]);
    dst[1] = sub[2] + (d - sub[3]);
#endif
    if (t1 == 0 || t2 == 0) {
        align(0, 1, t1 == 0 ? &dst[0] : &dst[1]);
    }
    if (t1 == 1 || t2 == 1) {
        align(3, 2, t1 == 1 ? &dst[0] : &dst[1]);
    }
    if (AlmostBequalUlps(dst[0].fX, a.fX)) {
        dst[0].fX = a.fX;
    }
    if (AlmostBequalUlps(dst[0].fY, a.fY)) {
        dst[0].fY = a.fY;
    }
    if (AlmostBequalUlps(dst[1].fX, d.fX)) {
        dst[1].fX = d.fX;
    }
    if (AlmostBequalUlps(dst[1].fY, d.fY)) {
        dst[1].fY = d.fY;
    }
}

/* classic one t subdivision */
static void interp_cubic_coords(const double* src, double* dst, double t) {
    double ab = SkDInterp(src[0], src[2], t);
    double bc = SkDInterp(src[2], src[4], t);
    double cd = SkDInterp(src[4], src[6], t);
    double abc = SkDInterp(ab, bc, t);
    double bcd = SkDInterp(bc, cd, t);
    double abcd = SkDInterp(abc, bcd, t);

    dst[0] = src[0];
    dst[2] = ab;
    dst[4] = abc;
    dst[6] = abcd;
    dst[8] = bcd;
    dst[10] = cd;
    dst[12] = src[6];
}

SkDCubicPair SkDCubic::chopAt(double t) const {
    SkDCubicPair dst;
    if (t == 0.5) {
        dst.pts[0] = fPts[0];
        dst.pts[1].fX = (fPts[0].fX + fPts[1].fX) / 2;
        dst.pts[1].fY = (fPts[0].fY + fPts[1].fY) / 2;
        dst.pts[2].fX = (fPts[0].fX + 2 * fPts[1].fX + fPts[2].fX) / 4;
        dst.pts[2].fY = (fPts[0].fY + 2 * fPts[1].fY + fPts[2].fY) / 4;
        dst.pts[3].fX = (fPts[0].fX + 3 * (fPts[1].fX + fPts[2].fX) + fPts[3].fX) / 8;
        dst.pts[3].fY = (fPts[0].fY + 3 * (fPts[1].fY + fPts[2].fY) + fPts[3].fY) / 8;
        dst.pts[4].fX = (fPts[1].fX + 2 * fPts[2].fX + fPts[3].fX) / 4;
        dst.pts[4].fY = (fPts[1].fY + 2 * fPts[2].fY + fPts[3].fY) / 4;
        dst.pts[5].fX = (fPts[2].fX + fPts[3].fX) / 2;
        dst.pts[5].fY = (fPts[2].fY + fPts[3].fY) / 2;
        dst.pts[6] = fPts[3];
        return dst;
    }
    interp_cubic_coords(&fPts[0].fX, &dst.pts[0].fX, t);
    interp_cubic_coords(&fPts[0].fY, &dst.pts[0].fY, t);
    return dst;
}
