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
        SkASSERT(!fMappedResource);
    }

    ID3D12Resource* d3dResource() const {
        SkASSERT(fResource);
        return fResource->fD3DResource.get();
    }
    sk_sp<Resource> resource() const {
        SkASSERT(fResource);
        return fResource;
    }

protected:
    GrD3DBuffer(GrD3DGpu*, size_t size, GrGpuBufferType, GrAccessPattern, const sk_sp<Resource>&,
                D3D12_RESOURCE_STATES);

    void onAbandon() override;
    void onRelease() override;

    D3D12_RESOURCE_STATES fResourceState;

private:
    void onMap() override;
    void onUnmap() override;
    bool onUpdateData(const void* src, size_t srcSizeInBytes) override;

    void internalMap(size_t size);
    void internalUnmap(size_t size);

    void validate() const;

    GrD3DGpu* getD3DGpu() const {
        SkASSERT(!this->wasDestroyed());
        return reinterpret_cast<GrD3DGpu*>(this->getGpu());
    }

    class Resource : public GrRecycledResource {
    public:
        explicit Resource()
            : fD3DResource(nullptr) {
        }

        Resource(const gr_cp<ID3D12Resource>& bufferResource)
            : fD3DResource(bufferResource) {
            SkASSERT(bufferResource.get() &&
                     bufferResource->GetDesc().Dimension == D3D12_RESOURCE_DIMENSION_BUFFER);
        }

        static sk_sp<Resource> Make(GrD3DGpu*, size_t size, GrGpuBufferType, GrAccessPattern,
                                    D3D12_RESOURCE_STATES*);

        size_t size() {
            if (!fD3DResource.get()) {
                return 0;
            }
            return fD3DResource->GetDesc().Width;
        }

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
    sk_sp<Resource> fMappedResource;

    typedef GrGpuBuffer INHERITED;
};

#endif
