/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLinearBitmapPipeline_tile_DEFINED
#define SkLinearBitmapPipeline_tile_DEFINED

#include "SkLinearBitmapPipeline_core.h"
#include "SkPM4f.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace {
class ClampStrategy {
public:
    ClampStrategy(X max)
        : fXMin{0.0f}, fXMax{max - 1.0f} { }

    ClampStrategy(Y max)
        : fYMin{0.0f}, fYMax{max - 1.0f} { }

    ClampStrategy(SkSize max)
        : fXMin{0.0f}, fYMin{0.0f}, fXMax{X(max) - 1.0f}, fYMax{Y(max) - 1.0f} { }

    void processPoints(Sk4s* xs, Sk4s* ys) {
        *xs = Sk4s::Min(Sk4s::Max(*xs, fXMin), fXMax);
        *ys = Sk4s::Min(Sk4s::Max(*ys, fYMin), fYMax);
    }

    template<typename Next>
    bool maybeProcessSpan(Span originalSpan, Next* next) {
        SkASSERT(!originalSpan.isEmpty());
        SkPoint start;
        SkScalar length;
        int count;
        std::tie(start, length, count) = originalSpan;
        SkScalar xMin = fXMin[0];
        SkScalar xMax = fXMax[0] + 1.0f;
        SkScalar yMin = fYMin[0];
        SkScalar yMax = fYMax[0];
        SkScalar x = X(start);
        SkScalar y = std::min(std::max<SkScalar>(yMin, Y(start)), yMax);

        Span span{{x, y}, length, count};

        if (span.completelyWithin(xMin, xMax)) {
            next->pointSpan(span);
            return true;
        }
        if (1 == count || 0.0f == length) {
            return false;
        }

        SkScalar dx = length / (count - 1);

        //    A                 B     C
        // +-------+-------+-------++-------+-------+-------+     +-------+-------++------
        // |  *---*|---*---|*---*--||-*---*-|---*---|*---...|     |--*---*|---*---||*---*....
        // |       |       |       ||       |       |       | ... |       |       ||
        // |       |       |       ||       |       |       |     |       |       ||
        // +-------+-------+-------++-------+-------+-------+     +-------+-------++------
        //                         ^                                              ^
        //                         | xMin                                  xMax-1 | xMax
        //
        //     *---*---*---... - track of samples. * = sample
        //
        //     +-+                                 ||
        //     | |  - pixels in source space.      || - tile border.
        //     +-+                                 ||
        //
        // The length from A to B is the length in source space or 4 * dx or (count - 1) * dx
        // where dx is the distance between samples. There are 5 destination pixels
        // corresponding to 5 samples specified in the A, B span. The distance from A to the next
        // span starting at C is 5 * dx, so count * dx.
        // Remember, count is the number of pixels needed for the destination and the number of
        // samples.
        // Overall Strategy:
        // * Under - for portions of the span < xMin, take the color at pixel {xMin, y} and use it
        //   to fill in the 5 pixel sampled from A to B.
        // * Middle - for the portion of the span between xMin and xMax sample normally.
        // * Over - for the portion of the span > xMax, take the color at pixel {xMax-1, y} and
        //   use it to fill in the rest of the destination pixels.
        if (dx >= 0) {
            Span leftClamped = span.breakAt(xMin, dx);
            if (!leftClamped.isEmpty()) {
                leftClamped.clampToSinglePixel({xMin, y});
                next->pointSpan(leftClamped);
            }
            Span middle = span.breakAt(xMax, dx);
            if (!middle.isEmpty()) {
                next->pointSpan(middle);
            }
            if (!span.isEmpty()) {
                span.clampToSinglePixel({xMax - 1, y});
                next->pointSpan(span);
            }
        } else {
            Span rightClamped = span.breakAt(xMax, dx);

            if (!rightClamped.isEmpty()) {
                rightClamped.clampToSinglePixel({xMax - 1, y});
                next->pointSpan(rightClamped);
            }
            Span middle = span.breakAt(xMin, dx);
            if (!middle.isEmpty()) {
                next->pointSpan(middle);
            }
            if (!span.isEmpty()) {
                span.clampToSinglePixel({xMin, y});
                next->pointSpan(span);
            }
        }
        return true;
    }

