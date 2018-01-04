/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSGTransform.h"

#include "SkCanvas.h"

namespace sksg {

Matrix::Matrix(const SkMatrix& m) : fMatrix(m) {}

SkRect Matrix::onRevalidate(InvalidationController*, const SkMatrix&) {
    // Nothing to revalidate, we're just wrapping a SkMatrix.
    return SkRect::MakeEmpty();
}

ComposedMatrix::ComposedMatrix(sk_sp<Matrix> preMatrix, const SkMatrix& m)
    : INHERITED(m)
    , fPreMatrix(std::move(preMatrix)) {
    fPreMatrix->addInvalReceiver(this);
}

ComposedMatrix::~ComposedMatrix() {
    fPreMatrix->removeInvalReceiver(this);
}

SkRect ComposedMatrix::onRevalidate(InvalidationController* ic, const SkMatrix& ctm) {
    fPreMatrix->revalidate(ic, ctm);
    fTotalMatrix = SkMatrix::Concat(fPreMatrix->totalMatrix(), this->getMatrix());
    return SkRect::MakeEmpty();
}

Transform::Transform(sk_sp<RenderNode> child, sk_sp<Matrix> matrix)
    : INHERITED(std::move(child))
    , fMatrix(std::move(matrix)) {
    fMatrix->addInvalReceiver(this);
}

Transform::~Transform() {
    fMatrix->removeInvalReceiver(this);
}

void Transform::onRender(SkCanvas* canvas) const {
    const auto& m = fMatrix->totalMatrix();
    SkAutoCanvasRestore acr(canvas, !m.isIdentity());
    canvas->concat(m);
    this->INHERITED::onRender(canvas);
}

SkRect Transform::onRevalidate(InvalidationController* ic, const SkMatrix& ctm) {
    SkASSERT(this->hasInval());

    // We don't care about matrix reval bounds.
    fMatrix->revalidate(ic, ctm);

    const auto& m = fMatrix->totalMatrix();
    auto bounds = this->INHERITED::onRevalidate(ic, SkMatrix::Concat(ctm, m));
    m.mapRect(&bounds);

    return bounds;
}

} // namespace sksg
