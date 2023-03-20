/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef ProxyUtils_DEFINED
#define ProxyUtils_DEFINED

#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/ganesh/GrImageInfo.h"
#include "src/gpu/ganesh/GrPipeline.h"
#include "src/gpu/ganesh/GrTextureProxy.h"

class GrDirectContext;
class GrProgramInfo;
class GrCPixmap;

namespace sk_gpu_test {

/** Returns the proxy backing an image if it is texture backed, otherwise nullptr. */
GrTextureProxy* GetTextureImageProxy(SkImage*, GrRecordingContext*);

/** Makes a texture proxy containing the passed in color data. */
GrSurfaceProxyView MakeTextureProxyViewFromData(GrDirectContext*,
                                                GrRenderable,
                                                GrSurfaceOrigin,
                                                GrCPixmap pixmap);

#if defined(SK_GANESH)
GrProgramInfo* CreateProgramInfo(const GrCaps*,
                                 SkArenaAlloc*,
                                 const GrSurfaceProxyView& writeView,
                                 bool usesMSAASurface,
                                 GrAppliedClip&&,
                                 const GrDstProxyView&,
                                 GrGeometryProcessor*,
                                 SkBlendMode,
                                 GrPrimitiveType,
                                 GrXferBarrierFlags renderPassXferBarriers,
                                 GrLoadOp colorLoadOp,
                                 GrPipeline::InputFlags flags = GrPipeline::InputFlags::kNone,
                                 const GrUserStencilSettings* stencil =
                                                                &GrUserStencilSettings::kUnused);
#endif

}  // namespace sk_gpu_test

#endif
