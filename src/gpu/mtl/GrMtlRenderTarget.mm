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
                                     int sampleCnt,
                                     id<MTLTexture> colorTexture,
                                     id<MTLTexture> resolveTexture,
                                     Wrapped)
        : GrSurface(gpu, {desc.fWidth, desc.fHeight}, desc.fConfig, GrProtected::kNo)
        , GrRenderTarget(
                  gpu, {desc.fWidth, desc.fHeight}, desc.fConfig, sampleCnt, GrProtected::kNo)
        , fColorTexture(colorTexture)
        , fResolveTexture(resolveTexture) {
    SkASSERT(sampleCnt > 1);
    this->registerWithCacheWrapped(GrWrapCacheable::kNo);
}

GrMtlRenderTarget::GrMtlRenderTarget(GrMtlGpu* gpu,
                                     const GrSurfaceDesc& desc,
                                     id<MTLTexture> colorTexture,
                                     Wrapped)
        : GrSurface(gpu, {desc.fWidth, desc.fHeight}, desc.fConfig, GrProtected::kNo)
        , GrRenderTarget(gpu, {desc.fWidth, desc.fHeight}, desc.fConfig, 1, GrProtected::kNo)
        , fColorTexture(colorTexture)
        , fResolveTexture(nil) {
    this->registerWithCacheWrapped(GrWrapCacheable::kNo);
}

// Called by subclass constructors.
GrMtlRenderTarget::GrMtlRenderTarget(GrMtlGpu* gpu,
                                     const GrSurfaceDesc& desc,
                                     int sampleCnt,
                                     id<MTLTexture> colorTexture,
                                     id<MTLTexture> resolveTexture)
        : GrSurface(gpu, {desc.fWidth, desc.fHeight}, desc.fConfig, GrProtected::kNo)
        , GrRenderTarget(
                  gpu, {desc.fWidth, desc.fHeight}, desc.fConfig, sampleCnt, GrProtected::kNo)
        , fColorTexture(colorTexture)
        , fResolveTexture(resolveTexture) {
    SkASSERT(sampleCnt > 1);
}

GrMtlRenderTarget::GrMtlRenderTarget(GrMtlGpu* gpu,
                                     const GrSurfaceDesc& desc,
                                     id<MTLTexture> colorTexture)
        : GrSurface(gpu, {desc.fWidth, desc.fHeight}, desc.fConfig, GrProtected::kNo)
        , GrRenderTarget(gpu, {desc.fWidth, desc.fHeight}, desc.fConfig, 1, GrProtected::kNo)
        , fColorTexture(colorTexture)
        , fResolveTexture(nil) {}

sk_sp<GrMtlRenderTarget> GrMtlRenderTarget::MakeWrappedRenderTarget(GrMtlGpu* gpu,
                                                                    const GrSurfaceDesc& desc,
                                                                    int sampleCnt,
                                                                    id<MTLTexture> texture) {
    SkASSERT(nil != texture);
    SkASSERT(1 == texture.mipmapLevelCount);
    SkASSERT(MTLTextureUsageRenderTarget & texture.usage);

    GrMtlRenderTarget* mtlRT;
    if (sampleCnt > 1) {
        MTLPixelFormat format;
        if (!GrPixelConfigToMTLFormat(desc.fConfig, &format)) {
            return nullptr;
        }
        MTLTextureDescriptor* texDesc = [[MTLTextureDescriptor alloc] init];
        texDesc.textureType = MTLTextureType2DMultisample;
        texDesc.pixelFormat = format;
        texDesc.width = desc.fWidth;
        texDesc.height = desc.fHeight;
        texDesc.depth = 1;
        texDesc.mipmapLevelCount = 1;
        texDesc.sampleCount = sampleCnt;
        texDesc.arrayLength = 1;
        texDesc.storageMode = MTLStorageModePrivate;
        texDesc.usage = MTLTextureUsageShaderRead | MTLTextureUsageRenderTarget;

        id<MTLTexture> colorTexture = [gpu->device() newTextureWithDescriptor:texDesc];
        if (!colorTexture) {
            return nullptr;
        }
        SkASSERT((MTLTextureUsageShaderRead | MTLTextureUsageRenderTarget) & colorTexture.usage);
        mtlRT = new GrMtlRenderTarget(gpu, desc, sampleCnt, colorTexture, texture, kWrapped);
    } else {
        mtlRT = new GrMtlRenderTarget(gpu, desc, texture, kWrapped);
    }

    return sk_sp<GrMtlRenderTarget>(mtlRT);
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

