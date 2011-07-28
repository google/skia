
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrResource_DEFINED
#define GrResource_DEFINED

#include "GrRefCnt.h"

class GrGpu;

class GrResource : public GrRefCnt {
public:
    explicit GrResource(GrGpu* gpu);

    virtual ~GrResource() {
        // subclass should have released this.
        GrAssert(!isValid());
    }

    /**
     * Frees the resource in the underlying 3D API. It must be safe to call this
     * when the resource has been previously abandoned.
     */
    void release();

    /**
     * Removes references to objects in the underlying 3D API without freeing
     * them. Used when the API context has been torn down before the GrContext.
     */
    void abandon();

    /**
     * Tests whether a resource has been abandoned or released. All resources
     * will be in this state after their creating GrContext is destroyed or has
     * contextLost called. It's up to the client to test isValid() before
     * attempting to use a resource if it holds refs on resources across
     * ~GrContext, freeResources with the force flag, or contextLost.
     *
     * @return true if the resource has been released or abandoned,
     *         false otherwise.
     */
    bool isValid() const { return NULL != fGpu; }

    /**
     * Retrieves the size of the object in GPU memory. This is approximate since
     * we aren't aware of additional padding or copies made by the driver.
     *
     * @return the size of the buffer in bytes
     */
     virtual size_t sizeInBytes() const = 0;

protected:

    virtual void onRelease() = 0;
    virtual void onAbandon() = 0;

    GrGpu* getGpu() const { return fGpu; }

private:
    GrResource(); // unimpl

    GrGpu* fGpu; // not reffed. This can outlive the GrGpu.

    friend class GrGpu; // GrGpu manages list of resources.

    GrResource* fNext;      // dl-list of resources per-GrGpu
    GrResource* fPrevious;

    typedef GrRefCnt INHERITED;
};

#endif