    template <typename Next>
    bool maybeProcessBilerpSpan(BilerpSpan bSpan, Next* next) {
        return false;
    }

private:
    const Sk4s fXMin{SK_FloatNegativeInfinity};
    const Sk4s fYMin{SK_FloatNegativeInfinity};
    const Sk4s fXMax{SK_FloatInfinity};
    const Sk4s fYMax{SK_FloatInfinity};
};

class RepeatStrategy {
public:
    RepeatStrategy(X max) : fXMax{max}, fXInvMax{1.0f / max} { }

    RepeatStrategy(Y max) : fYMax{max}, fYInvMax{1.0f / max} { }

    RepeatStrategy(SkSize max)
        : fXMax{X(max)}, fXInvMax{1.0f / X(max)}, fYMax{Y(max)}, fYInvMax{1.0f / Y(max)} { }

    void processPoints(Sk4s* xs, Sk4s* ys) {
        Sk4s divX = (*xs * fXInvMax).floor();
        Sk4s divY = (*ys * fYInvMax).floor();
        Sk4s baseX = (divX * fXMax);
        Sk4s baseY = (divY * fYMax);
        *xs = *xs - baseX;
        *ys = *ys - baseY;
    }

    template<typename Next>
    bool maybeProcessSpan(Span originalSpan, Next* next) {
        SkASSERT(!originalSpan.isEmpty());
        SkPoint start;
        SkScalar length;
        int count;
        std::tie(start, length, count) = originalSpan;
        // Make x and y in range on the tile.
        SkScalar x = TileMod(X(start), fXMax[0]);
        SkScalar y = TileMod(Y(start), fYMax[0]);
        SkScalar xMax = fXMax[0];
        SkScalar xMin = 0.0f;
        SkScalar dx = length / (count - 1);

        // No need trying to go fast because the steps are larger than a tile or there is one point.
        if (SkScalarAbs(dx) >= xMax || count <= 1) {
            return false;
        }

        //             A        B     C                  D                Z
        // +-------+-------+-------++-------+-------+-------++     +-------+-------++------
        // |       |   *---|*---*--||-*---*-|---*---|*---*--||     |--*---*|       ||
        // |       |       |       ||       |       |       || ... |       |       ||
        // |       |       |       ||       |       |       ||     |       |       ||
        // +-------+-------+-------++-------+-------+-------++     +-------+-------++------
        //                         ^^                       ^^                     ^^
        //                    xMax || xMin             xMax || xMin           xMax || xMin
        //
        //     *---*---*---... - track of samples. * = sample
        //
        //     +-+                                 ||
        //     | |  - pixels in source space.      || - tile border.
        //     +-+                                 ||
        //
        //
        // The given span starts at A and continues on through several tiles to sample point Z.
        // The idea is to break this into several spans one on each tile the entire span
        // intersects. The A to B span only covers a partial tile and has a count of 3 and the
        // distance from A to B is (count - 1) * dx or 2 * dx. The distance from A to the start of
        // the next span is count * dx or 3 * dx. Span C to D covers an entire tile has a count
        // of 5 and a length of 4 * dx. Remember, count is the number of pixels needed for the
        // destination and the number of samples.
        //
        // Overall Strategy:
        // While the span hangs over the edge of the tile, draw the span covering the tile then
        // slide the span over to the next tile.

        // The guard could have been count > 0, but then a bunch of math would be done in the
        // common case.

        Span span({x, y}, length, count);
        if (dx > 0) {
            while (!span.isEmpty() && span.endX() >= xMax) {
                Span toDraw = span.breakAt(xMax, dx);
                next->pointSpan(toDraw);
                span.offset(-xMax);
            }
        } else {
            while (!span.isEmpty() && span.endX() < xMin) {
                Span toDraw = span.breakAt(xMin, dx);
                next->pointSpan(toDraw);
                span.offset(xMax);
            }
        }

        // All on a single tile.
        if (!span.isEmpty()) {
            next->pointSpan(span);
        }

        return true;
    }

    template <typename Next>
    bool maybeProcessBilerpSpan(BilerpSpan bSpan, Next* next) {
        return false;
    }

private:
    SkScalar TileMod(SkScalar x, SkScalar base) {
        return x - std::floor(x / base) * base;
    }
    const Sk4s fXMax{0.0f};
    const Sk4s fXInvMax{0.0f};
    const Sk4s fYMax{0.0f};
    const Sk4s fYInvMax{0.0f};
};

}  // namespace
#endif  // SkLinearBitmapPipeline_tile_DEFINED
