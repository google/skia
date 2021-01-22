/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/gl/GrGLTextureRenderTarget.h"

#include "include/core/SkTraceMemoryDump.h"
#include "include/gpu/GrDirectContext.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/gl/GrGLGpu.h"

GrGLTextureRenderTarget::GrGLTextureRenderTarget(GrGLGpu* gpu,
                                                 SkBudgeted budgeted,
                                                 int sampleCount,
                                                 const GrGLTexture::Desc& texDesc,
                                                 const GrGLRenderTarget::IDs& rtIDs,
                                                 GrMipmapStatus mipmapStatus)
        : GrSurface(gpu, texDesc.fSize, GrProtected::kNo)
        , GrGLTexture(gpu, texDesc, nullptr, mipmapStatus)
        , GrGLRenderTarget(gpu, texDesc.fSize, texDesc.fFormat, sampleCount, rtIDs) {
    this->registerWithCache(budgeted);
}

GrGLTextureRenderTarget::GrGLTextureRenderTarget(GrGLGpu* gpu,
                                                 int sampleCount,
                                                 const GrGLTexture::Desc& texDesc,
                                                 sk_sp<GrGLTextureParameters> parameters,
                                                 const GrGLRenderTarget::IDs& rtIDs,
                                                 GrWrapCacheable cacheable,
                                                 GrMipmapStatus mipmapStatus)
        : GrSurface(gpu, texDesc.fSize, GrProtected::kNo)
        , GrGLTexture(gpu, texDesc, std::move(parameters), mipmapStatus)
        , GrGLRenderTarget(gpu, texDesc.fSize, texDesc.fFormat, sampleCount,
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
        GrMipmapStatus mipmapStatus) {
    return sk_sp<GrGLTextureRenderTarget>(new GrGLTextureRenderTarget(
            gpu, sampleCount, texDesc, std::move(parameters), rtIDs, cacheable, mipmapStatus));
}

size_t GrGLTextureRenderTarget::onGpuMemorySize() const {
    return GrSurface::ComputeSize(this->backendFormat(), this->dimensions(),
                                  this->numSamplesOwnedPerPixel(), this->mipmapped());
}
