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

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-dynamic-exception-spec"
#endif
#include "D3D12MemAlloc.h"  // NO_G3_REWRITE
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

class GrD3DAMDMemoryAllocator : public GrD3DMemoryAllocator {
public:
    static sk_sp<GrD3DMemoryAllocator> Make(IDXGIAdapter* adapter, ID3D12Device* device);

    ~GrD3DAMDMemoryAllocator() override { fAllocator->Release(); }

    gr_cp<ID3D12Resource> createResource(D3D12_HEAP_TYPE, const D3D12_RESOURCE_DESC*,
                                         D3D12_RESOURCE_STATES initialResourceState,
                                         sk_sp<GrD3DAlloc>* allocation,
                                         const D3D12_CLEAR_VALUE*) override;

    gr_cp<ID3D12Resource> createAliasingResource(sk_sp<GrD3DAlloc>& allocation,
                                                 uint64_t localOffset,
                                                 const D3D12_RESOURCE_DESC*,
                                                 D3D12_RESOURCE_STATES initialResourceState,
                                                 const D3D12_CLEAR_VALUE*) override;

    class Alloc : public GrD3DAlloc {
    public:
        Alloc(D3D12MA::Allocation* allocation) : fAllocation(allocation) {}
        ~Alloc() override {
            fAllocation->Release();
        }
    private:
        friend class GrD3DAMDMemoryAllocator;
        D3D12MA::Allocation* fAllocation;
    };

private:
    GrD3DAMDMemoryAllocator(D3D12MA::Allocator* allocator) : fAllocator(allocator) {}

    D3D12MA::Allocator* fAllocator;
};

#endif
