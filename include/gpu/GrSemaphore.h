/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSemaphore_DEFINED
#define GrSemaphore_DEFINED

#include "SkRefCnt.h"
#include "gl/GrGLTypes.h"
#include "vk/GrVkTypes.h"

class GrGpu;

class GrSemaphore : public SkRefCnt {
public:
    // These methods are used when the client has already created the semaphore object and they just
    // need to wrap it inside of a GrSemaphore. We do not take ownership of the semaphore and thus
    // will not destroy it.
    static sk_sp<GrSemaphore> WrapVulkanSemaphore(VkSemaphore semaphore);
    static sk_sp<GrSemaphore> WrapGLSemaphore(GrGLsync sync);

    // Returns the backing VkSemaphore. This should be overriden by the subclass if it supports
    // returning a VkSemaphore object. Otherwise this call fails.
    virtual VkSemaphore getVulkanSemaphore() {
        SkASSERT(false);
        return VK_NULL_HANDLE;
    }

    // Returns the backing GrGLSync object. This should be overriden by the subclass if it supports
    // returning a GrGLsync object. Otherwise this call fails.
    virtual GrGLsync getGLSemaphore() {
        SkASSERT(false);
        return 0;
    }

private:
    // This function should only be used in the case of exporting and importing a GrSemaphore object
    // from one GrContext to another. When exporting, the GrSemaphore should be set to a null GrGpu,
    // and when importing it should be set to the GrGpu of the current context. Once exported, a
    // GrSemaphore should not be used with its old context.
    void resetGpu(const GrGpu* gpu) { fGpu = gpu; }

protected:
    explicit GrSemaphore(const GrGpu* gpu) : fGpu(gpu) {}

    friend class GrResourceProvider; // resetGpu

    const GrGpu* fGpu;
};

#endif
