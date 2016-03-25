
/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrTransferBuffer_DEFINED
#define GrTransferBuffer_DEFINED

#include "GrGpuResource.h"

class GrTransferBuffer : public GrGpuResource {
public:
    /**
    * Maps the buffer to be written by the CPU.
    *
    * The previous content of the buffer is invalidated. It is an error
    * to transfer to or from the buffer while it is mapped. It is an error to 
    * call map on an already mapped buffer. Must be matched by an unmap() call.
    * Currently only one map at a time is supported (no nesting of map/unmap).
    *
    * Note that buffer mapping does not go through GrContext and therefore is
    * not serialized with other operations.
    *
    * @return a pointer to the data or nullptr if the map fails.
    */
    void* map() { return (fMapPtr = this->onMap()); }

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

protected:
    GrTransferBuffer(GrGpu* gpu, size_t gpuMemorySize)
        : INHERITED(gpu, kUncached_LifeCycle)
        , fGpuMemorySize(gpuMemorySize) {
    }

private:
    virtual size_t onGpuMemorySize() const { return fGpuMemorySize; }

    virtual void* onMap() = 0;
    virtual void  onUnmap() = 0;

    void*    fMapPtr;
    size_t   fGpuMemorySize;

    typedef GrGpuResource INHERITED;
};

#endif
