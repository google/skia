/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <cstddef>
#include <cstring>
#include <type_traits>

#include "include/core/SkYUVAPixmaps.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/gpu/GrYUVABackendTextures.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/core/SkMipmap.h"
#include "src/core/SkScopeExit.h"
#include "src/gpu/GrClip.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrImageContextPriv.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/SurfaceFillContext.h"
#include "src/gpu/effects/GrBicubicEffect.h"
#include "src/gpu/effects/GrYUVtoRGBEffect.h"
#include "src/image/SkImage_Gpu.h"
#include "src/image/SkImage_GpuYUVA.h"

static constexpr auto kAssumedColorType = kRGBA_8888_SkColorType;

SkImage_GpuYUVA::SkImage_GpuYUVA(sk_sp<GrImageContext> context,
                                 uint32_t uniqueID,
                                 GrYUVATextureProxies proxies,
                                 sk_sp<SkColorSpace> imageColorSpace)
        : INHERITED(std::move(context),
                    SkImageInfo::Make(proxies.yuvaInfo().dimensions(),
                                      kAssumedColorType,
                                      // If an alpha channel is present we always use kPremul. This
                                      // is because, although the planar data is always un-premul,
                                      // the final interleaved RGBA sample produced in the shader
                                      // is premul (and similar if flattened via asView).
                                      proxies.yuvaInfo().hasAlpha() ? kPremul_SkAlphaType
                                                                    : kOpaque_SkAlphaType,
                                      std::move(imageColorSpace)),
                    uniqueID)
        , fYUVAProxies(std::move(proxies)) {
    // The caller should have checked this, just verifying.
    SkASSERT(fYUVAProxies.isValid());
}

// For onMakeColorSpace()
SkImage_GpuYUVA::SkImage_GpuYUVA(sk_sp<GrImageContext> context,
                                 const SkImage_GpuYUVA* image,
                                 sk_sp<SkColorSpace> targetCS)
        : INHERITED(std::move(context),
                    image->imageInfo().makeColorSpace(std::move(targetCS)),
                    kNeedNewImageUniqueID)
        , fYUVAProxies(image->fYUVAProxies)
        // Since null fFromColorSpace means no GrColorSpaceXform, we turn a null
        // image->refColorSpace() into an explicit SRGB.
        , fFromColorSpace(image->colorSpace() ? image->refColorSpace() : SkColorSpace::MakeSRGB()) {
}

