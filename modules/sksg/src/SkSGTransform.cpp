/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSGTransform.h"

#include "SkCanvas.h"

namespace sksg {
namespace {

class ComposedMatrix final : public Matrix {
public:
    ComposedMatrix(const SkMatrix& m, sk_sp<Matrix> parent)
        : INHERITED(m)
        , fParent(std::move(parent)) {
        SkASSERT(fParent);
        this->observeInval(fParent);
    }

    ~ComposedMatrix() override {
        this->unobserveInval(fParent);
    }

    const SkMatrix& getTotalMatrix() const override {
        SkASSERT(!this->hasInval());
        return fTotalMatrix;
    }

protected:
    SkRect onRevalidate(InvalidationController* ic, const SkMatrix& ctm) override {
        fParent->revalidate(ic, ctm);
        fTotalMatrix = SkMatrix::Concat(fParent->getTotalMatrix(), this->getMatrix());
        return SkRect::MakeEmpty();
    }

private:
    const sk_sp<Matrix> fParent;
    SkMatrix            fTotalMatrix; // cached during revalidation.

    using INHERITED = Matrix;
};

} // namespace

sk_sp<Matrix> Matrix::Make(const SkMatrix& m, sk_sp<Matrix> parent) {
    return sk_sp<Matrix>(parent ? new ComposedMatrix(m, std::move(parent))
                                : new Matrix(m));
}

// Matrix nodes don't generate damage on their own, but via aggregation ancestor Transform nodes.
Matrix::Matrix(const SkMatrix& m)
    : INHERITED(kBubbleDamage_Trait)
    , fMatrix(m) {}

const SkMatrix& Matrix::getTotalMatrix() const {
    return fMatrix;
}

SkRect Matrix::onRevalidate(InvalidationController*, const SkMatrix&) {
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
