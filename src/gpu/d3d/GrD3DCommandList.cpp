/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/d3d/GrD3DCommandList.h"

#include "src/gpu/d3d/GrD3DBuffer.h"
#include "src/gpu/d3d/GrD3DGpu.h"
#include "src/gpu/d3d/GrD3DTextureResource.h"

GrD3DCommandList::GrD3DCommandList(gr_cp<ID3D12CommandAllocator> allocator,
                                   gr_cp<ID3D12GraphicsCommandList> commandList)
    : fCommandList(std::move(commandList))
    , fAllocator(std::move(allocator)) {
}

bool GrD3DCommandList::close() {
    SkASSERT(fIsActive);
    this->submitResourceBarriers();
    HRESULT hr = fCommandList->Close();
    SkDEBUGCODE(fIsActive = false;)
    return SUCCEEDED(hr);
}

GrD3DCommandList::SubmitResult GrD3DCommandList::submit(ID3D12CommandQueue* queue) {
    SkASSERT(fIsActive);
    if (!this->hasWork()) {
        return SubmitResult::kNoWork;
    }

    if (!this->close()) {
        return SubmitResult::kFailure;
    }
    SkASSERT(!fIsActive);
    ID3D12CommandList* ppCommandLists[] = { fCommandList.get() };
    queue->ExecuteCommandLists(1, ppCommandLists);

    return SubmitResult::kSuccess;
}

void GrD3DCommandList::reset() {
    SkASSERT(!fIsActive);
    SkDEBUGCODE(HRESULT hr = ) fAllocator->Reset();
    SkASSERT(SUCCEEDED(hr));
    SkDEBUGCODE(hr = ) fCommandList->Reset(fAllocator.get(), nullptr);
    SkASSERT(SUCCEEDED(hr));

    this->releaseResources();

    SkDEBUGCODE(fIsActive = true;)
    fHasWork = false;
}

void GrD3DCommandList::releaseResources() {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    if (fTrackedResources.count() == 0 && fTrackedRecycledResources.count() == 0) {
        return;
    }
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

////////////////////////////////////////////////////////////////////////////////
// GraphicsCommandList commands
////////////////////////////////////////////////////////////////////////////////

void GrD3DCommandList::resourceBarrier(const GrManagedResource* resource,
                                       int numBarriers,
                                       D3D12_RESOURCE_TRANSITION_BARRIER* barriers) {
    SkASSERT(fIsActive);
    SkASSERT(barriers);
    for (int i = 0; i < numBarriers; ++i) {
        // D3D will apply barriers in order so we can just add onto the end
        D3D12_RESOURCE_BARRIER& newBarrier = fResourceBarriers.push_back();
        newBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        newBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        newBarrier.Transition = barriers[i];
    }

    fHasWork = true;
    if (resource) {
        this->addResource(resource);
    }
}

void GrD3DCommandList::submitResourceBarriers() {
    SkASSERT(fIsActive);

    if (fResourceBarriers.count()) {
        fCommandList->ResourceBarrier(fResourceBarriers.count(), fResourceBarriers.begin());
        fResourceBarriers.reset();
    }
    SkASSERT(!fResourceBarriers.count());
}

void GrD3DCommandList::copyBufferToTexture(GrD3DBuffer* srcBuffer,
                                           GrD3DTextureResource* dstTexture,
                                           uint32_t subresourceCount,
                                           D3D12_PLACED_SUBRESOURCE_FOOTPRINT* bufferFootprints,
                                           int left, int top) {
    SkASSERT(fIsActive);
    SkASSERT(subresourceCount == 1 || (left == 0 && top == 0));

    this->addingWork();
    this->addResource(srcBuffer->resource());
    this->addResource(dstTexture->resource());
    for (uint32_t subresource = 0; subresource < subresourceCount; ++subresource) {
        D3D12_TEXTURE_COPY_LOCATION src = {};
        src.pResource = srcBuffer->d3dResource();
        src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
        src.PlacedFootprint = bufferFootprints[subresource];

        D3D12_TEXTURE_COPY_LOCATION dst = {};
        dst.pResource = dstTexture->d3dResource();
        dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        dst.SubresourceIndex = subresource;

        fCommandList->CopyTextureRegion(&dst, left, top, 0, &src, nullptr);
    }
}

void GrD3DCommandList::copyTextureRegion(const GrManagedResource* dst,
                                         const D3D12_TEXTURE_COPY_LOCATION* dstLocation,
                                         UINT dstX, UINT dstY,
                                         const GrManagedResource* src,
                                         const D3D12_TEXTURE_COPY_LOCATION* srcLocation,
                                         const D3D12_BOX* srcBox) {
    SkASSERT(fIsActive);

    this->addingWork();
    this->addResource(dst);
    this->addResource(src);
    fCommandList->CopyTextureRegion(dstLocation, dstX, dstY, 0, srcLocation, srcBox);
}

void GrD3DCommandList::addingWork() {
    this->submitResourceBarriers();
    fHasWork = true;
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
