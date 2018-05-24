/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSGTransform.h"

#include "SkCanvas.h"

namespace sksg {
// Matrix nodes don't generate damage on their own, but via aggregation ancestor Transform nodes.
Matrix::Matrix(const SkMatrix& m, sk_sp<Matrix> parent)
    : INHERITED(kBubbleDamage_Trait)
    , fParent(std::move(parent))
    , fLocalMatrix(m) {
    if (fParent) {
        this->observeInval(fParent);
    }
}

Matrix::~Matrix() {
    if (fParent) {
        this->unobserveInval(fParent);
    }
}

SkRect Matrix::onRevalidate(InvalidationController* ic, const SkMatrix& ctm) {
    fTotalMatrix = fLocalMatrix;

    if (fParent) {
        fParent->revalidate(ic, ctm);
        fTotalMatrix.postConcat(fParent->getTotalMatrix());
    }

    return SkRect::MakeEmpty();
}

Transform::Transform(sk_sp<RenderNode> child, sk_sp<Matrix> matrix)
    : INHERITED(std::move(child))
    , fMatrix(std::move(matrix)) {
    this->observeInval(fMatrix);
}

Transform::~Transform() {
    this->unobserveInval(fMatrix);
}

void Transform::onRender(SkCanvas* canvas) const {
    const auto& m = fMatrix->getTotalMatrix();
    SkAutoCanvasRestore acr(canvas, !m.isIdentity());
    canvas->concat(m);
    this->INHERITED::onRender(canvas);
}

SkRect Transform::onRevalidate(InvalidationController* ic, const SkMatrix& ctm) {
    SkASSERT(this->hasInval());

    // We don't care about matrix reval results.
    fMatrix->revalidate(ic, ctm);

    const auto& m = fMatrix->getTotalMatrix();
    auto bounds = this->INHERITED::onRevalidate(ic, SkMatrix::Concat(ctm, m));
    m.mapRect(&bounds);

    return bounds;
}

} // namespace sksg
