/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/GrRecordingContextPriv.h"

#include "include/core/SkColorSpace.h"
#include "src/gpu/ganesh/Device.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrDrawingManager.h"
#include "src/gpu/ganesh/GrProxyProvider.h"
#include "src/gpu/ganesh/GrRenderTargetProxy.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/gpu/ganesh/SurfaceDrawContext.h"
#include "src/gpu/ganesh/SurfaceFillContext.h"

void GrRecordingContextPriv::addOnFlushCallbackObject(GrOnFlushCallbackObject* onFlushCBObject) {
    this->context()->addOnFlushCallbackObject(onFlushCBObject);
}

sk_sp<skgpu::ganesh::Device> GrRecordingContextPriv::createDevice(
        GrColorType colorType,
        sk_sp<GrSurfaceProxy> proxy,
        sk_sp<SkColorSpace> colorSpace,
        GrSurfaceOrigin origin,
        const SkSurfaceProps& props,
        skgpu::ganesh::Device::InitContents init) {
    return skgpu::ganesh::Device::Make(this->context(),
                                       colorType,
                                       std::move(proxy),
                                       std::move(colorSpace),
                                       origin,
                                       props,
                                       init);
}

sk_sp<skgpu::ganesh::Device> GrRecordingContextPriv::createDevice(
        skgpu::Budgeted budgeted,
        const SkImageInfo& ii,
        SkBackingFit fit,
        int sampleCount,
        skgpu::Mipmapped mipmapped,
        GrProtected isProtected,
        GrSurfaceOrigin origin,
        const SkSurfaceProps& props,
        skgpu::ganesh::Device::InitContents init) {
    return skgpu::ganesh::Device::Make(this->context(),
                                       budgeted,
                                       ii,
                                       fit,
                                       sampleCount,
                                       mipmapped,
                                       isProtected,
                                       origin,
                                       props,
                                       init);
}

void GrRecordingContextPriv::moveRenderTasksToDDL(GrDeferredDisplayList* ddl) {
    this->context()->drawingManager()->moveRenderTasksToDDL(ddl);
}

sktext::gpu::SDFTControl GrRecordingContextPriv::getSDFTControl(bool useSDFTForSmallText) const {
#if !defined(SK_DISABLE_SDF_TEXT)
    return sktext::gpu::SDFTControl{
            this->caps()->shaderCaps()->supportsDistanceFieldText(),
            useSDFTForSmallText,
            !this->caps()->disablePerspectiveSDFText(),
            this->options().fMinDistanceFieldFontSize,
            this->options().fGlyphsAsPathsFontSize};
#else
    return sktext::gpu::SDFTControl{};
#endif
}

std::unique_ptr<skgpu::ganesh::SurfaceContext> GrRecordingContextPriv::makeSC(
        GrSurfaceProxyView readView, const GrColorInfo& info) {
    // It is probably not necessary to check if the context is abandoned here since uses of the
    // SurfaceContext which need the context will mostly likely fail later on w/o an issue.
    // However having this here adds some reassurance in case there is a path that doesn't
    // handle an abandoned context correctly. It also lets us early out of some extra work.
    if (this->context()->abandoned()) {
        return nullptr;
    }
    GrSurfaceProxy* proxy = readView.proxy();
    SkASSERT(proxy && proxy->asTextureProxy());

    std::unique_ptr<skgpu::ganesh::SurfaceContext> sc;
    if (proxy->asRenderTargetProxy()) {
        // Will we ever want a swizzle that is not the default write swizzle for the format and
        // colorType here? If so we will need to manually pass that in.
        skgpu::Swizzle writeSwizzle;
        if (info.colorType() != GrColorType::kUnknown) {
            writeSwizzle = this->caps()->getWriteSwizzle(proxy->backendFormat(),
                                                         info.colorType());
        }
        GrSurfaceProxyView writeView(readView.refProxy(), readView.origin(), writeSwizzle);
        if (info.alphaType() == kPremul_SkAlphaType ||
            info.alphaType() == kOpaque_SkAlphaType) {
            sc = std::make_unique<skgpu::ganesh::SurfaceDrawContext>(this->context(),
                                                                     std::move(readView),
                                                                     std::move(writeView),
                                                                     info.colorType(),
                                                                     info.refColorSpace(),
                                                                     SkSurfaceProps());
        } else {
            sc = std::make_unique<skgpu::ganesh::SurfaceFillContext>(
                    this->context(), std::move(readView), std::move(writeView), info);
        }
    } else {
        sc = std::make_unique<skgpu::ganesh::SurfaceContext>(
                this->context(), std::move(readView), info);
    }
    SkDEBUGCODE(sc->validate();)
    return sc;
}

