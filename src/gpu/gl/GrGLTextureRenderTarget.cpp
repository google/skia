/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLTextureRenderTarget.h"

#include "SkTraceMemoryDump.h"

// GrGLTextureRenderTarget must dump both of its superclasses.
void GrGLTextureRenderTarget::dumpMemoryStatistics(
    SkTraceMemoryDump* traceMemoryDump) const {
  GrGLRenderTarget::dumpMemoryStatistics(traceMemoryDump);

  // Also dump the GrGLTexture's memory. Due to this resource having both a
  // texture and a
  // renderbuffer component, dump as skia/gpu_resources/resource_#/texture
  SkString dumpName("skia/gpu_resources/resource_");
  dumpName.appendS32(this->getUniqueID());
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
