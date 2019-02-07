/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGpuBuffer_DEFINED
#define GrGpuBuffer_DEFINED

#include "GrBuffer.h"
#include "GrGpuResource.h"

class GrGpu;

class GrGpuBuffer : public GrGpuResource, public GrBuffer {
public:
    /**
     * Computes a scratch key for a GPU-side buffer with a "dynamic" access pattern. (Buffers with
     * "static" and "stream" patterns are disqualified by nature from being cached and reused.)
     */
    static void ComputeScratchKeyForDynamicVBO(size_t size, GrGpuBufferType, GrScratchKey*);

    GrAccessPattern accessPattern() const { return fAccessPattern; }

    size_t size() const final { return fSizeInBytes; }

    void ref() const final { GrGpuResource::ref(); }

    void unref() const final { GrGpuResource::unref(); }

    /**
     * Maps the buffer to be written by the CPU.
     *
     * The previous content of the buffer is invalidated. It is an error
     * to draw from the buffer while it is mapped. It may fail if the backend
     * doesn't support mapping the buffer. If the buffer is CPU backed then
     * it will always succeed and is a free operation. Once a buffer is mapped,
     * subsequent calls to map() are ignored.
     *
     * Note that buffer mapping does not go through GrContext and therefore is
     * not serialized with other operations.
     *
     * @return a pointer to the data or nullptr if the map fails.
     */
    void* map() {
        if (!fMapPtr) {
            this->onMap();
        }
        return fMapPtr;
    }

    /**
     * Unmaps the buffer.
     *
     * The pointer returned by the previous map call will no longer be valid.
     */
    void unmap() {
        SkASSERT(fMapPtr);
        this->onUnmap();
        fMapPtr = nullptr;
    }

    /**
     Queries whether the buffer has been mapped.

     @return true if the buffer is mapped, false otherwise.
     */
    bool isMapped() const { return SkToBool(fMapPtr); }

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
     * Note that buffer updates do not go through GrContext and therefore are
     * not serialized with other operations.
     *
     * @return returns true if the update succeeds, false otherwise.
     */
    bool updateData(const void* src, size_t srcSizeInBytes) {
        SkASSERT(!this->isMapped());
        SkASSERT(srcSizeInBytes <= fSizeInBytes);
        return this->onUpdateData(src, srcSizeInBytes);
    }

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
    void computeScratchKey(GrScratchKey* key) const override;

    size_t            fSizeInBytes;
    GrAccessPattern   fAccessPattern;
    GrGpuBufferType   fIntendedType;
};

#endif
