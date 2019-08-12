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
                                                     dawn::Texture texture,
                                                     dawn::TextureView textureView,
                                                     const GrSurfaceDesc& desc,
                                                     int sampleCnt,
                                                     const GrDawnImageInfo& info,
                                                     GrMipMapsStatus mipMapsStatus)
        : GrSurface(gpu, {desc.fWidth, desc.fHeight}, desc.fConfig, GrProtected::kNo)
        , GrDawnTexture(gpu, texture, textureView, desc, info, mipMapsStatus)
        , GrDawnRenderTarget(gpu, desc, sampleCnt, info) {
}

bool GrDawnTextureRenderTarget::canAttemptStencilAttachment() const {
    return true;
}

size_t GrDawnTextureRenderTarget::onGpuMemorySize() const {
    return GrSurface::ComputeSize(this->config(), this->width(), this->height(),
                                  1, // F(XME: for MSAA
                                  this->texturePriv().mipMapped());
}
