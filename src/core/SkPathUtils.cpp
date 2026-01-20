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
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkStrokeRec.h"
#include "src/core/SkMatrixPriv.h"

namespace skpathutils {

#ifdef SK_SUPPORT_MUTABLE_PATHEFFECT
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
    SkPathBuilder builder(*dst);
    bool isFilled = FillPathWithPaint(src, paint, &builder, cullRect, ctm);
    *dst = builder.detach();
    return isFilled;
}
#endif

bool FillPathWithPaint(const SkPath& origSrc, const SkPaint& paint, SkPathBuilder* builder,
                       const SkRect* cullRect, const SkMatrix& ctm) {
    if (!origSrc.isFinite()) {
        builder->reset();
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

    const SkPath* srcPtr = &origSrc;
    SkPath pathStorage;
    SkPathEffect* pe = paint.getPathEffect();
    if (pe && pe->filterPath(builder, origSrc, &rec, cullRect, ctm)) {
        pathStorage = builder->detach();
        srcPtr = &pathStorage;
    }
    if (!rec.applyToPath(builder, *srcPtr)) {
        *builder = *srcPtr;
    }

    if (!builder->isFinite()) {
        builder->reset();
    }
    return !rec.isHairlineStyle();
}

bool FillPathWithPaint(const SkPath& src, const SkPaint& paint, SkPathBuilder* dst) {
    return FillPathWithPaint(src, paint, dst, nullptr, SkMatrix::I());
}

SkPath FillPathWithPaint(const SkPath& src, const SkPaint& paint, bool* isFillPtr) {
    SkPathBuilder builder;
    bool isFill = FillPathWithPaint(src, paint, &builder);
    if (isFillPtr) {
        *isFillPtr = isFill;
    }
    return builder.detach();
}


} // namespace skpathutils
