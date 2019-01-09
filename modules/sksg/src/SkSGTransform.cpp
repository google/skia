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

// Always compose in 4x4 for now.
class Concat final : public Transform {
public:
    Concat(sk_sp<Transform> a, sk_sp<Transform> b)
        : fA(std::move(a)), fB(std::move(b)) {
        SkASSERT(fA);
        SkASSERT(fB);

        this->observeInval(fA);
        this->observeInval(fB);
    }

    ~Concat() override {
        this->unobserveInval(fA);
        this->unobserveInval(fB);
    }

    SkMatrix asMatrix() const override {
        return fComposed;
    }

    SkMatrix44 asMatrix44() const override {
        return fComposed;
    }

protected:
    SkRect onRevalidate(InvalidationController* ic, const SkMatrix& ctm) override {
        fA->revalidate(ic, ctm);
        fB->revalidate(ic, ctm);

        fComposed.setConcat(fA->asMatrix44(), fB->asMatrix44());
        return SkRect::MakeEmpty();
    }

private:
    const sk_sp<Transform> fA, fB;
    SkMatrix44             fComposed;

    using INHERITED = Transform;
};

} // namespace

// Transform nodes don't generate damage on their own, but via ancestor TransformEffects.
Transform::Transform() : INHERITED(kBubbleDamage_Trait) {}

sk_sp<Transform> Transform::MakeConcat(sk_sp<Transform> a, sk_sp<Transform> b) {
    if (!a) {
        return b;
    }

    return b ? sk_make_sp<Concat>(std::move(a), std::move(b))
             : a;
}

sk_sp<Matrix> Matrix::Make(const SkMatrix& m) {
    return sk_sp<Matrix>(new Matrix(m));
}

Matrix::Matrix(const SkMatrix& m) : fMatrix(m) {}

SkMatrix Matrix::asMatrix() const {
    return fMatrix;
}

SkMatrix44 Matrix::asMatrix44() const {
    return fMatrix;
}

SkRect Matrix::onRevalidate(InvalidationController*, const SkMatrix&) {
    return SkRect::MakeEmpty();
}

sk_sp<Matrix44> Matrix44::Make(const SkMatrix44& m) {
    return sk_sp<Matrix44>(new Matrix44(m));
}

Matrix44::Matrix44(const SkMatrix44& m) : fMatrix(m) {}

SkMatrix Matrix44::asMatrix() const {
    return fMatrix;
}

SkMatrix44 Matrix44::asMatrix44() const {
    return fMatrix;
}

SkRect Matrix44::onRevalidate(InvalidationController*, const SkMatrix&) {
    return SkRect::MakeEmpty();
}

TransformEffect::TransformEffect(sk_sp<RenderNode> child, sk_sp<Transform> transform)
    : INHERITED(std::move(child))
    , fTransform(std::move(transform)) {
    this->observeInval(fTransform);
}

TransformEffect::~TransformEffect() {
    this->unobserveInval(fTransform);
}

void TransformEffect::onRender(SkCanvas* canvas, const RenderContext* ctx) const {
    const auto m = fTransform->asMatrix();
    SkAutoCanvasRestore acr(canvas, !m.isIdentity());
    canvas->concat(m);
    this->INHERITED::onRender(canvas, ctx);
}

SkRect TransformEffect::onRevalidate(InvalidationController* ic, const SkMatrix& ctm) {
    SkASSERT(this->hasInval());

    // We don't care about matrix reval results.
    fTransform->revalidate(ic, ctm);

    const auto m = fTransform->asMatrix();
    auto bounds = this->INHERITED::onRevalidate(ic, SkMatrix::Concat(ctm, m));
    m.mapRect(&bounds);

    return bounds;
}

} // namespace sksg
