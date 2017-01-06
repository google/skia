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

    void processPoints(Sk4s* xs, Sk4s* ys) const {
        *xs = *xs + fXOffset;
        *ys = *ys + fYOffset;
    }

    template <typename Next>
    bool maybeProcessSpan(Span span, Next* next) const {
        SkPoint start; SkScalar length; int count;
        std::tie(start, length, count) = span;
        next->pointSpan(Span{start + SkPoint{fXOffset, fYOffset}, length, count});
        return true;
    }

private:
    const SkScalar fXOffset, fYOffset;
};

class ScaleMatrixStrategy {
public:
    ScaleMatrixStrategy(SkVector offset, SkVector scale)
        : fXOffset{X(offset)}, fYOffset{Y(offset)}
        ,  fXScale{X(scale)},   fYScale{Y(scale)} { }
    void processPoints(Sk4s* xs, Sk4s* ys) const {
        *xs = *xs * fXScale + fXOffset;
        *ys = *ys * fYScale + fYOffset;
    }

    template <typename Next>
    bool maybeProcessSpan(Span span, Next* next) const {
        SkPoint start; SkScalar length; int count;
        std::tie(start, length, count) = span;
        SkPoint newStart =
            SkPoint{X(start) * fXScale + fXOffset, Y(start) * fYScale + fYOffset};
        SkScalar newLength = length * fXScale;
        next->pointSpan(Span{newStart, newLength, count});
        return true;
    }

private:
    const SkScalar fXOffset, fYOffset;
    const SkScalar fXScale,  fYScale;
};

class AffineMatrixStrategy {
public:
    AffineMatrixStrategy(SkVector offset, SkVector scale, SkVector skew)
        : fXOffset{X(offset)}, fYOffset{Y(offset)}
        , fXScale{X(scale)},   fYScale{Y(scale)}
        , fXSkew{X(skew)},     fYSkew{Y(skew)} { }
    void processPoints(Sk4s* xs, Sk4s* ys) const {
        Sk4s newXs = fXScale * *xs +  fXSkew * *ys + fXOffset;
        Sk4s newYs =  fYSkew * *xs + fYScale * *ys + fYOffset;

        *xs = newXs;
        *ys = newYs;
    }

    template <typename Next>
    bool maybeProcessSpan(Span span, Next* next) const {
        return false;
    }

private:
    const SkScalar fXOffset, fYOffset;
    const SkScalar fXScale,  fYScale;
    const SkScalar fXSkew,   fYSkew;
};

class PerspectiveMatrixStrategy {
public:
    PerspectiveMatrixStrategy(SkVector offset, SkVector scale, SkVector skew,
                              SkVector zSkew, SkScalar zOffset)
        : fXOffset{X(offset)}, fYOffset{Y(offset)}, fZOffset{zOffset}
        , fXScale{X(scale)},   fYScale{Y(scale)}
        , fXSkew{X(skew)},     fYSkew{Y(skew)}, fZXSkew{X(zSkew)}, fZYSkew{Y(zSkew)} { }
    void processPoints(Sk4s* xs, Sk4s* ys) const {
        Sk4s newXs = fXScale * *xs +  fXSkew * *ys + fXOffset;
        Sk4s newYs =  fYSkew * *xs + fYScale * *ys + fYOffset;
        Sk4s newZs =  fZXSkew * *xs + fZYSkew * *ys + fZOffset;

        *xs = newXs / newZs;
        *ys = newYs / newZs;
    }

    template <typename Next>
    bool maybeProcessSpan(Span span, Next* next) const {
        return false;
    }

private:
    const SkScalar fXOffset, fYOffset, fZOffset;
    const SkScalar fXScale,  fYScale;
    const SkScalar fXSkew,   fYSkew,   fZXSkew, fZYSkew;
};


}  // namespace

#endif  // SkLinearBitmapPipeline_matrix_DEFINED
