/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrD3DAMDMemoryAllocator_DEFINED
#define GrD3DAMDMemoryAllocator_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/d3d/GrD3DTypes.h"

#include "src/D3D12MemAlloc.h"


class GrD3DAMDMemoryAllocator : public SkRefCnt {
public:
    static sk_sp<GrD3DAMDMemoryAllocator> Make(IDXGIAdapter* adapter, ID3D12Device* device);

    ~GrD3DAMDMemoryAllocator() { fAllocator->Release(); }

    class Allocation : public SkRefCnt {
    public:
        Allocation(D3D12MA::Allocation* allocation) : fAllocation(allocation) {}

    private:
        D3D12MA::Allocation* fAllocation;
    };

    ComPtr<ID3D12Resource> createResource(D3D12_HEAP_TYPE, const D3D12_RESOURCE_DESC*,
                                          D3D12_RESOURCE_STATES initialResourceState,
                                          sk_sp<Allocation>* allocation,
                                          const D3D12_CLEAR_VALUE*, REFIID riidResource,
                                          void** ppvResource);

    ComPtr<ID3D12Resource> createAliasingResource(D3D12_HEAP_TYPE, const D3D12_RESOURCE_DESC*,
                                                  D3D12_RESOURCE_STATES initialResourceState,
                                                  sk_sp<Allocation>* allocation,
                                                  const D3D12_CLEAR_VALUE*, REFIID riidResource,
                                                  void** ppvResource);

private:
    GrD3DAMDMemoryAllocator(D3D12MA::Allocator* allocator)
            : fAllocator(allocator) {}

    D3D12MA::Allocator* fAllocator;
};

#endif
