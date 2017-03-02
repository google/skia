/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrBitmapTextureMaker.h"

#include "GrContext.h"
#include "GrGpuResourcePriv.h"
#include "GrTextureProvider.h"
#include "SkBitmap.h"
#include "SkGrPriv.h"
#include "SkPixelRef.h"

static bool bmp_is_alpha_only(const SkBitmap& bm) { return kAlpha_8_SkColorType == bm.colorType(); }

GrBitmapTextureMaker::GrBitmapTextureMaker(GrContext* context, const SkBitmap& bitmap)
    : INHERITED(context, bitmap.width(), bitmap.height(), bmp_is_alpha_only(bitmap))
    , fBitmap(bitmap) {
    if (!bitmap.isVolatile()) {
        SkIPoint origin = bitmap.pixelRefOrigin();
        SkIRect subset = SkIRect::MakeXYWH(origin.fX, origin.fY, bitmap.width(),
                                           bitmap.height());
        GrMakeKeyFromImageID(&fOriginalKey, bitmap.pixelRef()->getGenerationID(), subset);
    }
}

GrTexture* GrBitmapTextureMaker::refOriginalTexture(bool willBeMipped,
                                                    SkColorSpace* dstColorSpace) {
    GrTexture* tex = nullptr;

    if (fOriginalKey.isValid()) {
        tex = this->context()->textureProvider()->findAndRefTextureByUniqueKey(fOriginalKey);
        if (tex) {
            return tex;
        }
    }
    if (willBeMipped) {
        tex = GrGenerateMipMapsAndUploadToTexture(this->context(), fBitmap, dstColorSpace);
    }
    if (!tex) {
        tex = GrUploadBitmapToTexture(this->context(), fBitmap);
    }
    if (tex && fOriginalKey.isValid()) {
        this->context()->textureProvider()->assignUniqueKeyToTexture(fOriginalKey, tex);
        GrInstallBitmapUniqueKeyInvalidator(fOriginalKey, fBitmap.pixelRef());
    }
    return tex;
}

void GrBitmapTextureMaker::makeCopyKey(const CopyParams& copyParams, GrUniqueKey* copyKey,
                                       SkColorSpace* dstColorSpace) {
    // Destination color space is irrelevant - we always upload the bitmap's contents as-is
    if (fOriginalKey.isValid()) {
        MakeCopyKeyFromOrigKey(fOriginalKey, copyParams, copyKey);
    }
}

void GrBitmapTextureMaker::didCacheCopy(const GrUniqueKey& copyKey) {
    GrInstallBitmapUniqueKeyInvalidator(copyKey, fBitmap.pixelRef());
}

SkAlphaType GrBitmapTextureMaker::alphaType() const {
    return fBitmap.alphaType();
}

sk_sp<SkColorSpace> GrBitmapTextureMaker::getColorSpace(SkColorSpace* dstColorSpace) {
    // Color space doesn't depend on destination color space - it's just whatever is in the bitmap
    return fBitmap.refColorSpace();
}
