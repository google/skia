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

void Transform::onRevalidate(InvalidationController* ic, const SkMatrix& ctm) {
    const auto localCTM = SkMatrix::Concat(ctm, fMatrix);
    this->INHERITED::onRevalidate(ic, localCTM);
}

} // namespace sksg
