/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrD3DAttachmentViewManager_DEFINED
#define GrD3DAttachmentViewManager_DEFINED

#include "src/gpu/d3d/GrD3DDescriptorHeap.h"

class GrD3DGpu;

class GrD3DAttachmentViewManager {
public:
    GrD3DAttachmentViewManager(GrD3DGpu*);

    D3D12_CPU_DESCRIPTOR_HANDLE createRenderTargetView(GrD3DGpu*, ID3D12Resource* textureResource);
    void recycleRenderTargetView(D3D12_CPU_DESCRIPTOR_HANDLE*);

    D3D12_CPU_DESCRIPTOR_HANDLE createDepthStencilView(GrD3DGpu*, ID3D12Resource* textureResource);
    void recycleDepthStencilView(D3D12_CPU_DESCRIPTOR_HANDLE*);

private:
    std::vector<sk_sp<GrD3DDescriptorHeap>> fRTVDescriptorHeaps;
    int fMaxAvailableRTVDescriptors;
    std::vector<sk_sp<GrD3DDescriptorHeap>> fDSVDescriptorHeaps;
    int fMaxAvailableDSVDescriptors;
};

#endif
