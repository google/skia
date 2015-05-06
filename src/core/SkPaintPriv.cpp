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

bool isPaintOpaque(const SkPaint* paint, SkPaintBitmapOpacity contentType) {
    if (!paint) {
        return contentType != kUnknown_SkPaintBitmapOpacity;
    }
    SkXfermode::SrcColorOpacity opacityType = SkXfermode::kUnknown_SrcColorOpacity;

    if (!paint->getColorFilter() ||
        ((paint->getColorFilter()->getFlags() &
          SkColorFilter::kAlphaUnchanged_Flag) != 0)) {
        if (0xff == paint->getAlpha() &&
            contentType != kUnknown_SkPaintBitmapOpacity &&
            (!paint->getShader() || paint->getShader()->isOpaque())) {
            opacityType = SkXfermode::kOpaque_SrcColorOpacity;
        } else if (0 == paint->getColor() &&
                   contentType == kNoBitmap_SkPaintBitmapOpacity &&
                   !paint->getShader()) {
            opacityType = SkXfermode::kTransparentBlack_SrcColorOpacity;
        } else if (0 == paint->getAlpha()) {
            opacityType = SkXfermode::kTransparentAlpha_SrcColorOpacity;
        }
    }

    return SkXfermode::IsOpaque(paint->getXfermode(), opacityType);
}

bool isPaintOpaque(const SkPaint* paint, const SkBitmap* bmpReplacesShader) {
    SkPaintBitmapOpacity contentType;

    if(!bmpReplacesShader)
        contentType = kNoBitmap_SkPaintBitmapOpacity;
    else if(bmpReplacesShader->isOpaque())
        contentType = kOpaque_SkPaintBitmapOpacity;
    else
        contentType = kUnknown_SkPaintBitmapOpacity;

    return isPaintOpaque(paint, contentType);
}

bool isPaintOpaque(const SkPaint* paint, const SkImage* image) {
    return isPaintOpaque(paint, image->isOpaque() ?
                         kOpaque_SkPaintBitmapOpacity : kUnknown_SkPaintBitmapOpacity);
}
