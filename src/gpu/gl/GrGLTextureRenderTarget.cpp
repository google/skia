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
                                                 const GrSurfaceDesc& desc,
                                                 int sampleCount,
                                                 const GrGLTexture::IDDesc& texIDDesc,
                                                 const GrGLRenderTarget::IDDesc& rtIDDesc,
                                                 GrMipMapsStatus mipMapsStatus)
        : GrSurface(gpu, {desc.fWidth, desc.fHeight}, desc.fConfig, GrProtected::kNo)
        , GrGLTexture(gpu, desc, texIDDesc, nullptr, mipMapsStatus)
        , GrGLRenderTarget(gpu, desc, sampleCount, texIDDesc.fInfo.fFormat, rtIDDesc) {
    this->registerWithCache(budgeted);
}

GrGLTextureRenderTarget::GrGLTextureRenderTarget(GrGLGpu* gpu,
                                                 const GrSurfaceDesc& desc,
                                                 int sampleCount,
                                                 const GrGLTexture::IDDesc& texIDDesc,
                                                 sk_sp<GrGLTextureParameters> parameters,
                                                 const GrGLRenderTarget::IDDesc& rtIDDesc,
                                                 GrWrapCacheable cacheable,
                                                 GrMipMapsStatus mipMapsStatus)
        : GrSurface(gpu, {desc.fWidth, desc.fHeight}, desc.fConfig, GrProtected::kNo)
        , GrGLTexture(gpu, desc, texIDDesc, std::move(parameters), mipMapsStatus)
        , GrGLRenderTarget(gpu, desc, sampleCount, texIDDesc.fInfo.fFormat, rtIDDesc) {
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
        GrGLGpu* gpu, const GrSurfaceDesc& desc, int sampleCount,
        const GrGLTexture::IDDesc& texIDDesc, sk_sp<GrGLTextureParameters> parameters,
        const GrGLRenderTarget::IDDesc& rtIDDesc, GrWrapCacheable cacheable,
        GrMipMapsStatus mipMapsStatus) {
    return sk_sp<GrGLTextureRenderTarget>(
            new GrGLTextureRenderTarget(gpu, desc, sampleCount, texIDDesc, std::move(parameters),
                                        rtIDDesc, cacheable, mipMapsStatus));
}

size_t GrGLTextureRenderTarget::onGpuMemorySize() const {
    return GrSurface::ComputeSize(this->config(), this->width(), this->height(),
                                  this->numSamplesOwnedPerPixel(), this->texturePriv().mipMapped());
}
