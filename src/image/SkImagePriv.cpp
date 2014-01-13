/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImagePriv.h"
#include "SkCanvas.h"
#include "SkPicture.h"

SkBitmap::Config SkColorTypeToBitmapConfig(SkColorType colorType) {
    switch (colorType) {
        case kAlpha_8_SkColorType:
            return SkBitmap::kA8_Config;

        case kARGB_4444_SkColorType:
            return SkBitmap::kARGB_4444_Config;

        case kRGB_565_SkColorType:
            return SkBitmap::kRGB_565_Config;

        case kPMColor_SkColorType:
            return SkBitmap::kARGB_8888_Config;

        case kIndex_8_SkColorType:
            return SkBitmap::kIndex8_Config;

        default:
            // break for unsupported colortypes
            break;
    }
    return SkBitmap::kNo_Config;
}

SkBitmap::Config SkImageInfoToBitmapConfig(const SkImageInfo& info) {
    return SkColorTypeToBitmapConfig(info.fColorType);
}

bool SkBitmapConfigToColorType(SkBitmap::Config config, SkColorType* ctOut) {
    SkColorType ct;
    switch (config) {
        case SkBitmap::kA8_Config:
            ct = kAlpha_8_SkColorType;
            break;
        case SkBitmap::kIndex8_Config:
            ct = kIndex_8_SkColorType;
            break;
        case SkBitmap::kRGB_565_Config:
            ct = kRGB_565_SkColorType;
            break;
        case SkBitmap::kARGB_4444_Config:
            ct = kARGB_4444_SkColorType;
            break;
        case SkBitmap::kARGB_8888_Config:
            ct = kPMColor_SkColorType;
            break;
        case SkBitmap::kNo_Config:
        default:
            return false;
    }
    if (ctOut) {
        *ctOut = ct;
    }
    return true;
}


SkImage* SkNewImageFromBitmap(const SkBitmap& bm, bool canSharePixelRef) {
    SkImageInfo info;
    if (!bm.asImageInfo(&info)) {
        return NULL;
    }

    SkImage* image = NULL;
    if (canSharePixelRef || bm.isImmutable()) {
        image = SkNewImageFromPixelRef(bm.pixelRef());
    } else {
        bm.lockPixels();
        if (bm.getPixels()) {
            image = SkImage::NewRasterCopy(info, bm.getPixels(), bm.rowBytes());
        }
        bm.unlockPixels();
    }
    return image;
}

static bool needs_layer(const SkPaint& paint) {
    return  0xFF != paint.getAlpha() ||
    paint.getColorFilter() ||
    paint.getImageFilter() ||
    SkXfermode::IsMode(paint.getXfermode(), SkXfermode::kSrcOver_Mode);
}

void SkImagePrivDrawPicture(SkCanvas* canvas, SkPicture* picture,
                            SkScalar x, SkScalar y, const SkPaint* paint) {
    int saveCount = canvas->getSaveCount();

    if (paint && needs_layer(*paint)) {
        SkRect bounds;
        bounds.set(x, y,
                   x + SkIntToScalar(picture->width()),
                   y + SkIntToScalar(picture->height()));
        canvas->saveLayer(&bounds, paint);
        canvas->translate(x, y);
    } else if (x || y) {
        canvas->save();
        canvas->translate(x, y);
    }

    canvas->drawPicture(*picture);
    canvas->restoreToCount(saveCount);
}

void SkImagePrivDrawPicture(SkCanvas* canvas, SkPicture* picture,
                            const SkRect* src,  const SkRect& dst, const SkPaint* paint) {
    int saveCount = canvas->getSaveCount();

    SkMatrix matrix;
    SkRect   tmpSrc;

    if (NULL != src) {
        tmpSrc = *src;
    } else {
        tmpSrc.set(0, 0,
                   SkIntToScalar(picture->width()),
                   SkIntToScalar(picture->height()));
    }

    matrix.setRectToRect(tmpSrc, dst, SkMatrix::kFill_ScaleToFit);
    if (paint && needs_layer(*paint)) {
        canvas->saveLayer(&dst, paint);
    } else {
        canvas->save();
    }
    canvas->concat(matrix);
    if (!paint || !needs_layer(*paint)) {
        canvas->clipRect(tmpSrc);
    }

    canvas->drawPicture(*picture);
    canvas->restoreToCount(saveCount);
}
