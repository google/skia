/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/d3d/GrD3DAMDMemoryAllocator.h"
#include "src/gpu/d3d/GrD3DUtil.h"

sk_sp<GrD3DMemoryAllocator> GrD3DAMDMemoryAllocator::Make(IDXGIAdapter* adapter,
                                                             ID3D12Device* device) {
    D3D12MA::ALLOCATOR_DESC allocatorDesc = {};
    allocatorDesc.pAdapter = adapter;
    allocatorDesc.pDevice = device;
    allocatorDesc.Flags = D3D12MA::ALLOCATOR_FLAG_SINGLETHREADED; // faster if we're single-threaded

    D3D12MA::Allocator* allocator;
    HRESULT hr = D3D12MA::CreateAllocator(&allocatorDesc, &allocator);
    if (!SUCCEEDED(hr)) {
        return nullptr;
    }

    return sk_sp<GrD3DMemoryAllocator>(new GrD3DAMDMemoryAllocator(allocator));
}

gr_cp<ID3D12Resource> GrD3DAMDMemoryAllocator::createResource(
        D3D12_HEAP_TYPE heapType, const D3D12_RESOURCE_DESC* resourceDesc,
        D3D12_RESOURCE_STATES initialResourceState, sk_sp<GrD3DAlloc>* allocation,
        const D3D12_CLEAR_VALUE* clearValue) {
    D3D12MA::ALLOCATION_DESC allocationDesc = {};
    allocationDesc.HeapType = heapType;
    // TODO: Determine flags. For now create new heaps regardless of budget,
    //       and always suballocate and use CreatePlacedResource.
    // allocationDesc.Flags = ?

    gr_cp<ID3D12Resource> resource;
    D3D12MA::Allocation* d3d12maAllocation;
    HRESULT hr = fAllocator->CreateResource(&allocationDesc, resourceDesc,
                                            initialResourceState, clearValue,
                                            &d3d12maAllocation, IID_PPV_ARGS(&resource));
    if (!SUCCEEDED(hr)) {
        return nullptr;
    }

    allocation->reset(new Alloc(d3d12maAllocation));
    return resource;
}
