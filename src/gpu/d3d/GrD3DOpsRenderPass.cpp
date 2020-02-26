/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/d3d/GrD3DOpsRenderPass.h"

#include "src/gpu/d3d/GrD3DGpu.h"

GrD3DOpsRenderPass::GrD3DOpsRenderPass(GrD3DGpu* gpu) : fGpu(gpu) {}

bool GrD3DOpsRenderPass::set(GrRenderTarget* rt, GrSurfaceOrigin origin, const SkIRect& bounds,
                             const GrOpsRenderPass::LoadAndStoreInfo& colorInfo,
                             const GrOpsRenderPass::StencilLoadAndStoreInfo& stencilInfo,
                             const SkTArray<GrSurfaceProxy*, true>& sampledProxies) {
    return true;
}

GrD3DOpsRenderPass::~GrD3DOpsRenderPass() {}

GrGpu* GrD3DOpsRenderPass::gpu() { return fGpu; }
