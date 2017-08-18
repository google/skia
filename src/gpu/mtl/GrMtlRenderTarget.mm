/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrMtlRenderTarget.h"

#include "GrMtlGpu.h"
#include "GrMtlUtil.h"

sk_sp<GrMtlRenderTarget> GrMtlRenderTarget::CreateNewRenderTarget(GrMtlGpu* gpu,
                                                                  const GrSurfaceDesc& desc,
                                                                  SkBudgeted budgeted) {
    MTLPixelFormat format;
    if (!GrPixelConfigToMTLFormat(desc.fConfig, &format)) {
        return nullptr;
    }

    if (desc.fSampleCnt) {
        return nullptr;
    }

    MTLTextureDescriptor* descriptor = [[MTLTextureDescriptor alloc] init];
    descriptor.textureType = MTLTextureType2D;
    descriptor.pixelFormat = format;
    descriptor.width = desc.fWidth;
    descriptor.height = desc.fHeight;
    descriptor.depth = 1;
    descriptor.mipmapLevelCount = 1;
    descriptor.sampleCount = 1;
    descriptor.arrayLength = 1;
    // descriptor.resourceOptions This looks to be set by setting cpuCacheMode and storageModes
    descriptor.cpuCacheMode = MTLCPUCacheModeWriteCombined;
    // RenderTargets never need to be mapped so their storage mode is set to private
    descriptor.storageMode = MTLStorageModePrivate;

    descriptor.usage = MTLTextureUsageRenderTarget;

    id<MTLTexture> texture = [gpu->device() newTextureWithDescriptor:descriptor];

    return sk_sp<GrMtlRenderTarget>(new GrMtlRenderTarget(gpu, desc, budgeted, texture));
}

GrMtlRenderTarget::GrMtlRenderTarget(GrMtlGpu* gpu,
                                     const GrSurfaceDesc& desc,
                                     SkBudgeted budgeted,
                                     id<MTLTexture> renderTexture)
        : GrSurface(gpu, desc)
        , GrRenderTarget(gpu, desc)
        , fRenderTexture(renderTexture)
        , fResolveTexture(nil) {
}

GrMtlRenderTarget::~GrMtlRenderTarget() {
    SkASSERT(nil == fRenderTexture);
    SkASSERT(nil == fResolveTexture);
}

GrMtlGpu* GrMtlRenderTarget::getMtlGpu() const {
    SkASSERT(!this->wasDestroyed());
    return static_cast<GrMtlGpu*>(this->getGpu());
}

GrBackendObject GrMtlRenderTarget::getRenderTargetHandle() const {
    void* voidRT = (__bridge_retained void*)fRenderTexture;
    return (GrBackendObject)voidRT;
}

void GrMtlRenderTarget::onAbandon() {
    fRenderTexture = nil;
    fResolveTexture = nil;
}

void GrMtlRenderTarget::onRelease() {
    fRenderTexture = nil;
    fResolveTexture = nil;
}

bool completeStencilAttachment() {
    // TODO: fill this out
    return true;
}


