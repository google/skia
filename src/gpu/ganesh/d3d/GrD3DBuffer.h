/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrD3DBuffer_DEFINED

#define GrD3DBuffer_DEFINED

#include "include/gpu/ganesh/d3d/GrD3DTypes.h"
#include "src/gpu/ganesh/GrGpuBuffer.h"
#include "src/gpu/ganesh/GrManagedResource.h"

class GrD3DGpu;

class GrD3DBuffer : public GrGpuBuffer {
public:
    static sk_sp<GrD3DBuffer> Make(GrD3DGpu*, size_t size, GrGpuBufferType, GrAccessPattern);

    ~GrD3DBuffer() override {}

    ID3D12Resource* d3dResource() const {
        SkASSERT(fD3DResource);
        return fD3DResource.get();
    }

    void setResourceState(const GrD3DGpu* gpu, D3D12_RESOURCE_STATES newResourceState);

protected:
    GrD3DBuffer(GrD3DGpu*, size_t size, GrGpuBufferType, GrAccessPattern, gr_cp<ID3D12Resource>,
                sk_sp<GrD3DAlloc>, D3D12_RESOURCE_STATES, std::string_view label);

    void onAbandon() override;
    void onRelease() override;

    D3D12_RESOURCE_STATES fResourceState;

private:
    void releaseResource();

    void onMap(MapType) override;
    void onUnmap(MapType) override;
    bool onClearToZero() override;
    bool onUpdateData(const void* src, size_t offset, size_t size, bool preserve) override;

    void* internalMap(MapType, size_t offset, size_t size);
    void internalUnmap(MapType, size_t offset, size_t size);

#ifdef SK_DEBUG
    void validate() const;
#endif

    void onSetLabel() override;

    GrD3DGpu* getD3DGpu() const {
        SkASSERT(!this->wasDestroyed());
        return reinterpret_cast<GrD3DGpu*>(this->getGpu());
    }

    gr_cp<ID3D12Resource> fD3DResource;
    sk_sp<GrD3DAlloc> fAlloc;
    ID3D12Resource* fStagingBuffer = nullptr;
    size_t fStagingOffset = 0;

    using INHERITED = GrGpuBuffer;
};

#endif
