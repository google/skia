/*
 * Copyright 2018 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrPath.h"
#include "src/gpu/GrPathRenderer.h"
#include "src/gpu/GrPathRendering.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/gl/GrGLGpu.h"
#include "src/gpu/gl/GrGLPathRendering.h"
#include "src/gpu/ops/GrStencilAndCoverPathRenderer.h"
#include "src/gpu/ops/GrStencilPathOp.h"

class GrRecordingContext;

GrPathRenderer* GrStencilAndCoverPathRenderer::Create(GrResourceProvider* resourceProvider,
                                                      const GrCaps& caps) {
    return nullptr;
}

GrGLPathRendering::GrGLPathRendering(GrGLGpu* gpu)
    : GrPathRendering(gpu)
    , fPreallocatedPathCount(0) {}

GrGLPathRendering::~GrGLPathRendering() {}

void GrGLPathRendering::disconnect(GrGpu::DisconnectType) {}

void GrGLPathRendering::resetContext() {}

void GrGLPathRendering::setProgramPathFragmentInputTransform(GrGLuint, GrGLint,
                                                             GrGLenum, GrGLint,
                                                             const SkMatrix&) {}

void GrGLPathRendering::setProjectionMatrix(const SkMatrix&, const SkISize&, GrSurfaceOrigin) {}

sk_sp<GrPath> GrGLPathRendering::createPath(const SkPath&, const GrStyle&) { return nullptr; }

void GrGLPathRendering::onDrawPath(GrRenderTarget*, GrSurfaceOrigin,
                                   const GrPrimitiveProcessor&,
                                   const GrPipeline&,
                                   const GrPipeline::FixedDynamicState&,
                                   const GrStencilSettings&,
                                   const GrPath*) {}

void GrGLPathRendering::onStencilPath(const StencilPathArgs&, const GrPath*) {}

std::unique_ptr<GrOp> GrStencilPathOp::Make(GrRecordingContext*,
                                            const SkMatrix&,
                                            bool,
                                            bool,
                                            const GrScissorState&,
                                            sk_sp<const GrPath>) { return nullptr; }

void GrPath::ComputeKey(const GrShape&, GrUniqueKey*, bool*) {}
