/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSGPlane.h"

#include "SkCanvas.h"
#include "SkPath.h"

namespace sksg {

Plane::Plane() = default;

void Plane::onClip(SkCanvas*, bool) const {}

void Plane::onDraw(SkCanvas* canvas, const SkPaint& paint) const {
    canvas->drawPaint(paint);
}

SkRect Plane::onRevalidate(InvalidationController*, const SkMatrix&) {
    SkASSERT(this->hasInval());

    return SkRect::MakeLTRB(SK_ScalarMin, SK_ScalarMin, SK_ScalarMax, SK_ScalarMax);
}

SkPath Plane::onAsPath() const {
    SkPath path;
    path.setFillType(SkPath::kInverseWinding_FillType);

    return path;
}

} // namespace sksg
