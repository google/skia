/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/mtl/GrMtlRenderTarget.h"

#include "src/gpu/mtl/GrMtlGpu.h"
#include "src/gpu/mtl/GrMtlUtil.h"

#if !__has_feature(objc_arc)
#error This file must be compiled with Arc. Use -fobjc-arc flag
#endif

// Called for wrapped non-texture render targets.
GrMtlRenderTarget::GrMtlRenderTarget(GrMtlGpu* gpu,
                                     const GrSurfaceDesc& desc,
                                     id<MTLTexture> colorTexture,
                                     id<MTLTexture> resolveTexture,
                                     Wrapped)
        : GrSurface(gpu, desc)
        , GrRenderTarget(gpu, desc)
        , fColorTexture(colorTexture)
        , fResolveTexture(resolveTexture) {
    SkASSERT(desc.fSampleCnt > 1);
    this->registerWithCacheWrapped(GrWrapCacheable::kNo);
}

GrMtlRenderTarget::GrMtlRenderTarget(GrMtlGpu* gpu,
                                     const GrSurfaceDesc& desc,
                                     id<MTLTexture> colorTexture,
                                     Wrapped)
        : GrSurface(gpu, desc)
        , GrRenderTarget(gpu, desc)
        , fColorTexture(colorTexture)
        , fResolveTexture(nil) {
    SkASSERT(1 == desc.fSampleCnt);
    this->registerWithCacheWrapped(GrWrapCacheable::kNo);
}

// Called by subclass constructors.
GrMtlRenderTarget::GrMtlRenderTarget(GrMtlGpu* gpu,
                                     const GrSurfaceDesc& desc,
                                     id<MTLTexture> colorTexture,
                                     id<MTLTexture> resolveTexture)
        : GrSurface(gpu, desc)
        , GrRenderTarget(gpu, desc)
        , fColorTexture(colorTexture)
        , fResolveTexture(resolveTexture) {
    SkASSERT(desc.fSampleCnt > 1);
}
GrMtlRenderTarget::GrMtlRenderTarget(GrMtlGpu* gpu,
                                     const GrSurfaceDesc& desc,
                                     id<MTLTexture> colorTexture)
        : GrSurface(gpu, desc)
        , GrRenderTarget(gpu, desc)
        , fColorTexture(colorTexture)
        , fResolveTexture(nil) {
    SkASSERT(1 == desc.fSampleCnt);
}

sk_sp<GrMtlRenderTarget>
GrMtlRenderTarget::MakeWrappedRenderTarget(GrMtlGpu* gpu, const GrSurfaceDesc& desc,
                                           id<MTLTexture> colorTexture) {
    SkASSERT(nil != colorTexture);
    SkASSERT(1 == colorTexture.mipmapLevelCount);
    SkASSERT(MTLTextureUsageRenderTarget & colorTexture.usage);
    return sk_sp<GrMtlRenderTarget>(new GrMtlRenderTarget(gpu, desc, colorTexture, kWrapped));
}

GrMtlRenderTarget::~GrMtlRenderTarget() {
    SkASSERT(nil == fColorTexture);
    SkASSERT(nil == fResolveTexture);
}

GrBackendRenderTarget GrMtlRenderTarget::getBackendRenderTarget() const {
    GrMtlTextureInfo info;
    info.fTexture.reset(GrRetainPtrFromId(fColorTexture));
    return GrBackendRenderTarget(this->width(), this->height(), fColorTexture.sampleCount, info);
}

GrBackendFormat GrMtlRenderTarget::backendFormat() const {
    return GrBackendFormat::MakeMtl(fColorTexture.pixelFormat);
}

GrMtlGpu* GrMtlRenderTarget::getMtlGpu() const {
    SkASSERT(!this->wasDestroyed());
    return static_cast<GrMtlGpu*>(this->getGpu());
}

void GrMtlRenderTarget::onAbandon() {
    fColorTexture = nil;
    fResolveTexture = nil;
    INHERITED::onAbandon();
}

void GrMtlRenderTarget::onRelease() {
    fColorTexture = nil;
    fResolveTexture = nil;
    INHERITED::onRelease();
}

bool GrMtlRenderTarget::completeStencilAttachment() {
    return true;
}

