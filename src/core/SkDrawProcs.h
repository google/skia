/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDrawProcs_DEFINED
#define SkDrawProcs_DEFINED

#include "SkDraw.h"
#include "SkGlyph.h"

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

    // Returns the glyph position, which may be rounded or not by the caller
    //   e.g. subpixel doesn't round.
    void operator()(const SkPoint& loc, const SkGlyph& glyph, SkPoint* dst) {
        if (SkPaint::kLeft_Align == fAlign) {
            dst->set(loc.fX, loc.fY);
        } else if (SkPaint::kCenter_Align == fAlign) {
            dst->set(loc.fX - SkFloatToScalar(glyph.fAdvanceX) / 2,
                     loc.fY - SkFloatToScalar(glyph.fAdvanceY) / 2);
        } else {
            SkASSERT(SkPaint::kRight_Align == fAlign);
            dst->set(loc.fX - SkFloatToScalar(glyph.fAdvanceX),
                     loc.fY - SkFloatToScalar(glyph.fAdvanceY));
        }
    }
private:
    const SkPaint::Align fAlign;
};

#endif
