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
#include "GrProxyProvider.h"
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

    GrProxyProvider* proxyProvider = this->context()->contextPriv().proxyProvider();
    sk_sp<GrTextureProxy> proxy;

    if (fOriginalKey.isValid()) {
        proxy = proxyProvider->findOrCreateProxyByUniqueKey(fOriginalKey, kTopLeft_GrSurfaceOrigin);
        if (proxy && (!willBeMipped || GrMipMapped::kYes == proxy->mipMapped())) {
            return proxy;
        }
    }

    if (!proxy) {
        if (willBeMipped) {
            proxy = GrGenerateMipMapsAndUploadToTextureProxy(proxyProvider, fBitmap, dstColorSpace);
        }
        if (!proxy) {
            proxy = GrUploadBitmapToTextureProxy(proxyProvider, fBitmap, dstColorSpace);
        }
        if (proxy) {
            if (fOriginalKey.isValid()) {
                proxyProvider->assignUniqueKeyToProxy(fOriginalKey, proxy.get());
            }
            if (!willBeMipped || GrMipMapped::kYes == proxy->mipMapped()) {
                SkASSERT(proxy->origin() == kTopLeft_GrSurfaceOrigin);
                if (fOriginalKey.isValid()) {
                    GrInstallBitmapUniqueKeyInvalidator(fOriginalKey, fBitmap.pixelRef());
                }
                return proxy;
            }
        }
    }

    if (proxy) {
        SkASSERT(willBeMipped);
        SkASSERT(GrMipMapped::kNo == proxy->mipMapped());
        // We need a mipped proxy, but we either found a proxy earlier that wasn't mipped or
        // generated a non mipped proxy. Thus we generate a new mipped surface and copy the original
        // proxy into the base layer. We will then let the gpu generate the rest of the mips.
        if (auto mippedProxy = GrCopyBaseMipMapToTextureProxy(this->context(), proxy.get())) {
            SkASSERT(mippedProxy->origin() == kTopLeft_GrSurfaceOrigin);
            if (fOriginalKey.isValid()) {
                // In this case we are stealing the key from the original proxy which should only
                // happen when we have just generated mipmaps for an originally unmipped
                // proxy/texture. This means that all future uses of the key will access the
                // mipmapped version. The texture backing the unmipped version will remain in the
                // resource cache until the last texture proxy referencing it is deleted at which
                // time it too will be deleted or recycled.
                proxyProvider->removeUniqueKeyFromProxy(fOriginalKey, proxy.get());
                proxyProvider->assignUniqueKeyToProxy(fOriginalKey, mippedProxy.get());
                GrInstallBitmapUniqueKeyInvalidator(fOriginalKey, fBitmap.pixelRef());
            }
            return mippedProxy;
        }
        // We failed to make a mipped proxy with the base copied into it. This could have
        // been from failure to make the proxy or failure to do the copy. Thus we will fall
        // back to just using the non mipped proxy; See skbug.com/7094.
        return proxy;
    }
    return nullptr;
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
