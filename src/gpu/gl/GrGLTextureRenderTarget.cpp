/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTraceMemoryDump.h"
#include "include/gpu/GrContext.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrTexturePriv.h"
#include "src/gpu/gl/GrGLGpu.h"
#include "src/gpu/gl/GrGLTextureRenderTarget.h"

GrGLTextureRenderTarget::GrGLTextureRenderTarget(GrGLGpu* gpu,
                                                 SkBudgeted budgeted,
                                                 int sampleCount,
                                                 const GrGLTexture::Desc& texDesc,
                                                 const GrGLRenderTarget::IDs& rtIDs,
                                                 GrMipMapsStatus mipMapsStatus)
        : GrSurface(gpu, texDesc.fSize, texDesc.fConfig, GrProtected::kNo)
        , GrGLTexture(gpu, texDesc, nullptr, mipMapsStatus)
        , GrGLRenderTarget(gpu, texDesc.fSize, texDesc.fFormat, texDesc.fConfig, sampleCount,
                           rtIDs) {
    this->registerWithCache(budgeted);
}

GrGLTextureRenderTarget::GrGLTextureRenderTarget(GrGLGpu* gpu,
                                                 int sampleCount,
                                                 const GrGLTexture::Desc& texDesc,
                                                 sk_sp<GrGLTextureParameters> parameters,
                                                 const GrGLRenderTarget::IDs& rtIDs,
                                                 GrWrapCacheable cacheable,
                                                 GrMipMapsStatus mipMapsStatus)
        : GrSurface(gpu, texDesc.fSize, texDesc.fConfig, GrProtected::kNo)
        , GrGLTexture(gpu, texDesc, std::move(parameters), mipMapsStatus)
        , GrGLRenderTarget(gpu, texDesc.fSize, texDesc.fFormat, texDesc.fConfig, sampleCount,
                           rtIDs) {
    this->registerWithCacheWrapped(cacheable);
}

void GrGLTextureRenderTarget::dumpMemoryStatistics(
    SkTraceMemoryDump* traceMemoryDump) const {
#ifndef SK_BUILD_FOR_ANDROID_FRAMEWORK
    // Delegate to the base classes
    GrGLRenderTarget::dumpMemoryStatistics(traceMemoryDump);
    GrGLTexture::dumpMemoryStatistics(traceMemoryDump);
#else
    SkString resourceName = this->getResourceName();
    resourceName.append("/texture_renderbuffer");
    this->dumpMemoryStatisticsPriv(traceMemoryDump, resourceName, "RenderTarget",
                                   this->gpuMemorySize());
#endif
}

bool GrGLTextureRenderTarget::canAttemptStencilAttachment() const {
    // The RT FBO of GrGLTextureRenderTarget is never created from a
    // wrapped FBO, so we only care about the flag.
    return !this->getGpu()->getContext()->priv().caps()->avoidStencilBuffers();
}

sk_sp<GrGLTextureRenderTarget> GrGLTextureRenderTarget::MakeWrapped(
        GrGLGpu* gpu,
        int sampleCount,
        const GrGLTexture::Desc& texDesc,
        sk_sp<GrGLTextureParameters> parameters,
        const GrGLRenderTarget::IDs& rtIDs,
        GrWrapCacheable cacheable,
        GrMipMapsStatus mipMapsStatus) {
    return sk_sp<GrGLTextureRenderTarget>(new GrGLTextureRenderTarget(
            gpu, sampleCount, texDesc, std::move(parameters), rtIDs, cacheable, mipMapsStatus));
}

size_t GrGLTextureRenderTarget::onGpuMemorySize() const {
    return GrSurface::ComputeSize(this->config(), this->width(), this->height(),
                                  this->numSamplesOwnedPerPixel(), this->texturePriv().mipMapped());
}
