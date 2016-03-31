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
class XClampStrategy {
public:
    XClampStrategy(int32_t max)
        : fXsMax{SkScalar(max - 0.5f)}
        , fXMax{SkScalar(max)} { }

    void tileXPoints(Sk4s* xs) {
        *xs = Sk4s::Min(Sk4s::Max(*xs, 0.0f), fXsMax);
        SkASSERT(0 <= (*xs)[0] && (*xs)[0] < fXMax);
        SkASSERT(0 <= (*xs)[1] && (*xs)[1] < fXMax);
        SkASSERT(0 <= (*xs)[2] && (*xs)[2] < fXMax);
        SkASSERT(0 <= (*xs)[3] && (*xs)[3] < fXMax);
    }

    template<typename Next>
    bool maybeProcessSpan(Span originalSpan, Next* next) {
        SkASSERT(!originalSpan.isEmpty());
        SkPoint start; SkScalar length; int count;
        std::tie(start, length, count) = originalSpan;
        SkScalar x = X(start);
        SkScalar y = Y(start);
        Span span{{x, y}, length, count};

        if (span.completelyWithin(0.0f, fXMax)) {
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
            Span leftClamped = span.breakAt(0.0f, dx);
            if (!leftClamped.isEmpty()) {
                leftClamped.clampToSinglePixel({0.0f, y});
                next->pointSpan(leftClamped);
            }
            Span center = span.breakAt(fXMax, dx);
            if (!center.isEmpty()) {
                next->pointSpan(center);
            }
            if (!span.isEmpty()) {
                span.clampToSinglePixel({fXMax - 1, y});
                next->pointSpan(span);
            }
        } else {
            Span center = span.breakAt(fXMax, dx);

            if (!span.isEmpty()) {
                span.clampToSinglePixel({fXMax - 1, y});
                next->pointSpan(span);
            }
            Span leftEdge = center.breakAt(0.0f, dx);
            if (!center.isEmpty()) {
                next->pointSpan(center);
            }
            if (!leftEdge.isEmpty()) {
                leftEdge.clampToSinglePixel({0.0f, y});
                next->pointSpan(leftEdge);
            }
        }
        return true;
    }

private:
    const Sk4s     fXsMax;
    const SkScalar fXMax;
};

class YClampStrategy {
public:
    YClampStrategy(int32_t max)
        : fYMax{SkScalar(max) - 0.5f}
        , fYsMax{SkScalar(max) - 0.5f} { }

    void tileYPoints(Sk4s* ys) {
        *ys = Sk4s::Min(Sk4s::Max(*ys, 0.0f), fYsMax);
        SkASSERT(0 <= (*ys)[0] && (*ys)[0] <= fYMax);
        SkASSERT(0 <= (*ys)[1] && (*ys)[1] <= fYMax);
        SkASSERT(0 <= (*ys)[2] && (*ys)[2] <= fYMax);
        SkASSERT(0 <= (*ys)[3] && (*ys)[3] <= fYMax);
    }

    SkScalar tileY(SkScalar y) {
        return std::min(std::max<SkScalar>(0.0f, y), fYMax);
    }

private:
    const SkScalar fYMax;
    const Sk4s     fYsMax;
};

SkScalar tile_mod(SkScalar x, SkScalar base) {
    return x - SkScalarFloorToScalar(x / base) * base;
}

class XRepeatStrategy {
public:
    XRepeatStrategy(int32_t max)
        : fXMax{SkScalar(max)}
        , fXsMax{SkScalar(max)}
        , fXsCap{SkScalar(nextafterf(SkScalar(max), 0.0f))}
        , fXsInvMax{1.0f / SkScalar(max)} { }

    void tileXPoints(Sk4s* xs) {
        Sk4s divX = *xs * fXsInvMax;
        Sk4s modX = *xs - divX.floor() * fXsMax;
        *xs = Sk4s::Min(fXsCap, modX);
        SkASSERT(0 <= (*xs)[0] && (*xs)[0] < fXMax);
        SkASSERT(0 <= (*xs)[1] && (*xs)[1] < fXMax);
        SkASSERT(0 <= (*xs)[2] && (*xs)[2] < fXMax);
        SkASSERT(0 <= (*xs)[3] && (*xs)[3] < fXMax);
    }

    template<typename Next>
    bool maybeProcessSpan(Span originalSpan, Next* next) {
        SkASSERT(!originalSpan.isEmpty());
        SkPoint start; SkScalar length; int count;
        std::tie(start, length, count) = originalSpan;
        // Make x and y in range on the tile.
        SkScalar x = tile_mod(X(start), fXMax);
        SkScalar y = Y(start);
        SkScalar dx = length / (count - 1);

        // No need trying to go fast because the steps are larger than a tile or there is one point.
        if (SkScalarAbs(dx) >= fXMax || count <= 1) {
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
            while (!span.isEmpty() && span.endX() >= fXMax) {
                Span toDraw = span.breakAt(fXMax, dx);
                next->pointSpan(toDraw);
                span.offset(-fXMax);
            }
        } else {
            while (!span.isEmpty() && span.endX() < 0.0f) {
                Span toDraw = span.breakAt(0.0f, dx);
                next->pointSpan(toDraw);
                span.offset(fXMax);
            }
        }

        // All on a single tile.
        if (!span.isEmpty()) {
            next->pointSpan(span);
        }

        return true;
    }

private:
    const SkScalar fXMax;
    const Sk4s     fXsMax;
    const Sk4s     fXsCap;
    const Sk4s     fXsInvMax;
};

