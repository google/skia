/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSGTransform_DEFINED
#define SkSGTransform_DEFINED

#include "SkSGEffectNode.h"

#include "SkMatrix.h"
#include "SkMatrix44.h"

namespace sksg {

/**
 * Transformations base class.
 */
class Transform : public Node {
public:
    // Compose T = A x B
    static sk_sp<Transform> MakeConcat(sk_sp<Transform> a, sk_sp<Transform> b);

protected:
    Transform();

    virtual SkMatrix   asMatrix  () const = 0;
    virtual SkMatrix44 asMatrix44() const = 0;

private:
    friend class TransformPriv;

    using INHERITED = Node;
};

/**
 * Concrete, SkMatrix-backed transformation.
 */
class Matrix final : public Transform {
public:
    static sk_sp<Matrix> Make(const SkMatrix& m);

    SG_ATTRIBUTE(Matrix, SkMatrix, fMatrix)

protected:
    explicit Matrix(const SkMatrix&);

    SkRect onRevalidate(InvalidationController*, const SkMatrix&) override;

    SkMatrix   asMatrix  () const override;
    SkMatrix44 asMatrix44() const override;

private:
    SkMatrix fMatrix;

    using INHERITED = Transform;
};

/**
 * Concrete, SkMatrix44-backed transformation.
 */
class Matrix44 final : public Transform {
public:
    static sk_sp<Matrix44> Make(const SkMatrix44& m);

    SG_ATTRIBUTE(Matrix, SkMatrix44, fMatrix)

protected:
    explicit Matrix44(const SkMatrix44&);

    SkRect onRevalidate(InvalidationController*, const SkMatrix&) override;

    SkMatrix   asMatrix  () const override;
    SkMatrix44 asMatrix44() const override;

private:
    SkMatrix44 fMatrix;

    using INHERITED = Transform;
};

/**
 * Concrete Effect node, binding a Transform to a RenderNode.
 */
class TransformEffect final : public EffectNode {
public:
    static sk_sp<TransformEffect> Make(sk_sp<RenderNode> child, sk_sp<Transform> transform) {
        return child && transform
            ? sk_sp<TransformEffect>(new TransformEffect(std::move(child), std::move(transform)))
            : nullptr;
    }

    static sk_sp<TransformEffect> Make(sk_sp<RenderNode> child, const SkMatrix& m) {
        return Make(std::move(child), Matrix::Make(m));
    }

    ~TransformEffect() override;

    const sk_sp<Transform>& getTransform() const { return fTransform; }

protected:
    void onRender(SkCanvas*, const RenderContext*) const override;

    SkRect onRevalidate(InvalidationController*, const SkMatrix&) override;

private:
    TransformEffect(sk_sp<RenderNode>, sk_sp<Transform>);

    const sk_sp<Transform> fTransform;

    typedef EffectNode INHERITED;
};

} // namespace sksg

#endif // SkSGTransform_DEFINED
