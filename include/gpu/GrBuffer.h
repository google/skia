/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBuffer_DEFINED
#define GrBuffer_DEFINED

#include "GrGpuResource.h"

class GrGpu;

class GrBuffer : public GrGpuResource {
public:
    /**
     * Computes a scratch key for a buffer with a "dynamic" access pattern. (Buffers with "static"
     * and "stream" access patterns are disqualified by nature from being cached and reused.)
     */
    static void ComputeScratchKeyForDynamicBuffer(size_t size, GrBufferType intendedType,
                                                  GrScratchKey* key) {
        static const GrScratchKey::ResourceType kType = GrScratchKey::GenerateResourceType();
        GrScratchKey::Builder builder(key, kType, 1 + (sizeof(size_t) + 3) / 4);
        // TODO: There's not always reason to cache a buffer by type. In some (all?) APIs it's just
        // a chunk of memory we can use/reuse for any type of data. We really only need to
        // differentiate between the "read" types (e.g. kGpuToCpu_BufferType) and "draw" types.
        builder[0] = intendedType;
        builder[1] = (uint32_t)size;
        if (sizeof(size_t) > 4) {
            builder[2] = (uint32_t)((uint64_t)size >> 32);
        }
    }

    GrAccessPattern accessPattern() const { return fAccessPattern; }

    /**
     * Returns true if the buffer is a wrapper around a CPU array. If true it
     * indicates that map will always succeed and will be free.
     */
    bool isCPUBacked() const { return fCPUBacked; }

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
     * Returns the same ptr that map() returned at time of map or nullptr if the
     * is not mapped.
     *
     * @return ptr to mapped buffer data or nullptr if buffer is not mapped.
     */
     void* mapPtr() const { return fMapPtr; }

    /**
     Queries whether the buffer has been mapped.

     @return true if the buffer is mapped, false otherwise.
     */
     bool isMapped() const { return SkToBool(fMapPtr); }

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
        SkASSERT(srcSizeInBytes <= fGpuMemorySize);
        return this->onUpdateData(src, srcSizeInBytes);
    }

protected:
    GrBuffer(GrGpu* gpu, size_t gpuMemorySize, GrBufferType intendedType,
             GrAccessPattern accessPattern, bool cpuBacked)
        : INHERITED(gpu, kCached_LifeCycle),
          fMapPtr(nullptr),
          fGpuMemorySize(gpuMemorySize), // TODO: Zero for cpu backed buffers?
          fAccessPattern(accessPattern),
          fCPUBacked(cpuBacked) {
        if (!fCPUBacked && SkIsPow2(fGpuMemorySize) && kDynamic_GrAccessPattern == fAccessPattern) {
            GrScratchKey key;
            ComputeScratchKeyForDynamicBuffer(fGpuMemorySize, intendedType, &key);
            this->setScratchKey(key);
        }
    }

    void* fMapPtr;

private:
    virtual size_t onGpuMemorySize() const { return fGpuMemorySize; }

    virtual void onMap() = 0;
    virtual void onUnmap() = 0;
    virtual bool onUpdateData(const void* src, size_t srcSizeInBytes) = 0;

    size_t            fGpuMemorySize;
    GrAccessPattern   fAccessPattern;
    bool              fCPUBacked;

    typedef GrGpuResource INHERITED;
};

#endif
