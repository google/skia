/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/d3d/GrD3DCommandList.h"

#include "src/gpu/d3d/GrD3DGpu.h"

GrD3DCommandList::GrD3DCommandList(gr_cp<ID3D12GraphicsCommandList> commandList)
    : fCommandList(std::move(commandList)) {
}

 ////////////////////////////////////////////////////////////////////////////////////////////////////

std::unique_ptr<GrD3DDirectCommandList> GrD3DDirectCommandList::Make(
        ID3D12Device* device, ID3D12CommandAllocator* cmdAllocator) {
    gr_cp<ID3D12GraphicsCommandList> commandList;

    SkDEBUGCODE(HRESULT hr = ) device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                                                         cmdAllocator, nullptr,
                                                         IID_PPV_ARGS(&commandList));
    SkASSERT(SUCCEEDED(hr));

    auto grCL = new GrD3DDirectCommandList(std::move(commandList));
    return std::unique_ptr<GrD3DDirectCommandList>(grCL);
}

GrD3DDirectCommandList::GrD3DDirectCommandList(gr_cp<ID3D12GraphicsCommandList> commandList)
    : GrD3DCommandList(std::move(commandList)) {
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::unique_ptr<GrD3DCopyCommandList> GrD3DCopyCommandList::Make(
        ID3D12Device* device, ID3D12CommandAllocator* cmdAllocator) {
    gr_cp<ID3D12GraphicsCommandList> commandList;

    SkDEBUGCODE(HRESULT hr = ) device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COPY,
                                                         cmdAllocator, nullptr,
                                                         IID_PPV_ARGS(&commandList));
    SkASSERT(SUCCEEDED(hr));
    auto grCL = new GrD3DCopyCommandList(std::move(commandList));
    return std::unique_ptr<GrD3DCopyCommandList>(grCL);
}

GrD3DCopyCommandList::GrD3DCopyCommandList(gr_cp<ID3D12GraphicsCommandList> commandList)
    : GrD3DCommandList(std::move(commandList)) {
}
