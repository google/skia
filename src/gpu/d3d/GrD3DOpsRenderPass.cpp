/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/d3d/GrD3DOpsRenderPass.h"

#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrProgramDesc.h"
#include "src/gpu/GrRenderTargetPriv.h"
#include "src/gpu/d3d/GrD3DGpu.h"
#include "src/gpu/d3d/GrD3DPipelineState.h"
#include "src/gpu/d3d/GrD3DPipelineStateBuilder.h"

GrD3DOpsRenderPass::GrD3DOpsRenderPass(GrD3DGpu* gpu) : fGpu(gpu) {}

bool GrD3DOpsRenderPass::set(GrRenderTarget* rt, GrSurfaceOrigin origin, const SkIRect& bounds,
                             const GrOpsRenderPass::LoadAndStoreInfo& colorInfo,
                             const GrOpsRenderPass::StencilLoadAndStoreInfo& stencilInfo,
                             const SkTArray<GrSurfaceProxy*, true>& sampledProxies) {
    SkASSERT(!fRenderTarget);
    SkASSERT(fGpu == rt->getContext()->priv().getGpu());

    this->INHERITED::set(rt, origin);

    // TODO

    return true;
}

GrD3DOpsRenderPass::~GrD3DOpsRenderPass() {}

GrGpu* GrD3DOpsRenderPass::gpu() { return fGpu; }

bool GrD3DOpsRenderPass::onBindPipeline(const GrProgramInfo& info, const SkRect& drawBounds) {
    GrProgramDesc desc = fGpu->caps()->makeDesc(fRenderTarget, info);
    std::unique_ptr<GrD3DPipelineState> pipelineState =
        GrD3DPipelineStateBuilder::CreatePipelineState(fGpu, fRenderTarget, desc, info);

    // TODO: When this gets implemented fully make sure we bind the stencil reference value since
    // that is not part of the pipline in d3d12.
    return true;
}
