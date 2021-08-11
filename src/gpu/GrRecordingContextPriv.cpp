/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrRecordingContextPriv.h"

#include "src/gpu/GrCaps.h"
#include "src/gpu/GrDrawingManager.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRenderTargetProxy.h"
#include "src/gpu/GrSurfaceProxyView.h"

#if SK_GPU_V1
#include "src/gpu/v1/Device_v1.h"
#include "src/gpu/v1/SurfaceDrawContext_v1.h"
#include "src/gpu/v1/SurfaceFillContext_v1.h"
#endif
#if SK_GPU_V2
#include "src/gpu/v2/Device_v2.h"
#include "src/gpu/v2/SurfaceDrawContext_v2.h"
#include "src/gpu/v2/SurfaceFillContext_v2.h"
#endif

void GrRecordingContextPriv::addOnFlushCallbackObject(GrOnFlushCallbackObject* onFlushCBObject) {
    this->context()->addOnFlushCallbackObject(onFlushCBObject);
}

sk_sp<skgpu::BaseDevice> GrRecordingContextPriv::createDevice(GrColorType colorType,
                                                              sk_sp<GrSurfaceProxy> proxy,
                                                              sk_sp<SkColorSpace> colorSpace,
                                                              GrSurfaceOrigin origin,
                                                              const SkSurfaceProps& props,
                                                              skgpu::BaseDevice::InitContents init) {
#if GR_TEST_UTILS
    if (this->options().fUseSkGpuV2 == GrContextOptions::Enable::kYes) {
#if SK_GPU_V2
        return skgpu::v2::Device::Make(this->context(), colorType, std::move(proxy),
                                       std::move(colorSpace), origin, props, init);
#else
        return nullptr;
#endif // GR_TEST_UTILS
    } else
#endif
    {
#if SK_GPU_V1
        return skgpu::v1::Device::Make(this->context(), colorType, std::move(proxy),
                                       std::move(colorSpace), origin, props, init);
#else
        return nullptr;
#endif
    }
}

sk_sp<skgpu::BaseDevice> GrRecordingContextPriv::createDevice(SkBudgeted budgeted,
                                                              const SkImageInfo& ii,
                                                              SkBackingFit fit,
                                                              int sampleCount,
                                                              GrMipmapped mipmapped,
                                                              GrProtected isProtected,
                                                              GrSurfaceOrigin origin,
                                                              const SkSurfaceProps& props,
                                                              skgpu::BaseDevice::InitContents init) {
#if GR_TEST_UTILS
    if (this->options().fUseSkGpuV2 == GrContextOptions::Enable::kYes) {
#if SK_GPU_V2
        return skgpu::v2::Device::Make(this->context(), budgeted, ii, fit, sampleCount,
                                       mipmapped, isProtected, origin, props, init);
#else
        return nullptr;
#endif
    } else
#endif // GR_TEST_UTILS
    {
#if SK_GPU_V1
        return skgpu::v1::Device::Make(this->context(), budgeted, ii, fit, sampleCount,
                                       mipmapped, isProtected, origin, props, init);
#else
        return nullptr;
#endif
    }
}

void GrRecordingContextPriv::moveRenderTasksToDDL(SkDeferredDisplayList* ddl) {
    this->context()->drawingManager()->moveRenderTasksToDDL(ddl);
}

GrSDFTControl GrRecordingContextPriv::getSDFTControl(bool useSDFTForSmallText) const {
    return GrSDFTControl{
            this->caps()->shaderCaps()->supportsDistanceFieldText(),
            useSDFTForSmallText,
            this->options().fMinDistanceFieldFontSize,
            this->options().fGlyphsAsPathsFontSize};
}

std::unique_ptr<skgpu::SurfaceContext> GrRecordingContextPriv::makeSC(GrSurfaceProxyView readView,
                                                                      const GrColorInfo& info) {
#if GR_TEST_UTILS
    if (this->options().fUseSkGpuV2 == GrContextOptions::Enable::kYes) {
#if SK_GPU_V2
        return nullptr;
#endif
    } else
#endif // GR_TEST_UTILS
    {
#if SK_GPU_V1
        // It is probably not necessary to check if the context is abandoned here since uses of the
        // SurfaceContext which need the context will mostly likely fail later on w/o an issue.
        // However having this hear adds some reassurance in case there is a path doesn't handle an
        // abandoned context correctly. It also lets us early out of some extra work.
        if (this->context()->abandoned()) {
            return nullptr;
        }
        GrSurfaceProxy* proxy = readView.proxy();
        SkASSERT(proxy && proxy->asTextureProxy());

        std::unique_ptr<skgpu::SurfaceContext> sc;
        if (proxy->asRenderTargetProxy()) {
            // Will we ever want a swizzle that is not the default write swizzle for the format and
            // colorType here? If so we will need to manually pass that in.
            GrSwizzle writeSwizzle;
            if (info.colorType() != GrColorType::kUnknown) {
                writeSwizzle = this->caps()->getWriteSwizzle(proxy->backendFormat(),
                                                             info.colorType());
            }
            GrSurfaceProxyView writeView(readView.refProxy(), readView.origin(), writeSwizzle);
            if (info.alphaType() == kPremul_SkAlphaType ||
                info.alphaType() == kOpaque_SkAlphaType) {
                sc = std::make_unique<skgpu::v1::SurfaceDrawContext>(this->context(),
                                                                     std::move(readView),
                                                                     std::move(writeView),
                                                                     info.colorType(),
                                                                     info.refColorSpace(),
                                                                     SkSurfaceProps());
            } else {
                sc = std::make_unique<skgpu::v1::SurfaceFillContext>(this->context(),
                                                                     std::move(readView),
                                                                     std::move(writeView),
                                                                     info);
            }
        } else {
            sc = std::make_unique<skgpu::SurfaceContext>(this->context(),
                                                         std::move(readView),
                                                         info);
        }
        SkDEBUGCODE(sc->validate();)
        return sc;
#endif
    }

    return nullptr;
}

