/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCurveMeasure.h"

// for abs
#include <cmath>

static inline Sk8f evaluateDerivativeLength(const Sk8f& ts,
                                            const Sk8f (&xCoeff)[3],
                                            const Sk8f (&yCoeff)[3],
                                            const SkSegType segType) {
    Sk8f x;
    Sk8f y;
    switch (segType) {
        case kQuad_SegType:
            x = xCoeff[0]*ts + xCoeff[1];
            y = yCoeff[0]*ts + yCoeff[1];
            break;
        case kLine_SegType:
            SkDebugf("Unimplemented");
            break;
        case kCubic_SegType:
            x = (xCoeff[0]*ts + xCoeff[1])*ts + xCoeff[2];
            y = (yCoeff[0]*ts + yCoeff[1])*ts + yCoeff[2];
            break;
        case kConic_SegType:
            SkDebugf("Unimplemented");
            break;
        default:
            SkDebugf("Unimplemented");
    }

    x = x * x;
    y = y * y;

    return (x + y).sqrt();
}
ArcLengthIntegrator::ArcLengthIntegrator(const SkPoint* pts, SkSegType segType)
    : fSegType(segType) {
    switch (fSegType) {
        case kQuad_SegType: {
            float Ax = pts[0].x();
            float Bx = pts[1].x();
            float Cx = pts[2].x();
            float Ay = pts[0].y();
            float By = pts[1].y();
            float Cy = pts[2].y();

            // precompute coefficients for derivative
            xCoeff[0] = Sk8f(2.0f*(Ax - 2*Bx + Cx));
            xCoeff[1] = Sk8f(2.0f*(Bx - Ax));

            yCoeff[0] = Sk8f(2.0f*(Ay - 2*By + Cy));
            yCoeff[1] = Sk8f(2.0f*(By - Ay));
        }
            break;
        case kLine_SegType:
            SkDEBUGF(("Unimplemented"));
            break;
        case kCubic_SegType:
        {
            float Ax = pts[0].x();
            float Bx = pts[1].x();
            float Cx = pts[2].x();
            float Dx = pts[3].x();
            float Ay = pts[0].y();
            float By = pts[1].y();
            float Cy = pts[2].y();
            float Dy = pts[3].y();

            xCoeff[0] = Sk8f(3.0f*(-Ax + 3.0f*(Bx - Cx) + Dx));
            xCoeff[1] = Sk8f(3.0f*(2.0f*(Ax - 2.0f*Bx + Cx)));
            xCoeff[2] = Sk8f(3.0f*(-Ax + Bx));

            yCoeff[0] = Sk8f(3.0f*(-Ay + 3.0f*(By - Cy) + Dy));
            yCoeff[1] = Sk8f(3.0f * -Ay + By + 2.0f*(Ay - 2.0f*By + Cy));
            yCoeff[2] = Sk8f(3.0f*(-Ay + By));
        }
            break;
        case kConic_SegType:
            SkDEBUGF(("Unimplemented"));
            break;
        default:
            SkDEBUGF(("Unimplemented"));
    }
}

// We use Gaussian quadrature
// (https://en.wikipedia.org/wiki/Gaussian_quadrature)
// to approximate the arc length integral here, because it is amenable to SIMD.
SkScalar ArcLengthIntegrator::computeLength(SkScalar t) {
    SkScalar length = 0.0f;

    Sk8f lengths = evaluateDerivativeLength(absc*t, xCoeff, yCoeff, fSegType);
    lengths = weights*lengths;
    // is it faster or more accurate to sum and then multiply or vice versa?
    lengths = lengths*(t*0.5f);

    // Why does SkNx index with ints? does negative index mean something?
    for (int i = 0; i < 8; i++) {
        length += lengths[i];
    }
    return length;
}

SkCurveMeasure::SkCurveMeasure(const SkPoint* pts, SkSegType segType)
    : fSegType(segType) {
    switch (fSegType) {
        case SkSegType::kQuad_SegType:
            for (size_t i = 0; i < 3; i++) {
                fPts[i] = pts[i];
            }
            break;
        case SkSegType::kLine_SegType:
            SkDebugf("Unimplemented");
            break;
        case SkSegType::kCubic_SegType:
            for (size_t i = 0; i < 4; i++) {
                fPts[i] = pts[i];
            }
            break;
        case SkSegType::kConic_SegType:
            SkDebugf("Unimplemented");
            break;
        default:
            SkDEBUGF(("Unimplemented"));
            break;
    }
    fIntegrator = ArcLengthIntegrator(fPts, fSegType);
}

SkScalar SkCurveMeasure::getLength() {
    if (-1.0f == fLength) {
        fLength = fIntegrator.computeLength(1.0f);
    }
    return fLength;
}

