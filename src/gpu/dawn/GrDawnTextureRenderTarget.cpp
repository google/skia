/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/dawn/GrDawnTextureRenderTarget.h"

#include "include/core/SkTraceMemoryDump.h"
#include "include/gpu/GrContext.h"
#include "src/gpu/GrTexturePriv.h"
#include "src/gpu/dawn/GrDawnGpu.h"

GrDawnTextureRenderTarget::GrDawnTextureRenderTarget(GrDawnGpu* gpu,
                                                     SkISize dimensions,
                                                     wgpu::TextureView textureView,
                                                     int sampleCnt,
                                                     const GrDawnTextureInfo& textureInfo,
                                                     GrMipMapsStatus mipMapsStatus)
        : GrSurface(gpu, dimensions, GrProtected::kNo)
        , GrDawnTexture(gpu, dimensions, textureView, textureInfo, mipMapsStatus)
        , GrDawnRenderTarget(gpu, dimensions, sampleCnt,
                             GrDawnRenderTargetInfo(textureInfo)) {}

bool GrDawnTextureRenderTarget::canAttemptStencilAttachment() const {
    return true;
}

size_t GrDawnTextureRenderTarget::onGpuMemorySize() const {
    const GrCaps& caps = *this->getGpu()->caps();
    return GrSurface::ComputeSize(caps, this->backendFormat(), this->dimensions(),
                                  1, // FIXME: for MSAA
                                  this->texturePriv().mipMapped());
}
