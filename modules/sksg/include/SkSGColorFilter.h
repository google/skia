/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSGColorFilter_DEFINED
#define SkSGColorFilter_DEFINED

#include "include/core/SkColorFilter.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "modules/sksg/include/SkSGEffectNode.h"
#include "modules/sksg/include/SkSGNode.h"

#include <vector>

class SkCanvas;
class SkMatrix;
enum class SkBlendMode;
struct SkPoint;

namespace sksg {

class Color;
class InvalidationController;
class RenderNode;

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

    using INHERITED = EffectNode;
};

/**
 * Wrapper for externally-managed SkColorFilters.
 *
 * Allows attaching non-sksg color filters to the render tree.
 */
class ExternalColorFilter final : public EffectNode {
public:
    static sk_sp<ExternalColorFilter> Make(sk_sp<RenderNode> child);

    ~ExternalColorFilter() override;

    SG_ATTRIBUTE(ColorFilter, sk_sp<SkColorFilter>, fColorFilter)

protected:
    void onRender(SkCanvas*, const RenderContext*) const override;

private:
    explicit ExternalColorFilter(sk_sp<RenderNode>);

    sk_sp<SkColorFilter> fColorFilter;

    using INHERITED = EffectNode;
};

/**
 * Concrete SkBlendModeColorFilter Effect node.
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

    using INHERITED = ColorFilter;
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

} // namespace sksg

#endif // SkSGColorFilter_DEFINED
