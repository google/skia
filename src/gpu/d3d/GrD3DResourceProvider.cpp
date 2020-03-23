/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/d3d/GrD3DResourceProvider.h"

#include "src/gpu/d3d/GrD3DCommandList.h"
#include "src/gpu/d3d/GrD3DGpu.h"

GrD3DResourceProvider::GrD3DResourceProvider(GrD3DGpu* gpu) : fGpu(gpu) {
    SkDEBUGCODE(HRESULT hr = ) gpu->device()->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&fDirectCommandAllocator));
    SkASSERT(SUCCEEDED(hr));
}

std::unique_ptr<GrD3DDirectCommandList> GrD3DResourceProvider::findOrCreateDirectCommandList() {
    return GrD3DDirectCommandList::Make(fGpu->device(), fDirectCommandAllocator.Get());
}
