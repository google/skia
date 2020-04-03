/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrD3DBuffer_DEFINED

#define GrD3DBuffer_DEFINED

#include "include/gpu/d3d/GrD3DTypes.h"
#include "src/gpu/GrGpuBuffer.h"
#include "src/gpu/GrManagedResource.h"

class GrD3DGpu;

class GrD3DBuffer : public GrGpuBuffer {
private:
    class Resource;

public:
    static sk_sp<GrD3DBuffer> Make(GrD3DGpu*, size_t size, GrGpuBufferType, GrAccessPattern);

    ~GrD3DBuffer() override {
        // release should have been called by the owner of this object.
        SkASSERT(!fResource);
    }

    ID3D12Resource* d3dResource() const {
        SkASSERT(fResource);
        return fResource->fD3DResource.get();
    }
    const Resource* resource() const {
        SkASSERT(fResource);
        return fResource.get();
    }

protected:
    GrD3DBuffer(GrD3DGpu*, size_t size, GrGpuBufferType, GrAccessPattern, const sk_sp<Resource>&,
                D3D12_RESOURCE_STATES);

    void onAbandon() override;
    void onRelease() override;

    D3D12_RESOURCE_STATES fResourceState;

private:
    void onMap() override {} // TODO
    void onUnmap() override {} // TODO
    bool onUpdateData(const void* src, size_t srcSizeInBytes) override { return false; } // TODO

    class Resource : public GrRecycledResource {
    public:
        explicit Resource()
            : fD3DResource(nullptr) {
        }

        Resource(const gr_cp<ID3D12Resource>& bufferResource)
            : fD3DResource(bufferResource) {
        }

        static sk_sp<Resource> Make(GrD3DGpu*, size_t size, GrGpuBufferType, GrAccessPattern,
                                    D3D12_RESOURCE_STATES*);

#ifdef SK_TRACE_MANAGED_RESOURCES
        void dumpInfo() const override {
            SkDebugf("GrD3DBuffer: %d (%d refs)\n", fD3DResource.get(), this->getRefCnt());
        }
#endif
        mutable gr_cp<ID3D12Resource> fD3DResource;

    private:
        void freeGPUData() const override {
            fD3DResource.reset();
        }
        void onRecycle() const override { this->unref(); } // TODO

        typedef GrRecycledResource INHERITED;
    };

    sk_sp<Resource> fResource;

    typedef GrGpuBuffer INHERITED;
};

#endif
