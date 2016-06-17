/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLinearBitmapPipeline_matrix_DEFINED
#define SkLinearBitmapPipeline_matrix_DEFINED

#include "SkLinearBitmapPipeline_core.h"

namespace {
class TranslateMatrixStrategy {
public:
    TranslateMatrixStrategy(SkVector offset)
        : fXOffset{X(offset)}
        , fYOffset{Y(offset)} { }

    void processPoints(Sk4s* xs, Sk4s* ys) {
        *xs = *xs + fXOffset;
        *ys = *ys + fYOffset;
    }

    template <typename Next>
    bool maybeProcessSpan(Span span, Next* next) {
        SkPoint start; SkScalar length; int count;
        std::tie(start, length, count) = span;
        next->pointSpan(Span{start + SkPoint{fXOffset[0], fYOffset[0]}, length, count});
        return true;
    }

private:
    const Sk4s fXOffset, fYOffset;
};

class ScaleMatrixStrategy {
public:
    ScaleMatrixStrategy(SkVector offset, SkVector scale)
        : fXOffset{X(offset)}, fYOffset{Y(offset)}
        ,  fXScale{X(scale)},   fYScale{Y(scale)} { }
    void processPoints(Sk4s* xs, Sk4s* ys) {
        *xs = *xs * fXScale + fXOffset;
        *ys = *ys * fYScale + fYOffset;
    }

    template <typename Next>
    bool maybeProcessSpan(Span span, Next* next) {
        SkPoint start; SkScalar length; int count;
        std::tie(start, length, count) = span;
        SkPoint newStart =
            SkPoint{X(start) * fXScale[0] + fXOffset[0], Y(start) * fYScale[0] + fYOffset[0]};
        SkScalar newLength = length * fXScale[0];
        next->pointSpan(Span{newStart, newLength, count});
        return true;
    }

private:
    const Sk4s fXOffset, fYOffset;
    const Sk4s fXScale, fYScale;
};

class AffineMatrixStrategy {
public:
    AffineMatrixStrategy(SkVector offset, SkVector scale, SkVector skew)
        : fXOffset{X(offset)}, fYOffset{Y(offset)}
        , fXScale{X(scale)},   fYScale{Y(scale)}
        , fXSkew{X(skew)},     fYSkew{Y(skew)} { }
    void processPoints(Sk4s* xs, Sk4s* ys) {
        Sk4s newXs = fXScale * *xs +  fXSkew * *ys + fXOffset;
        Sk4s newYs =  fYSkew * *xs + fYScale * *ys + fYOffset;

        *xs = newXs;
        *ys = newYs;
    }

    template <typename Next>
    bool maybeProcessSpan(Span span, Next* next) {
        return false;
    }

private:
    const Sk4s fXOffset, fYOffset;
    const Sk4s fXScale,  fYScale;
    const Sk4s fXSkew,   fYSkew;
};

class PerspectiveMatrixStrategy {
public:
    PerspectiveMatrixStrategy(SkVector offset, SkVector scale, SkVector skew,
                              SkVector zSkew, SkScalar zOffset)
        : fXOffset{X(offset)}, fYOffset{Y(offset)}, fZOffset{zOffset}
        , fXScale{X(scale)},   fYScale{Y(scale)}
        , fXSkew{X(skew)},     fYSkew{Y(skew)}, fZXSkew{X(zSkew)}, fZYSkew{Y(zSkew)} { }
    void processPoints(Sk4s* xs, Sk4s* ys) {
        Sk4s newXs = fXScale * *xs +  fXSkew * *ys + fXOffset;
        Sk4s newYs =  fYSkew * *xs + fYScale * *ys + fYOffset;
        Sk4s newZs =  fZXSkew * *xs + fZYSkew * *ys + fZOffset;

        *xs = newXs / newZs;
        *ys = newYs / newZs;
    }

    template <typename Next>
    bool maybeProcessSpan(Span span, Next* next) {
        return false;
    }

private:
    const Sk4s fXOffset, fYOffset, fZOffset;
    const Sk4s fXScale,  fYScale;
    const Sk4s fXSkew,   fYSkew, fZXSkew, fZYSkew;
};


}  // namespace

#endif  // SkLinearBitmapPipeline_matrix_DEFINED
