/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSGMaskEffect_DEFINED
#define SkSGMaskEffect_DEFINED

#include "SkSGEffectNode.h"

namespace sksg {

/**
 * Concrete Effect node, applying a mask to its descendants.
 *
 */
class MaskEffect final : public EffectNode {
public:
    static sk_sp<MaskEffect> Make(sk_sp<RenderNode> child, sk_sp<RenderNode> mask) {
        return (child && mask)
            ? sk_sp<MaskEffect>(new MaskEffect(std::move(child), std::move(mask)))
            : nullptr;
    }

    ~MaskEffect() override;

protected:
    MaskEffect(sk_sp<RenderNode>, sk_sp<RenderNode> mask);

    void onRender(SkCanvas*) const override;

    SkRect onRevalidate(InvalidationController*, const SkMatrix&) override;

private:
    sk_sp<RenderNode> fMaskNode;

    typedef EffectNode INHERITED;
};

} // namespace sksg

#endif // SkSGMaskEffect_DEFINED
