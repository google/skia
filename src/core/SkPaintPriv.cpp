/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColorFilter.h"
#include "include/core/SkPaint.h"
#include "src/core/SkPaintPriv.h"
#include "src/core/SkXfermodePriv.h"
#include "src/shaders/SkShaderBase.h"

static bool changes_alpha(const SkPaint& paint) {
    SkColorFilter* cf = paint.getColorFilter();
    return cf && !(cf->getFlags() & SkColorFilter::kAlphaUnchanged_Flag);
}

bool SkPaintPriv::Overwrites(const SkPaint* paint, ShaderOverrideOpacity overrideOpacity) {
    if (!paint) {
        // No paint means we default to SRC_OVER, so we overwrite iff our shader-override
        // is opaque, or we don't have one.
        return overrideOpacity != kNotOpaque_ShaderOverrideOpacity;
    }

    SkXfermode::SrcColorOpacity opacityType = SkXfermode::kUnknown_SrcColorOpacity;

    if (!changes_alpha(*paint)) {
        const unsigned paintAlpha = paint->getAlpha();
        if (0xff == paintAlpha && overrideOpacity != kNotOpaque_ShaderOverrideOpacity &&
            (!paint->getShader() || paint->getShader()->isOpaque()))
        {
            opacityType = SkXfermode::kOpaque_SrcColorOpacity;
        } else if (0 == paintAlpha) {
            if (overrideOpacity == kNone_ShaderOverrideOpacity && !paint->getShader()) {
                opacityType = SkXfermode::kTransparentBlack_SrcColorOpacity;
            } else {
                opacityType = SkXfermode::kTransparentAlpha_SrcColorOpacity;
            }
        }
    }

    return SkXfermode::IsOpaque(paint->getBlendMode(), opacityType);
}

bool SkPaintPriv::ShouldDither(const SkPaint& p, SkColorType dstCT) {
    // The paint dither flag can veto.
    if (!p.isDither()) {
        return false;
    }

    // We always dither 565 or 4444 when requested.
    if (dstCT == kRGB_565_SkColorType || dstCT == kARGB_4444_SkColorType) {
        return true;
    }

    // Otherwise, dither is only needed for non-const paints.
    return p.getImageFilter() || p.getMaskFilter()
        || !p.getShader() || !as_SB(p.getShader())->isConstant();
}

// return true if the paint is just a single color (i.e. not a shader). If its
// a shader, then we can't compute a const luminance for it :(
static bool just_a_color(const SkPaint& paint, SkColor* color) {
    SkColor c = paint.getColor();

    const auto* shader = as_SB(paint.getShader());
    if (shader && !shader->asLuminanceColor(&c)) {
        return false;
    }
    if (paint.getColorFilter()) {
        c = paint.getColorFilter()->filterColor(c);
    }
    if (color) {
        *color = c;
    }
    return true;
}

SkColor SkPaintPriv::ComputeLuminanceColor(const SkPaint& paint) {
    SkColor c;
    if (!just_a_color(paint, &c)) {
        c = SkColorSetRGB(0x7F, 0x80, 0x7F);
    }
    return c;
}
