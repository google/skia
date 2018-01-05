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
 * Concrete Effect node, wrapping an SkMatrix.
 */
class Transform : public EffectNode {
public:
    static sk_sp<Transform> Make(sk_sp<RenderNode> child, const SkMatrix& matrix) {
        return sk_sp<Transform>(new Transform(std::move(child), matrix));
    }

    SG_ATTRIBUTE(Matrix, SkMatrix, fMatrix)

protected:
    Transform(sk_sp<RenderNode>, const SkMatrix&);

    void onRender(SkCanvas*) const override;

    RevalidationResult onRevalidate(InvalidationController*, const SkMatrix&) override;

private:
    SkMatrix fMatrix;

    typedef EffectNode INHERITED;
};

} // namespace sksg

#endif // SkSGTransform_DEFINED
