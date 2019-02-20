/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSGColorFilter_DEFINED
#define SkSGColorFilter_DEFINED

#include "SkSGEffectNode.h"

#include "SkBlendMode.h"

class SkColorFilter;

namespace sksg {

class Color;

/**
 * Base class for nodes which apply a color filter when rendering their descendants.
 */
class ColorFilter : public EffectNode {
protected:
    explicit ColorFilter(sk_sp<RenderNode>);

    void onRender(SkCanvas*, const RenderContext*) const final;
    const RenderNode* onNodeAt(const SkPoint&)     const final;

    SkRect onRevalidate(InvalidationController*, const SkMatrix&) final;

    virtual sk_sp<SkColorFilter> onRevalidateFilter() = 0;

private:
    sk_sp<SkColorFilter> fColorFilter;

    typedef EffectNode INHERITED;
};

/**
 * Concrete SkModeColorFilter Effect node.
 */
class ModeColorFilter final : public ColorFilter {
public:
    ~ModeColorFilter() override;

    static sk_sp<ModeColorFilter> Make(sk_sp<RenderNode> child,
                                       sk_sp<Color> color,
                                       SkBlendMode mode);

protected:
    sk_sp<SkColorFilter> onRevalidateFilter() override;

private:
    ModeColorFilter(sk_sp<RenderNode>, sk_sp<Color>, SkBlendMode);

    const sk_sp<Color> fColor;
    const SkBlendMode  fMode;

    typedef ColorFilter INHERITED;
};

/**
 * Tint color effect: maps RGB colors to the [c0,c1] gradient based on input luminance
 * (while leaving the alpha channel unchanged), then mixes with the input based on weight.
 */

class TintColorFilter final : public ColorFilter {
public:
    ~TintColorFilter() override;

    static sk_sp<TintColorFilter> Make(sk_sp<RenderNode> child,
                                       sk_sp<Color> color0,
                                       sk_sp<Color> color1);

    SG_ATTRIBUTE(Weight, float, fWeight)

protected:
    sk_sp<SkColorFilter> onRevalidateFilter() override;

private:
    TintColorFilter(sk_sp<RenderNode>, sk_sp<Color>, sk_sp<Color>);

    const sk_sp<Color> fColor0,
                       fColor1;

    float              fWeight = 0;

    typedef ColorFilter INHERITED;
};

} // namespace sksg

#endif // SkSGColorFilter_DEFINED
