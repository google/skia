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
Matrix::Matrix(const SkMatrix& m)
    : INHERITED(kBubbleDamage_Trait)
    , fMatrix(m) {}

SkRect Matrix::onRevalidate(InvalidationController*, const SkMatrix&) {
    return SkRect::MakeEmpty();
}

SkMatrix Matrix::getTotalMatrix() const {
    return fMatrix;
}

ComposedMatrix::ComposedMatrix(const SkMatrix& m, sk_sp<Matrix> parent)
    : INHERITED(m)
    , fParent(std::move(parent)) {
    SkASSERT(fParent);
    this->observeInval(fParent);
}

ComposedMatrix::~ComposedMatrix() {
    this->unobserveInval(fParent);
}

SkRect ComposedMatrix::onRevalidate(InvalidationController* ic, const SkMatrix& ctm) {
    fParent->revalidate(ic, ctm);

    return SkRect::MakeEmpty();
}

SkMatrix ComposedMatrix::getTotalMatrix() const {
    return SkMatrix::Concat(fParent->getTotalMatrix(), fMatrix);
}

Transform::Transform(sk_sp<RenderNode> child, sk_sp<Matrix> matrix)
    : INHERITED(std::move(child))
    , fMatrix(std::move(matrix)) {
    this->observeInval(fMatrix);
}

Transform::~Transform() {
    this->unobserveInval(fMatrix);
}

void Transform::onRender(SkCanvas* canvas, const RenderContext* ctx) const {
    const auto& m = fMatrix->getTotalMatrix();
    SkAutoCanvasRestore acr(canvas, !m.isIdentity());
    canvas->concat(m);
    this->INHERITED::onRender(canvas, ctx);
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
