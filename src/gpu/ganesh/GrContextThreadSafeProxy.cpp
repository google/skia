/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrContextThreadSafeProxy.h"

#include "include/core/SkTextureCompressionType.h"
#include "include/private/chromium/GrSurfaceCharacterization.h"
#include "src/gpu/ganesh/GrBaseContextPriv.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrContextThreadSafeProxyPriv.h"
#include "src/gpu/ganesh/GrThreadSafeCache.h"
#include "src/gpu/ganesh/GrThreadSafePipelineBuilder.h"
#include "src/gpu/ganesh/effects/GrSkSLFP.h"
#include "src/gpu/ganesh/surface/SkSurface_Ganesh.h"

#include <memory>

static uint32_t next_id() {
    static std::atomic<uint32_t> nextID{1};
    uint32_t id;
    do {
        id = nextID.fetch_add(1, std::memory_order_relaxed);
    } while (id == SK_InvalidGenID);
    return id;
}

GrContextThreadSafeProxy::GrContextThreadSafeProxy(GrBackendApi backend,
                                                   const GrContextOptions& options)
        : fBackend(backend), fOptions(options), fContextID(next_id()) {
}

GrContextThreadSafeProxy::~GrContextThreadSafeProxy() = default;

void GrContextThreadSafeProxy::init(sk_sp<const GrCaps> caps,
                                    sk_sp<GrThreadSafePipelineBuilder> pipelineBuilder) {
    fCaps = std::move(caps);
    fTextBlobRedrawCoordinator =
            std::make_unique<sktext::gpu::TextBlobRedrawCoordinator>(fContextID);
    fThreadSafeCache = std::make_unique<GrThreadSafeCache>();
    fPipelineBuilder = std::move(pipelineBuilder);
}

#if !defined(SK_DISABLE_LEGACY_CREATE_CHARACTERIZATION)
GrSurfaceCharacterization GrContextThreadSafeProxy::createCharacterization(
            size_t cacheMaxResourceBytes,
            const SkImageInfo& ii,
            const GrBackendFormat& backendFormat,
            int sampleCount,
            GrSurfaceOrigin origin,
            const SkSurfaceProps& surfaceProps,
            bool isMipmapped,
            bool willUseGLFBO0,
            bool isTextureable,
            skgpu::Protected isProtected,
            bool vkRTSupportsInputAttachment,
            bool forVulkanSecondaryCommandBuffer) {
        return this->createCharacterization(
                cacheMaxResourceBytes,
                ii,
                backendFormat,
                sampleCount,
                origin,
                surfaceProps,
                isMipmapped ? skgpu::Mipmapped::kYes : skgpu::Mipmapped::kNo,
                willUseGLFBO0,
                isTextureable,
                isProtected,
                vkRTSupportsInputAttachment,
                forVulkanSecondaryCommandBuffer);
}
#endif