std::unique_ptr<skgpu::SurfaceFillContext> GrRecordingContextPriv::makeSFC(GrImageInfo info,
                                                                           SkBackingFit fit,
                                                                           int sampleCount,
                                                                           GrMipmapped mipmapped,
                                                                           GrProtected isProtected,
                                                                           GrSurfaceOrigin origin,
                                                                           SkBudgeted budgeted) {

#if GR_TEST_UTILS
    if (this->options().fUseSkGpuV2 == GrContextOptions::Enable::kYes) {
#if SK_GPU_V2
        return nullptr;
#endif
    } else
#endif // GR_TEST_UTILS
    {
#if SK_GPU_V1
        if (info.alphaType() == kPremul_SkAlphaType || info.alphaType() == kOpaque_SkAlphaType) {
            return skgpu::v1::SurfaceDrawContext::Make(this->context(),
                                                       info.colorType(),
                                                       info.refColorSpace(),
                                                       fit,
                                                       info.dimensions(),
                                                       SkSurfaceProps(),
                                                       sampleCount,
                                                       mipmapped,
                                                       isProtected,
                                                       origin,
                                                       budgeted);
        }
        GrBackendFormat format = this->caps()->getDefaultBackendFormat(info.colorType(),
                                                                       GrRenderable::kYes);
        sk_sp<GrTextureProxy> proxy = this->proxyProvider()->createProxy(format,
                                                                         info.dimensions(),
                                                                         GrRenderable::kYes,
                                                                         sampleCount,
                                                                         mipmapped,
                                                                         fit,
                                                                         budgeted,
                                                                         isProtected);
        if (!proxy) {
            return nullptr;
        }
        GrSwizzle readSwizzle  = this->caps()->getReadSwizzle (format, info.colorType());
        GrSwizzle writeSwizzle = this->caps()->getWriteSwizzle(format, info.colorType());

        GrSurfaceProxyView readView(            proxy, origin,  readSwizzle);
        GrSurfaceProxyView writeView(std::move(proxy), origin, writeSwizzle);
        std::unique_ptr<skgpu::SurfaceFillContext> sfc;
        sfc = std::make_unique<skgpu::v1::SurfaceFillContext>(this->context(),
                                                              std::move(readView),
                                                              std::move(writeView),
                                                              info.colorInfo());
        sfc->discard();
        return sfc;
#endif
    }

    return nullptr;
}

std::unique_ptr<skgpu::SurfaceFillContext> GrRecordingContextPriv::makeSFC(
        SkAlphaType alphaType,
        sk_sp<SkColorSpace> colorSpace,
        SkISize dimensions,
        SkBackingFit fit,
        const GrBackendFormat& format,
        int sampleCount,
        GrMipmapped mipmapped,
        GrProtected isProtected,
        GrSwizzle readSwizzle,
        GrSwizzle writeSwizzle,
        GrSurfaceOrigin origin,
        SkBudgeted budgeted) {

#if GR_TEST_UTILS
    if (this->context()->options().fUseSkGpuV2 == GrContextOptions::Enable::kYes) {
#if SK_GPU_V2
        return nullptr;
#endif
    } else
#endif // GR_TEST_UTILS
    {
#if SK_GPU_V1
        SkASSERT(!dimensions.isEmpty());
        SkASSERT(sampleCount >= 1);
        SkASSERT(format.isValid() && format.backend() == fContext->backend());
        if (alphaType == kPremul_SkAlphaType || alphaType == kOpaque_SkAlphaType) {
            return skgpu::v1::SurfaceDrawContext::Make(this->context(),
                                                       std::move(colorSpace),
                                                       fit,
                                                       dimensions,
                                                       format,
                                                       sampleCount,
                                                       mipmapped,
                                                       isProtected,
                                                       readSwizzle,
                                                       writeSwizzle,
                                                       origin,
                                                       budgeted,
                                                       SkSurfaceProps());
        }

        sk_sp<GrTextureProxy> proxy = this->proxyProvider()->createProxy(format,
                                                                         dimensions,
                                                                         GrRenderable::kYes,
                                                                         sampleCount,
                                                                         mipmapped,
                                                                         fit,
                                                                         budgeted,
                                                                         isProtected);
        if (!proxy) {
            return nullptr;
        }
        GrImageInfo info(GrColorType::kUnknown, alphaType, std::move(colorSpace), dimensions);
        GrSurfaceProxyView readView(            proxy, origin,  readSwizzle);
        GrSurfaceProxyView writeView(std::move(proxy), origin, writeSwizzle);
        std::unique_ptr<skgpu::SurfaceFillContext> sfc;
        sfc = std::make_unique<skgpu::v1::SurfaceFillContext>(this->context(),
                                                              std::move(readView),
                                                              std::move(writeView),
                                                              info.colorInfo());
        sfc->discard();
        return sfc;
#endif
    }

    return nullptr;
}

