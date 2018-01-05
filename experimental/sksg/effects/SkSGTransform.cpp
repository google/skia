/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSGTransform.h"

#include "SkCanvas.h"

namespace sksg {

Matrix::Matrix(const SkMatrix& m, sk_sp<Matrix> parent)
    : fParent(std::move(parent))
    , fLocalMatrix(m) {
    if (fParent) {
        fParent->addInvalReceiver(this);
    }
}

Matrix::~Matrix() {
    if (fParent) {
        fParent->removeInvalReceiver(this);
    }
}

Node::RevalidationResult Matrix::onRevalidate(InvalidationController* ic, const SkMatrix& ctm) {
    fTotalMatrix = fLocalMatrix;

    if (fParent) {
        fParent->revalidate(ic, ctm);
        fTotalMatrix.postConcat(fParent->getTotalMatrix());
    }

    // A free-floating matrix contributes no damage.
    return { SkRect::MakeEmpty(), Damage::kBlockSelf };
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
    const auto& m = fMatrix->getTotalMatrix();
    SkAutoCanvasRestore acr(canvas, !m.isIdentity());
    canvas->concat(m);
    this->INHERITED::onRender(canvas);
}

Node::RevalidationResult Transform::onRevalidate(InvalidationController* ic, const SkMatrix& ctm) {
    SkASSERT(this->hasInval());

    // We don't care about matrix reval results, but we do care whether it was invalidated.
    const auto localDamage = fMatrix->hasInval() ? Damage::kForceSelf : Damage::kDefault;
    fMatrix->revalidate(ic, ctm);

    const auto& m = fMatrix->getTotalMatrix();
    auto result = this->INHERITED::onRevalidate(ic, SkMatrix::Concat(ctm, m));
    m.mapRect(&result.fBounds);
    result.fDamage = localDamage;

    return result;
}

} // namespace sksg
