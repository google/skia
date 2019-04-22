/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/sksg/include/SkSGTransform.h"

#include "include/core/SkCanvas.h"
#include "modules/sksg/src/SkSGTransformPriv.h"

namespace sksg {

namespace {

template <typename T>
class Concat final : public Transform {
public:
    template <typename = std::enable_if<std::is_same<T, SkMatrix  >::value ||
                                        std::is_same<T, SkMatrix44>::value >>
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

protected:
    SkRect onRevalidate(InvalidationController* ic, const SkMatrix& ctm) override {
        fA->revalidate(ic, ctm);
        fB->revalidate(ic, ctm);

        fComposed.setConcat(TransformPriv::As<T>(fA),
                            TransformPriv::As<T>(fB));
        return SkRect::MakeEmpty();
    }

    bool is44() const override { return std::is_same<T, SkMatrix44>::value; }

    SkMatrix asMatrix() const override {
        return fComposed;
    }

    SkMatrix44 asMatrix44() const override {
        return fComposed;
    }

private:
    const sk_sp<Transform> fA, fB;
    T                      fComposed;

    using INHERITED = Transform;
};

} // namespace

// Transform nodes don't generate damage on their own, but via ancestor TransformEffects.
Transform::Transform() : INHERITED(kBubbleDamage_Trait) {}

sk_sp<Transform> Transform::MakeConcat(sk_sp<Transform> a, sk_sp<Transform> b) {
    if (!a) {
        return b;
    }

    if (!b) {
        return a;
    }

    return TransformPriv::Is44(a) || TransformPriv::Is44(b)
        ? sk_sp<Transform>(new Concat<SkMatrix44>(std::move(a), std::move(b)))
        : sk_sp<Transform>(new Concat<SkMatrix  >(std::move(a), std::move(b)));
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
    const auto m = TransformPriv::As<SkMatrix>(fTransform);
    SkAutoCanvasRestore acr(canvas, !m.isIdentity());
    canvas->concat(m);
    this->INHERITED::onRender(canvas, ctx);
}

const RenderNode* TransformEffect::onNodeAt(const SkPoint& p) const {
    const auto m = TransformPriv::As<SkMatrix>(fTransform);

    SkPoint mapped_p;
    m.mapPoints(&mapped_p, &p, 1);

    return this->INHERITED::onNodeAt(mapped_p);
}

SkRect TransformEffect::onRevalidate(InvalidationController* ic, const SkMatrix& ctm) {
    SkASSERT(this->hasInval());

    // We don't care about matrix reval results.
    fTransform->revalidate(ic, ctm);

    const auto m = TransformPriv::As<SkMatrix>(fTransform);
    auto bounds = this->INHERITED::onRevalidate(ic, SkMatrix::Concat(ctm, m));
    m.mapRect(&bounds);

    return bounds;
}

} // namespace sksg
