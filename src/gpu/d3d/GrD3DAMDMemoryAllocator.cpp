/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/d3d/GrD3DAMDMemoryAllocator.h"
#include "src/gpu/d3d/GrD3DUtil.h"

sk_sp<GrD3DAMDMemoryAllocator> GrD3DAMDMemoryAllocator::Make(IDXGIAdapter* adapter,
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

    return sk_sp<GrD3DAMDMemoryAllocator>(new GrD3DAMDMemoryAllocator(allocator));
}
