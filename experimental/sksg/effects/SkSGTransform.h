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
 * Concrete node, wrapping an SkMatrix, with an optional parent Matrix (to allow chaining):
 *
 *    M' = parent x M
 */
class Matrix : public Node {
public:
    static sk_sp<Matrix> Make(const SkMatrix& m, sk_sp<Matrix> parent = nullptr) {
        return sk_sp<Matrix>(new Matrix(m, std::move(parent)));
    }

    ~Matrix() override;

    SG_ATTRIBUTE(Matrix, SkMatrix, fLocalMatrix)

    const SkMatrix& getTotalMatrix() const { return fTotalMatrix; }

protected:
    Matrix(const SkMatrix&, sk_sp<Matrix>);

    SkRect onRevalidate(InvalidationController*, const SkMatrix&) override;

private:
    sk_sp<Matrix> fParent;
    SkMatrix      fLocalMatrix,
                  fTotalMatrix; // cached during revalidation

    typedef Node INHERITED;
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
