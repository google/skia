/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSGGeometryNode.h"

#include "SkSGInvalidationController.h"

namespace sksg {

GeometryNode::GeometryNode()
    : fBounds(SkRect::MakeLTRB(SK_ScalarMin, SK_ScalarMin, SK_ScalarMax, SK_ScalarMax)) {}

void GeometryNode::draw(SkCanvas* canvas, const SkPaint& paint) const {
    SkASSERT(!this->isInvalidated());
    this->onDraw(canvas, paint);
}

void GeometryNode::onRevalidate(InvalidationController* ic) {
    SkASSERT(this->isInvalidated());

    const auto oldBounds = fBounds;
    fBounds = this->onComputeBounds();

    ic->inval(oldBounds);
    if (fBounds != oldBounds) {
        ic->inval(fBounds);
    }
}

} // namespace sksg
