/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPaintPriv.h"

#include "SkBitmap.h"
#include "SkColorFilter.h"
#include "SkPaint.h"
#include "SkShader.h"

bool isPaintOpaque(const SkPaint* paint, SkPaintBitmapOpacity contentType) {
    // TODO: SkXfermode should have a virtual isOpaque method, which would
    // make it possible to test modes that do not have a Coeff representation.

    if (!paint) {
        return contentType != kUnknown_SkPaintBitmapOpacity;
    }

    SkXfermode::Coeff srcCoeff, dstCoeff;
    if (SkXfermode::AsCoeff(paint->getXfermode(), &srcCoeff, &dstCoeff)){
        if (SkXfermode::kDA_Coeff == srcCoeff || SkXfermode::kDC_Coeff == srcCoeff ||
            SkXfermode::kIDA_Coeff == srcCoeff || SkXfermode::kIDC_Coeff == srcCoeff) {
            return false;
        }
        switch (dstCoeff) {
        case SkXfermode::kZero_Coeff:
            return true;
        case SkXfermode::kISA_Coeff:
            if (paint->getAlpha() != 255) {
                break;
            }
            if (contentType == kUnknown_SkPaintBitmapOpacity) {
                break;
            } else if (paint->getShader() && !paint->getShader()->isOpaque()) {
                break;
            }
            if (paint->getColorFilter() &&
                ((paint->getColorFilter()->getFlags() &
                SkColorFilter::kAlphaUnchanged_Flag) == 0)) {
                break;
            }
            return true;
        case SkXfermode::kSA_Coeff:
            if (paint->getAlpha() != 0) {
                break;
            }
            if (paint->getColorFilter() &&
                ((paint->getColorFilter()->getFlags() &
                SkColorFilter::kAlphaUnchanged_Flag) == 0)) {
                break;
            }
            return true;
        case SkXfermode::kSC_Coeff:
            if (paint->getColor() != 0) { // all components must be 0
                break;
            }
            if (contentType != kNoBitmap_SkPaintBitmapOpacity || paint->getShader()) {
                break;
            }
            if (paint->getColorFilter() && (
                (paint->getColorFilter()->getFlags() &
                SkColorFilter::kAlphaUnchanged_Flag) == 0)) {
                break;
            }
            return true;
        default:
            break;
        }
    }
    return false;
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
