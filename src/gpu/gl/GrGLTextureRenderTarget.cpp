/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLTextureRenderTarget.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrGLGpu.h"
#include "GrTexturePriv.h"
#include "SkTraceMemoryDump.h"

GrGLTextureRenderTarget::GrGLTextureRenderTarget(GrGLGpu* gpu,
                                                 SkBudgeted budgeted,
                                                 const GrSurfaceDesc& desc,
                                                 const GrGLTexture::IDDesc& texIDDesc,
                                                 const GrGLRenderTarget::IDDesc& rtIDDesc,
                                                 GrMipMapsStatus mipMapsStatus)
        : GrSurface(gpu, desc)
        , GrGLTexture(gpu, desc, texIDDesc, mipMapsStatus)
        , GrGLRenderTarget(gpu, desc, texIDDesc.fInfo.fFormat, rtIDDesc) {
    this->registerWithCache(budgeted);
}

GrGLTextureRenderTarget::GrGLTextureRenderTarget(GrGLGpu* gpu,
                                                 const GrSurfaceDesc& desc,
                                                 const GrGLTexture::IDDesc& texIDDesc,
                                                 const GrGLRenderTarget::IDDesc& rtIDDesc,
                                                 GrWrapCacheable cacheable,
                                                 GrMipMapsStatus mipMapsStatus)
        : GrSurface(gpu, desc)
        , GrGLTexture(gpu, desc, texIDDesc, mipMapsStatus)
        , GrGLRenderTarget(gpu, desc, texIDDesc.fInfo.fFormat, rtIDDesc) {
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
        GrGLGpu* gpu, const GrSurfaceDesc& desc, const GrGLTexture::IDDesc& texIDDesc,
        const GrGLRenderTarget::IDDesc& rtIDDesc, GrWrapCacheable cacheable,
        GrMipMapsStatus mipMapsStatus) {
    return sk_sp<GrGLTextureRenderTarget>(
            new GrGLTextureRenderTarget(gpu, desc, texIDDesc, rtIDDesc, cacheable, mipMapsStatus));
}

size_t GrGLTextureRenderTarget::onGpuMemorySize() const {
    return GrSurface::ComputeSize(this->config(), this->width(), this->height(),
                                    this->numSamplesOwnedPerPixel(),
                                    this->texturePriv().mipMapped());
}
