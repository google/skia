/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrNXTTextureRenderTarget.h"

#include "GrContext.h"
#include "GrNXTGpu.h"
#include "GrTexturePriv.h"
#include "SkTraceMemoryDump.h"

GrNXTTextureRenderTarget::GrNXTTextureRenderTarget(GrNXTGpu* gpu,
                                                   nxt::Texture texture,
                                                   nxt::TextureView textureView,
                                                   const GrSurfaceDesc& desc,
                                                   const GrNXTImageInfo& info,
                                                   GrMipMapsStatus mipMapsStatus)
        : GrSurface(gpu, desc)
        , GrNXTTexture(gpu, texture.Clone(), textureView.Clone(), desc, info, mipMapsStatus)
        , GrNXTRenderTarget(gpu, desc, info) {
}

// GrNXTTextureRenderTarget must dump both of its superclasses.
void GrNXTTextureRenderTarget::dumpMemoryStatistics(
    SkTraceMemoryDump* traceMemoryDump) const {
  GrNXTRenderTarget::dumpMemoryStatistics(traceMemoryDump);

  // Also dump the GrNXTTexture's memory. Due to this resource having both a
  // texture and a
  // renderbuffer component, dump as skia/gpu_resources/resource_#/texture
  SkString dumpName("skia/gpu_resources/resource_");
  dumpName.appendU32(this->uniqueID().asUInt());
  dumpName.append("/texture");

  // Use the texture's gpuMemorySize, not our own, which includes the
  // renderbuffer as well.
  size_t size = GrNXTTexture::gpuMemorySize();

  traceMemoryDump->dumpNumericValue(dumpName.c_str(), "size", "bytes", size);

  if (this->isPurgeable()) {
    traceMemoryDump->dumpNumericValue(dumpName.c_str(), "purgeable_size",
                                      "bytes", size);
  }
}

bool GrNXTTextureRenderTarget::canAttemptStencilAttachment() const {
    return true;
}

size_t GrNXTTextureRenderTarget::onGpuMemorySize() const {
    return GrSurface::ComputeSize(this->config(), this->width(), this->height(),
                                  1, // F(XME: for MSAA
                                  this->texturePriv().mipMapped());
}
