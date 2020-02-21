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
SkMatrix AsSkMatrix(const T&);

template <>
SkMatrix AsSkMatrix<SkMatrix>(const SkMatrix& m) { return m; }

template <>
SkMatrix AsSkMatrix<SkM44>(const SkM44& m) { return m.asM33(); }

template <typename T>
SkM44 AsSkM44(const T&);

template <>
SkM44 AsSkM44<SkMatrix>(const SkMatrix& m) { return SkM44(m); }

template <>
SkM44 AsSkM44<SkM44>(const SkM44& m) { return m; }

template <typename T>
class Concat final : public Transform {
public:
    template <typename = std::enable_if<std::is_same<T, SkMatrix>::value ||
                                        std::is_same<T, SkM44   >::value >>
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

    bool is44() const override { return std::is_same<T, SkM44>::value; }

    SkMatrix asMatrix() const override {
        SkASSERT(!this->hasInval());
        return AsSkMatrix(fComposed);
    }

    SkM44 asM44() const override {
        SkASSERT(!this->hasInval());
        return AsSkM44(fComposed);
    }

private:
    const sk_sp<Transform> fA, fB;
    T                      fComposed;

    using INHERITED = Transform;
};

template <typename T>
class Inverse final : public Transform {
public:
    template <typename = std::enable_if<std::is_same<T, SkMatrix>::value ||
                                        std::is_same<T, SkM44   >::value >>
    explicit Inverse(sk_sp<Transform> t)
        : fT(std::move(t)) {
        SkASSERT(fT);

        this->observeInval(fT);
    }

    ~Inverse() override {
        this->unobserveInval(fT);
    }

protected:
    SkRect onRevalidate(InvalidationController* ic, const SkMatrix& ctm) override {
        fT->revalidate(ic, ctm);

        if (!TransformPriv::As<T>(fT).invert(&fInverted)) {
            fInverted.setIdentity();
        }

        return SkRect::MakeEmpty();
    }

    bool is44() const override { return std::is_same<T, SkM44>::value; }

    SkMatrix asMatrix() const override {
        SkASSERT(!this->hasInval());
        return AsSkMatrix(fInverted);
    }

    SkM44 asM44() const override {
        SkASSERT(!this->hasInval());
        return AsSkM44(fInverted);
    }

private:
    const sk_sp<Transform> fT;
    T                      fInverted;

    using INHERITED = Transform;
};

} // namespace

template <>
SkMatrix Matrix<SkMatrix>::asMatrix() const { return fMatrix; }

template <>
SkM44 Matrix<SkMatrix>::asM44() const { return SkM44(fMatrix); }

template <>
SkMatrix Matrix<SkM44>::asMatrix() const { return fMatrix.asM33(); }

template <>
SkM44 Matrix<SkM44>::asM44() const { return fMatrix; }

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
        ? sk_sp<Transform>(new Concat<SkM44   >(std::move(a), std::move(b)))
        : sk_sp<Transform>(new Concat<SkMatrix>(std::move(a), std::move(b)));
}

sk_sp<Transform> Transform::MakeInverse(sk_sp<Transform> t) {
    if (!t) {
        return nullptr;
    }

    return TransformPriv::Is44(t)
        ? sk_sp<Transform>(new Inverse<SkM44   >(std::move(t)))
        : sk_sp<Transform>(new Inverse<SkMatrix>(std::move(t)));
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
    SkAutoCanvasRestore acr(canvas, true);
    canvas->concat44(TransformPriv::As<SkM44>(fTransform));

    this->INHERITED::onRender(canvas, ctx);
}

const RenderNode* TransformEffect::onNodeAt(const SkPoint& p) const {
    const auto p4 = TransformPriv::As<SkM44>(fTransform).map(p.fX, p.fY, 0, 0);

    return this->INHERITED::onNodeAt({p4.x, p4.y});
}

SkRect TransformEffect::onRevalidate(InvalidationController* ic, const SkMatrix& ctm) {
    SkASSERT(this->hasInval());

    // We don't care about matrix reval results.
    fTransform->revalidate(ic, ctm);

    // TODO: need to update all the reval plumbing for m44.
    const auto m = TransformPriv::As<SkMatrix>(fTransform);
    auto bounds = this->INHERITED::onRevalidate(ic, SkMatrix::Concat(ctm, m));
    m.mapRect(&bounds);

    return bounds;
}

} // namespace sksg
