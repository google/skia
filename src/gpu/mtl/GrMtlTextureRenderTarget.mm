/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrMtlTextureRenderTarget.h"
#include "GrMtlGpu.h"
#include "GrMtlUtil.h"

GrMtlTextureRenderTarget::GrMtlTextureRenderTarget(GrMtlGpu* gpu,
                                                   SkBudgeted budgeted,
                                                   const GrSurfaceDesc& desc,
                                                   id<MTLTexture> renderTexture,
                                                   GrMipMapsStatus mipMapsStatus)
        : GrSurface(gpu, desc)
        , GrMtlTexture(gpu, desc, renderTexture, mipMapsStatus)
        , GrMtlRenderTarget(gpu, desc, renderTexture) {
    this->registerWithCache(budgeted);
}

GrMtlTextureRenderTarget::GrMtlTextureRenderTarget(GrMtlGpu* gpu,
                                                   const GrSurfaceDesc& desc,
                                                   id<MTLTexture> renderTexture,
                                                   GrMipMapsStatus mipMapsStatus)
        : GrSurface(gpu, desc)
        , GrMtlTexture(gpu, desc, renderTexture, mipMapsStatus)
        , GrMtlRenderTarget(gpu, desc, renderTexture) {
    this->registerWithCacheWrapped();
}

sk_sp<GrMtlTextureRenderTarget>
GrMtlTextureRenderTarget::Make(GrMtlGpu* gpu,
                               const GrSurfaceDesc& desc,
                               id<MTLTexture> renderTexture,
                               int mipLevels,
                               SkBudgeted budgeted,
                               bool isWrapped) {
    SkASSERT(nil != renderTexture);
    if (desc.fSampleCnt > 1) {
        SkASSERT(false); // Currently don't support MSAA
        return nullptr;
    }
    GrMipMapsStatus mipMapsStatus = mipLevels > 1 ? GrMipMapsStatus::kValid
                                                  : GrMipMapsStatus::kNotAllocated;
    if (!isWrapped) {
        return sk_sp<GrMtlTextureRenderTarget>(new GrMtlTextureRenderTarget(gpu,
                                                                            budgeted,
                                                                            desc,
                                                                            renderTexture,
                                                                            mipMapsStatus));
    } else {
        return sk_sp<GrMtlTextureRenderTarget>(new GrMtlTextureRenderTarget(gpu,
                                                                            desc,
                                                                            renderTexture,
                                                                            mipMapsStatus));
    }
}


sk_sp<GrMtlTextureRenderTarget>
GrMtlTextureRenderTarget::CreateNewTextureRenderTarget(GrMtlGpu* gpu,
                                                       SkBudgeted budgeted,
                                                       const GrSurfaceDesc& desc,
                                                       int mipLevels) {
    MTLPixelFormat format;
    if (!GrPixelConfigToMTLFormat(desc.fConfig, &format)) {
        return nullptr;
    }

    MTLTextureDescriptor* descriptor = [[MTLTextureDescriptor alloc] init];
    descriptor.textureType = MTLTextureType2D;
    descriptor.pixelFormat = format;
    descriptor.width = desc.fWidth;
    descriptor.height = desc.fHeight;
    descriptor.depth = 1;
    descriptor.mipmapLevelCount = mipLevels;
    descriptor.sampleCount = 1;
    descriptor.arrayLength = 1;
    // descriptor.resourceOptions This looks to be set by setting cpuCacheMode and storageModes
    descriptor.cpuCacheMode = MTLCPUCacheModeWriteCombined;
    // RenderTargets never need to be mapped so their storage mode is set to private
    descriptor.storageMode = MTLStorageModePrivate;

    descriptor.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;

    id<MTLTexture> texture = [gpu->device() newTextureWithDescriptor:descriptor];

    return Make(gpu, desc, texture, mipLevels, budgeted, false);
}

sk_sp<GrMtlTextureRenderTarget>
GrMtlTextureRenderTarget::MakeWrappedTextureRenderTarget(GrMtlGpu* gpu,
                                                         const GrSurfaceDesc& desc,
                                                         id<MTLTexture> texture) {

    SkASSERT(nil != texture);
    SkASSERT((MTLTextureUsageShaderRead | MTLTextureUsageRenderTarget) & texture.usage);
    return Make(gpu, desc, texture, texture.mipmapLevelCount, SkBudgeted::kNo, true);
}

