/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrRecordingContext.h"
#include "src/gpu/GrColorSpaceXform.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrTextureAdjuster.h"
#include "src/gpu/SkGr.h"

GrTextureAdjuster::GrTextureAdjuster(GrRecordingContext* context,
                                     GrSurfaceProxyView original,
                                     const GrColorInfo& colorInfo,
                                     uint32_t uniqueID)
        : INHERITED(context, {colorInfo, original.proxy()->dimensions()})
        , fOriginal(std::move(original))
        , fUniqueID(uniqueID) {}

GrSurfaceProxyView GrTextureAdjuster::makeMippedCopy() {
    GrProxyProvider* proxyProvider = this->context()->priv().proxyProvider();

    GrUniqueKey mipmappedKey;
    if (fUniqueID != SK_InvalidUniqueID) {
        GrUniqueKey baseKey;
        GrMakeKeyFromImageID(&baseKey, fUniqueID, SkIRect::MakeSize(this->dimensions()));
        SkASSERT(baseKey.isValid());
        static const GrUniqueKey::Domain kMipMappedDomain = GrUniqueKey::GenerateDomain();
        {   // No extra values beyond the domain are required. Must name the var to please
            // clang-tidy.
            GrUniqueKey::Builder b(&mipmappedKey, baseKey, kMipMappedDomain, 0);
        }
        SkASSERT(mipmappedKey.isValid());
        if (sk_sp<GrTextureProxy> cachedCopy =
                    proxyProvider->findOrCreateProxyByUniqueKey(mipmappedKey)) {
            return {std::move(cachedCopy), fOriginal.origin(), fOriginal.swizzle()};
        }
    }

    auto copy = GrCopyBaseMipMapToView(this->context(), fOriginal);
    if (!copy) {
        return {};
    }
    if (mipmappedKey.isValid()) {
        // TODO: If we move listeners up from SkImage_Lazy to SkImage_Base then add one here.
        proxyProvider->assignUniqueKeyToProxy(mipmappedKey, copy.asTextureProxy());
    }
    return copy;
}

GrSurfaceProxyView GrTextureAdjuster::onView(GrMipmapped mipMapped) {
    if (this->context()->abandoned()) {
        // The texture was abandoned.
        return {};
    }

    SkASSERT(this->width() <= this->context()->priv().caps()->maxTextureSize() &&
             this->height() <= this->context()->priv().caps()->maxTextureSize());

    GrTextureProxy* texProxy = fOriginal.asTextureProxy();
    SkASSERT(texProxy);
    if (mipMapped == GrMipmapped::kNo || texProxy->mipmapped() == GrMipmapped::kYes) {
        return fOriginal;
    }

    GrSurfaceProxyView copy = this->makeMippedCopy();
    if (!copy) {
        // If we were unable to make a copy and we only needed a copy for mips, then we will return
        // the source texture here and require that the GPU backend is able to fall back to using
        // linear filtering if mips are required.
        return fOriginal;
    }
    SkASSERT(copy.asTextureProxy());
    return copy;
}
