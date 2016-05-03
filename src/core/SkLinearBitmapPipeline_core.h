/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLinearBitmapPipeline_core_DEFINED
#define SkLinearBitmapPipeline_core_DEFINED

#include <cmath>
#include "SkNx.h"

// New bilerp strategy:
// Pass through on bilerpList4 and bilerpListFew (analogs to pointList), introduce bilerpEdge
// which takes 4 points. If the sample spans an edge, then break it into a bilerpEdge. Bilerp
// span then becomes a normal span except in special cases where an extra Y is given. The bilerp
// need to stay single point calculations until the tile layer.
// TODO:
//  - edge span predicate.
//  - introduce new point API
//  - Add tile for new api.

// Tweak ABI of functions that pass Sk4f by value to pass them via registers.
#if defined(_MSC_VER) && SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2
    #define VECTORCALL __vectorcall
#elif defined(SK_CPU_ARM32) && defined(SK_ARM_HAS_NEON)
    #define VECTORCALL __attribute__((pcs("aapcs-vfp")))
#else
    #define VECTORCALL
#endif

namespace {
struct X {
    explicit X(SkScalar val) : fVal{val} { }
    explicit X(SkPoint pt)   : fVal{pt.fX} { }
    explicit X(SkSize s)     : fVal{s.fWidth} { }
    explicit X(SkISize s)    : fVal((SkScalar)s.fWidth) { }
    operator SkScalar () const {return fVal;}
private:
    SkScalar fVal;
};

struct Y {
    explicit Y(SkScalar val) : fVal{val} { }
    explicit Y(SkPoint pt)   : fVal{pt.fY} { }
    explicit Y(SkSize s)     : fVal{s.fHeight} { }
    explicit Y(SkISize s)    : fVal((SkScalar)s.fHeight) { }
    operator SkScalar () const {return fVal;}
private:
    SkScalar fVal;
};

// The Span class enables efficient processing horizontal spans of pixels.
// * start - the point where to start the span.
// * length - the number of pixels to traverse in source space.
// * count - the number of pixels to produce in destination space.
// Both start and length are mapped through the inversion matrix to produce values in source
// space. After the matrix operation, the tilers may break the spans up into smaller spans.
// The tilers can produce spans that seem nonsensical.
// * The clamp tiler can create spans with length of 0. This indicates to copy an edge pixel out
//   to the edge of the destination scan.
// * The mirror tiler can produce spans with negative length. This indicates that the source
//   should be traversed in the opposite direction to the destination pixels.
class Span {
public:
    Span(SkPoint start, SkScalar length, int count)
        : fStart(start)
        , fLength(length)
        , fCount{count} {
        SkASSERT(std::isfinite(length));
    }

    operator std::tuple<SkPoint&, SkScalar&, int&>() {
        return std::tie(fStart, fLength, fCount);
    }

    bool isEmpty() const { return 0 == fCount; }
    void clear() { fCount = 0; }
    int count() const { return fCount; }
    SkScalar length() const { return fLength; }
    SkScalar startX() const { return X(fStart); }
    SkScalar endX() const { return this->startX() + this->length(); }
    SkScalar startY() const { return Y(fStart); }
    Span emptySpan() { return Span{{0.0, 0.0}, 0.0f, 0}; }

    bool completelyWithin(SkScalar xMin, SkScalar xMax) const {
        SkScalar sMin, sMax;
        std::tie(sMin, sMax) = std::minmax(startX(), endX());
        return xMin <= sMin && sMax < xMax;
    }

    void offset(SkScalar offsetX) {
        fStart.offset(offsetX, 0.0f);
    }

    Span breakAt(SkScalar breakX, SkScalar dx) {
        SkASSERT(std::isfinite(breakX));
        SkASSERT(std::isfinite(dx));
        SkASSERT(dx != 0.0f);

        if (this->isEmpty()) {
            return this->emptySpan();
        }

        int dxSteps = SkScalarFloorToInt((breakX - this->startX()) / dx);

        if (dxSteps < 0) {
            // The span is wholly after breakX.
            return this->emptySpan();
        } else if (dxSteps >= fCount) {
            // The span is wholly before breakX.
            Span answer = *this;
            this->clear();
            return answer;
        }

        // Calculate the values for the span to cleave off.
        SkScalar newLength = dxSteps * dx;

        // If the last (or first if count = 1) sample lands directly on the boundary. Include it
        // when dx < 0 and exclude it when dx > 0.
        // Reasoning:
        //  dx > 0: The sample point on the boundary is part of the next span because the entire
        // pixel is after the boundary.
        //  dx < 0: The sample point on the boundary is part of the current span because the
        // entire pixel is before the boundary.
        if (this->startX() + newLength == breakX && dx > 0) {
            if (dxSteps > 0) {
                dxSteps -= 1;
                newLength -= dx;
            } else {
                return this->emptySpan();
            }
        }

        // Calculate new span parameters
        SkPoint newStart = fStart;
        int newCount = dxSteps + 1;
        SkASSERT(newCount > 0);

        // Update this span to reflect the break.
        SkScalar lengthToStart = newLength + dx;
        fLength -= lengthToStart;
        fCount -= newCount;
        fStart = {this->startX() + lengthToStart, Y(fStart)};

        return Span{newStart, newLength, newCount};
    }

    void clampToSinglePixel(SkPoint pixel) {
        fStart = pixel;
        fLength = 0.0f;
    }

private:
    SkPoint  fStart;
    SkScalar fLength;
    int      fCount;
};

template<typename Stage>
void span_fallback(Span span, Stage* stage) {
    SkPoint start;
    SkScalar length;
    int count;
    std::tie(start, length, count) = span;
    Sk4f xs{X(start)};
    Sk4f ys{Y(start)};

    // Initializing this is not needed, but some compilers can't figure this out.
    Sk4s fourDx{0.0f};
    if (count > 1) {
        SkScalar dx = length / (count - 1);
        xs = xs + Sk4f{0.0f, 1.0f, 2.0f, 3.0f} * dx;
        // Only used if count is >= 4.
        fourDx = Sk4f{4.0f * dx};
    }

    while (count >= 4) {
        stage->pointList4(xs, ys);
        xs = xs + fourDx;
        count -= 4;
    }
    if (count > 0) {
        stage->pointListFew(count, xs, ys);
    }
}
}  // namespace

#endif // SkLinearBitmapPipeline_core_DEFINED
