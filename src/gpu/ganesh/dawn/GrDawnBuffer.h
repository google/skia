/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDawnBuffer_DEFINED
#define GrDawnBuffer_DEFINED

#include "src/gpu/ganesh/GrGpuBuffer.h"
#include "webgpu/webgpu_cpp.h"

#include <functional>

class GrDawnGpu;

// GrDawnBuffer is the GrGpuBuffer implementation for the Dawn backend.
//
// Some notes on the implementation:
//
// LIFETIME AND CREATION
// ---------------------
// When a GrDawnBuffer is constructed, it allocates a GPU buffer. Depending on the requested access
// pattern, the buffer is typically immediately mapped at creation (which happens synchronously and
// relatively fast). If a client requests to create a buffer with data, then it will be immediately
// unmapped after the data is copied into the buffer (see `GrDawnGpu::onCreateBuffer` and
// `GrDawnBuffer::onUpdateData`).
//
// Clients usually create buffers through a GrResourceProvider or a GrStagingBufferManager. These
// buffers are constructed in `GrDawnGpu::onCreateBuffer` and GrDawnGpu is involved in their
// lifetime and mapping. Depending on the requested buffer type, a GrDawnBuffer that is
// initialized as `Mappable::kNot` can itself be backed by another GrDawnBuffer that is owned by a
// GrStagingBufferManager. In this case the CPU mapping happens via the `fStagingBuffer` member
// instead of `fBuffer`. The backing `fStagingBuffer` is initialized in `GrDawnBuffer::onMap` and
// its contents are instructed to be copied into `fBuffer` in `GrDawnBuffer::onUnmap` (which does
// not take effect until the command is submitted to the GPU).
//
// ASYNC MAP/UNMAP
// ---------------
// The Dawn API provides two ways to map the CPU-accessible memory of a wgpu::Buffer:
//    * wgpu::Device::CreateBuffer which can synchronously map the buffer at creation;
//    * wgpu::Buffer::MapAsync which asynchronously maps a buffer at any time.
//
// When a GrDawnBuffer gets created it starts out as mapped (except it gets unmapped immediately if
// initialized with data). A buffer gets unmapped when its owner calls `GrGpuBuffer::unmap()`. A
// buffer that is managed by a GrStagingBufferManager is always unmapped before its ownership is
// passed to the associated GrDawnGpu.
//
// Dawn only provides an asynchronous API for mapping an unmapped buffer and `GrGpuBuffer::map()`
// must work synchronously. However, blocking in a busy-wait that yields to the underlying event
// loop can stall the calling thread in the order of milliseconds. We optimize this specifically
// for staging buffers:
//    1. GrStagingBufferManager first unmaps the buffer and passes its ownership to GrDawnBuffer; at
//       this stage no client is expected to access the buffer and it can remain unmapped.
//    2. GrDawnBuffer requests to map the buffer asynchronously and does not return it back to the
//       backing resource provider until the map finishes. Thus, the buffer is never handed back to
//       clients in an unmapped state.
//    3. If a client needs a staging buffer before the map finishes, they will need to allocate a
//       new buffer which can get mapped at creation and avoid an async map.
//
// For all other buffers, a blocking map procedure is provided which allows them to remap a buffer
// if needed. For instance, a write-only non-staging buffer can be safely unmapped and mapped by a
// client.
class GrDawnBuffer : public GrGpuBuffer {
public:
    static sk_sp<GrDawnBuffer> Make(GrDawnGpu* gpu,
                                    size_t sizeInBytes,
                                    GrGpuBufferType type,
                                    GrAccessPattern pattern,
                                    std::string_view label);
    ~GrDawnBuffer() override = default;

    void onMap(MapType) override;
    void onUnmap(MapType) override;
    bool onClearToZero() override;
    void onRelease() override;
    bool onUpdateData(const void* src, size_t offset, size_t size, bool preserve) override;

    GrDawnGpu* getDawnGpu() const;
    wgpu::Buffer get() const { return fBuffer; }

    // Map this buffer using the asynchronous map procedure. This function is intended to be used by
    // the owning GrDawnGpu to manage the lifetime of this buffer and it has the following
    // restrictions:
    //    - It must not be called while an async map is already in progress.
    //    - It must not be called on a buffer that is already mapped.
    //    - It must not be called on a buffer that is initialized as "unmappable".
    //
    // `callback` is called asynchronously with the result of this procedure once it's complete.
    using MapAsyncCallback = std::function<void(bool success)>;
    void mapAsync(MapAsyncCallback callback);

private:
    enum class Mappable {
        // Corresponds to Vertex and Index buffers. When a mapping is requested, these buffers are
        // always backed by a staging buffer. NOTE: Staging buffers that are created by
        // GrStagingBufferManager themselves are always `Mappable::kWriteOnly`.
        kNot,

        // Corresponds to `GrGpuBufferType::kXferGpuToCpu`. NOT mapped at creation. Will use a
        // blocking-map if a mapping is requested.
        kReadOnly,

        // Corresponds to `GrGpuBufferType::kXferCpuToGpu`. Always mapped at creation. Will use a
        // blocking-map if a mapping is requested. IF this is a staging buffer, then it will be
        // asynchronously mapped by GrDawnGpu.
        kWriteOnly,
    };

    GrDawnBuffer(GrDawnGpu* gpu,
                 size_t sizeInBytes,
                 GrGpuBufferType type,
                 GrAccessPattern pattern,
                 std::string_view label,
                 Mappable mappable,
                 wgpu::Buffer buffer,
                 void* mapPtr);

    void* internalMap(MapType type, size_t offset, size_t size);
    void internalUnmap(MapType type, size_t offset, size_t size);

    // Called to handle the asynchronous mapAsync callback.
    void mapAsyncDone(WGPUBufferMapAsyncStatus status);

    // Map a buffer and busy-wait until the asynchronous mapping procedure completes. This function
    // only needs to be called for a buffer that has been unmapped since buffers start out as mapped
    // at creation.
    //
    // The blocking map incurs a cost in the form of yielding to the underlying event loop until the
    // map finishes and can block the calling thread in the order of milliseconds. This might be
    // undesirable for buffers that are mapped and unmapped frequently.
    //
    // This procedure is used to cover the case where a buffer that is not managed by a
    // GrStagingBufferManager (and thus not asynchronously mapped by the owning GrDawnGpu) is
    // unmapped and needs to get re-mapped for use (e.g. in onUpdateData()).
    //
    // Returns nullptr if the buffer fails to map.
    void* blockingMap(size_t offset, size_t size);

    wgpu::Buffer fBuffer;
    Mappable fMappable = Mappable::kNot;
    bool fUnmapped;

    // A callback is only present when a request for MapAsync is pending. The callback is reset once
    // the procedure is complete.
    MapAsyncCallback fMapAsyncCallback;

    // Buffers that are of the "not mappable" type are backed by another GrDawnBuffer that is
    // managed by a GrStagingBufferManager.
    wgpu::Buffer fStagingBuffer;
    size_t fStagingOffset = 0;

    using INHERITED = GrGpuBuffer;
};

#endif
