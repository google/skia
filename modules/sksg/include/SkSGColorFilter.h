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

#include <vector>

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
 * Tint/multi-tone color effect: maps RGB colors to the [C0,C1][C1,C2]..[Cn-1,Cn] gradient
 * based on input luminance (where the colors are evenly distributed across the luminance domain),
 * then mixes with the input based on weight.  Leaves alpha unchanged.
 */
class GradientColorFilter final : public ColorFilter {
public:
    ~GradientColorFilter() override;

    static sk_sp<GradientColorFilter> Make(sk_sp<RenderNode> child,
                                           sk_sp<Color> c0, sk_sp<Color> c1);
    static sk_sp<GradientColorFilter> Make(sk_sp<RenderNode> child,
                                           std::vector<sk_sp<Color>>);

    SG_ATTRIBUTE(Weight, float, fWeight)

protected:
    sk_sp<SkColorFilter> onRevalidateFilter() override;

private:
    GradientColorFilter(sk_sp<RenderNode>, std::vector<sk_sp<Color>>);

    const std::vector<sk_sp<Color>> fColors;

    float                           fWeight = 0;

    using INHERITED = ColorFilter;
};


class LevelsColorFilter final : public ColorFilter {
public:
    ~LevelsColorFilter() override;

    static sk_sp<LevelsColorFilter> Make(sk_sp<RenderNode> child);

    enum : uint32_t {
        // These match the SkColor masks.
        kA_Channel = 0xffu << 24,
        kR_Channel = 0xffu << 16,
        kG_Channel = 0xffu <<  8,
        kB_Channel = 0xffu <<  0,
    };

    SG_ATTRIBUTE(Channels, uint32_t, fChannels)
    SG_ATTRIBUTE(InBlack ,    float, fInBlack )
    SG_ATTRIBUTE(InWhite ,    float, fInWhite )
    SG_ATTRIBUTE(OutBlack,    float, fOutBlack)
    SG_ATTRIBUTE(OutWhite,    float, fOutWhite)
    SG_ATTRIBUTE(Gamma   ,    float, fGamma   )

protected:
    sk_sp<SkColorFilter> onRevalidateFilter() override;

private:
    explicit LevelsColorFilter(sk_sp<RenderNode>);

    uint32_t fChannels = kA_Channel | kR_Channel | kG_Channel | kB_Channel;
    float    fInBlack  = 0,
             fInWhite  = 0,
             fOutBlack = 0,
             fOutWhite = 0,
             fGamma    = 1;

    using INHERITED = ColorFilter;
};

} // namespace sksg

#endif // SkSGColorFilter_DEFINED
