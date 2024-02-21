/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPathUtils.h"

#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkStrokeRec.h"
#include "src/core/SkMatrixPriv.h"

namespace skpathutils {

bool FillPathWithPaint(const SkPath& src, const SkPaint& paint, SkPath* dst) {
    return skpathutils::FillPathWithPaint(src, paint, dst, nullptr, 1);
}

bool FillPathWithPaint(const SkPath& src, const SkPaint& paint, SkPath* dst,
                       const SkRect* cullRect, SkScalar resScale) {
    return skpathutils::FillPathWithPaint(src, paint, dst, cullRect,
                                          SkMatrix::Scale(resScale, resScale));
}

bool FillPathWithPaint(const SkPath& src, const SkPaint& paint, SkPath* dst,
                       const SkRect* cullRect, const SkMatrix& ctm) {
    if (!src.isFinite()) {
        dst->reset();
        return false;
    }

    const SkScalar resScale = SkMatrixPriv::ComputeResScaleForStroking(ctm);
    SkStrokeRec rec(paint, resScale);

#if defined(SK_BUILD_FOR_FUZZER)
    // Prevent lines with small widths from timing out.
    if (rec.getStyle() == SkStrokeRec::Style::kStroke_Style && rec.getWidth() < 0.001) {
        return false;
    }
#endif

    const SkPath* srcPtr = &src;
    SkPath tmpPath;

    SkPathEffect* pe = paint.getPathEffect();
    if (pe && pe->filterPath(&tmpPath, src, &rec, cullRect, ctm)) {
        srcPtr = &tmpPath;
    }

    if (!rec.applyToPath(dst, *srcPtr)) {
        if (srcPtr == &tmpPath) {
            // If path's were copy-on-write, this trick would not be needed.
            // As it is, we want to save making a deep-copy from tmpPath -> dst
            // since we know we're just going to delete tmpPath when we return,
            // so the swap saves that copy.
            dst->swap(tmpPath);
        } else {
            *dst = *srcPtr;
        }
    }

    if (!dst->isFinite()) {
        dst->reset();
        return false;
    }
    return !rec.isHairlineStyle();
}

} // namespace skpathutils

bool FillPathWithPaint(const SkPath& src,
                       const SkPaint& paint,
                       SkPath* dst,
                       const SkRect* cullRect,
                       SkScalar resScale) {
    return skpathutils::FillPathWithPaint(src, paint, dst, cullRect, resScale);
}

bool FillPathWithPaint(const SkPath& src,
                       const SkPaint& paint,
                       SkPath* dst,
                       const SkRect* cullRect,
                       const SkMatrix& ctm) {
    return skpathutils::FillPathWithPaint(src, paint, dst, cullRect, ctm);
}

bool FillPathWithPaint(const SkPath& src, const SkPaint& paint, SkPath* dst) {
    return skpathutils::FillPathWithPaint(src, paint, dst);
}
