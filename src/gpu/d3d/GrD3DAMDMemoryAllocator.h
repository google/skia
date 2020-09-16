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

class GrD3DAMDMemoryAllocator : public GrD3DMemoryAllocator {
public:
    static sk_sp<GrD3DMemoryAllocator> Make(IDXGIAdapter* adapter, ID3D12Device* device);

    ~GrD3DAMDMemoryAllocator() override { fAllocator->Release(); }

    ComPtr<ID3D12Resource> createResource(D3D12_HEAP_TYPE, const D3D12_RESOURCE_DESC*,
                                          D3D12_RESOURCE_STATES initialResourceState,
                                          sk_sp<GrD3DAlloc>* allocation,
                                          const D3D12_CLEAR_VALUE*, REFIID riidResource,
                                          void** ppvResource) override;

    class Alloc : public GrD3DAlloc {
    public:
        Alloc(D3D12MA::Allocation* allocation)
                : fAllocation(allocation) {}
        ~Alloc() {
            fAllocation->Release();
        }
    private:
        D3D12MA::Allocation* fAllocation;
    };

private:
    GrD3DAMDMemoryAllocator(D3D12MA::Allocator* allocator)
            : fAllocator(allocator) {}

    D3D12MA::Allocator* fAllocator;
};

#endif
