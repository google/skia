/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrD3DCommandList_DEFINED
#define GrD3DCommandList_DEFINED

#include "include/gpu/GrTypes.h"
#include "include/gpu/d3d/GrD3DTypes.h"
#include "src/gpu/GrManagedResource.h"

#include <memory>

class GrD3DGpu;

class GrD3DCommandList {
public:
    void close();

    void submit(ID3D12CommandQueue* queue);

    void reset();

    // Add ref-counted resource that will be tracked and released when this command buffer finishes
    // execution
    void addResource(const GrManagedResource* resource) {
        SkASSERT(resource);
        resource->ref();
        resource->notifyQueuedForWorkOnGpu();
        fTrackedResources.append(1, &resource);
    }

    // Add ref-counted resource that will be tracked and released when this command buffer finishes
    // execution. When it is released, it will signal that the resource can be recycled for reuse.
    void addRecycledResource(const GrRecycledResource* resource) {
        resource->ref();
        resource->notifyQueuedForWorkOnGpu();
        fTrackedRecycledResources.append(1, &resource);
    }

    void releaseResources();

protected:
    GrD3DCommandList(gr_cp<ID3D12CommandAllocator> allocator,
                     gr_cp<ID3D12GraphicsCommandList> commandList);

    gr_cp<ID3D12GraphicsCommandList> fCommandList;

    SkTDArray<const GrManagedResource*> fTrackedResources;
    SkTDArray<const GrRecycledResource*> fTrackedRecycledResources;

    // When we create a command list it starts in an active recording state
    SkDEBUGCODE(bool fIsActive = true;)

private:
    gr_cp<ID3D12CommandAllocator> fAllocator;

    static const int kInitialTrackedResourcesCount = 32;
    // When resetting the command buffer, we remove the tracked resources from their arrays, and
    // we prefer to not free all the memory every time so usually we just rewind. However, to avoid
    // all arrays growing to the max size, after so many resets we'll do a full reset of the tracked
    // resource arrays.
    static const int kNumRewindResetsBeforeFullReset = 8;
    int              fNumResets = 0;
};

class GrD3DDirectCommandList : public GrD3DCommandList {
public:
    static std::unique_ptr<GrD3DDirectCommandList> Make(ID3D12Device* device);

private:
    GrD3DDirectCommandList(gr_cp<ID3D12CommandAllocator> allocator,
                           gr_cp<ID3D12GraphicsCommandList> commandList);
};

class GrD3DCopyCommandList : public GrD3DCommandList {
public:
    static std::unique_ptr<GrD3DCopyCommandList> Make(ID3D12Device* device);

private:
    GrD3DCopyCommandList(gr_cp<ID3D12CommandAllocator> allocator,
                         gr_cp<ID3D12GraphicsCommandList> commandList);
};
#endif
