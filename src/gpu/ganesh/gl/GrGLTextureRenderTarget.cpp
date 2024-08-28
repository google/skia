/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/ganesh/gl/GrGLTextureRenderTarget.h"

#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/private/base/SkAssert.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrGpu.h"
#include "src/gpu/ganesh/GrSurface.h"
#include "src/gpu/ganesh/gl/GrGLGpu.h"
#include "src/gpu/ganesh/gl/GrGLTypesPriv.h"

#include <utility>

GrGLTextureRenderTarget::GrGLTextureRenderTarget(GrGLGpu* gpu,
                                                 skgpu::Budgeted budgeted,
                                                 int sampleCount,
                                                 const GrGLTexture::Desc& texDesc,
                                                 const GrGLRenderTarget::IDs& rtIDs,
                                                 GrMipmapStatus mipmapStatus,
                                                 std::string_view label)
        : GrSurface(gpu, texDesc.fSize, texDesc.fIsProtected, label)
        , GrGLTexture(gpu, texDesc, nullptr, mipmapStatus, label)
        , GrGLRenderTarget(gpu, texDesc.fSize, texDesc.fFormat, sampleCount, rtIDs,
                           texDesc.fIsProtected, label) {
    this->registerWithCache(budgeted);
}

GrGLTextureRenderTarget::GrGLTextureRenderTarget(GrGLGpu* gpu,
                                                 int sampleCount,
                                                 const GrGLTexture::Desc& texDesc,
                                                 sk_sp<GrGLTextureParameters> parameters,
                                                 const GrGLRenderTarget::IDs& rtIDs,
                                                 GrWrapCacheable cacheable,
                                                 GrMipmapStatus mipmapStatus,
                                                 std::string_view label)
        : GrSurface(gpu, texDesc.fSize, texDesc.fIsProtected, label)
        , GrGLTexture(gpu, texDesc, std::move(parameters), mipmapStatus, label)
        , GrGLRenderTarget(gpu, texDesc.fSize, texDesc.fFormat, sampleCount, rtIDs,
                           texDesc.fIsProtected, label) {
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

bool GrGLTextureRenderTarget::canAttemptStencilAttachment(bool useMultisampleFBO) const {
    // This cap should have been handled at a higher level.
    SkASSERT(!this->getGpu()->getContext()->priv().caps()->avoidStencilBuffers());
    // The RT FBO of GrGLTextureRenderTarget is never created from a wrapped FBO.
    return true;
}

sk_sp<GrGLTextureRenderTarget> GrGLTextureRenderTarget::MakeWrapped(
        GrGLGpu* gpu,
        int sampleCount,
        const GrGLTexture::Desc& texDesc,
        sk_sp<GrGLTextureParameters> parameters,
        const GrGLRenderTarget::IDs& rtIDs,
        GrWrapCacheable cacheable,
        GrMipmapStatus mipmapStatus,
        std::string_view label) {
    return sk_sp<GrGLTextureRenderTarget>(
            new GrGLTextureRenderTarget(gpu,
                                        sampleCount,
                                        texDesc,
                                        std::move(parameters),
                                        rtIDs,
                                        cacheable,
                                        mipmapStatus,
                                        label));
}

size_t GrGLTextureRenderTarget::onGpuMemorySize() const {
    return GrSurface::ComputeSize(this->backendFormat(), this->dimensions(),
                                  this->totalMemorySamplesPerPixel(), this->mipmapped());
}

void GrGLTextureRenderTarget::onSetLabel() {
    GrGLTexture::onSetLabel();
}