std::unique_ptr<skgpu::SurfaceFillContext> GrRecordingContextPriv::makeSFCWithFallback(
        GrImageInfo info,
        SkBackingFit fit,
        int sampleCount,
        GrMipmapped mipmapped,
        GrProtected isProtected,
        GrSurfaceOrigin origin,
        SkBudgeted budgeted) {

#if GR_TEST_UTILS
    if (this->options().fUseSkGpuV2 == GrContextOptions::Enable::kYes) {
#if SK_GPU_V2
        return nullptr;
#endif
    } else
#endif // GR_TEST_UTILS
    {
#if SK_GPU_V1
        if (info.alphaType() == kPremul_SkAlphaType || info.alphaType() == kOpaque_SkAlphaType) {
            return skgpu::v1::SurfaceDrawContext::MakeWithFallback(this->context(),
                                                                   info.colorType(),
                                                                   info.refColorSpace(),
                                                                   fit,
                                                                   info.dimensions(),
                                                                   SkSurfaceProps(),
                                                                   sampleCount,
                                                                   mipmapped,
                                                                   isProtected,
                                                                   origin,
                                                                   budgeted);
        }
        const GrCaps* caps = this->caps();

        auto [ct, _] = caps->getFallbackColorTypeAndFormat(info.colorType(), sampleCount);
        if (ct == GrColorType::kUnknown) {
            return nullptr;
        }
        info = info.makeColorType(ct);
        return this->makeSFC(info,
                             fit,
                             sampleCount,
                             mipmapped,
                             isProtected,
                             origin,
                             budgeted);
#endif
    }

    return nullptr;
}

std::unique_ptr<skgpu::SurfaceFillContext> GrRecordingContextPriv::makeSFCFromBackendTexture(
        GrColorInfo info,
        const GrBackendTexture& tex,
        int sampleCount,
        GrSurfaceOrigin origin,
        sk_sp<GrRefCntedCallback> releaseHelper) {

#if GR_TEST_UTILS
    if (this->options().fUseSkGpuV2 == GrContextOptions::Enable::kYes) {
#if SK_GPU_V2
        return nullptr;
#endif
    } else
#endif // GR_TEST_UTILS
    {
#if SK_GPU_V1
        SkASSERT(sampleCount > 0);

        if (info.alphaType() == kPremul_SkAlphaType || info.alphaType() == kOpaque_SkAlphaType) {
            return skgpu::v1::SurfaceDrawContext::MakeFromBackendTexture(this->context(),
                                                                         info.colorType(),
                                                                         info.refColorSpace(),
                                                                         tex,
                                                                         sampleCount,
                                                                         origin,
                                                                         SkSurfaceProps(),
                                                                         std::move(releaseHelper));
        }
        const GrBackendFormat& format = tex.getBackendFormat();
        GrSwizzle readSwizzle, writeSwizzle;
        if (info.colorType() != GrColorType::kUnknown) {
            if (!this->caps()->areColorTypeAndFormatCompatible(info.colorType(), format)) {
                return nullptr;
            }
            readSwizzle  = this->caps()->getReadSwizzle (format, info.colorType());
            writeSwizzle = this->caps()->getWriteSwizzle(format, info.colorType());
        }

        sk_sp<GrTextureProxy> proxy(this->proxyProvider()->wrapRenderableBackendTexture(
                tex, sampleCount, kBorrow_GrWrapOwnership, GrWrapCacheable::kNo,
                std::move(releaseHelper)));
        if (!proxy) {
            return nullptr;
        }

        GrSurfaceProxyView readView(            proxy, origin,  readSwizzle);
        GrSurfaceProxyView writeView(std::move(proxy), origin, writeSwizzle);

        return std::make_unique<skgpu::v1::SurfaceFillContext>(this->context(),
                                                               std::move(readView),
                                                               std::move(writeView),
                                                               std::move(info));
#endif
    }

    return nullptr;
}
