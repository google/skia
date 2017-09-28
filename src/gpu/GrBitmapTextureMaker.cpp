/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrBitmapTextureMaker.h"

#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrGpuResourcePriv.h"
#include "GrResourceProvider.h"
#include "GrSurfaceContext.h"
#include "SkBitmap.h"
#include "SkGr.h"
#include "SkMipMap.h"
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

sk_sp<GrTextureProxy> GrBitmapTextureMaker::refOriginalTextureProxy(bool willBeMipped,
                                                                    SkColorSpace* dstColorSpace,
                                                                    AllowedTexGenType onlyIfFast) {
    if (AllowedTexGenType::kCheap == onlyIfFast) {
        return nullptr;
    }

    sk_sp<GrTextureProxy> originalProxy;

    if (fOriginalKey.isValid()) {
        originalProxy = this->context()->resourceProvider()->findOrCreateProxyByUniqueKey(
                                                            fOriginalKey, kTopLeft_GrSurfaceOrigin);
        if (originalProxy && (!willBeMipped || originalProxy->isMipMapped())) {
            return originalProxy;
        }
    }

    sk_sp<GrTextureProxy> proxy;
    if (willBeMipped) {
        if (!originalProxy) {
            proxy = GrGenerateMipMapsAndUploadToTextureProxy(this->context(), fBitmap,
                                                             dstColorSpace);
        } else {
            proxy = GrCopyBaseMipMapToTextureProxy(this->context(), originalProxy.get(),
                                                   dstColorSpace);
        }
    }
    if (!proxy) {
        proxy = GrUploadBitmapToTextureProxy(this->context()->resourceProvider(), fBitmap,
                                             dstColorSpace);
    }
    if (proxy && fOriginalKey.isValid()) {
        SkASSERT(proxy->origin() == kTopLeft_GrSurfaceOrigin);
        if (originalProxy) {
            // In this case we are stealing the key from the original proxy which should only happen
            // when we have just generated mipmaps for an originally unmipped proxy/texture. This
            // means that all future uses of the key will access the mipmapped version. The texture
            // backing the unmipped version will remain in the resource cache until the last texture
            // proxy referencing it is deleted at which time it too will be deleted or recycled.
            this->context()->resourceProvider()->removeUniqueKeyFromProxy(fOriginalKey,
                                                                          originalProxy.get());
        }
        this->context()->resourceProvider()->assignUniqueKeyToProxy(fOriginalKey, proxy.get());
        GrInstallBitmapUniqueKeyInvalidator(fOriginalKey, fBitmap.pixelRef());
    }
    return proxy;
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
