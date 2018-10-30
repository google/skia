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

#endif
