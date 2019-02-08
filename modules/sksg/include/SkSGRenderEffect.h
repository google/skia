/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSGRenderEffect_DEFINED
#define SkSGRenderEffect_DEFINED

#include "SkSGEffectNode.h"

// TODO: merge EffectNode.h with this header

class SkImageFilter;

namespace sksg {

class Color;

class DropShadowEffect final : public EffectNode {
public:
    enum class Mode { kShadowAndForeground, kShadowOnly };

    static sk_sp<DropShadowEffect> Make(sk_sp<RenderNode> child, sk_sp<Color>);

    ~DropShadowEffect() override;

    SG_ATTRIBUTE(Offset, SkVector, fOffset)
    SG_ATTRIBUTE(Sigma , SkVector, fSigma )
    SG_ATTRIBUTE(Mode  , Mode    , fMode  )

protected:
    void onRender(SkCanvas*, const RenderContext*) const override;

    SkRect onRevalidate(InvalidationController*, const SkMatrix&) override;

private:
    DropShadowEffect(sk_sp<RenderNode>, sk_sp<Color>);

    const sk_sp<Color>   fColor;

    sk_sp<SkImageFilter> fFilter;

    SkVector             fOffset = { 0, 0 },
                         fSigma  = { 0, 0 };
    Mode                 fMode   = Mode::kShadowAndForeground;

    using INHERITED = EffectNode;
};

} // namespace sksg

#endif // SkSGRenderEffect_DEFINED
