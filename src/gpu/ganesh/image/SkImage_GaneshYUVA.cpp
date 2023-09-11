/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/image/SkImage_GaneshYUVA.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkSize.h"
#include "include/core/SkSurface.h"
#include "include/core/SkYUVAInfo.h"
#include "include/core/SkYUVAPixmaps.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrBackendSurface.h"  // IWYU pragma: keep
#include "include/gpu/GrContextThreadSafeProxy.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/gpu/GrTypes.h"
#include "include/gpu/GrYUVABackendTextures.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "include/private/base/SkAssert.h"
#include "include/private/chromium/SkImageChromium.h"
#include "include/private/gpu/ganesh/GrImageContext.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/core/SkImageInfoPriv.h"
#include "src/core/SkSamplingPriv.h"
#include "src/gpu/RefCntedCallback.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/Swizzle.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrColorInfo.h"
#include "src/gpu/ganesh/GrColorSpaceXform.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/GrImageContextPriv.h"
#include "src/gpu/ganesh/GrImageInfo.h"
#include "src/gpu/ganesh/GrProxyProvider.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrSamplerState.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/gpu/ganesh/GrTextureProxy.h"
#include "src/gpu/ganesh/SkGr.h"
#include "src/gpu/ganesh/SurfaceFillContext.h"
#include "src/gpu/ganesh/effects/GrBicubicEffect.h"
#include "src/gpu/ganesh/effects/GrYUVtoRGBEffect.h"
#include "src/image/SkImage_Base.h"

#include <algorithm>
#include <utility>

enum class SkTileMode;
struct SkRect;

static constexpr auto kAssumedColorType = kRGBA_8888_SkColorType;

