/*
 * Copyright 2018 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrCaps.h"
#include "GrGpu.h"
#include "GrPath.h"
#include "GrPathRenderer.h"
#include "GrPathRendering.h"
#include "GrResourceProvider.h"
#include "SkTypes.h"
#include "gl/GrGLGpu.h"
#include "gl/GrGLPathRendering.h"
#include "ops/GrStencilAndCoverPathRenderer.h"
#include "ops/GrStencilPathOp.h"

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
                                            GrPathRendering::FillType,
                                            bool,
                                            const GrScissorState&,
                                            const GrPath*) { return nullptr; }

void GrPath::ComputeKey(const GrShape&, GrUniqueKey*, bool*) {}
