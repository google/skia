/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGpuBuffer_DEFINED
#define GrGpuBuffer_DEFINED

#include "src/gpu/GrBuffer.h"
#include "src/gpu/GrGpuResource.h"

class GrGpu;

class GrGpuBuffer : public GrGpuResource, public GrBuffer {
public:
    /**
     * Computes a scratch key for a GPU-side buffer with a "dynamic" access pattern. (Buffers with
     * "static" and "stream" patterns are disqualified by nature from being cached and reused.)
     */
    static void ComputeScratchKeyForDynamicBuffer(size_t size, GrGpuBufferType, skgpu::ScratchKey*);

    GrAccessPattern accessPattern() const { return fAccessPattern; }

    size_t size() const final { return fSizeInBytes; }

    void ref() const final { GrGpuResource::ref(); }

    void unref() const final { GrGpuResource::unref(); }

    /**
     * Maps the buffer to be read or written by the CPU.
     *
     * It is an error to draw from the buffer while it is mapped or transfer to/from the buffer. It
     * may fail if the backend doesn't support mapping the buffer. Once a buffer is mapped,
     * subsequent calls to map() trivially succeed. No matter how many times map() is called,
     * umap() will unmap the buffer on the first call if it is mapped.
     *
     * If the buffer is of type GrGpuBufferType::kXferGpuToCpu then it is mapped for reading only.
     * Otherwise it is mapped writing only. Writing to a buffer that is mapped for reading or vice
     * versa produces undefined results. If the buffer is mapped for writing then the buffer's
     * previous contents are invalidated.
     *
     * @return a pointer to the data or nullptr if the map fails.
     */
    void* map();

    /**
     * Unmaps the buffer if it is mapped.
     *
     * The pointer returned by the previous map call will no longer be valid.
     */
    void unmap();

    /**
     * Queries whether the buffer has been mapped.
     *
     * @return true if the buffer is mapped, false otherwise.
     */
    bool isMapped() const;

    bool isCpuBuffer() const final { return false; }

    /**
     * Updates the buffer data.
     *
     * The size of the buffer will be preserved. The src data will be
     * placed at the beginning of the buffer and any remaining contents will
     * be undefined. srcSizeInBytes must be <= to the buffer size.
     *
     * The buffer must not be mapped.
     *
     * Fails for GrGpuBufferType::kXferGpuToCpu.
     *
     * Note that buffer updates do not go through GrContext and therefore are
     * not serialized with other operations.
     *
     * @return returns true if the update succeeds, false otherwise.
     */
    bool updateData(const void* src, size_t srcSizeInBytes);

protected:
    GrGpuBuffer(GrGpu*, size_t sizeInBytes, GrGpuBufferType, GrAccessPattern);
    GrGpuBufferType intendedType() const { return fIntendedType; }

    void* fMapPtr;

private:
    virtual void onMap() = 0;
    virtual void onUnmap() = 0;
    virtual bool onUpdateData(const void* src, size_t srcSizeInBytes) = 0;

    size_t onGpuMemorySize() const override { return fSizeInBytes; }
    const char* getResourceType() const override { return "Buffer Object"; }
    void computeScratchKey(skgpu::ScratchKey* key) const override;

    size_t            fSizeInBytes;
    GrAccessPattern   fAccessPattern;
    GrGpuBufferType   fIntendedType;

#ifdef SK_DEBUG
    // Static and stream access buffers are only ever written to once. This is used to track that
    // and assert it is true.
    bool              fHasWrittenToBuffer = false;
#endif
};

#endif
