/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/d3d/GrD3DCommandList.h"

#include "src/gpu/d3d/GrD3DGpu.h"

GrD3DCommandList::GrD3DCommandList(gr_cp<ID3D12CommandAllocator> allocator,
                                   gr_cp<ID3D12GraphicsCommandList> commandList)
    : fCommandList(std::move(commandList))
    , fAllocator(std::move(allocator)) {
}

void GrD3DCommandList::close() {
    SkASSERT(fIsActive);
    SkDEBUGCODE(HRESULT hr = ) fCommandList->Close();
    SkASSERT(SUCCEEDED(hr));
    SkDEBUGCODE(fIsActive = false;)
}

void GrD3DCommandList::submit(ID3D12CommandQueue* queue) {
    SkASSERT(!fIsActive);
    ID3D12CommandList* ppCommandLists[] = { fCommandList.get() };
    queue->ExecuteCommandLists(1, ppCommandLists);
}

void GrD3DCommandList::reset() {
    SkASSERT(!fIsActive);
    SkDEBUGCODE(HRESULT hr = ) fAllocator->Reset();
    SkASSERT(SUCCEEDED(hr));
    SkDEBUGCODE(hr = ) fCommandList->Reset(fAllocator.get(), nullptr);
    SkASSERT(SUCCEEDED(hr));

    SkDEBUGCODE(fIsActive = true;)
}

void GrD3DCommandList::releaseResources() {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    SkASSERT(!fIsActive);
    for (int i = 0; i < fTrackedResources.count(); ++i) {
        fTrackedResources[i]->notifyFinishedWithWorkOnGpu();
        fTrackedResources[i]->unref();
    }
    for (int i = 0; i < fTrackedRecycledResources.count(); ++i) {
        fTrackedRecycledResources[i]->notifyFinishedWithWorkOnGpu();
        fTrackedRecycledResources[i]->recycle();
    }

    if (++fNumResets > kNumRewindResetsBeforeFullReset) {
        fTrackedResources.reset();
        fTrackedRecycledResources.reset();
        fTrackedResources.setReserve(kInitialTrackedResourcesCount);
        fTrackedRecycledResources.setReserve(kInitialTrackedResourcesCount);
        fNumResets = 0;
    } else {
        fTrackedResources.rewind();
        fTrackedRecycledResources.rewind();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::unique_ptr<GrD3DDirectCommandList> GrD3DDirectCommandList::Make(ID3D12Device* device) {
    gr_cp<ID3D12CommandAllocator> allocator;
    SkDEBUGCODE(HRESULT hr = ) device->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator));
    SkASSERT(SUCCEEDED(hr));

    gr_cp<ID3D12GraphicsCommandList> commandList;
    SkDEBUGCODE(hr = ) device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                                                 allocator.get(), nullptr,
                                                 IID_PPV_ARGS(&commandList));
    SkASSERT(SUCCEEDED(hr));

    auto grCL = new GrD3DDirectCommandList(std::move(allocator), std::move(commandList));
    return std::unique_ptr<GrD3DDirectCommandList>(grCL);
}

GrD3DDirectCommandList::GrD3DDirectCommandList(gr_cp<ID3D12CommandAllocator> allocator,
                                               gr_cp<ID3D12GraphicsCommandList> commandList)
    : GrD3DCommandList(std::move(allocator), std::move(commandList)) {
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::unique_ptr<GrD3DCopyCommandList> GrD3DCopyCommandList::Make(ID3D12Device* device) {
    gr_cp<ID3D12CommandAllocator> allocator;
    SkDEBUGCODE(HRESULT hr = ) device->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator));
    SkASSERT(SUCCEEDED(hr));

    gr_cp<ID3D12GraphicsCommandList> commandList;
    SkDEBUGCODE(hr = ) device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COPY, allocator.get(),
                                                 nullptr, IID_PPV_ARGS(&commandList));
    SkASSERT(SUCCEEDED(hr));
    auto grCL = new GrD3DCopyCommandList(std::move(allocator), std::move(commandList));
    return std::unique_ptr<GrD3DCopyCommandList>(grCL);
}

GrD3DCopyCommandList::GrD3DCopyCommandList(gr_cp<ID3D12CommandAllocator> allocator,
                                           gr_cp<ID3D12GraphicsCommandList> commandList)
    : GrD3DCommandList(std::move(allocator), std::move(commandList)) {
}
