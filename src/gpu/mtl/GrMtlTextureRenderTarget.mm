/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrMtlTextureRenderTarget.h"

sk_sp<GrMtlTextureRenderTarget>
GrMtlTextureRenderTarget::MakeNewTextureRenderTarget(GrMtlGpu* gpu,
                                                     const GrSurfaceDesc& desc,
                                                     SkBudgeted budgeted,
                                                     int mipLevels) {
    MTLPixelFormat format;
    if (!GrPixelConfigToMTLFormat(desc.fConfig, &format)) {
        return nullptr;
    }

    if (desc.fSampleCnt) {
        // Currently we don't support msaa
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

    return Make(gpu, desc, budgeted,
    return sk_sp<GrMtlTextureRenderTarget>(new GrMtlTextureRenderTarget(gpu, desc, budgeted,
                                                                        texture));
}

sk_sp<GrMtlTextureRenderTarget> GrMtlTextureRenderTarget::Make(GrMtlGpu* gpu,
                                                               const GrSurfaceDesc& desc,
                                                               SkBudgeted budgeted,
                                                               id<MTLTexture> resolveTexture) {
    id<MTLTexture> renderTexture;
    if (desc.fSampleCnt) {
        // Currently multisampling is not supported
        SkASSERT(false);
        return nullptr;
    } else {
        renderTexture = resolveTexture;
    }
}
