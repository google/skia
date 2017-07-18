/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkColorFilter.h"
#include "SkPaintPriv.h"
#include "SkImage.h"
#include "SkPaint.h"
#include "SkShaderBase.h"
#include "SkXfermodePriv.h"

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

bool SkPaintPriv::Overwrites(const SkBitmap& bitmap, const SkPaint* paint) {
    return Overwrites(paint, bitmap.isOpaque() ? kOpaque_ShaderOverrideOpacity
                                               : kNotOpaque_ShaderOverrideOpacity);
}

bool SkPaintPriv::Overwrites(const SkImage* image, const SkPaint* paint) {
    return Overwrites(paint, image->isOpaque() ? kOpaque_ShaderOverrideOpacity
                                               : kNotOpaque_ShaderOverrideOpacity);
}

void SkPaintPriv::ScaleFontMetrics(SkPaint::FontMetrics* metrics, SkScalar scale) {
    metrics->fTop *= scale;
    metrics->fAscent *= scale;
    metrics->fDescent *= scale;
    metrics->fBottom *= scale;
    metrics->fLeading *= scale;
    metrics->fAvgCharWidth *= scale;
    metrics->fXMin *= scale;
    metrics->fXMax *= scale;
    metrics->fXHeight *= scale;
    metrics->fUnderlineThickness *= scale;
    metrics->fUnderlinePosition *= scale;
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