bool SkImage_GpuYUVA::setupMipmapsForPlanes(GrRecordingContext* context) const {
    if (!context || !fContext->priv().matches(context)) {
        return false;
    }
    if (!context->priv().caps()->mipmapSupport()) {
        // We succeed in this case by doing nothing.
        return true;
    }
    int n = fYUVAProxies.yuvaInfo().numPlanes();
    sk_sp<GrSurfaceProxy> newProxies[4];
    for (int i = 0; i < n; ++i) {
        auto* t = fYUVAProxies.proxy(i)->asTextureProxy();
        if (t->mipmapped() == GrMipmapped::kNo && (t->width() > 1 || t->height() > 1)) {
            auto newView = GrCopyBaseMipMapToView(context, fYUVAProxies.makeView(i));
            if (!newView) {
                return false;
            }
            SkASSERT(newView.swizzle() == fYUVAProxies.makeView(i).swizzle());
            newProxies[i] = newView.detachProxy();
        } else {
            newProxies[i] = fYUVAProxies.refProxy(i);
        }
    }
    fYUVAProxies = GrYUVATextureProxies(fYUVAProxies.yuvaInfo(),
                                        newProxies,
                                        fYUVAProxies.textureOrigin());
    SkASSERT(fYUVAProxies.isValid());
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

GrSemaphoresSubmitted SkImage_GpuYUVA::onFlush(GrDirectContext* dContext, const GrFlushInfo& info) {
    if (!fContext->priv().matches(dContext) || dContext->abandoned()) {
        if (info.fSubmittedProc) {
            info.fSubmittedProc(info.fSubmittedContext, false);
        }
        if (info.fFinishedProc) {
            info.fFinishedProc(info.fFinishedContext);
        }
        return GrSemaphoresSubmitted::kNo;
    }

    GrSurfaceProxy* proxies[SkYUVAInfo::kMaxPlanes] = {};
    size_t numProxies = fYUVAProxies.numPlanes();
    for (size_t i = 0; i < numProxies; ++i) {
        proxies[i] = fYUVAProxies.proxy(i);
    }
    return dContext->priv().flushSurfaces({proxies, numProxies},
                                          SkSurface::BackendSurfaceAccess::kNoAccess,
                                          info);
}

bool SkImage_GpuYUVA::onHasMipmaps() const { return fYUVAProxies.mipmapped() == GrMipmapped::kYes; }

size_t SkImage_GpuYUVA::onTextureSize() const {
    size_t size = 0;
    for (int i = 0; i < fYUVAProxies.numPlanes(); ++i) {
        size += fYUVAProxies.proxy(i)->gpuMemorySize();
    }
    return size;
}

sk_sp<SkImage> SkImage_GpuYUVA::onMakeColorTypeAndColorSpace(SkColorType,
                                                             sk_sp<SkColorSpace> targetCS,
                                                             GrDirectContext* direct) const {
    // We explicitly ignore color type changes, for now.

    // we may need a mutex here but for now we expect usage to be in a single thread
    if (fOnMakeColorSpaceTarget &&
        SkColorSpace::Equals(targetCS.get(), fOnMakeColorSpaceTarget.get())) {
        return fOnMakeColorSpaceResult;
    }
    sk_sp<SkImage> result = sk_sp<SkImage>(new SkImage_GpuYUVA(sk_ref_sp(direct), this, targetCS));
    if (result) {
        fOnMakeColorSpaceTarget = targetCS;
        fOnMakeColorSpaceResult = result;
    }
    return result;
}

sk_sp<SkImage> SkImage_GpuYUVA::onReinterpretColorSpace(sk_sp<SkColorSpace> newCS) const {
    return sk_sp<SkImage>(new SkImage_GpuYUVA(fContext, this, std::move(newCS)));
}

std::tuple<GrSurfaceProxyView, GrColorType> SkImage_GpuYUVA::onAsView(
        GrRecordingContext* rContext,
        GrMipmapped mipmapped,
        GrImageTexGenPolicy) const {
    if (!fContext->priv().matches(rContext)) {
        return {};
    }
    auto sfc = rContext->priv().makeSFC(this->imageInfo(),
                                        SkBackingFit::kExact,
                                        /*sample count*/ 1,
                                        mipmapped,
                                        GrProtected::kNo,
                                        kTopLeft_GrSurfaceOrigin,
                                        SkBudgeted::kYes);
    if (!sfc) {
        return {};
    }

    const GrCaps& caps = *rContext->priv().caps();
    auto fp = GrYUVtoRGBEffect::Make(fYUVAProxies, GrSamplerState::Filter::kNearest, caps);
    if (fFromColorSpace) {
        fp = GrColorSpaceXformEffect::Make(std::move(fp),
                                           fFromColorSpace.get(), this->alphaType(),
                                           this->colorSpace()   , this->alphaType());
    }
    sfc->fillWithFP(std::move(fp));

    return {sfc->readSurfaceView(), sfc->colorInfo().colorType()};
}

std::unique_ptr<GrFragmentProcessor> SkImage_GpuYUVA::onAsFragmentProcessor(
        GrRecordingContext* context,
        SkSamplingOptions sampling,
        const SkTileMode tileModes[2],
        const SkMatrix& m,
        const SkRect* subset,
        const SkRect* domain) const {
    if (!fContext->priv().matches(context)) {
        return {};
    }
    auto wmx = SkTileModeToWrapMode(tileModes[0]);
    auto wmy = SkTileModeToWrapMode(tileModes[1]);
    GrSamplerState sampler(wmx, wmy, sampling.filter, sampling.mipmap);
    if (sampler.mipmapped() == GrMipmapped::kYes && !this->setupMipmapsForPlanes(context)) {
        sampler.setMipmapMode(GrSamplerState::MipmapMode::kNone);
    }

    const auto& yuvM = sampling.useCubic ? SkMatrix::I() : m;
    auto fp = GrYUVtoRGBEffect::Make(fYUVAProxies,
                                     sampler,
                                     *context->priv().caps(),
                                     yuvM,
                                     subset,
                                     domain);
    if (sampling.useCubic) {
        fp = GrBicubicEffect::Make(std::move(fp),
                                   this->alphaType(),
                                   m,
                                   sampling.cubic,
                                   GrBicubicEffect::Direction::kXY);
    }
    if (fFromColorSpace) {
        fp = GrColorSpaceXformEffect::Make(std::move(fp),
                                           fFromColorSpace.get(), this->alphaType(),
                                           this->colorSpace()   , this->alphaType());
    }
    return fp;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkImage> SkImage::MakeFromYUVATextures(GrRecordingContext* context,
                                             const GrYUVABackendTextures& yuvaTextures,
                                             sk_sp<SkColorSpace> imageColorSpace,
                                             TextureReleaseProc textureReleaseProc,
                                             ReleaseContext releaseContext) {
    auto releaseHelper = GrRefCntedCallback::Make(textureReleaseProc, releaseContext);

    GrProxyProvider* proxyProvider = context->priv().proxyProvider();
    int numPlanes = yuvaTextures.yuvaInfo().numPlanes();
    sk_sp<GrSurfaceProxy> proxies[SkYUVAInfo::kMaxPlanes];
    for (int plane = 0; plane < numPlanes; ++plane) {
        proxies[plane] = proxyProvider->wrapBackendTexture(yuvaTextures.texture(plane),
                                                           kBorrow_GrWrapOwnership,
                                                           GrWrapCacheable::kNo,
                                                           kRead_GrIOType,
                                                           releaseHelper);
        if (!proxies[plane]) {
            return {};
        }
    }
    GrYUVATextureProxies yuvaProxies(yuvaTextures.yuvaInfo(),
                                     proxies,
                                     yuvaTextures.textureOrigin());

    if (!yuvaProxies.isValid()) {
        return nullptr;
    }

    return sk_make_sp<SkImage_GpuYUVA>(sk_ref_sp(context),
                                       kNeedNewImageUniqueID,
                                       yuvaProxies,
                                       imageColorSpace);
}

sk_sp<SkImage> SkImage::MakeFromYUVAPixmaps(GrRecordingContext* context,
                                            const SkYUVAPixmaps& pixmaps,
                                            GrMipmapped buildMips,
                                            bool limitToMaxTextureSize,
                                            sk_sp<SkColorSpace> imageColorSpace) {
    if (!context) {
        return nullptr;  // until we impl this for raster backend
    }

    if (!pixmaps.isValid()) {
        return nullptr;
    }

    if (!context->priv().caps()->mipmapSupport()) {
        buildMips = GrMipMapped::kNo;
    }

    // Resize the pixmaps if necessary.
    int numPlanes = pixmaps.numPlanes();
    int maxTextureSize = context->priv().caps()->maxTextureSize();
    int maxDim = std::max(pixmaps.yuvaInfo().width(), pixmaps.yuvaInfo().height());

    SkYUVAPixmaps tempPixmaps;
    const SkYUVAPixmaps* pixmapsToUpload = &pixmaps;
    // We assume no plane is larger than the image size (and at least one plane is as big).
    if (maxDim > maxTextureSize) {
        if (!limitToMaxTextureSize) {
            return nullptr;
        }
        float scale = static_cast<float>(maxTextureSize)/maxDim;
        SkISize newDimensions = {
            std::min(static_cast<int>(pixmaps.yuvaInfo().width() *scale), maxTextureSize),
            std::min(static_cast<int>(pixmaps.yuvaInfo().height()*scale), maxTextureSize)
        };
        SkYUVAInfo newInfo = pixmaps.yuvaInfo().makeDimensions(newDimensions);
        SkYUVAPixmapInfo newPixmapInfo(newInfo, pixmaps.dataType(), /*row bytes*/ nullptr);
        tempPixmaps = SkYUVAPixmaps::Allocate(newPixmapInfo);
        SkSamplingOptions sampling(SkFilterMode::kLinear);
        if (!tempPixmaps.isValid()) {
            return nullptr;
        }
        for (int i = 0; i < numPlanes; ++i) {
            if (!pixmaps.plane(i).scalePixels(tempPixmaps.plane(i), sampling)) {
                return nullptr;
            }
        }
        pixmapsToUpload = &tempPixmaps;
    }

    // Convert to texture proxies.
    GrSurfaceProxyView views[SkYUVAInfo::kMaxPlanes];
    GrColorType pixmapColorTypes[SkYUVAInfo::kMaxPlanes];
    for (int i = 0; i < numPlanes; ++i) {
        // Turn the pixmap into a GrTextureProxy
        SkBitmap bmp;
        bmp.installPixels(pixmapsToUpload->plane(i));
        std::tie(views[i], std::ignore) = GrMakeUncachedBitmapProxyView(context, bmp, buildMips);
        if (!views[i]) {
            return nullptr;
        }
        pixmapColorTypes[i] = SkColorTypeToGrColorType(bmp.colorType());
    }

    GrYUVATextureProxies yuvaProxies(pixmapsToUpload->yuvaInfo(), views, pixmapColorTypes);
    SkASSERT(yuvaProxies.isValid());
    return sk_make_sp<SkImage_GpuYUVA>(sk_ref_sp(context),
                                       kNeedNewImageUniqueID,
                                       std::move(yuvaProxies),
                                       std::move(imageColorSpace));
}

/////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkImage> SkImage::MakePromiseYUVATexture(sk_sp<GrContextThreadSafeProxy> threadSafeProxy,
                                               const GrYUVABackendTextureInfo& backendTextureInfo,
                                               sk_sp<SkColorSpace> imageColorSpace,
                                               PromiseImageTextureFulfillProc textureFulfillProc,
                                               PromiseImageTextureReleaseProc textureReleaseProc,
                                               PromiseImageTextureContext textureContexts[]) {
    if (!backendTextureInfo.isValid()) {
        return nullptr;
    }

    SkISize planeDimensions[SkYUVAInfo::kMaxPlanes];
    int n = backendTextureInfo.yuvaInfo().planeDimensions(planeDimensions);

    // Our contract is that we will always call the release proc even on failure.
    // We use the helper to convey the context, so we need to ensure make doesn't fail.
    textureReleaseProc = textureReleaseProc ? textureReleaseProc : [](void*) {};
    sk_sp<GrRefCntedCallback> releaseHelpers[4];
    for (int i = 0; i < n; ++i) {
        releaseHelpers[i] = GrRefCntedCallback::Make(textureReleaseProc, textureContexts[i]);
    }

    if (!threadSafeProxy) {
        return nullptr;
    }

    SkAlphaType at = backendTextureInfo.yuvaInfo().hasAlpha() ? kPremul_SkAlphaType
                                                              : kOpaque_SkAlphaType;
    SkImageInfo info = SkImageInfo::Make(backendTextureInfo.yuvaInfo().dimensions(),
                                         kAssumedColorType, at, imageColorSpace);
    if (!SkImageInfoIsValid(info)) {
        return nullptr;
    }

    // Make a lazy proxy for each plane and wrap in a view.
    sk_sp<GrSurfaceProxy> proxies[4];
    for (int i = 0; i < n; ++i) {
        proxies[i] = SkImage_GpuBase::MakePromiseImageLazyProxy(threadSafeProxy.get(),
                                                                planeDimensions[i],
                                                                backendTextureInfo.planeFormat(i),
                                                                GrMipmapped::kNo,
                                                                textureFulfillProc,
                                                                std::move(releaseHelpers[i]));
        if (!proxies[i]) {
            return nullptr;
        }
    }
    GrYUVATextureProxies yuvaTextureProxies(backendTextureInfo.yuvaInfo(),
                                            proxies,
                                            backendTextureInfo.textureOrigin());
    SkASSERT(yuvaTextureProxies.isValid());
    sk_sp<GrImageContext> ctx(GrImageContextPriv::MakeForPromiseImage(std::move(threadSafeProxy)));
    return sk_make_sp<SkImage_GpuYUVA>(std::move(ctx),
                                       kNeedNewImageUniqueID,
                                       std::move(yuvaTextureProxies),
                                       std::move(imageColorSpace));
}
