
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrGeometryBuffer_DEFINED
#define GrGeometryBuffer_DEFINED

#include "GrGpuResource.h"

class GrGpu;

/**
 * Parent class for vertex and index buffers
 */
class GrGeometryBuffer : public GrGpuResource {
public:
    SK_DECLARE_INST_COUNT(GrGeometryBuffer);

    /**
     *Retrieves whether the buffer was created with the dynamic flag
     *
     * @return true if the buffer was created with the dynamic flag
     */
    bool dynamic() const { return fDynamic; }

    /**
     * Returns true if the buffer is a wrapper around a CPU array. If true it
     * indicates that map will always succeed and will be free.
     */
    bool isCPUBacked() const { return fCPUBacked; }

    /**
     * Maps the buffer to be written by the CPU.
     *
     * The previous content of the buffer is invalidated. It is an error
     * to draw from the buffer while it is mapped. It is an error to call map
     * on an already mapped buffer. It may fail if the backend doesn't support
     * mapping the buffer. If the buffer is CPU backed then it will always
     * succeed and is a free operation. Must be matched by an unmap() call.
     * Currently only one map at a time is supported (no nesting of
     * map/unmap).
     *
     * Note that buffer mapping does not go through GrContext and therefore is
     * not serialized with other operations.
     *
     * @return a pointer to the data or NULL if the map fails.
     */
     void* map() { return (fMapPtr = this->onMap()); }

    /**
     * Unmaps the buffer.
     *
     * The pointer returned by the previous map call will no longer be valid.
     */
     void unmap() {
         SkASSERT(NULL != fMapPtr);
         this->onUnmap();
         fMapPtr = NULL;
     }

    /**
     * Returns the same ptr that map() returned at time of map or NULL if the
     * is not mapped.
     *
     * @return ptr to mapped buffer data or NULL if buffer is not mapped.
     */
     void* mapPtr() const { return fMapPtr; }

    /**
     Queries whether the buffer has been mapped.

     @return true if the buffer is mapped, false otherwise.
     */
     bool isMapped() const { return NULL != fMapPtr; }

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

    // GrGpuResource overrides
    virtual size_t gpuMemorySize() const { return fGpuMemorySize; }

protected:
    GrGeometryBuffer(GrGpu* gpu, bool isWrapped, size_t gpuMemorySize, bool dynamic, bool cpuBacked)
        : INHERITED(gpu, isWrapped)
        , fMapPtr(NULL)
        , fGpuMemorySize(gpuMemorySize)
        , fDynamic(dynamic)
        , fCPUBacked(cpuBacked) {}

private:
    virtual void* onMap() = 0;
    virtual void onUnmap() = 0;
    virtual bool onUpdateData(const void* src, size_t srcSizeInBytes) = 0;

    void*    fMapPtr;
    size_t   fGpuMemorySize;
    bool     fDynamic;
    bool     fCPUBacked;

    typedef GrGpuResource INHERITED;
};

#endif
