/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrImageIDTextureAdjuster.h"

#include "GrContext.h"
#include "GrGpuResourcePriv.h"
#include "SkBitmap.h"
#include "SkGrPriv.h"
#include "SkImage_Base.h"
#include "SkPixelRef.h"

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

//////////////////////////////////////////////////////////////////////////////

GrBitmapTextureMaker::GrBitmapTextureMaker(GrContext* context, const SkBitmap& bitmap)
    : INHERITED(context, bitmap.width(), bitmap.height())
    , fBitmap(bitmap) {
    SkASSERT(!bitmap.getTexture());
    if (!bitmap.isVolatile()) {
        SkIPoint origin = bitmap.pixelRefOrigin();
        SkIRect subset = SkIRect::MakeXYWH(origin.fX, origin.fY, bitmap.width(),
                                           bitmap.height());
        GrMakeKeyFromImageID(&fOriginalKey, bitmap.pixelRef()->getGenerationID(), subset);
    }
}

GrTexture* GrBitmapTextureMaker::refOriginalTexture() {
    GrTexture* tex;

    if (fOriginalKey.isValid()) {
        tex = this->context()->textureProvider()->findAndRefTextureByUniqueKey(fOriginalKey);
        if (tex) {
            return tex;
        }
    }

    tex = GrUploadBitmapToTexture(this->context(), fBitmap);
    if (tex && fOriginalKey.isValid()) {
        tex->resourcePriv().setUniqueKey(fOriginalKey);
        GrInstallBitmapUniqueKeyInvalidator(fOriginalKey, fBitmap.pixelRef());
    }
    return tex;
}

void GrBitmapTextureMaker::makeCopyKey(const CopyParams& copyParams, GrUniqueKey* copyKey) {
    if (fOriginalKey.isValid()) {
        MakeCopyKeyFromOrigKey(fOriginalKey, copyParams, copyKey);
    }
}

void GrBitmapTextureMaker::didCacheCopy(const GrUniqueKey& copyKey) {
    GrInstallBitmapUniqueKeyInvalidator(copyKey, fBitmap.pixelRef());
}
