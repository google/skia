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
                                     SkISize dimensions,
                                     int sampleCnt,
                                     id<MTLTexture> colorTexture,
                                     id<MTLTexture> resolveTexture,
                                     Wrapped)
        : GrSurface(gpu, dimensions, GrProtected::kNo)
        , GrRenderTarget(gpu, dimensions, sampleCnt, GrProtected::kNo)
        , fColorTexture(colorTexture)
        , fResolveTexture(resolveTexture) {
    SkASSERT(sampleCnt > 1);
    this->registerWithCacheWrapped(GrWrapCacheable::kNo);
}

GrMtlRenderTarget::GrMtlRenderTarget(GrMtlGpu* gpu,
                                     SkISize dimensions,
                                     id<MTLTexture> colorTexture,
                                     Wrapped)
        : GrSurface(gpu, dimensions, GrProtected::kNo)
        , GrRenderTarget(gpu, dimensions, 1, GrProtected::kNo)
        , fColorTexture(colorTexture)
        , fResolveTexture(nil) {
    this->registerWithCacheWrapped(GrWrapCacheable::kNo);
}

// Called by subclass constructors.
GrMtlRenderTarget::GrMtlRenderTarget(GrMtlGpu* gpu,
                                     SkISize dimensions,
                                     int sampleCnt,
                                     id<MTLTexture> colorTexture,
                                     id<MTLTexture> resolveTexture)
        : GrSurface(gpu, dimensions, GrProtected::kNo)
        , GrRenderTarget(gpu, dimensions, sampleCnt, GrProtected::kNo)
        , fColorTexture(colorTexture)
        , fResolveTexture(resolveTexture) {
    SkASSERT(sampleCnt > 1);
}

GrMtlRenderTarget::GrMtlRenderTarget(GrMtlGpu* gpu, SkISize dimensions, id<MTLTexture> colorTexture)
        : GrSurface(gpu, dimensions, GrProtected::kNo)
        , GrRenderTarget(gpu, dimensions, 1, GrProtected::kNo)
        , fColorTexture(colorTexture)
        , fResolveTexture(nil) {}

sk_sp<GrMtlRenderTarget> GrMtlRenderTarget::MakeWrappedRenderTarget(GrMtlGpu* gpu,
                                                                    SkISize dimensions,
                                                                    int sampleCnt,
                                                                    id<MTLTexture> texture) {
    SkASSERT(nil != texture);
    SkASSERT(1 == texture.mipmapLevelCount);
    if (@available(macOS 10.11, iOS 9.0, *)) {
        SkASSERT(MTLTextureUsageRenderTarget & texture.usage);
    }

    GrMtlRenderTarget* mtlRT;
    if (sampleCnt > 1) {
        if ([texture sampleCount] == 1) {
            MTLPixelFormat format = texture.pixelFormat;
            if (!gpu->mtlCaps().isFormatRenderable(format, sampleCnt)) {
                return nullptr;
            }
            MTLTextureDescriptor* texDesc = [[MTLTextureDescriptor alloc] init];
            texDesc.textureType = MTLTextureType2DMultisample;
            texDesc.pixelFormat = format;
            texDesc.width = dimensions.fWidth;
            texDesc.height = dimensions.fHeight;
            texDesc.depth = 1;
            texDesc.mipmapLevelCount = 1;
            texDesc.sampleCount = sampleCnt;
            texDesc.arrayLength = 1;
            if (@available(macOS 10.11, iOS 9.0, *)) {
                texDesc.storageMode = MTLStorageModePrivate;
                texDesc.usage = MTLTextureUsageShaderRead | MTLTextureUsageRenderTarget;
            }

            id<MTLTexture> colorTexture = [gpu->device() newTextureWithDescriptor:texDesc];
            if (!colorTexture) {
                return nullptr;
            }
            if (@available(macOS 10.11, iOS 9.0, *)) {
                SkASSERT((MTLTextureUsageShaderRead | MTLTextureUsageRenderTarget) &
                         colorTexture.usage);
            }
            mtlRT = new GrMtlRenderTarget(
                    gpu, dimensions, sampleCnt, colorTexture, texture, kWrapped);
        } else {
            SkASSERT(sampleCnt == static_cast<int>([texture sampleCount]));
            mtlRT = new GrMtlRenderTarget(gpu, dimensions, sampleCnt, texture, nil, kWrapped);
        }
    } else {
        mtlRT = new GrMtlRenderTarget(gpu, dimensions, texture, kWrapped);
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
    return GrBackendRenderTarget(this->width(), this->height(), info);
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

