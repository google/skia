/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkanOpacityEffect_DEFINED
#define SkanOpacityEffect_DEFINED

#include "SkanEffectNode.h"

namespace skan {

/**
 * Concrete Effect node, applying opacity to its descendants.
 *
 */
class OpacityEffect final : public EffectNode {
public:
    static sk_sp<OpacityEffect> Make(sk_sp<RenderNode> child, float opacity = 1) {
        return child ? sk_sp<OpacityEffect>(new OpacityEffect(std::move(child), opacity)) : nullptr;
    }

    SG_ATTRIBUTE(Opacity, float, fOpacity)

protected:
    OpacityEffect(sk_sp<RenderNode>, float);

    void onRender(SkCanvas*) const override;

    SkRect onRevalidate(InvalidationController*, const SkMatrix&) override;

private:
    float fOpacity;

    typedef EffectNode INHERITED;
};

} // namespace skan

#endif // SkanOpacityEffect_DEFINED