std::unique_ptr<skgpu::ganesh::SurfaceContext> GrRecordingContextPriv::makeSC(
        const GrImageInfo& info,
        const GrBackendFormat& format,
        std::string_view label,
        SkBackingFit fit,
        GrSurfaceOrigin origin,
        GrRenderable renderable,
        int sampleCount,
        skgpu::Mipmapped mipmapped,
        GrProtected isProtected,
        skgpu::Budgeted budgeted) {
    SkASSERT(renderable == GrRenderable::kYes || sampleCount == 1);
    if (this->abandoned()) {
        return nullptr;
    }
    sk_sp<GrTextureProxy> proxy =
            this->proxyProvider()->createProxy(format,
                                               info.dimensions(),
                                               renderable,
                                               sampleCount,
                                               mipmapped,
                                               fit,
                                               budgeted,
                                               isProtected,
                                               label);
    if (!proxy) {
        return nullptr;
    }

    skgpu::Swizzle swizzle;
    if (info.colorType() != GrColorType::kUnknown &&
        !this->caps()->isFormatCompressed(format)) {
        swizzle = this->caps()->getReadSwizzle(format, info.colorType());
    }

    GrSurfaceProxyView view(std::move(proxy), origin, swizzle);
    return this->makeSC(std::move(view), info.colorInfo());
}

std::unique_ptr<skgpu::ganesh::SurfaceFillContext> GrRecordingContextPriv::makeSFC(
        GrImageInfo info,
        std::string_view label,
        SkBackingFit fit,
        int sampleCount,
        skgpu::Mipmapped mipmapped,
        GrProtected isProtected,
        GrSurfaceOrigin origin,
        skgpu::Budgeted budgeted) {
    if (info.alphaType() == kPremul_SkAlphaType || info.alphaType() == kOpaque_SkAlphaType) {
        return skgpu::ganesh::SurfaceDrawContext::Make(this->context(),
                                                       info.colorType(),
                                                       info.refColorSpace(),
                                                       fit,
                                                       info.dimensions(),
                                                       SkSurfaceProps(),
                                                       label,
                                                       sampleCount,
                                                       mipmapped,
                                                       isProtected,
                                                       origin,
                                                       budgeted);
    }
    GrBackendFormat format = this->caps()->getDefaultBackendFormat(info.colorType(),
                                                                   GrRenderable::kYes);
    sk_sp<GrTextureProxy> proxy =
            this->proxyProvider()->createProxy(format,
                                               info.dimensions(),
                                               GrRenderable::kYes,
                                               sampleCount,
                                               mipmapped,
                                               fit,
                                               budgeted,
                                               isProtected,
                                               label);
    if (!proxy) {
        return nullptr;
    }
    skgpu::Swizzle readSwizzle  = this->caps()->getReadSwizzle (format, info.colorType());
    skgpu::Swizzle writeSwizzle = this->caps()->getWriteSwizzle(format, info.colorType());

    GrSurfaceProxyView readView(            proxy, origin,  readSwizzle);
    GrSurfaceProxyView writeView(std::move(proxy), origin, writeSwizzle);
    std::unique_ptr<skgpu::ganesh::SurfaceFillContext> sfc;
    sfc = std::make_unique<skgpu::ganesh::SurfaceFillContext>(
            this->context(), std::move(readView), std::move(writeView), info.colorInfo());
    sfc->discard();
    return sfc;
}

