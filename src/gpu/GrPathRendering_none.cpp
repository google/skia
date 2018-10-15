/*
 * Copyright 2018 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrPath.h"
#include "GrCaps.h"
#include "gl/GrGLPathRendering.h"
#include "GrGpu.h"
#include "GrPathRenderer.h"
#include "GrPathRendering.h"
#include "GrResourceProvider.h"
#include "SkTypes.h"
#include "gl/GrGLGpu.h"
#include "ops/GrStencilAndCoverPathRenderer.h"

GrPathRenderer* GrStencilAndCoverPathRenderer::Create(GrResourceProvider* resourceProvider,
                                                      const GrCaps& caps) {
    SkDEBUGFAIL("NVPR disabled");
    return nullptr;
}

GrGLPathRendering::GrGLPathRendering(GrGLGpu* gpu)
    : GrPathRendering(gpu)
    , fPreallocatedPathCount(0) {}

GrGLPathRendering::~GrGLPathRendering() {}

void GrGLPathRendering::disconnect(GrGpu::DisconnectType type) {}

void GrGLPathRendering::resetContext() {}

void GrGLPathRendering::setProgramPathFragmentInputTransform(GrGLuint program, GrGLint location,
                                                             GrGLenum genMode, GrGLint components,
                                                             const SkMatrix& matrix) {}

void GrGLPathRendering::setProjectionMatrix(const SkMatrix& matrix,
                                            const SkISize& renderTargetSize,
                                            GrSurfaceOrigin renderTargetOrigin) {}

sk_sp<GrPath> GrGLPathRendering::createPath(const SkPath& inPath, const GrStyle& style) {
    return nullptr;
}

void GrGLPathRendering::onDrawPath(const GrPrimitiveProcessor& primProc,
                                   const GrPipeline& pipeline,
                                   const GrPipeline::FixedDynamicState& fixedDynamicState,
                                   const GrStencilSettings& stencilPassSettings,
                                   const GrPath* path) {}

void GrGLPathRendering::onStencilPath(const StencilPathArgs& args, const GrPath* path) {}

