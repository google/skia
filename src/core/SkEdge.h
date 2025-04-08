/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkEdge_DEFINED
#define SkEdge_DEFINED

#include "include/core/SkPoint.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkFixed.h"
#include "include/private/base/SkMath.h"
#include "src/core/SkFDot6.h"

#include <cstdint>
#include <utility>

struct SkIRect;

// This correctly favors the lower-pixel when y0 is on a 1/2 pixel boundary
#define SkEdge_Compute_DY(top, y0)  (SkLeftShift(top, 6) + 32 - (y0))

/**
    SkEdge approximates monotonic curves with one or more line segments in a way that makes
    computing scan lines (rows of horizontal pixels between multiple edges) efficient and easier
    to do. In particular, the line segments will be in terms of Y instead of a general Bezier curve
    which is in terms of t.

    The number of segments will be a power of 2 (for easy division) based on internal heuristics
    about how bendy the curve is.

    The general flow is to create an SkEdge (or one of its subclasses), use the fields to represent
    a line segment, and then use updateQuadratic or updateCubic to update the fields with the next
    line segment's values.
*/
class SkEdge {
public:
    virtual ~SkEdge() = default;
    enum class Type : int8_t {
        kLine,
        kQuad,
        kCubic,
    };
    enum class Winding : int8_t {
        kCW = 1,    // clockwise
        kCCW = -1,  // counter clockwise
    };

    // Can be used to join edges together
    SkEdge* fNext;
    SkEdge* fPrev;

    // The current line segment starts at (fX, fFirstY + 0.5). It has slope
    // fDxDy (yes, run over rise, because this is geared toward horizontal scanlines)
    // and stops once Y gets to fLastY + 0.5.
    SkFixed fX;
    SkFixed fDxDy;

    // These are integers because they represent a discrete pixel. Mathematically, these are
    // treated as half way inside the pixel, so 6 -> 6.5.
    int32_t fFirstY;
    int32_t fLastY;

    Type    fEdgeType;      // Remembers the *initial* edge type
    Winding fWinding;

    // Represent a straight line with an optional clip. This will always be a single segment.
    // Returns false if the line has height 0.
    bool setLine(const SkPoint& p0, const SkPoint& p1, const SkIRect* clip);
    // call this version if you know you don't have a clip
    inline bool setLine(const SkPoint& p0, const SkPoint& p1);

    bool hasNextSegment() const {
        return fSegmentCount != 0;
    }
    // Update fX, fDxDY, fFirstY, and fLastY to represent the segment. It will skip over
    // any lines that have a height of 0 pixels and return false if there were only 0-height
    // segments remaining. For quadratic and cubic curves this will involve forward-differencing
    // (see the subclasses for those values).
    virtual bool nextSegment();

    uint8_t segmentsLeft() const {
        return fSegmentCount;
    }

protected:
    inline bool updateLine(SkFixed ax, SkFixed ay, SkFixed bx, SkFixed by);

    uint8_t fSegmentCount; // only non-zero for Quad and Cubics
    // How much to shift the derivatives to multiply by deltaT when doing forward-differencing.
    // For cubics, this is log_2(N) and for quadratics this is log_2(N) - 1.
    uint8_t fCurveShift;

private:
    void chopLineWithClip(const SkIRect& clip);

#if defined(SK_DEBUG)
public:
    virtual void dump() const;
    void validate() const {
        SkASSERT(fPrev && fNext);
        SkASSERT(fPrev->fNext == this);
        SkASSERT(fNext->fPrev == this);

        SkASSERT(fFirstY <= fLastY);
        SkASSERT(fWinding == Winding::kCW || fWinding == Winding::kCCW);
    }
#endif
};

class SkQuadraticEdge final : public SkEdge {
public:
    // Sets up the line segments. Returns false if the line would be of height 0.
    bool setQuadratic(const SkPoint pts[3]);
    bool nextSegment() override;

private:
    // These are the non-rounded points that the current line segment ends at.
    SkFixed fQx, fQy;
    // These represent the first derivatives of the quadratic curve evaluated
    // at the midpoint of the next line segment. To avoid overflows, we store them as half
    // their normal value. During the forward-difference step, instead of multiplying by
    // a deltaT of 1/N, we'll multiply by 2/N instead.
    SkFixed fQDxDt, fQDyDt;
    // These are the second derivatives of the quadratic curve pre-multiplied by 1/N.
    SkFixed fQD2xDt2, fQD2yDt2;

    // The non-rounded end points for the entire curve. On the last segment, these
    // will be used instead of the results from our forward-differnce technique
    // to make sure cumulative error doesn't result in a dramatically different line.
    SkFixed fQLastX, fQLastY;

#if defined(SK_DEBUG)
public:
    void dump() const override;
#endif
};

class SkCubicEdge final : public SkEdge {
public:
    // Sets up the line segments. Returns false if the line would be of height 0.
    bool setCubic(const SkPoint pts[4]);
    bool nextSegment() override;

private:
    // These are the non-rounded points that the current line segment ends at.
    SkFixed fCx, fCy;
    SkFixed fCDxDt, fCDyDt;
    SkFixed fCD2xDt2, fCD2yDt2;
    SkFixed fCD3xDt3, fCD3yDt3;

    // The non-rounded end points for the entire curve. On the last segment, these
    // will be used instead of the results from our forward-difference technique
    // to make sure cumulative error doesn't result in a dramatically different line.
    SkFixed fCLastX, fCLastY;

    uint8_t fCubicDShift;   // applied to fCDxDt and fCDyDt only in cubic

#if defined(SK_DEBUG)
public:
    void dump() const override;
#endif
};

bool SkEdge::setLine(const SkPoint& p0, const SkPoint& p1) {
    SkFDot6 x0, y0, x1, y1;

#if defined(SK_RASTERIZE_EVEN_ROUNDING)
    x0 = SkScalarRoundToFDot6(p0.fX, 0);
    y0 = SkScalarRoundToFDot6(p0.fY, 0);
    x1 = SkScalarRoundToFDot6(p1.fX, 0);
    y1 = SkScalarRoundToFDot6(p1.fY, 0);
#else
    x0 = SkFloatToFDot6(p0.fX);
    y0 = SkFloatToFDot6(p0.fY);
    x1 = SkFloatToFDot6(p1.fX);
    y1 = SkFloatToFDot6(p1.fY);
#endif

    Winding winding = Winding::kCW;

    if (y0 > y1) {
        using std::swap;
        swap(x0, x1);
        swap(y0, y1);
        winding = Winding::kCCW;
    }

    int top = SkFDot6Round(y0);
    int bot = SkFDot6Round(y1);

    // are we a zero-height line?
    if (top == bot) {
        return false;
    }

    SkFixed slope = SkFDot6Div(x1 - x0, y1 - y0);
    const SkFDot6 dy  = SkEdge_Compute_DY(top, y0);

    // Note that SkFixedMul(SkFixed, SkFDot6) produces results in SkFDot6
    fX          = SkFDot6ToFixed(x0 + SkFixedMul(slope, dy));
    fDxDy       = slope;
    fFirstY     = top;
    fLastY      = bot - 1;
    fEdgeType   = Type::kLine;
    fSegmentCount = 0;
    fWinding    = winding;
    fCurveShift = 0;
    return true;
}

#endif
