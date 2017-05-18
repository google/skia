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
     * Creates a client-side buffer.
     */
    static SK_WARN_UNUSED_RESULT GrBuffer* CreateCPUBacked(GrGpu*, size_t sizeInBytes, GrBufferType,
                                                           const void* data = nullptr);

    /**
     * Computes a scratch key for a GPU-side buffer with a "dynamic" access pattern. (Buffers with
     * "static" and "stream" patterns are disqualified by nature from being cached and reused.)
     */
    static void ComputeScratchKeyForDynamicVBO(size_t size, GrBufferType, GrScratchKey*);

    GrAccessPattern accessPattern() const { return fAccessPattern; }
    size_t sizeInBytes() const { return fSizeInBytes; }

    /**
     * Returns true if the buffer is a wrapper around a CPU array. If true it
     * indicates that map will always succeed and will be free.
     */
    bool isCPUBacked() const { return SkToBool(fCPUData); }
    size_t baseOffset() const { return reinterpret_cast<size_t>(fCPUData); }

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
        SkASSERT(srcSizeInBytes <= fSizeInBytes);
        return this->onUpdateData(src, srcSizeInBytes);
    }

    ~GrBuffer() override {
        sk_free(fCPUData);
    }

protected:
    GrBuffer(GrGpu*, size_t sizeInBytes, GrBufferType, GrAccessPattern);

    void* fMapPtr;

private:
    /**
     * Internal constructor to make a CPU-backed buffer.
     */
    GrBuffer(GrGpu*, size_t sizeInBytes, GrBufferType, void* cpuData);

    virtual void onMap() { SkASSERT(this->isCPUBacked()); fMapPtr = fCPUData; }
    virtual void onUnmap() { SkASSERT(this->isCPUBacked()); }
    virtual bool onUpdateData(const void* src, size_t srcSizeInBytes);

    size_t onGpuMemorySize() const override { return fSizeInBytes; } // TODO: zero for cpu backed?
    void computeScratchKey(GrScratchKey* key) const override;

    size_t            fSizeInBytes;
    GrAccessPattern   fAccessPattern;
    void*             fCPUData;
    GrBufferType      fIntendedType;

    typedef GrGpuResource INHERITED;
};

#endif
