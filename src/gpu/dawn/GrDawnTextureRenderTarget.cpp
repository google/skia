/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDawnTextureRenderTarget.h"

#include "include/gpu/GrContext.h"
#include "src/gpu/dawn/GrDawnGpu.h"
#include "src/gpu/GrTexturePriv.h"
#include "include/core/SkTraceMemoryDump.h"

GrDawnTextureRenderTarget::GrDawnTextureRenderTarget(GrDawnGpu* gpu,
                                                   dawn::Texture texture,
                                                   dawn::TextureView textureView,
                                                   const GrSurfaceDesc& desc,
                                                   const GrDawnImageInfo& info,
                                                   GrMipMapsStatus mipMapsStatus)
        : GrSurface(gpu, desc)
        , GrDawnTexture(gpu, texture, textureView, desc, info, mipMapsStatus)
        , GrDawnRenderTarget(gpu, desc, info) {
}

// GrDawnTextureRenderTarget must dump both of its superclasses.
void GrDawnTextureRenderTarget::dumpMemoryStatistics(
    SkTraceMemoryDump* traceMemoryDump) const {
  GrDawnRenderTarget::dumpMemoryStatistics(traceMemoryDump);

  // Also dump the GrDawnTexture's memory. Due to this resource having both a
  // texture and a
  // renderbuffer component, dump as skia/gpu_resources/resource_#/texture
  SkString dumpName("skia/gpu_resources/resource_");
  dumpName.appendU32(this->uniqueID().asUInt());
  dumpName.append("/texture");

  // Use the texture's gpuMemorySize, not our own, which includes the
  // renderbuffer as well.
  size_t size = GrDawnTexture::gpuMemorySize();

  traceMemoryDump->dumpNumericValue(dumpName.c_str(), "size", "bytes", size);

  if (this->resourcePriv().isPurgeable()) {
    traceMemoryDump->dumpNumericValue(dumpName.c_str(), "purgeable_size",
                                      "bytes", size);
  }
}

bool GrDawnTextureRenderTarget::canAttemptStencilAttachment() const {
    return true;
}

size_t GrDawnTextureRenderTarget::onGpuMemorySize() const {
    return GrSurface::ComputeSize(this->config(), this->width(), this->height(),
                                  1, // F(XME: for MSAA
                                  this->texturePriv().mipMapped());
}
