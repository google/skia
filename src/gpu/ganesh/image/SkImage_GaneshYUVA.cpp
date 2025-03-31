/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/image/SkImage_GaneshYUVA.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkSurface.h"
#include "include/core/SkYUVAInfo.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/ganesh/GrBackendSurface.h"  // IWYU pragma: keep
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/GrRecordingContext.h"
#include "include/gpu/ganesh/GrTypes.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "include/private/gpu/ganesh/GrImageContext.h"
#include "src/core/SkSamplingPriv.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/Swizzle.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrColorInfo.h"
#include "src/gpu/ganesh/GrColorSpaceXform.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/GrImageContextPriv.h"
#include "src/gpu/ganesh/GrImageInfo.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrSamplerState.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/gpu/ganesh/GrTextureProxy.h"
#include "src/gpu/ganesh/SkGr.h"
#include "src/gpu/ganesh/SurfaceDrawContext.h"
#include "src/gpu/ganesh/SurfaceFillContext.h"
#include "src/gpu/ganesh/effects/GrBicubicEffect.h"
#include "src/gpu/ganesh/effects/GrYUVtoRGBEffect.h"
#include "src/image/SkImage_Base.h"

#include <utility>

enum class SkTileMode;
struct SkRect;

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
        if (t->mipmapped() == skgpu::Mipmapped::kNo && (t->width() > 1 || t->height() > 1)) {
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
    return fYUVAProxies.mipmapped() == skgpu::Mipmapped::kYes;
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

std::tuple<GrSurfaceProxyView, GrColorType> SkImage_GaneshYUVA::asView(GrRecordingContext* rContext,
                                                                       skgpu::Mipmapped mipmapped,
                                                                       GrImageTexGenPolicy) const {
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
        skgpu::ganesh::SurfaceDrawContext* sdc,
        SkSamplingOptions sampling,
        const SkTileMode tileModes[2],
        const SkMatrix& m,
        const SkRect* subset,
        const SkRect* domain) const {
    GrRecordingContext* context = sdc->recordingContext();
    if (!fContext->priv().matches(context)) {
        return {};
    }
    // At least for now we do not attempt aniso filtering on YUVA images.
    if (sampling.isAniso()) {
        sampling =
                SkSamplingPriv::AnisoFallback(fYUVAProxies.mipmapped() == skgpu::Mipmapped::kYes);
    }

    auto wmx = SkTileModeToWrapMode(tileModes[0]);
    auto wmy = SkTileModeToWrapMode(tileModes[1]);
    GrSamplerState sampler(wmx, wmy, sampling.filter, sampling.mipmap);
    if (sampler.mipmapped() == skgpu::Mipmapped::kYes && !this->setupMipmapsForPlanes(context)) {
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
