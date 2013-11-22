
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

class SkAAClip;
class SkBlitter;

struct SkDraw1Glyph {
    const SkDraw* fDraw;
    SkBounder* fBounder;
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
#if SK_DISTANCEFIELD_FONTS
    uint32_t            fFlags;

    enum Flags {
        /**
         * Disable baked glyph transforms
         */
        kSkipBakedGlyphTransform_Flag = 0x1,
        /**
         * Scale glyphs to get different point sizes
         */
        kUseScaledGlyphs_Flag         = 0x2,
    };

    static const int kBaseDFFontSize = 32;
#endif
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

#endif
