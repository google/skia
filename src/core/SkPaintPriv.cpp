/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPaintPriv.h"

#include "SkBitmap.h"
#include "SkColorFilter.h"
#include "SkImage.h"
#include "SkPaint.h"
#include "SkShader.h"

enum ShaderOverrideOpacity {
    kNone_ShaderOverrideOpacity,        //!< there is no overriding shader (bitmap or image)
    kOpaque_ShaderOverrideOpacity,      //!< the overriding shader is opaque
    kNotOpaque_ShaderOverrideOpacity,   //!< the overriding shader may not be opaque
};

static bool changes_alpha(const SkPaint& paint) {
    SkColorFilter* cf = paint.getColorFilter();
    return cf && !(cf->getFlags() & SkColorFilter::kAlphaUnchanged_Flag);
}

static bool overwrites(const SkPaint* paint, ShaderOverrideOpacity overrideOpacity) {
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

    return SkXfermode::IsOpaque(paint->getXfermode(), opacityType);
}

bool SkPaintPriv::Overwrites(const SkPaint& paint) {
    return overwrites(&paint, kNone_ShaderOverrideOpacity);
}

bool SkPaintPriv::Overwrites(const SkBitmap& bitmap, const SkPaint* paint) {
    return overwrites(paint, bitmap.isOpaque() ? kOpaque_ShaderOverrideOpacity
                                               : kNotOpaque_ShaderOverrideOpacity);
}

bool SkPaintPriv::Overwrites(const SkImage* image, const SkPaint* paint) {
    return overwrites(paint, image->isOpaque() ? kOpaque_ShaderOverrideOpacity
                                               : kNotOpaque_ShaderOverrideOpacity);
}
