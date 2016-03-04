/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLinearBitmapPipeline_core_DEFINED
#define SkLinearBitmapPipeline_core_DEFINED

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
    explicit X(SkISize s)    : fVal(s.fWidth) { }
    operator SkScalar () const {return fVal;}
private:
    SkScalar fVal;
};

struct Y {
    explicit Y(SkScalar val) : fVal{val} { }
    explicit Y(SkPoint pt)   : fVal{pt.fY} { }
    explicit Y(SkSize s)     : fVal{s.fHeight} { }
    explicit Y(SkISize s)    : fVal(s.fHeight) { }
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
    SkScalar length() const { return fLength; }
    SkScalar startX() const { return X(fStart); }
    SkScalar endX() const { return startX() + length(); }
    void clear() {
        fCount = 0;
    }

    bool completelyWithin(SkScalar xMin, SkScalar xMax) const {
        SkScalar sMin, sMax;
        std::tie(sMin, sMax) = std::minmax(startX(), endX());
        return xMin <= sMin && sMax <= xMax;
    }

    void offset(SkScalar offsetX) {
        fStart.offset(offsetX, 0.0f);
    }

    Span breakAt(SkScalar breakX, SkScalar dx) {
        SkASSERT(std::isfinite(breakX));
        SkASSERT(std::isfinite(dx));
        SkASSERT(dx != 0.0f);

        if (this->isEmpty()) {
            return Span{{0.0, 0.0}, 0.0f, 0};
        }

        int dxSteps = SkScalarFloorToInt((breakX - this->startX()) / dx);
        if (dxSteps < 0) {
            // The span is wholly after breakX.
            return Span{{0.0, 0.0}, 0.0f, 0};
        } else if (dxSteps > fCount) {
            // The span is wholly before breakX.
            Span answer = *this;
            this->clear();
            return answer;
        }

        // Calculate the values for the span to cleave off.
        SkPoint newStart = fStart;
        SkScalar newLength = dxSteps * dx;
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

// BilerpSpans are similar to Spans, but they represent four source samples converting to single
// destination pixel per count. The pixels for the four samples are collect along two horizontal
// lines; one starting at {x, y0} and the other starting at {x, y1}. There are two distinct lines
// to deal with the edge case of the tile mode. For example, y0 may be at the last y position in
// a tile while y1 would be at the first.
// The step of a Bilerp (dx) is still length / (count - 1) and the start to the next sample is
// still dx * count, but the bounds are complicated by the sampling kernel so that the pixels
// touched are from x to x + length + 1.
class BilerpSpan {
public:
    BilerpSpan(SkScalar x, SkScalar y0, SkScalar y1, SkScalar length, int count)
        : fX{x}, fY0{y0}, fY1{y1}, fLength{length}, fCount{count} {
        SkASSERT(count >= 0);
        SkASSERT(std::isfinite(length));
        SkASSERT(std::isfinite(x));
        SkASSERT(std::isfinite(y0));
        SkASSERT(std::isfinite(y1));
    }

    operator std::tuple<SkScalar&, SkScalar&, SkScalar&, SkScalar&, int&>() {
        return std::tie(fX, fY0, fY1, fLength, fCount);
    }

    bool isEmpty() const { return 0 == fCount; }

private:
    SkScalar fX;
    SkScalar fY0;
    SkScalar fY1;
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

template <typename Next>
void bilerp_span_fallback(BilerpSpan span, Next* next) {
    SkScalar x, y0, y1; SkScalar length; int count;
    std::tie(x, y0, y1, length, count) = span;

    SkASSERT(!span.isEmpty());
    float dx = length / (count - 1);

    Sk4f xs = Sk4f{x} + Sk4f{0.0f,  1.0f, 0.0f, 1.0f};
    Sk4f ys = Sk4f{y0, y0,  y1, y1};

    // If count == 1 then dx will be inf or NaN, but that is ok because the resulting addition is
    // never used.
    while (count > 0) {
        next->bilerpList(xs, ys);
        xs = xs + dx;
        count -= 1;
    }
}
}  // namespace

#endif // SkLinearBitmapPipeline_core_DEFINED
