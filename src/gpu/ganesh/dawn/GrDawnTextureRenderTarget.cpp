/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/dawn/GrDawnTextureRenderTarget.h"

#include "include/core/SkTraceMemoryDump.h"
#include "src/gpu/ganesh/GrTexture.h"
#include "src/gpu/ganesh/dawn/GrDawnGpu.h"

GrDawnTextureRenderTarget::GrDawnTextureRenderTarget(GrDawnGpu* gpu,
                                                     SkISize dimensions,
                                                     int sampleCnt,
                                                     const GrDawnTextureInfo& textureInfo,
                                                     GrMipmapStatus mipmapStatus,
                                                     std::string_view label)
        : GrSurface(gpu, dimensions, GrProtected::kNo, label)
        , GrDawnTexture(gpu, dimensions, textureInfo, mipmapStatus, label)
        , GrDawnRenderTarget(
                  gpu, dimensions, sampleCnt, GrDawnRenderTargetInfo(textureInfo), label) {}

bool GrDawnTextureRenderTarget::canAttemptStencilAttachment(bool useMSAASurface) const {
    SkASSERT(useMSAASurface == (this->numSamples() > 1));
    return true;
}

size_t GrDawnTextureRenderTarget::onGpuMemorySize() const {
    return GrSurface::ComputeSize(this->backendFormat(), this->dimensions(),
                                  1,  // FIXME: for MSAA
                                  this->mipmapped());
}
