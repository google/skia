/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDrawProcs_DEFINED
#define SkDrawProcs_DEFINED

#include "SkBlitter.h"
#include "SkDraw.h"
#include "SkGlyph.h"

class SkAAClip;
class SkBlitter;

struct SkDraw1Glyph {
    const SkDraw* fDraw;
    const SkRegion* fClip;
    const SkAAClip* fAAClip;
    SkBlitter* fBlitter;
    SkGlyphCache* fCache;
    const SkPaint* fPaint;
    SkIRect fClipBounds;
    /** Half the sampling frequency of the rasterized glyph in x. */
    SkFixed fHalfSampleX;
    /** Half the sampling frequency of the rasterized glyph in y. */
    SkFixed fHalfSampleY;

    /** Draws one glyph.
     *
     *  The x and y are pre-biased, so implementations may just truncate them.
     *  i.e. half the sampling frequency has been added.
     *  e.g. 1/2 or 1/(2^(SkGlyph::kSubBits+1)) has already been added.
     *  This added bias can be found in fHalfSampleX,Y.
     */
    typedef void (*Proc)(const SkDraw1Glyph&, SkFixed x, SkFixed y, const SkGlyph&);

    Proc init(const SkDraw* draw, SkBlitter* blitter, SkGlyphCache* cache,
              const SkPaint&);

    // call this instead of fBlitter->blitMask() since this wrapper will handle
    // the case when the mask is ARGB32_Format
    //
    void blitMask(const SkMask& mask, const SkIRect& clip) const {
        if (SkMask::kARGB32_Format == mask.fFormat) {
            this->blitMaskAsSprite(mask);
        } else {
            fBlitter->blitMask(mask, clip);
        }
    }

    // mask must be kARGB32_Format
    void blitMaskAsSprite(const SkMask& mask) const;
};

struct SkDrawProcs {
    SkDraw1Glyph::Proc  fD1GProc;
};

bool SkDrawTreatAAStrokeAsHairline(SkScalar strokeWidth, const SkMatrix&,
                                   SkScalar* coverage);

/**
 *  If the current paint is set to stroke and the stroke-width when applied to
 *  the matrix is <= 1.0, then this returns true, and sets coverage (simulating
 *  a stroke by drawing a hairline with partial coverage). If any of these
 *  conditions are false, then this returns false and coverage is ignored.
 */
inline bool SkDrawTreatAsHairline(const SkPaint& paint, const SkMatrix& matrix,
                                  SkScalar* coverage) {
    if (SkPaint::kStroke_Style != paint.getStyle()) {
        return false;
    }

    SkScalar strokeWidth = paint.getStrokeWidth();
    if (0 == strokeWidth) {
        *coverage = SK_Scalar1;
        return true;
    }

    if (!paint.isAntiAlias()) {
        return false;
    }

    return SkDrawTreatAAStrokeAsHairline(strokeWidth, matrix, coverage);
}

class SkTextAlignProc {
public:
    SkTextAlignProc(SkPaint::Align align)
        : fAlign(align) {
    }

    // Returns the position of the glyph in fixed point, which may be rounded or not
    //         by the caller e.g. subpixel doesn't round.
    // @param point interpreted as SkFixed [x, y].
    void operator()(const SkPoint& loc, const SkGlyph& glyph, SkIPoint* dst) {
        if (SkPaint::kLeft_Align == fAlign) {
            dst->set(SkScalarToFixed(loc.fX), SkScalarToFixed(loc.fY));
        } else if (SkPaint::kCenter_Align == fAlign) {
            dst->set(SkScalarToFixed(loc.fX) - (glyph.fAdvanceX >> 1),
                     SkScalarToFixed(loc.fY) - (glyph.fAdvanceY >> 1));
        } else {
            SkASSERT(SkPaint::kRight_Align == fAlign);
            dst->set(SkScalarToFixed(loc.fX) - glyph.fAdvanceX,
                     SkScalarToFixed(loc.fY) - glyph.fAdvanceY);
        }
    }
private:
    const SkPaint::Align fAlign;
};

class SkTextAlignProcScalar {
public:
    SkTextAlignProcScalar(SkPaint::Align align)
        : fAlign(align) {
    }

    // Returns the glyph position, which may be rounded or not by the caller
    //   e.g. subpixel doesn't round.
    void operator()(const SkPoint& loc, const SkGlyph& glyph, SkPoint* dst) {
        if (SkPaint::kLeft_Align == fAlign) {
            dst->set(loc.fX, loc.fY);
        } else if (SkPaint::kCenter_Align == fAlign) {
            dst->set(loc.fX - SkFixedToScalar(glyph.fAdvanceX >> 1),
                     loc.fY - SkFixedToScalar(glyph.fAdvanceY >> 1));
        } else {
            SkASSERT(SkPaint::kRight_Align == fAlign);
            dst->set(loc.fX - SkFixedToScalar(glyph.fAdvanceX),
                     loc.fY - SkFixedToScalar(glyph.fAdvanceY));
        }
    }
private:
    const SkPaint::Align fAlign;
};

#endif
