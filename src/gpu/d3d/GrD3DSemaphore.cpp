/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/d3d/GrD3DSemaphore.h"

#include "src/gpu/d3d/GrD3DGpu.h"


std::unique_ptr<GrD3DSemaphore> GrD3DSemaphore::Make(GrD3DGpu* gpu) {
    GrD3DFenceInfo fenceInfo;
    gpu->device()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fenceInfo.fFence));
    fenceInfo.fValue = 1;

    return std::unique_ptr<GrD3DSemaphore>(new GrD3DSemaphore(fenceInfo));
}

std::unique_ptr<GrD3DSemaphore> GrD3DSemaphore::MakeWrapped(const GrD3DFenceInfo& fenceInfo) {
    return std::unique_ptr<GrD3DSemaphore>(new GrD3DSemaphore(fenceInfo));
}

GrD3DSemaphore::GrD3DSemaphore(const GrD3DFenceInfo& fenceInfo) : fFenceInfo(fenceInfo) {}

GrBackendSemaphore GrD3DSemaphore::backendSemaphore() const {
    GrBackendSemaphore backendSemaphore;
    backendSemaphore.initDirect3D(fFenceInfo);
    return backendSemaphore;
}
