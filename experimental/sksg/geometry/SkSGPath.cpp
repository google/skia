/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSGPath.h"

#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkRectPriv.h"

namespace sksg {

Path::Path(const SkPath& path) : fPath(path) {}

void Path::onClip(SkCanvas* canvas, bool antiAlias) const {
    canvas->clipPath(fPath, SkClipOp::kIntersect, antiAlias);
}

void Path::onDraw(SkCanvas* canvas, const SkPaint& paint) const {
    canvas->drawPath(fPath, paint);
}

SkRect Path::onRevalidate(InvalidationController*, const SkMatrix&) {
    SkASSERT(this->hasInval());

    switch (fPath.getFillType()) {
    case SkPath::kWinding_FillType:
        // Fallthrough
    case SkPath::kEvenOdd_FillType:
        return fPath.computeTightBounds();
    case SkPath::kInverseWinding_FillType:
        // Fallthrough
    case SkPath::kInverseEvenOdd_FillType:
        return SkRectPriv::MakeLargeS32();
    }
}

SkPath Path::onAsPath() const {
    return fPath;
}

} // namespace sksg
