/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/sksg/include/SkSGPath.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "src/core/SkRectPriv.h"

namespace sksg {

Path::Path(const SkPath& path) : fPath(path) {}

void Path::onClip(SkCanvas* canvas, bool antiAlias) const {
    canvas->clipPath(fPath, SkClipOp::kIntersect, antiAlias);
}

void Path::onDraw(SkCanvas* canvas, const SkPaint& paint) const {
    canvas->drawPath(fPath, paint);
}

bool Path::onContains(const SkPoint& p) const {
    return fPath.contains(p.x(), p.y());
}

SkRect Path::onRevalidate(InvalidationController*, const SkMatrix&) {
    SkASSERT(this->hasInval());

    const auto ft = fPath.getFillType();
    return (ft == SkPath::kWinding_FillType || ft == SkPath::kEvenOdd_FillType)
        // "Containing" fills have finite bounds.
        ? fPath.computeTightBounds()
        // Inverse fills are "infinite".
        : SkRectPriv::MakeLargeS32();
}

SkPath Path::onAsPath() const {
    return fPath;
}

} // namespace sksg
