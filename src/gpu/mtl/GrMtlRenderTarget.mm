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
                                     SkISize dimensions,
                                     int sampleCnt,
                                     sk_cf_obj<id<MTLTexture>> colorTexture,
                                     sk_cf_obj<id<MTLTexture>> resolveTexture,
                                     Wrapped)
        : GrSurface(gpu, dimensions, GrProtected::kNo)
        , GrRenderTarget(gpu, dimensions, sampleCnt, GrProtected::kNo)
        , fColorTexture(std::move(colorTexture))
        , fResolveTexture(std::move(resolveTexture)) {
    SkASSERT(sampleCnt > 1);
    this->registerWithCacheWrapped(GrWrapCacheable::kNo);
}

GrMtlRenderTarget::GrMtlRenderTarget(GrMtlGpu* gpu,
                                     SkISize dimensions,
                                     sk_cf_obj<id<MTLTexture>> colorTexture,
                                     Wrapped)
        : GrSurface(gpu, dimensions, GrProtected::kNo)
        , GrRenderTarget(gpu, dimensions, 1, GrProtected::kNo)
        , fColorTexture(std::move(colorTexture))
        , fResolveTexture(nil) {
    this->registerWithCacheWrapped(GrWrapCacheable::kNo);
}

// Called by subclass constructors.
GrMtlRenderTarget::GrMtlRenderTarget(GrMtlGpu* gpu,
                                     SkISize dimensions,
                                     int sampleCnt,
                                     sk_cf_obj<id<MTLTexture>> colorTexture,
                                     sk_cf_obj<id<MTLTexture>> resolveTexture)
        : GrSurface(gpu, dimensions, GrProtected::kNo)
        , GrRenderTarget(gpu, dimensions, sampleCnt, GrProtected::kNo)
        , fColorTexture(std::move(colorTexture))
        , fResolveTexture(std::move(resolveTexture)) {
    SkASSERT(sampleCnt > 1);
}

GrMtlRenderTarget::GrMtlRenderTarget(GrMtlGpu* gpu, SkISize dimensions,
                                     sk_cf_obj<id<MTLTexture>> colorTexture)
        : GrSurface(gpu, dimensions, GrProtected::kNo)
        , GrRenderTarget(gpu, dimensions, 1, GrProtected::kNo)
        , fColorTexture(std::move(colorTexture))
        , fResolveTexture(nil) {}

sk_sp<GrMtlRenderTarget> GrMtlRenderTarget::MakeWrappedRenderTarget(
        GrMtlGpu* gpu, SkISize dimensions, int sampleCnt, sk_cf_obj<id<MTLTexture>> texture) {
    SkASSERT(texture);
    SkASSERT(1 == (*texture).mipmapLevelCount);
    if (@available(macOS 10.11, iOS 9.0, *)) {
        SkASSERT(MTLTextureUsageRenderTarget & (*texture).usage);
    }

    GrMtlRenderTarget* mtlRT;
    if (sampleCnt > 1) {
        // TODO: share with routine in GrMtlTextureRenderTarget
        MTLPixelFormat format = (*texture).pixelFormat;
        if (!gpu->mtlCaps().isFormatRenderable(format, sampleCnt)) {
            return nullptr;
        }
        sk_cf_obj<MTLTextureDescriptor*> texDesc([[MTLTextureDescriptor alloc] init]);
        (*texDesc).textureType = MTLTextureType2DMultisample;
        (*texDesc).pixelFormat = format;
        (*texDesc).width = dimensions.fWidth;
        (*texDesc).height = dimensions.fHeight;
        (*texDesc).depth = 1;
        (*texDesc).mipmapLevelCount = 1;
        (*texDesc).sampleCount = sampleCnt;
        (*texDesc).arrayLength = 1;
        if (@available(macOS 10.11, iOS 9.0, *)) {
            (*texDesc).storageMode = MTLStorageModePrivate;
            (*texDesc).usage = MTLTextureUsageShaderRead | MTLTextureUsageRenderTarget;
        }

        sk_cf_obj<id<MTLTexture>> colorTexture(
                [gpu->device() newTextureWithDescriptor:texDesc.get()]);
        if (!colorTexture) {
            return nullptr;
        }
        if (@available(macOS 10.11, iOS 9.0, *)) {
            SkASSERT((MTLTextureUsageShaderRead|MTLTextureUsageRenderTarget) &
                             (*colorTexture).usage);
        }
        mtlRT = new GrMtlRenderTarget(gpu, dimensions, sampleCnt, std::move(colorTexture),
                                      std::move(texture), kWrapped);
    } else {
        mtlRT = new GrMtlRenderTarget(gpu, dimensions, std::move(texture), kWrapped);
    }

    return sk_sp<GrMtlRenderTarget>(mtlRT);
}

GrMtlRenderTarget::~GrMtlRenderTarget() {
    SkASSERT(nil == fColorTexture);
    SkASSERT(nil == fResolveTexture);
}

GrBackendRenderTarget GrMtlRenderTarget::getBackendRenderTarget() const {
    GrMtlTextureInfo info;
    info.fTexture.retain(fColorTexture.get());
    return GrBackendRenderTarget(this->width(), this->height(), (*fColorTexture).sampleCount, info);
}

GrBackendFormat GrMtlRenderTarget::backendFormat() const {
    return GrBackendFormat::MakeMtl((*fColorTexture).pixelFormat);
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

