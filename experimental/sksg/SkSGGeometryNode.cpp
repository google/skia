/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSGGeometryNode.h"

namespace sksg {

GeometryNode::GeometryNode()
    : fBounds(SkRect::MakeLTRB(SK_ScalarMin, SK_ScalarMin, SK_ScalarMax, SK_ScalarMax)) {}

void GeometryNode::draw(SkCanvas* canvas, const SkPaint& paint) const {
    this->onDraw(canvas, paint);
}

const SkRect& GeometryNode::getBounds() {
    if (this->getInvalState() & kGeometry_Inval) {
        fBounds = this->onComputeBounds();
        this->clearInvalState(kGeometry_Inval);
    }

    return fBounds;
}

} // namespace sksg
