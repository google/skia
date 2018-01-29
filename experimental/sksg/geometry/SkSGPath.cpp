/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSGPath.h"

#include "SkCanvas.h"
#include "SkPaint.h"

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

    return fPath.computeTightBounds();
}

SkPath Path::onAsPath() const {
    return fPath;
}

} // namespace sksg
