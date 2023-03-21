/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/image/SkImage_Base.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRect.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkSize.h"
#include "include/core/SkTypes.h"
#include "src/core/SkBitmapCache.h"
#include "src/core/SkSamplingPriv.h"
#include "src/image/SkRescaleAndReadPixels.h"

#include <atomic>
#include <string_view>
#include <tuple>
#include <utility>

#if defined(SK_GANESH)
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/private/gpu/ganesh/GrImageContext.h"
#include "src/gpu/ResourceKey.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/GrProxyProvider.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrSamplerState.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/gpu/ganesh/GrTextureProxy.h"
#include "src/gpu/ganesh/SkGr.h"
#include "src/gpu/ganesh/effects/GrBicubicEffect.h"
#include "src/gpu/ganesh/effects/GrTextureEffect.h"
enum class GrColorType;
#endif

#if defined(SK_GRAPHITE)
#include "src/core/SkColorSpacePriv.h"
#include "src/gpu/graphite/Image_Graphite.h"
#include "src/gpu/graphite/Log.h"
#endif

SkImage_Base::SkImage_Base(const SkImageInfo& info, uint32_t uniqueID)
        : SkImage(info, uniqueID), fAddedToRasterCache(false) {}

SkImage_Base::~SkImage_Base() {
    if (fAddedToRasterCache.load()) {
        SkNotifyBitmapGenIDIsStale(this->uniqueID());
    }
}

void SkImage_Base::onAsyncRescaleAndReadPixels(const SkImageInfo& info,
                                               SkIRect origSrcRect,
                                               RescaleGamma rescaleGamma,
                                               RescaleMode rescaleMode,
                                               ReadPixelsCallback callback,
                                               ReadPixelsContext context) const {
    SkBitmap src;
    SkPixmap peek;
    SkIRect srcRect;
    if (this->peekPixels(&peek)) {
        src.installPixels(peek);
        srcRect = origSrcRect;
    } else {
        // Context TODO: Elevate GrDirectContext requirement to public API.
        auto dContext = as_IB(this)->directContext();
        src.setInfo(this->imageInfo().makeDimensions(origSrcRect.size()));
        src.allocPixels();
        if (!this->readPixels(dContext, src.pixmap(), origSrcRect.x(), origSrcRect.y())) {
            callback(context, nullptr);
            return;
        }
        srcRect = SkIRect::MakeSize(src.dimensions());
    }
    return SkRescaleAndReadPixels(src, info, srcRect, rescaleGamma, rescaleMode, callback, context);
}

void SkImage_Base::onAsyncRescaleAndReadPixelsYUV420(SkYUVColorSpace,
                                                     sk_sp<SkColorSpace> dstColorSpace,
                                                     SkIRect srcRect,
                                                     SkISize dstSize,
                                                     RescaleGamma,
                                                     RescaleMode,
                                                     ReadPixelsCallback callback,
                                                     ReadPixelsContext context) const {
    // TODO: Call non-YUV asyncRescaleAndReadPixels and then make our callback convert to YUV and
    // call client's callback.
    callback(context, nullptr);
}

#if defined(SK_GANESH)
std::tuple<GrSurfaceProxyView, GrColorType> SkImage_Base::asView(GrRecordingContext* context,
                                                                 GrMipmapped mipmapped,
                                                                 GrImageTexGenPolicy policy) const {
    if (!context) {
        return {};
    }
    if (!context->priv().caps()->mipmapSupport() || this->dimensions().area() <= 1) {
        mipmapped = GrMipmapped::kNo;
    }
    return this->onAsView(context, mipmapped, policy);
}

std::unique_ptr<GrFragmentProcessor> SkImage_Base::asFragmentProcessor(
        GrRecordingContext* rContext,
        SkSamplingOptions sampling,
        const SkTileMode tileModes[2],
        const SkMatrix& m,
        const SkRect* subset,
        const SkRect* domain) const {
    if (!rContext) {
        return {};
    }
    if (sampling.useCubic && !GrValidCubicResampler(sampling.cubic)) {
        return {};
    }
    if (sampling.mipmap != SkMipmapMode::kNone &&
        (!rContext->priv().caps()->mipmapSupport() || this->dimensions().area() <= 1)) {
        sampling = SkSamplingOptions(sampling.filter);
    }
    return this->onAsFragmentProcessor(rContext, sampling, tileModes, m, subset, domain);
}

