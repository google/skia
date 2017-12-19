/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSGGeometryNode.h"

#include "SkMatrix.h"
#include "SkSGInvalidationController.h"

namespace sksg {

GeometryNode::GeometryNode()
    : fBounds(SkRect::MakeLTRB(SK_ScalarMin, SK_ScalarMin, SK_ScalarMax, SK_ScalarMax)) {}

void GeometryNode::draw(SkCanvas* canvas, const SkPaint& paint) const {
    SkASSERT(!this->isInvalidated());
    this->onDraw(canvas, paint);
}

static void inval_rect(const SkRect& r, const SkMatrix& ctm, InvalidationController* ic) {
    if (ctm.isIdentity()) {
        ic->inval(r);
        return;
    }

    SkRect mappedRect;
    if (!ctm.mapRect(&mappedRect, r)) {
        mappedRect = SkRect::MakeLTRB(SK_ScalarMin, SK_ScalarMin, SK_ScalarMax, SK_ScalarMax);
    }
    ic->inval(mappedRect);
}

void GeometryNode::onRevalidate(InvalidationController* ic, const SkMatrix& ctm) {
    SkASSERT(this->isInvalidated());

    const auto oldBounds = fBounds;
    fBounds = this->onComputeBounds();

    inval_rect(oldBounds, ctm, ic);
    if (fBounds != oldBounds) {
        inval_rect(fBounds, ctm, ic);
    }
}

} // namespace sksg
