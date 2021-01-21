/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrBitmapTextureMaker.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkPixelRef.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/private/SkIDChangeListener.h"
#include "src/gpu/GrGpuResourcePriv.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrSurfaceContext.h"
#include "src/gpu/SkGr.h"

static GrImageInfo get_image_info(GrRecordingContext* context, const SkBitmap& bitmap) {
    GrColorType ct = SkColorTypeToGrColorType(bitmap.info().colorType());
    GrBackendFormat format = context->priv().caps()->getDefaultBackendFormat(ct, GrRenderable::kNo);
    if (!format.isValid()) {
        ct = GrColorType::kRGBA_8888;
    }
    return {ct, bitmap.alphaType(), bitmap.refColorSpace(), bitmap.dimensions()};
}

GrBitmapTextureMaker::GrBitmapTextureMaker(GrRecordingContext* context,
                                           const SkBitmap& bitmap,
                                           SkBackingFit fit)
        : GrBitmapTextureMaker(context, bitmap, GrImageTexGenPolicy::kNew_Uncached_Budgeted, fit) {}

GrBitmapTextureMaker::GrBitmapTextureMaker(GrRecordingContext* context,
                                           const SkBitmap& bitmap,
                                           GrImageTexGenPolicy cachePolicy)
        : GrBitmapTextureMaker(context, bitmap, cachePolicy, SkBackingFit::kExact) {}

GrBitmapTextureMaker::GrBitmapTextureMaker(GrRecordingContext* context,
                                           const SkBitmap& bitmap,
                                           GrImageTexGenPolicy cachePolicy,
                                           SkBackingFit fit)
        : INHERITED(context, get_image_info(context, bitmap))
        , fBitmap(bitmap)
        , fFit(fit)
        , fBudgeted(cachePolicy == GrImageTexGenPolicy::kNew_Uncached_Unbudgeted
                            ? SkBudgeted::kNo
                            : SkBudgeted::kYes) {
    if (cachePolicy == GrImageTexGenPolicy::kDraw) {
        SkIPoint origin = bitmap.pixelRefOrigin();
        SkIRect subset = SkIRect::MakeXYWH(origin.fX, origin.fY, bitmap.width(),
                                           bitmap.height());
        GrMakeKeyFromImageID(&fKey, bitmap.pixelRef()->getGenerationID(), subset);
    }
}

GrSurfaceProxyView GrBitmapTextureMaker::refOriginalTextureProxyView(GrMipmapped mipMapped) {
    GrProxyProvider* proxyProvider = this->context()->priv().proxyProvider();
    sk_sp<GrTextureProxy> proxy;
    GrSwizzle swizzle;

    auto installKey = [&](GrTextureProxy* proxy) {
        auto listener = GrMakeUniqueKeyInvalidationListener(&fKey, proxyProvider->contextID());
        fBitmap.pixelRef()->addGenIDChangeListener(std::move(listener));
        proxyProvider->assignUniqueKeyToProxy(fKey, proxy);
    };

    if (fKey.isValid()) {
        proxy = proxyProvider->findOrCreateProxyByUniqueKey(fKey);
        if (proxy) {
            swizzle = this->context()->priv().caps()->getReadSwizzle(proxy->backendFormat(),
                                                                     this->colorType());
            if (mipMapped == GrMipmapped::kNo || proxy->mipmapped() == GrMipmapped::kYes) {
                return {std::move(proxy), kTopLeft_GrSurfaceOrigin, swizzle};
            }
        }
    }

    if (!proxy) {
        if (this->colorType() != SkColorTypeToGrColorType(fBitmap.info().colorType())) {
            SkASSERT(this->colorType() == GrColorType::kRGBA_8888);
            SkBitmap copy8888;
            if (!copy8888.tryAllocPixels(fBitmap.info().makeColorType(kRGBA_8888_SkColorType)) ||
                !fBitmap.readPixels(copy8888.pixmap())) {
                return {};
            }
            copy8888.setImmutable();
            proxy = proxyProvider->createProxyFromBitmap(copy8888, mipMapped, fFit, fBudgeted);
        } else {
            proxy = proxyProvider->createProxyFromBitmap(fBitmap, mipMapped, fFit, fBudgeted);
        }
        if (proxy) {
            swizzle = this->context()->priv().caps()->getReadSwizzle(proxy->backendFormat(),
                                                                     this->colorType());
            SkASSERT(mipMapped == GrMipmapped::kNo || proxy->mipmapped() == GrMipmapped::kYes);
            if (fKey.isValid()) {
                installKey(proxy.get());
            }
            return {std::move(proxy), kTopLeft_GrSurfaceOrigin, swizzle};
        }
    }

    if (proxy) {
        SkASSERT(mipMapped == GrMipmapped::kYes);
        SkASSERT(proxy->mipmapped() == GrMipmapped::kNo);
        SkASSERT(fKey.isValid());
        // We need a mipped proxy, but we found a proxy earlier that wasn't mipped. Thus we generate
        // a new mipped surface and copy the original proxy into the base layer. We will then let
        // the gpu generate the rest of the mips.
        auto mippedProxy = GrCopyBaseMipMapToTextureProxy(this->context(), proxy,
                                                          kTopLeft_GrSurfaceOrigin);
        if (!mippedProxy) {
            // We failed to make a mipped proxy with the base copied into it. This could have
            // been from failure to make the proxy or failure to do the copy. Thus we will fall
            // back to just using the non mipped proxy; See skbug.com/7094.
            return {std::move(proxy), kTopLeft_GrSurfaceOrigin, swizzle};
        }
        // In this case we are stealing the key from the original proxy which should only happen
        // when we have just generated mipmaps for an originally unmipped proxy/texture. This
        // means that all future uses of the key will access the mipmapped version. The texture
        // backing the unmipped version will remain in the resource cache until the last texture
        // proxy referencing it is deleted at which time it too will be deleted or recycled.
        SkASSERT(proxy->getUniqueKey() == fKey);
        proxyProvider->removeUniqueKeyFromProxy(proxy.get());
        installKey(mippedProxy->asTextureProxy());
        return {std::move(mippedProxy), kTopLeft_GrSurfaceOrigin, swizzle};
    }
    return {};
}