std::unique_ptr<GrFragmentProcessor> SkImage_Base::MakeFragmentProcessorFromView(
        GrRecordingContext* rContext,
        GrSurfaceProxyView view,
        SkAlphaType at,
        SkSamplingOptions sampling,
        const SkTileMode tileModes[2],
        const SkMatrix& m,
        const SkRect* subset,
        const SkRect* domain) {
    if (!view) {
        return nullptr;
    }
    const GrCaps& caps = *rContext->priv().caps();
    auto wmx = SkTileModeToWrapMode(tileModes[0]);
    auto wmy = SkTileModeToWrapMode(tileModes[1]);
    if (sampling.useCubic) {
        if (subset) {
            if (domain) {
                return GrBicubicEffect::MakeSubset(std::move(view),
                                                   at,
                                                   m,
                                                   wmx,
                                                   wmy,
                                                   *subset,
                                                   *domain,
                                                   sampling.cubic,
                                                   GrBicubicEffect::Direction::kXY,
                                                   *rContext->priv().caps());
            }
            return GrBicubicEffect::MakeSubset(std::move(view),
                                               at,
                                               m,
                                               wmx,
                                               wmy,
                                               *subset,
                                               sampling.cubic,
                                               GrBicubicEffect::Direction::kXY,
                                               *rContext->priv().caps());
        }
        return GrBicubicEffect::Make(std::move(view),
                                     at,
                                     m,
                                     wmx,
                                     wmy,
                                     sampling.cubic,
                                     GrBicubicEffect::Direction::kXY,
                                     *rContext->priv().caps());
    }
    if (sampling.isAniso()) {
        if (!rContext->priv().caps()->anisoSupport()) {
            // Fallback to linear
            sampling = SkSamplingPriv::AnisoFallback(view.mipmapped() == GrMipmapped::kYes);
        }
    } else if (view.mipmapped() == GrMipmapped::kNo) {
        sampling = SkSamplingOptions(sampling.filter);
    }
    GrSamplerState sampler;
    if (sampling.isAniso()) {
        sampler = GrSamplerState::Aniso(wmx, wmy, sampling.maxAniso, view.mipmapped());
    } else {
        sampler = GrSamplerState(wmx, wmy, sampling.filter, sampling.mipmap);
    }
    if (subset) {
        if (domain) {
            return GrTextureEffect::MakeSubset(std::move(view),
                                               at,
                                               m,
                                               sampler,
                                               *subset,
                                               *domain,
                                               caps);
        }
        return GrTextureEffect::MakeSubset(std::move(view),
                                           at,
                                           m,
                                           sampler,
                                           *subset,
                                           caps);
    } else {
        return GrTextureEffect::Make(std::move(view), at, m, sampler, caps);
    }
}

GrSurfaceProxyView SkImage_Base::FindOrMakeCachedMipmappedView(GrRecordingContext* rContext,
                                                               GrSurfaceProxyView view,
                                                               uint32_t imageUniqueID) {
    SkASSERT(rContext);
    SkASSERT(imageUniqueID != SK_InvalidUniqueID);

    if (!view || view.proxy()->asTextureProxy()->mipmapped() == GrMipmapped::kYes) {
        return view;
    }
    GrProxyProvider* proxyProvider = rContext->priv().proxyProvider();

    skgpu::UniqueKey baseKey;
    GrMakeKeyFromImageID(&baseKey, imageUniqueID, SkIRect::MakeSize(view.dimensions()));
    SkASSERT(baseKey.isValid());
    skgpu::UniqueKey mipmappedKey;
    static const skgpu::UniqueKey::Domain kMipmappedDomain = skgpu::UniqueKey::GenerateDomain();
    {  // No extra values beyond the domain are required. Must name the var to please
       // clang-tidy.
        skgpu::UniqueKey::Builder b(&mipmappedKey, baseKey, kMipmappedDomain, 0);
    }
    SkASSERT(mipmappedKey.isValid());
    if (sk_sp<GrTextureProxy> cachedMippedView =
                proxyProvider->findOrCreateProxyByUniqueKey(mipmappedKey)) {
        return {std::move(cachedMippedView), view.origin(), view.swizzle()};
    }

    auto copy = GrCopyBaseMipMapToView(rContext, view);
    if (!copy) {
        return view;
    }
    // TODO: If we move listeners up from SkImage_Lazy to SkImage_Base then add one here.
    proxyProvider->assignUniqueKeyToProxy(mipmappedKey, copy.asTextureProxy());
    return copy;
}

