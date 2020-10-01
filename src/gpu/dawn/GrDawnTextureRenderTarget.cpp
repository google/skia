/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/dawn/GrDawnTextureRenderTarget.h"

#include "include/core/SkTraceMemoryDump.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/dawn/GrDawnGpu.h"

GrDawnTextureRenderTarget::GrDawnTextureRenderTarget(GrDawnGpu* gpu,
                                                     SkISize dimensions,
                                                     int sampleCnt,
                                                     const GrDawnTextureInfo& textureInfo,
                                                     GrMipmapStatus mipmapStatus)
        : GrSurface(gpu, dimensions, GrProtected::kNo)
        , GrDawnTexture(gpu, dimensions, textureInfo, mipmapStatus)
        , GrDawnRenderTarget(gpu, dimensions, sampleCnt,
                             GrDawnRenderTargetInfo(textureInfo)) {}

bool GrDawnTextureRenderTarget::canAttemptStencilAttachment() const {
    return true;
}

size_t GrDawnTextureRenderTarget::onGpuMemorySize() const {
    return GrSurface::ComputeSize(this->backendFormat(), this->dimensions(),
                                  1,  // FIXME: for MSAA
                                  this->mipmapped());
}
