/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/mtl/GrMtlRenderTarget.h"

#include "src/gpu/mtl/GrMtlGpu.h"
#include "src/gpu/mtl/GrMtlUtil.h"

// Called for wrapped non-texture render targets.
GrMtlRenderTarget::GrMtlRenderTarget(GrMtlGpu* gpu,
                                     const GrSurfaceDesc& desc,
                                     id<MTLTexture> renderTexture,
                                     Wrapped)
        : GrSurface(gpu, desc)
        , GrRenderTarget(gpu, desc)
        , fRenderTexture(renderTexture)
        , fResolveTexture(nil) {
    SkASSERT(1 == desc.fSampleCnt);
    this->registerWithCacheWrapped(GrWrapCacheable::kNo);
}

// Called by subclass constructors.
GrMtlRenderTarget::GrMtlRenderTarget(GrMtlGpu* gpu,
                                     const GrSurfaceDesc& desc,
                                     id<MTLTexture> renderTexture)
        : GrSurface(gpu, desc)
        , GrRenderTarget(gpu, desc)
        , fRenderTexture(renderTexture)
        , fResolveTexture(nil) {
    SkASSERT(1 == desc.fSampleCnt);
}

sk_sp<GrMtlRenderTarget>
GrMtlRenderTarget::MakeWrappedRenderTarget(GrMtlGpu* gpu, const GrSurfaceDesc& desc,
                                           id<MTLTexture> renderTexture) {
    SkASSERT(nil != renderTexture);
    SkASSERT(1 == renderTexture.mipmapLevelCount);
    SkASSERT(MTLTextureUsageRenderTarget & renderTexture.usage);
    return sk_sp<GrMtlRenderTarget>(new GrMtlRenderTarget(gpu, desc, renderTexture, kWrapped));
}

GrMtlRenderTarget::~GrMtlRenderTarget() {
    SkASSERT(nil == fRenderTexture);
    SkASSERT(nil == fResolveTexture);
}

GrBackendRenderTarget GrMtlRenderTarget::getBackendRenderTarget() const {
    GrMtlTextureInfo info;
    info.fTexture = GrGetPtrFromId(fRenderTexture);
    return GrBackendRenderTarget(this->width(), this->height(), fRenderTexture.sampleCount, info);
}

GrBackendFormat GrMtlRenderTarget::backendFormat() const {
    return GrBackendFormat::MakeMtl(fRenderTexture.pixelFormat);
}

GrMtlGpu* GrMtlRenderTarget::getMtlGpu() const {
    SkASSERT(!this->wasDestroyed());
    return static_cast<GrMtlGpu*>(this->getGpu());
}

void GrMtlRenderTarget::onAbandon() {
    fRenderTexture = nil;
    fResolveTexture = nil;
    INHERITED::onAbandon();
}

void GrMtlRenderTarget::onRelease() {
    fRenderTexture = nil;
    fResolveTexture = nil;
    INHERITED::onRelease();
}

bool GrMtlRenderTarget::completeStencilAttachment() {
    return true;
}

