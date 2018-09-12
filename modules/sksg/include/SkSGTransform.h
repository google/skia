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

namespace sksg {

/**
 * Concrete node, wrapping an SkMatrix.
 */
class Matrix : public Node {
public:
    static sk_sp<Matrix> Make(const SkMatrix& m) {
        return sk_sp<Matrix>(new Matrix(m));
    }

    SG_ATTRIBUTE(Matrix, SkMatrix, fMatrix)

    virtual SkMatrix getTotalMatrix() const;

protected:
    explicit Matrix(const SkMatrix&);

    SkRect onRevalidate(InvalidationController*, const SkMatrix&) override;

    SkMatrix fMatrix;

private:
    typedef Node INHERITED;
};

/**
 * Same as Matrix, but with an optional parent matrix (to support chaining):
 *
 *     M' = parent x M
 */
class ComposedMatrix final : public Matrix {
public:
    static sk_sp<Matrix> Make(const SkMatrix& m, sk_sp<Matrix> parent) {
        return parent ? sk_sp<Matrix>(new ComposedMatrix(m, std::move(parent)))
                      : Matrix::Make(m);
    }

    ~ComposedMatrix() override;

    SkMatrix getTotalMatrix() const override;

protected:
    SkRect onRevalidate(InvalidationController*, const SkMatrix&) override;

private:
    ComposedMatrix(const SkMatrix& m, sk_sp<Matrix> parent);

    const sk_sp<Matrix> fParent;

    using INHERITED = Matrix;
};

/**
 * Concrete Effect node, binding a Matrix to a RenderNode.
 */
class Transform final : public EffectNode {
public:
    static sk_sp<Transform> Make(sk_sp<RenderNode> child, sk_sp<Matrix> matrix) {
        return child && matrix
            ? sk_sp<Transform>(new Transform(std::move(child), std::move(matrix)))
            : nullptr;
    }

    static sk_sp<Transform> Make(sk_sp<RenderNode> child, const SkMatrix& m) {
        return Make(std::move(child), Matrix::Make(m));
    }

    ~Transform() override;

    const sk_sp<Matrix>& getMatrix() const { return fMatrix; }

protected:
    void onRender(SkCanvas*, const RenderContext*) const override;

    SkRect onRevalidate(InvalidationController*, const SkMatrix&) override;

private:
    Transform(sk_sp<RenderNode>, sk_sp<Matrix>);

    const sk_sp<Matrix> fMatrix;

    typedef EffectNode INHERITED;
};

} // namespace sksg

#endif // SkSGTransform_DEFINED