SkImage_GaneshYUVA::SkImage_GaneshYUVA(sk_sp<GrImageContext> context,
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

// For onMakeColorTypeAndColorSpace() / onReinterpretColorSpace()
SkImage_GaneshYUVA::SkImage_GaneshYUVA(sk_sp<GrImageContext> context,
                                       const SkImage_GaneshYUVA* image,
                                       sk_sp<SkColorSpace> targetCS,
                                       ColorSpaceMode csMode)
        : INHERITED(std::move(context),
                    image->imageInfo().makeColorSpace(std::move(targetCS)),
                    kNeedNewImageUniqueID)
        , fYUVAProxies(image->fYUVAProxies)
        // If we're *reinterpreting* in a new color space, leave fFromColorSpace null.
        // If we're *converting* to a new color space, it must be non-null, so turn null into sRGB.
        , fFromColorSpace(csMode == ColorSpaceMode::kReinterpret
                                  ? nullptr
                                  : (image->colorSpace() ? image->refColorSpace()
                                                         : SkColorSpace::MakeSRGB())) {}

bool SkImage_GaneshYUVA::setupMipmapsForPlanes(GrRecordingContext* context) const {
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
    fYUVAProxies =
            GrYUVATextureProxies(fYUVAProxies.yuvaInfo(), newProxies, fYUVAProxies.textureOrigin());
    SkASSERT(fYUVAProxies.isValid());
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

GrSemaphoresSubmitted SkImage_GaneshYUVA::flush(GrDirectContext* dContext,
                                                const GrFlushInfo& info) const {
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
    return dContext->priv().flushSurfaces(
            {proxies, numProxies}, SkSurfaces::BackendSurfaceAccess::kNoAccess, info);
}

bool SkImage_GaneshYUVA::onHasMipmaps() const {
    return fYUVAProxies.mipmapped() == GrMipmapped::kYes;
}

bool SkImage_GaneshYUVA::onIsProtected() const {
    skgpu::Protected isProtected = fYUVAProxies.proxy(0)->isProtected();

#if defined(SK_DEBUG)
    for (int i = 1; i < fYUVAProxies.numPlanes(); ++i) {
        SkASSERT(isProtected == fYUVAProxies.proxy(i)->isProtected());
    }
#endif

    return isProtected == skgpu::Protected::kYes;
}


size_t SkImage_GaneshYUVA::textureSize() const {
    size_t size = 0;
    for (int i = 0; i < fYUVAProxies.numPlanes(); ++i) {
        size += fYUVAProxies.proxy(i)->gpuMemorySize();
    }
    return size;
}

sk_sp<SkImage> SkImage_GaneshYUVA::onMakeColorTypeAndColorSpace(SkColorType,
                                                                sk_sp<SkColorSpace> targetCS,
                                                                GrDirectContext* direct) const {
    // We explicitly ignore color type changes, for now.

    // we may need a mutex here but for now we expect usage to be in a single thread
    if (fOnMakeColorSpaceTarget &&
        SkColorSpace::Equals(targetCS.get(), fOnMakeColorSpaceTarget.get())) {
        return fOnMakeColorSpaceResult;
    }
    sk_sp<SkImage> result = sk_sp<SkImage>(
            new SkImage_GaneshYUVA(sk_ref_sp(direct), this, targetCS, ColorSpaceMode::kConvert));
    if (result) {
        fOnMakeColorSpaceTarget = targetCS;
        fOnMakeColorSpaceResult = result;
    }
    return result;
}

sk_sp<SkImage> SkImage_GaneshYUVA::onReinterpretColorSpace(sk_sp<SkColorSpace> newCS) const {
    return sk_sp<SkImage>(
            new SkImage_GaneshYUVA(fContext, this, std::move(newCS), ColorSpaceMode::kReinterpret));
}

std::tuple<GrSurfaceProxyView, GrColorType> SkImage_GaneshYUVA::asView(
        GrRecordingContext* rContext, GrMipmapped mipmapped, GrImageTexGenPolicy) const {
    if (!fContext->priv().matches(rContext)) {
        return {};
    }
    auto sfc = rContext->priv().makeSFC(this->imageInfo(),
                                        "Image_GpuYUVA_ReinterpretColorSpace",
                                        SkBackingFit::kExact,
                                        /*sample count*/ 1,
                                        mipmapped,
                                        GrProtected::kNo,
                                        fYUVAProxies.textureOrigin(),
                                        skgpu::Budgeted::kYes);
    if (!sfc) {
        return {};
    }

    const GrCaps& caps = *rContext->priv().caps();
    auto fp = GrYUVtoRGBEffect::Make(fYUVAProxies, GrSamplerState::Filter::kNearest, caps);
    if (fFromColorSpace) {
        fp = GrColorSpaceXformEffect::Make(std::move(fp),
                                           fFromColorSpace.get(),
                                           this->alphaType(),
                                           this->colorSpace(),
                                           this->alphaType());
    }
    sfc->fillWithFP(std::move(fp));

    return {sfc->readSurfaceView(), sfc->colorInfo().colorType()};
}

std::unique_ptr<GrFragmentProcessor> SkImage_GaneshYUVA::asFragmentProcessor(
        GrRecordingContext* context,
        SkSamplingOptions sampling,
        const SkTileMode tileModes[2],
        const SkMatrix& m,
        const SkRect* subset,
        const SkRect* domain) const {
    if (!fContext->priv().matches(context)) {
        return {};
    }
    // At least for now we do not attempt aniso filtering on YUVA images.
    if (sampling.isAniso()) {
        sampling = SkSamplingPriv::AnisoFallback(fYUVAProxies.mipmapped() == GrMipmapped::kYes);
    }

    auto wmx = SkTileModeToWrapMode(tileModes[0]);
    auto wmy = SkTileModeToWrapMode(tileModes[1]);
    GrSamplerState sampler(wmx, wmy, sampling.filter, sampling.mipmap);
    if (sampler.mipmapped() == GrMipmapped::kYes && !this->setupMipmapsForPlanes(context)) {
        sampler = GrSamplerState(sampler.wrapModeX(),
                                 sampler.wrapModeY(),
                                 sampler.filter(),
                                 GrSamplerState::MipmapMode::kNone);
    }

    const auto& yuvM = sampling.useCubic ? SkMatrix::I() : m;
    auto fp = GrYUVtoRGBEffect::Make(
            fYUVAProxies, sampler, *context->priv().caps(), yuvM, subset, domain);
    if (sampling.useCubic) {
        fp = GrBicubicEffect::Make(std::move(fp),
                                   this->alphaType(),
                                   m,
                                   sampling.cubic,
                                   GrBicubicEffect::Direction::kXY);
    }
    if (fFromColorSpace) {
        fp = GrColorSpaceXformEffect::Make(std::move(fp),
                                           fFromColorSpace.get(),
                                           this->alphaType(),
                                           this->colorSpace(),
                                           this->alphaType());
    }
    return fp;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
namespace SkImages {
sk_sp<SkImage> TextureFromYUVATextures(GrRecordingContext* context,
                                       const GrYUVABackendTextures& yuvaTextures) {
    return TextureFromYUVATextures(context, yuvaTextures, nullptr, nullptr, nullptr);
}

sk_sp<SkImage> TextureFromYUVATextures(GrRecordingContext* context,
                                       const GrYUVABackendTextures& yuvaTextures,
                                       sk_sp<SkColorSpace> imageColorSpace,
                                       TextureReleaseProc textureReleaseProc,
                                       ReleaseContext releaseContext) {
    auto releaseHelper = skgpu::RefCntedCallback::Make(textureReleaseProc, releaseContext);

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
    GrYUVATextureProxies yuvaProxies(
            yuvaTextures.yuvaInfo(), proxies, yuvaTextures.textureOrigin());

    if (!yuvaProxies.isValid()) {
        return nullptr;
    }

    return sk_make_sp<SkImage_GaneshYUVA>(
            sk_ref_sp(context), kNeedNewImageUniqueID, yuvaProxies, imageColorSpace);
}

sk_sp<SkImage> TextureFromYUVAPixmaps(GrRecordingContext* context,
                                      const SkYUVAPixmaps& pixmaps,
                                      GrMipmapped buildMips,
                                      bool limitToMaxTextureSize) {
    return TextureFromYUVAPixmaps(context, pixmaps, buildMips, limitToMaxTextureSize, nullptr);
}

sk_sp<SkImage> TextureFromYUVAPixmaps(GrRecordingContext* context,
                                      const SkYUVAPixmaps& pixmaps,
                                      GrMipmapped buildMips,
                                      bool limitToMaxTextureSize,
                                      sk_sp<SkColorSpace> imageColorSpace) {
    if (!context) {
        return nullptr;
    }

    if (!pixmaps.isValid()) {
        return nullptr;
    }

    if (!context->priv().caps()->mipmapSupport()) {
        buildMips = GrMipmapped::kNo;
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
        float scale = static_cast<float>(maxTextureSize) / maxDim;
        SkISize newDimensions = {
                std::min(static_cast<int>(pixmaps.yuvaInfo().width() * scale), maxTextureSize),
                std::min(static_cast<int>(pixmaps.yuvaInfo().height() * scale), maxTextureSize)};
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
    return sk_make_sp<SkImage_GaneshYUVA>(sk_ref_sp(context),
                                          kNeedNewImageUniqueID,
                                          std::move(yuvaProxies),
                                          std::move(imageColorSpace));
}

sk_sp<SkImage> PromiseTextureFromYUVA(sk_sp<GrContextThreadSafeProxy> threadSafeProxy,
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
    sk_sp<skgpu::RefCntedCallback> releaseHelpers[4];
    for (int i = 0; i < n; ++i) {
        releaseHelpers[i] = skgpu::RefCntedCallback::Make(textureReleaseProc, textureContexts[i]);
    }

    if (!threadSafeProxy) {
        return nullptr;
    }

    SkAlphaType at =
            backendTextureInfo.yuvaInfo().hasAlpha() ? kPremul_SkAlphaType : kOpaque_SkAlphaType;
    SkImageInfo info = SkImageInfo::Make(
            backendTextureInfo.yuvaInfo().dimensions(), kAssumedColorType, at, imageColorSpace);
    if (!SkImageInfoIsValid(info)) {
        return nullptr;
    }

    // Make a lazy proxy for each plane
    sk_sp<GrSurfaceProxy> proxies[4];
    for (int i = 0; i < n; ++i) {
        proxies[i] =
                SkImage_GaneshBase::MakePromiseImageLazyProxy(threadSafeProxy.get(),
                                                              planeDimensions[i],
                                                              backendTextureInfo.planeFormat(i),
                                                              GrMipmapped::kNo,
                                                              textureFulfillProc,
                                                              std::move(releaseHelpers[i]));
        if (!proxies[i]) {
            return nullptr;
        }
    }
    GrYUVATextureProxies yuvaTextureProxies(
            backendTextureInfo.yuvaInfo(), proxies, backendTextureInfo.textureOrigin());
    SkASSERT(yuvaTextureProxies.isValid());
    sk_sp<GrImageContext> ctx(GrImageContextPriv::MakeForPromiseImage(std::move(threadSafeProxy)));
    return sk_make_sp<SkImage_GaneshYUVA>(std::move(ctx),
                                          kNeedNewImageUniqueID,
                                          std::move(yuvaTextureProxies),
                                          std::move(imageColorSpace));
}
}  // namespace SkImages