GrSurfaceCharacterization GrContextThreadSafeProxy::createCharacterization(
        size_t cacheMaxResourceBytes,
        const SkImageInfo& ii,
        const GrBackendFormat& backendFormat,
        int sampleCnt,
        GrSurfaceOrigin origin,
        const SkSurfaceProps& surfaceProps,
        skgpu::Mipmapped isMipmapped,
        bool willUseGLFBO0,
        bool isTextureable,
        skgpu::Protected isProtected,
        bool vkRTSupportsInputAttachment,
        bool forVulkanSecondaryCommandBuffer) {
    SkASSERT(fCaps);
    if (!backendFormat.isValid()) {
        return {};
    }

    SkASSERT(isTextureable || isMipmapped == skgpu::Mipmapped::kNo);

    if (GrBackendApi::kOpenGL != backendFormat.backend() && willUseGLFBO0) {
        // The willUseGLFBO0 flags can only be used for a GL backend.
        return {};
    }

    if (GrBackendApi::kVulkan != backendFormat.backend() &&
        (vkRTSupportsInputAttachment || forVulkanSecondaryCommandBuffer)) {
        // The vkRTSupportsInputAttachment and forVulkanSecondaryCommandBuffer flags can only be
        // used for a Vulkan backend.
        return {};
    }

    if (!fCaps->mipmapSupport()) {
        isMipmapped = skgpu::Mipmapped::kNo;
    }

    if (ii.width()  < 1 || ii.width()  > fCaps->maxRenderTargetSize() ||
        ii.height() < 1 || ii.height() > fCaps->maxRenderTargetSize()) {
        return {};
    }

    GrColorType grColorType = SkColorTypeToGrColorType(ii.colorType());

    if (!fCaps->areColorTypeAndFormatCompatible(grColorType, backendFormat)) {
        return {};
    }

    if (!fCaps->isFormatAsColorTypeRenderable(grColorType, backendFormat, sampleCnt)) {
        return {};
    }

    sampleCnt = fCaps->getRenderTargetSampleCount(sampleCnt, backendFormat);
    SkASSERT(sampleCnt);

    if (willUseGLFBO0 && isTextureable) {
        return {};
    }

    if (isTextureable && !fCaps->isFormatTexturable(backendFormat, backendFormat.textureType())) {
        // Skia doesn't agree that this is textureable.
        return {};
    }

    if (GrBackendApi::kVulkan == backendFormat.backend()) {
        if (!isValidCharacterizationForVulkan(fCaps,
                                              isTextureable,
                                              isMipmapped,
                                              isProtected,
                                              vkRTSupportsInputAttachment,
                                              forVulkanSecondaryCommandBuffer)) {
            return {};
        }
    }

    return GrSurfaceCharacterization(
            sk_ref_sp<GrContextThreadSafeProxy>(this),
            cacheMaxResourceBytes,
            ii,
            backendFormat,
            origin,
            sampleCnt,
            GrSurfaceCharacterization::Textureable(isTextureable),
            isMipmapped,
            GrSurfaceCharacterization::UsesGLFBO0(willUseGLFBO0),
            GrSurfaceCharacterization::VkRTSupportsInputAttachment(vkRTSupportsInputAttachment),
            GrSurfaceCharacterization::VulkanSecondaryCBCompatible(forVulkanSecondaryCommandBuffer),
            isProtected,
            surfaceProps);
}

bool GrContextThreadSafeProxy::isValidCharacterizationForVulkan(
        sk_sp<const GrCaps>,
        bool isTextureable,
        skgpu::Mipmapped isMipmapped,
        skgpu::Protected isProtected,
        bool vkRTSupportsInputAttachment,
        bool forVulkanSecondaryCommandBuffer) {
    return false;  // handled by a subclass
}

GrBackendFormat GrContextThreadSafeProxy::defaultBackendFormat(SkColorType skColorType,
                                                               GrRenderable renderable) const {
    SkASSERT(fCaps);
    GrColorType grColorType = SkColorTypeToGrColorType(skColorType);

    GrBackendFormat format = fCaps->getDefaultBackendFormat(grColorType, renderable);
    if (!format.isValid()) {
        return GrBackendFormat();
    }

    SkASSERT(renderable == GrRenderable::kNo ||
             fCaps->isFormatAsColorTypeRenderable(grColorType, format));

    return format;
}

GrBackendFormat GrContextThreadSafeProxy::compressedBackendFormat(SkTextureCompressionType c) const {
    SkASSERT(fCaps);

    GrBackendFormat format = fCaps->getBackendFormatFromCompressionType(c);

    SkASSERT(!format.isValid() || fCaps->isFormatTexturable(format, GrTextureType::k2D));
    return format;
}

int GrContextThreadSafeProxy::maxSurfaceSampleCountForColorType(SkColorType colorType) const {
    SkASSERT(fCaps);

    GrBackendFormat format = fCaps->getDefaultBackendFormat(SkColorTypeToGrColorType(colorType),
                                                            GrRenderable::kYes);
    return fCaps->maxRenderTargetSampleCount(format);
}

void GrContextThreadSafeProxy::abandonContext() {
    if (!fAbandoned.exchange(true)) {
        fTextBlobRedrawCoordinator->freeAll();
    }
}

bool GrContextThreadSafeProxy::abandoned() const {
    return fAbandoned;
}

////////////////////////////////////////////////////////////////////////////////
sk_sp<GrContextThreadSafeProxy> GrContextThreadSafeProxyPriv::Make(
                             GrBackendApi backend,
                             const GrContextOptions& options) {
    return sk_sp<GrContextThreadSafeProxy>(new GrContextThreadSafeProxy(backend, options));
}

void GrContextThreadSafeProxyPriv::init(sk_sp<const GrCaps> caps,
                                        sk_sp<GrThreadSafePipelineBuilder> builder) const {
    fProxy->init(std::move(caps), std::move(builder));
}
