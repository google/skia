/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrImageIDTextureAdjuster.h"

#include "SkBitmap.h"
#include "SkGrPriv.h"
#include "SkImage_Base.h"


GrBitmapTextureAdjuster::GrBitmapTextureAdjuster(const SkBitmap* bmp)
    : INHERITED(bmp->getTexture(), SkIRect::MakeWH(bmp->width(), bmp->height()))
    , fBmp(bmp) {}

void GrBitmapTextureAdjuster::makeCopyKey(const CopyParams& params, GrUniqueKey* copyKey) {
    if (fBmp->isVolatile()) {
        return;
    }
    // The content area must represent the whole bitmap. Texture-backed bitmaps don't support
    // extractSubset(). Therefore, either the bitmap and the texture are the same size or the
    // content's dimensions are the bitmap's dimensions which is pinned to the upper left
    // of the texture.
    GrUniqueKey baseKey;
    GrMakeKeyFromImageID(&baseKey, fBmp->getGenerationID(),
                         SkIRect::MakeWH(fBmp->width(), fBmp->height()));
    MakeCopyKeyFromOrigKey(baseKey, params, copyKey);
}

void GrBitmapTextureAdjuster::didCacheCopy(const GrUniqueKey& copyKey) {
    GrInstallBitmapUniqueKeyInvalidator(copyKey, fBmp->pixelRef());
}

//////////////////////////////////////////////////////////////////////////////

GrImageTextureAdjuster::GrImageTextureAdjuster(const SkImage_Base* img)
    : INHERITED(img->peekTexture(), SkIRect::MakeWH(img->width(), img->height()))
    , fImageBase(img) {}

void GrImageTextureAdjuster::makeCopyKey(const CopyParams& params, GrUniqueKey* copyKey) {
    // By construction this texture adjuster always represents an entire SkImage, so use the
    // image's width and height for the key's rectangle.
    GrUniqueKey baseKey;
    GrMakeKeyFromImageID(&baseKey, fImageBase->uniqueID(),
                         SkIRect::MakeWH(fImageBase->width(), fImageBase->height()));
    MakeCopyKeyFromOrigKey(baseKey, params, copyKey);
}

void GrImageTextureAdjuster::didCacheCopy(const GrUniqueKey& copyKey) {
    // We don't currently have a mechanism for notifications on Images!
}
