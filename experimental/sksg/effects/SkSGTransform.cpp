/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSGTransform.h"

#include "SkCanvas.h"

namespace sksg {

Transform::Transform(sk_sp<RenderNode> child, const SkMatrix& matrix)
    : INHERITED(std::move(child))
    , fMatrix(matrix) {}

void Transform::onRender(SkCanvas* canvas) const {
    SkAutoCanvasRestore acr(canvas, !fMatrix.isIdentity());
    canvas->concat(fMatrix);
    this->INHERITED::onRender(canvas);
}

Node::RevalidationResult Transform::onRevalidate(InvalidationController* ic, const SkMatrix& ctm) {
    SkASSERT(this->hasInval());

    auto result = this->INHERITED::onRevalidate(ic, SkMatrix::Concat(ctm, fMatrix));
    fMatrix.mapRect(&result.fBounds);

    return result;
}

} // namespace sksg
