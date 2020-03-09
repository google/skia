/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/d3d/GrD3DResourceProvider.h"

#include "src/gpu/d3d/GrD3DGpu.h"

GrD3DResourceProvider::GrD3DResourceProvider(GrD3DGpu* gpu) : fGpu(gpu) {
    SkDEBUGCODE(HRESULT hr = ) gpu->device()->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&fDirectCommandAllocator));
    SkASSERT(SUCCEEDED(hr));
}

gr_cp<ID3D12GraphicsCommandList> GrD3DResourceProvider::findOrCreateDirectCommandList() {
    gr_cp<ID3D12GraphicsCommandList> commandList;

    SkDEBUGCODE(HRESULT hr = ) fGpu->device()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
        fDirectCommandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList));
    SkASSERT(SUCCEEDED(hr));

    return commandList;
}
