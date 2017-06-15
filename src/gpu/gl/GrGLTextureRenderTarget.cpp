/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLTextureRenderTarget.h"

#include "GrContext.h"
#include "GrGLGpu.h"
#include "GrTexturePriv.h"
#include "SkTraceMemoryDump.h"

GrGLTextureRenderTarget::GrGLTextureRenderTarget(GrGLGpu* gpu,
                                                 SkBudgeted budgeted,
                                                 const GrSurfaceDesc& desc,
                                                 const GrGLTexture::IDDesc& texIDDesc,
                                                 const GrGLRenderTarget::IDDesc& rtIDDesc,
                                                 bool wasMipMapDataProvided)
        : GrSurface(gpu, desc)
        , GrGLTexture(gpu, desc, texIDDesc, wasMipMapDataProvided)
        , GrGLRenderTarget(gpu, desc, rtIDDesc) {
    this->registerWithCache(budgeted);
}

GrGLTextureRenderTarget::GrGLTextureRenderTarget(GrGLGpu* gpu,
                                                 const GrSurfaceDesc& desc,
                                                 const GrGLTexture::IDDesc& texIDDesc,
                                                 const GrGLRenderTarget::IDDesc& rtIDDesc,
                                                 bool wasMipMapDataProvided)
        : GrSurface(gpu, desc)
        , GrGLTexture(gpu, desc, texIDDesc, wasMipMapDataProvided)
        , GrGLRenderTarget(gpu, desc, rtIDDesc) {
    this->registerWithCacheWrapped();
}

// GrGLTextureRenderTarget must dump both of its superclasses.
void GrGLTextureRenderTarget::dumpMemoryStatistics(
    SkTraceMemoryDump* traceMemoryDump) const {
  GrGLRenderTarget::dumpMemoryStatistics(traceMemoryDump);

  // Also dump the GrGLTexture's memory. Due to this resource having both a
  // texture and a
  // renderbuffer component, dump as skia/gpu_resources/resource_#/texture
  SkString dumpName("skia/gpu_resources/resource_");
  dumpName.appendU32(this->uniqueID().asUInt());
  dumpName.append("/texture");

  // Use the texture's gpuMemorySize, not our own, which includes the
  // renderbuffer as well.
  size_t size = GrGLTexture::gpuMemorySize();

  traceMemoryDump->dumpNumericValue(dumpName.c_str(), "size", "bytes", size);

  if (this->isPurgeable()) {
    traceMemoryDump->dumpNumericValue(dumpName.c_str(), "purgeable_size",
                                      "bytes", size);
  }

  SkString texture_id;
  texture_id.appendU32(this->textureID());
  traceMemoryDump->setMemoryBacking(dumpName.c_str(), "gl_texture",
                                    texture_id.c_str());
}

bool GrGLTextureRenderTarget::canAttemptStencilAttachment() const {
    // The RT FBO of GrGLTextureRenderTarget is never created from a
    // wrapped FBO, so we only care about the flag.
    return !this->getGpu()->getContext()->caps()->avoidStencilBuffers();
}

sk_sp<GrGLTextureRenderTarget> GrGLTextureRenderTarget::MakeWrapped(
    GrGLGpu* gpu, const GrSurfaceDesc& desc,
    const GrGLTexture::IDDesc& texIDDesc, const GrGLRenderTarget::IDDesc& rtIDDesc)
{
    return sk_sp<GrGLTextureRenderTarget>(
        new GrGLTextureRenderTarget(gpu, desc, texIDDesc, rtIDDesc, false));
}

size_t GrGLTextureRenderTarget::onGpuMemorySize() const {
    return GrSurface::ComputeSize(this->config(), this->width(), this->height(),
                                    this->numSamplesOwnedPerPixel(),
                                    this->texturePriv().hasMipMaps());
}