GrBackendTexture SkImage_Base::onGetBackendTexture(bool flushPendingGrContextIO,
                                                   GrSurfaceOrigin* origin) const {
    return GrBackendTexture(); // invalid
}

GrSurfaceProxyView SkImage_Base::CopyView(GrRecordingContext* context,
                                                 GrSurfaceProxyView src,
                                                 GrMipmapped mipmapped,
                                                 GrImageTexGenPolicy policy,
                                                 std::string_view label) {
    skgpu::Budgeted budgeted = policy == GrImageTexGenPolicy::kNew_Uncached_Budgeted
                                       ? skgpu::Budgeted::kYes
                                       : skgpu::Budgeted::kNo;
    return GrSurfaceProxyView::Copy(context,
                                    std::move(src),
                                    mipmapped,
                                    SkBackingFit::kExact,
                                    budgeted,
                                    /*label=*/label);
}

#endif // defined(SK_GANESH)

#if defined(SK_GRAPHITE)
std::tuple<skgpu::graphite::TextureProxyView, SkColorType> SkImage_Base::asView(
        skgpu::graphite::Recorder* recorder,
        skgpu::Mipmapped mipmapped) const {
    if (!recorder) {
        return {};
    }

    if (!as_IB(this)->isGraphiteBacked()) {
        return {};
    }
    // TODO(b/238756380): YUVA not supported yet
    if (as_IB(this)->isYUVA()) {
        return {};
    }

    auto image = reinterpret_cast<const skgpu::graphite::Image*>(this);

    if (this->dimensions().area() <= 1) {
        mipmapped = skgpu::Mipmapped::kNo;
    }

    if (mipmapped == skgpu::Mipmapped::kYes &&
        image->textureProxyView().proxy()->mipmapped() != skgpu::Mipmapped::kYes) {
        SKGPU_LOG_W("Graphite does not auto-generate mipmap levels");
        return {};
    }

    SkColorType ct = this->colorType();
    return { image->textureProxyView(), ct };
}

sk_sp<SkImage> SkImage::makeColorSpace(sk_sp<SkColorSpace> targetColorSpace,
                                       skgpu::graphite::Recorder* recorder,
                                       RequiredImageProperties requiredProps) const {
    return this->makeColorTypeAndColorSpace(this->colorType(), std::move(targetColorSpace),
                                            recorder, requiredProps);
}

sk_sp<SkImage> SkImage::makeColorTypeAndColorSpace(SkColorType targetColorType,
                                                   sk_sp<SkColorSpace> targetColorSpace,
                                                   skgpu::graphite::Recorder* recorder,
                                                   RequiredImageProperties requiredProps) const {
    if (kUnknown_SkColorType == targetColorType || !targetColorSpace) {
        return nullptr;
    }

    SkColorType colorType = this->colorType();
    SkColorSpace* colorSpace = this->colorSpace();
    if (!colorSpace) {
        colorSpace = sk_srgb_singleton();
    }
    if (colorType == targetColorType &&
        (SkColorSpace::Equals(colorSpace, targetColorSpace.get()) || this->isAlphaOnly())) {
        return sk_ref_sp(const_cast<SkImage*>(this));
    }

    return as_IB(this)->onMakeColorTypeAndColorSpace(targetColorType,
                                                     std::move(targetColorSpace),
                                                     recorder,
                                                     requiredProps);
}

#endif // SK_GRAPHITE

GrDirectContext* SkImage_Base::directContext() const {
#if defined(SK_GANESH)
    return GrAsDirectContext(this->context());
#else
    return nullptr;
#endif
}

bool SkImage_Base::onAsLegacyBitmap(GrDirectContext* dContext, SkBitmap* bitmap) const {
    // As the base-class, all we can do is make a copy (regardless of mode).
    // Subclasses that want to be more optimal should override.
    SkImageInfo info = fInfo.makeColorType(kN32_SkColorType).makeColorSpace(nullptr);
    if (!bitmap->tryAllocPixels(info)) {
        return false;
    }

    if (!this->readPixels(dContext, bitmap->info(), bitmap->getPixels(), bitmap->rowBytes(),
                          0, 0)) {
        bitmap->reset();
        return false;
    }

    bitmap->setImmutable();
    return true;
}
