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

protected:
    explicit Matrix(const SkMatrix&);

    SkRect onRevalidate(InvalidationController*, const SkMatrix&) override;

private:
    const virtual SkMatrix& totalMatrix() const { return fMatrix; }
    friend class ComposedMatrix;
    friend class Transform;

    SkMatrix fMatrix;

    typedef Node INHERITED;
};

/**
 * Concrete node, concatenating and existing Matrix with an SkMatrix.
 */
class ComposedMatrix final : public Matrix {
public:
    static sk_sp<ComposedMatrix> Make(sk_sp<Matrix> preMatrix, const SkMatrix& m) {
        return sk_sp<ComposedMatrix>(new ComposedMatrix(std::move(preMatrix), m));
    }

    ~ComposedMatrix() override;

protected:
    SkRect onRevalidate(InvalidationController*, const SkMatrix&) override;

    const SkMatrix& totalMatrix() const override { return fTotalMatrix; }

private:
    ComposedMatrix(sk_sp<Matrix>, const SkMatrix&);

    sk_sp<Matrix> fPreMatrix;
    SkMatrix      fTotalMatrix;

    typedef Matrix INHERITED;
};

/**
 * Concrete Effect node, binding a Matrix to a RenderNode.
 */
class Transform : public EffectNode {
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
    Transform(sk_sp<RenderNode>, sk_sp<Matrix>);

    void onRender(SkCanvas*) const override;

    SkRect onRevalidate(InvalidationController*, const SkMatrix&) override;

private:
    sk_sp<Matrix> fMatrix;

    typedef EffectNode INHERITED;
};

} // namespace sksg

#endif // SkSGTransform_DEFINED
