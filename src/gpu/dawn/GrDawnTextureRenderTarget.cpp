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
                                                     const SkISize& size,
                                                     GrPixelConfig config,
                                                     dawn::TextureView textureView,
                                                     int sampleCnt,
                                                     const GrDawnImageInfo& info,
                                                     GrMipMapsStatus mipMapsStatus)
        : GrSurface(gpu, size, config, GrProtected::kNo)
        , GrDawnTexture(gpu, size, config, textureView, info, mipMapsStatus)
        , GrDawnRenderTarget(gpu, size, config, sampleCnt, info) {
}

bool GrDawnTextureRenderTarget::canAttemptStencilAttachment() const {
    return true;
}

size_t GrDawnTextureRenderTarget::onGpuMemorySize() const {
    return GrSurface::ComputeSize(this->config(), this->width(), this->height(),
                                  1, // FIXME: for MSAA
                                  this->texturePriv().mipMapped());
}
