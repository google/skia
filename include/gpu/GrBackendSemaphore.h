/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBackendSemaphore_DEFINED
#define GrBackendSemaphore_DEFINED

#include "GrTypes.h"

#include "gl/GrGLTypes.h"
#ifdef SK_VULKAN
#include "vk/GrVkTypes.h"
#endif

class GrBackendSemaphore {
public:
    explicit GrBackendSemaphore(GrGLsync sync)
            : fBackend(kOpenGL_GrBackend), fGLSync(sync) {}

#ifdef SK_VULKAN
    explicit GrBackendSemaphore(VkSemaphore semaphore)
            : fBackend(kVulkan_GrBackend), fVkSemaphore(semaphore) {}
#endif

    GrBackend backend() const { return fBackend; }

    GrGLsync glSync() {
        if (kOpenGL_GrBackend != fBackend) {
            return 0;
        }
        return fGLSync;
    }

#ifdef SK_VULKAN
    VkSemaphore vkSemaphore() {
        if (kVulkan_GrBackend != fBackend) {
            return VK_NULL_HANDLE;
        }
        return fVkSemaphore;
    }
#endif

private:
    GrBackend fBackend;
    union {
        GrGLsync    fGLSync;
#ifdef SK_VULKAN
        VkSemaphore fVkSemaphore;
#endif
    };
};

#endif
