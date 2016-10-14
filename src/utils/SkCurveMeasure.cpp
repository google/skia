/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCurveMeasure.h"
#include "SkGeometry.h"

// for abs
#include <cmath>

#define UNIMPLEMENTED SkDEBUGF(("%s:%d unimplemented\n", __FILE__, __LINE__))

/// Used inside SkCurveMeasure::getTime's Newton's iteration
static inline SkPoint evaluate(const SkPoint pts[4], SkSegType segType,
                               SkScalar t) {
    SkPoint pos;
    switch (segType) {
        case kQuad_SegType:
            pos = SkEvalQuadAt(pts, t);
            break;
        case kLine_SegType:
            pos = SkPoint::Make(SkScalarInterp(pts[0].x(), pts[1].x(), t),
                                SkScalarInterp(pts[0].y(), pts[1].y(), t));
            break;
        case kCubic_SegType:
            SkEvalCubicAt(pts, t, &pos, nullptr, nullptr);
            break;
        case kConic_SegType: {
            SkConic conic(pts, pts[3].x());
            conic.evalAt(t, &pos);
        }
            break;
        default:
            UNIMPLEMENTED;
    }

    return pos;
}

/// Used inside SkCurveMeasure::getTime's Newton's iteration
static inline SkVector evaluateDerivative(const SkPoint pts[4],
                                          SkSegType segType, SkScalar t) {
    SkVector tan;
    switch (segType) {
        case kQuad_SegType:
            tan = SkEvalQuadTangentAt(pts, t);
            break;
        case kLine_SegType:
            tan = pts[1] - pts[0];
            break;
        case kCubic_SegType:
            SkEvalCubicAt(pts, t, nullptr, &tan, nullptr);
            break;
        case kConic_SegType: {
            SkConic conic(pts, pts[3].x());
            conic.evalAt(t, nullptr, &tan);
        }
            break;
        default:
            UNIMPLEMENTED;
    }

    return tan;
}
/// Used in ArcLengthIntegrator::computeLength
static inline Sk8f evaluateDerivativeLength(const Sk8f& ts,
                                            const float (&xCoeff)[3][8],
                                            const float (&yCoeff)[3][8],
                                            const SkSegType segType) {
    Sk8f x;
    Sk8f y;

    Sk8f x0 = Sk8f::Load(&xCoeff[0]),
         x1 = Sk8f::Load(&xCoeff[1]),
         x2 = Sk8f::Load(&xCoeff[2]);

    Sk8f y0 = Sk8f::Load(&yCoeff[0]),
         y1 = Sk8f::Load(&yCoeff[1]),
         y2 = Sk8f::Load(&yCoeff[2]);

    switch (segType) {
        case kQuad_SegType:
            x = x0*ts + x1;
            y = y0*ts + y1;
            break;
        case kCubic_SegType:
            x = (x0*ts + x1)*ts + x2;
            y = (y0*ts + y1)*ts + y2;
            break;
        case kConic_SegType:
            UNIMPLEMENTED;
            break;
        default:
            UNIMPLEMENTED;
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
            Sk8f(2*(Ax - 2*Bx + Cx)).store(&xCoeff[0]);
            Sk8f(2*(Bx - Ax)).store(&xCoeff[1]);

            Sk8f(2*(Ay - 2*By + Cy)).store(&yCoeff[0]);
            Sk8f(2*(By - Ay)).store(&yCoeff[1]);
        }
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

            // precompute coefficients for derivative
            Sk8f(3*(-Ax + 3*(Bx - Cx) + Dx)).store(&xCoeff[0]);
            Sk8f(6*(Ax - 2*Bx + Cx)).store(&xCoeff[1]);
            Sk8f(3*(-Ax + Bx)).store(&xCoeff[2]);

            Sk8f(3*(-Ay + 3*(By - Cy) + Dy)).store(&yCoeff[0]);
            Sk8f(6*(Ay - 2*By + Cy)).store(&yCoeff[1]);
            Sk8f(3*(-Ay + By)).store(&yCoeff[2]);
        }
            break;
        case kConic_SegType:
            UNIMPLEMENTED;
            break;
        default:
            UNIMPLEMENTED;
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
            fPts[0] = pts[0];
            fPts[1] = pts[1];
            fLength = (fPts[1] - fPts[0]).length();
            break;
        case SkSegType::kCubic_SegType:
            for (size_t i = 0; i < 4; i++) {
                fPts[i] = pts[i];
            }
            break;
        case SkSegType::kConic_SegType:
            for (size_t i = 0; i < 4; i++) {
                fPts[i] = pts[i];
            }
            break;
        default:
            UNIMPLEMENTED;
            break;
    }
    if (kLine_SegType != segType) {
        fIntegrator = ArcLengthIntegrator(fPts, fSegType);
    }
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
    if (targetLength <= 0.0f) {
        return 0.0f;
    }

    SkScalar currentLength = getLength();

    if (targetLength > currentLength || (SkScalarNearlyEqual(targetLength, currentLength))) {
        return 1.0f;
    }
    if (kLine_SegType == fSegType) {
        return targetLength / currentLength;
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
            // This is just newton's formula.
            SkScalar dt = evaluateDerivative(fPts, fSegType, currentT).length();
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
        *pos = evaluate(fPts, fSegType, t);
    }
    if (tan) {
        *tan = evaluateDerivative(fPts, fSegType, t);
    }
}
