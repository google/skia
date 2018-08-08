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
 *
 */
class ColorFilter : public EffectNode {
protected:
    explicit ColorFilter(sk_sp<RenderNode>);

    void onRender(SkCanvas*, const RenderContext*) const final;

    sk_sp<SkColorFilter> fColorFilter;

private:
    typedef EffectNode INHERITED;
};

/**
 * Concrete SkModeColorFilter Effect node.
 *
 */
class ColorModeFilter final : public ColorFilter {
public:
    ~ColorModeFilter() override;

    static sk_sp<ColorModeFilter> Make(sk_sp<RenderNode> child, sk_sp<Color> color,
                                       SkBlendMode mode) {
        return (child && color)
            ? sk_sp<ColorModeFilter>(new ColorModeFilter(std::move(child), std::move(color), mode))
            : nullptr;
    }

    SG_ATTRIBUTE(Mode , SkBlendMode, fMode )

protected:
    SkRect onRevalidate(InvalidationController*, const SkMatrix&) override;

private:
    ColorModeFilter(sk_sp<RenderNode>, sk_sp<Color>, SkBlendMode);

    sk_sp<Color> fColor;
    SkBlendMode  fMode;

    typedef ColorFilter INHERITED;
};

} // namespace sksg

#endif // SkSGColorFilter_DEFINED
