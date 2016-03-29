/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapProvider.h"
#include "SkImage_Base.h"
#include "SkPixelRef.h"

int SkBitmapProvider::width() const {
    return fImage ? fImage->width() : fBitmap.width();
}

int SkBitmapProvider::height() const {
    return fImage ? fImage->height() : fBitmap.height();
}

uint32_t SkBitmapProvider::getID() const {
    return fImage ? fImage->uniqueID() : fBitmap.getGenerationID();
}

bool SkBitmapProvider::validForDrawing() const {
    if (!fImage) {
        if (0 == fBitmap.width() || 0 == fBitmap.height()) {
            return false;
        }
        if (nullptr == fBitmap.pixelRef()) {
            return false;   // no pixels to read
        }
        if (fBitmap.getTexture()) {
            // we can handle texture (ugh) since lockPixels will perform a read-back
            return true;
        }
        if (kIndex_8_SkColorType == fBitmap.colorType()) {
            SkAutoLockPixels alp(fBitmap); // but we need to call it before getColorTable() is safe.
            if (!fBitmap.getColorTable()) {
                return false;
            }
        }
    }
    return true;
}

SkImageInfo SkBitmapProvider::info() const {
    if (fImage) {
        SkAlphaType at = fImage->isOpaque() ? kOpaque_SkAlphaType : kPremul_SkAlphaType;
        return SkImageInfo::MakeN32(fImage->width(), fImage->height(), at);
    } else {
        return fBitmap.info();
    }
}

bool SkBitmapProvider::isVolatile() const {
    if (fImage) {
        return false;   // add flag to images?
    } else {
        return fBitmap.isVolatile();
    }
}

SkBitmapCacheDesc SkBitmapProvider::makeCacheDesc(int w, int h) const {
    return fImage ? SkBitmapCacheDesc::Make(fImage, w, h) : SkBitmapCacheDesc::Make(fBitmap, w, h);
}

SkBitmapCacheDesc SkBitmapProvider::makeCacheDesc() const {
    return fImage ? SkBitmapCacheDesc::Make(fImage) : SkBitmapCacheDesc::Make(fBitmap);
}

void SkBitmapProvider::notifyAddedToCache() const {
    if (fImage) {
        as_IB(fImage)->notifyAddedToCache();
    } else {
        fBitmap.pixelRef()->notifyAddedToCache();
    }
}

bool SkBitmapProvider::asBitmap(SkBitmap* bm) const {
    if (fImage) {
        return as_IB(fImage)->getROPixels(bm, SkImage::kAllow_CachingHint);
    } else {
        *bm = fBitmap;
        return true;
    }
}