// Given an arc length targetLength, we want to determine what t
// gives us the corresponding arc length along the curve.
// We do this by letting the arc length integral := f(t) and
// solving for the root of the equation f(t) - targetLength = 0
// using Newton's method and lerp-bisection.
// The computationally expensive parts are the integral approximation
// at each step, and computing the derivative of the arc length integral,
// which is equal to the length of the tangent (so we have to do a sqrt).

SkScalar SkCurveMeasure::getTime(SkScalar targetLength) {
    if (targetLength == 0.0f) {
        return 0.0f;
    }

    SkScalar currentLength = getLength();

    if (SkScalarNearlyEqual(targetLength, currentLength)) {
        return 1.0f;
    }

    // initial estimate of t is percentage of total length
    SkScalar currentT = targetLength / currentLength;
    SkScalar prevT = -1.0f;
    SkScalar newT;

    SkScalar minT = 0.0f;
    SkScalar maxT = 1.0f;

    int iterations = 0;
    while (iterations < kNewtonIters + kBisectIters) {
        currentLength = fIntegrator.computeLength(currentT);
        SkScalar lengthDiff = currentLength - targetLength;

        // Update root bounds.
        // If lengthDiff is positive, we have overshot the target, so
        // we know the current t is an upper bound, and similarly
        // for the lower bound.
        if (lengthDiff > 0.0f) {
            if (currentT < maxT) {
                maxT = currentT;
            }
        } else {
            if (currentT > minT) {
                minT = currentT;
            }
        }

        // We have a tolerance on both the absolute value of the difference and
        // on the t value
        // because we may not have enough precision in the t to get close enough
        // in the length.
        if ((std::abs(lengthDiff) < kTolerance) ||
            (std::abs(prevT - currentT) < kTolerance)) {
            break;
        }

        prevT = currentT;
        if (iterations < kNewtonIters) {
            // TODO(hstern) switch here on curve type.
            // This is just newton's formula.
            SkScalar dt = evaluateQuadDerivative(currentT).length();
            newT = currentT - (lengthDiff / dt);

            // If newT is out of bounds, bisect inside newton.
            if ((newT < 0.0f) || (newT > 1.0f)) {
                newT = (minT + maxT) * 0.5f;
            }
        } else if (iterations < kNewtonIters + kBisectIters) {
            if (lengthDiff > 0.0f) {
                maxT = currentT;
            } else {
                minT = currentT;
            }
            // TODO(hstern) do a lerp here instead of a bisection
            newT = (minT + maxT) * 0.5f;
        } else {
            SkDEBUGF(("%.7f %.7f didn't get close enough after bisection.\n",
                     currentT, currentLength));
            break;
        }
        currentT = newT;

        SkASSERT(minT <= maxT);

        iterations++;
    }

    // debug. is there an SKDEBUG or something for ifdefs?
    fIters = iterations;

    return currentT;
}

void SkCurveMeasure::getPosTanTime(SkScalar targetLength, SkPoint* pos,
                               SkVector* tan, SkScalar* time) {
    SkScalar t = getTime(targetLength);

    if (time) {
        *time = t;
    }
    if (pos) {
        // TODO(hstern) switch here on curve type.
        *pos = evaluateQuad(t);
    }
    if (tan) {
        // TODO(hstern) switch here on curve type.
        *tan = evaluateQuadDerivative(t);
    }
}

// this is why I feel that the ArcLengthIntegrator should be combined
// with some sort of evaluator that caches the constants computed from the
// control points. this is basically the same code in ArcLengthIntegrator
SkPoint SkCurveMeasure::evaluateQuad(SkScalar t) {
    SkScalar ti = 1.0f - t;

    SkScalar Ax = fPts[0].x();
    SkScalar Bx = fPts[1].x();
    SkScalar Cx = fPts[2].x();
    SkScalar Ay = fPts[0].y();
    SkScalar By = fPts[1].y();
    SkScalar Cy = fPts[2].y();

    SkScalar x = Ax*ti*ti + 2.0f*Bx*t*ti + Cx*t*t;
    SkScalar y = Ay*ti*ti + 2.0f*By*t*ti + Cy*t*t;
    return SkPoint::Make(x, y);
}

SkVector SkCurveMeasure::evaluateQuadDerivative(SkScalar t) {
    SkScalar Ax = fPts[0].x();
    SkScalar Bx = fPts[1].x();
    SkScalar Cx = fPts[2].x();
    SkScalar Ay = fPts[0].y();
    SkScalar By = fPts[1].y();
    SkScalar Cy = fPts[2].y();

    SkScalar A2BCx = 2.0f*(Ax - 2*Bx + Cx);
    SkScalar A2BCy = 2.0f*(Ay - 2*By + Cy);
    SkScalar ABx = 2.0f*(Bx - Ax);
    SkScalar ABy = 2.0f*(By - Ay);

    return SkPoint::Make(A2BCx*t + ABx, A2BCy*t + ABy);
}
