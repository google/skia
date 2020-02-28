/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef ProxyUtils_DEFINED
#define ProxyUtils_DEFINED

#include "include/private/GrTypesPriv.h"
#include "src/gpu/GrImageInfo.h"
#include "src/gpu/GrPipeline.h"
#include "src/gpu/GrTextureProxy.h"

class GrProgramInfo;

namespace sk_gpu_test {

/** Makes a texture proxy containing the passed in color data. */
sk_sp<GrTextureProxy> MakeTextureProxyFromData(GrContext*,
                                               GrRenderable,
                                               GrSurfaceOrigin,
                                               const GrImageInfo&,
                                               const void* data,
                                               size_t rowBytes);

GrProgramInfo* CreateProgramInfo(const GrCaps*,
                                 SkArenaAlloc*,
                                 const GrSurfaceProxyView* outputView,
                                 GrAppliedClip&&,
                                 const GrXferProcessor::DstProxyView&,
                                 GrGeometryProcessor*,
                                 SkBlendMode,
                                 GrPrimitiveType,
                                 GrPipeline::InputFlags flags = GrPipeline::InputFlags::kNone,
                                 const GrUserStencilSettings* stencil =
                                                                &GrUserStencilSettings::kUnused);


}  // namespace sk_gpu_test

#endif