class YRepeatStrategy {
public:
    YRepeatStrategy(int32_t max)
        : fYMax{SkScalar(max)}
        , fYsMax{SkScalar(max)}
        , fYsInvMax{1.0f / SkScalar(max)} { }

    void tileYPoints(Sk4s* ys) {
        Sk4s divY = *ys * fYsInvMax;
        Sk4s modY = *ys - divY.floor() * fYsMax;
        *ys = modY;
        SkASSERT(0 <= (*ys)[0] && (*ys)[0] < fYMax);
        SkASSERT(0 <= (*ys)[1] && (*ys)[1] < fYMax);
        SkASSERT(0 <= (*ys)[2] && (*ys)[2] < fYMax);
        SkASSERT(0 <= (*ys)[3] && (*ys)[3] < fYMax);
    }

    SkScalar tileY(SkScalar y) {
        SkScalar answer = tile_mod(y, fYMax);
        SkASSERT(0 <= answer && answer < fYMax);
        return answer;
    }

private:
    const SkScalar fYMax;
    const Sk4s     fYsMax;
    const Sk4s     fYsInvMax;
};
// max = 40
// mq2[x_] := Abs[(x - 40) - Floor[(x - 40)/80] * 80 - 40]
class XMirrorStrategy {
public:
    XMirrorStrategy(int32_t max)
        : fXsMax{SkScalar(max)}
        , fXsCap{SkScalar(nextafterf(SkScalar(max), 0.0f))}
        , fXsDoubleInvMax{1.0f / (2.0f * SkScalar(max))} { }

    void tileXPoints(Sk4s* xs) {
        Sk4f bias   = *xs - fXsMax;
        Sk4f div    = bias * fXsDoubleInvMax;
        Sk4f mod    = bias - div.floor() * 2.0f * fXsMax;
        Sk4f unbias = mod - fXsMax;
        *xs = Sk4f::Min(unbias.abs(), fXsCap);
        SkASSERT(0 <= (*xs)[0] && (*xs)[0] < fXsMax[0]);
        SkASSERT(0 <= (*xs)[1] && (*xs)[1] < fXsMax[0]);
        SkASSERT(0 <= (*xs)[2] && (*xs)[2] < fXsMax[0]);
        SkASSERT(0 <= (*xs)[3] && (*xs)[3] < fXsMax[0]);
    }

    template <typename Next>
    bool maybeProcessSpan(Span originalSpan, Next* next) { return false; }

private:
    Sk4f     fXsMax;
    Sk4f     fXsCap;
    Sk4f     fXsDoubleInvMax;
};

class YMirrorStrategy {
public:
    YMirrorStrategy(int32_t max)
        : fYMax{SkScalar(max)}
        , fYsMax{SkScalar(max)}
        , fYsCap{nextafterf(SkScalar(max), 0.0f)}
        , fYsDoubleInvMax{1.0f / (2.0f * SkScalar(max))} { }

    void tileYPoints(Sk4s* ys) {
        Sk4f bias   = *ys - fYsMax;
        Sk4f div    = bias * fYsDoubleInvMax;
        Sk4f mod    = bias - div.floor() * 2.0f * fYsMax;
        Sk4f unbias = mod - fYsMax;
        *ys = Sk4f::Min(unbias.abs(), fYsCap);
        SkASSERT(0 <= (*ys)[0] && (*ys)[0] < fYMax);
        SkASSERT(0 <= (*ys)[1] && (*ys)[1] < fYMax);
        SkASSERT(0 <= (*ys)[2] && (*ys)[2] < fYMax);
        SkASSERT(0 <= (*ys)[3] && (*ys)[3] < fYMax);
    }

    SkScalar tileY(SkScalar y) {
        SkScalar bias   = y - fYMax;
        SkScalar div    = bias * fYsDoubleInvMax[0];
        SkScalar mod    = bias - SkScalarFloorToScalar(div) * 2.0f * fYMax;
        SkScalar unbias = mod - fYMax;
        SkScalar answer = SkMinScalar(SkScalarAbs(unbias), fYsCap[0]);
        SkASSERT(0 <= answer && answer < fYMax);
        return answer;
    };

private:
    SkScalar fYMax;
    Sk4f     fYsMax;
    Sk4f     fYsCap;
    Sk4f     fYsDoubleInvMax;
};

}  // namespace
#endif  // SkLinearBitmapPipeline_tile_DEFINED