std::unique_ptr<skgpu::ganesh::SurfaceFillContext> GrRecordingContextPriv::makeSFC(
        SkAlphaType alphaType,
        sk_sp<SkColorSpace> colorSpace,
        SkISize dimensions,
        SkBackingFit fit,
        const GrBackendFormat& format,
        int sampleCount,
        skgpu::Mipmapped mipmapped,
        GrProtected isProtected,
        skgpu::Swizzle readSwizzle,
        skgpu::Swizzle writeSwizzle,
        GrSurfaceOrigin origin,
        skgpu::Budgeted budgeted,
        std::string_view label) {
    SkASSERT(!dimensions.isEmpty());
    SkASSERT(sampleCount >= 1);
    SkASSERT(format.isValid() && format.backend() == fContext->backend());
    if (alphaType == kPremul_SkAlphaType || alphaType == kOpaque_SkAlphaType) {
        return skgpu::ganesh::SurfaceDrawContext::Make(this->context(),
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
                                                       SkSurfaceProps(),
                                                       label);
    }

    sk_sp<GrTextureProxy> proxy =
            this->proxyProvider()->createProxy(format,
                                               dimensions,
                                               GrRenderable::kYes,
                                               sampleCount,
                                               mipmapped,
                                               fit,
                                               budgeted,
                                               isProtected,
                                               label);
    if (!proxy) {
        return nullptr;
    }
    GrImageInfo info(GrColorType::kUnknown, alphaType, std::move(colorSpace), dimensions);
    GrSurfaceProxyView readView(            proxy, origin,  readSwizzle);
    GrSurfaceProxyView writeView(std::move(proxy), origin, writeSwizzle);
    std::unique_ptr<skgpu::ganesh::SurfaceFillContext> sfc;
    sfc = std::make_unique<skgpu::ganesh::SurfaceFillContext>(
            this->context(), std::move(readView), std::move(writeView), info.colorInfo());
    sfc->discard();
    return sfc;
}

std::unique_ptr<skgpu::ganesh::SurfaceFillContext> GrRecordingContextPriv::makeSFCWithFallback(
        GrImageInfo info,
        SkBackingFit fit,
        int sampleCount,
        skgpu::Mipmapped mipmapped,
        GrProtected isProtected,
        GrSurfaceOrigin origin,
        skgpu::Budgeted budgeted) {
    if (info.alphaType() == kPremul_SkAlphaType || info.alphaType() == kOpaque_SkAlphaType) {
        return skgpu::ganesh::SurfaceDrawContext::MakeWithFallback(this->context(),
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
    return this->makeSFC(info, "MakeSurfaceContextWithFallback",
                         fit,
                         sampleCount,
                         mipmapped,
                         isProtected,
                         origin,
                         budgeted);
}

std::unique_ptr<skgpu::ganesh::SurfaceFillContext>
GrRecordingContextPriv::makeSFCFromBackendTexture(GrColorInfo info,
                                                  const GrBackendTexture& tex,
                                                  int sampleCount,
                                                  GrSurfaceOrigin origin,
                                                  sk_sp<skgpu::RefCntedCallback> releaseHelper) {
    SkASSERT(sampleCount > 0);

    if (info.alphaType() == kPremul_SkAlphaType || info.alphaType() == kOpaque_SkAlphaType) {
        return skgpu::ganesh::SurfaceDrawContext::MakeFromBackendTexture(this->context(),
                                                                         info.colorType(),
                                                                         info.refColorSpace(),
                                                                         tex,
                                                                         sampleCount,
                                                                         origin,
                                                                         SkSurfaceProps(),
                                                                         std::move(releaseHelper));
    }

    if (info.colorType() == GrColorType::kUnknown) {
        return nullptr;
    }

    const GrBackendFormat& format = tex.getBackendFormat();
    if (!this->caps()->areColorTypeAndFormatCompatible(info.colorType(), format)) {
        return nullptr;
    }
    skgpu::Swizzle readSwizzle  = this->caps()->getReadSwizzle (format, info.colorType());
    skgpu::Swizzle writeSwizzle = this->caps()->getWriteSwizzle(format, info.colorType());

    sk_sp<GrTextureProxy> proxy(this->proxyProvider()->wrapRenderableBackendTexture(
            tex, sampleCount, kBorrow_GrWrapOwnership, GrWrapCacheable::kNo,
            std::move(releaseHelper)));
    if (!proxy) {
        return nullptr;
    }

    GrSurfaceProxyView readView(            proxy, origin,  readSwizzle);
    GrSurfaceProxyView writeView(std::move(proxy), origin, writeSwizzle);

    return std::make_unique<skgpu::ganesh::SurfaceFillContext>(
            this->context(), std::move(readView), std::move(writeView), std::move(info));
}
