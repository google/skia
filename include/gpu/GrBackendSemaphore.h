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
#include "vk/GrVkTypes.h"

class GrBackendSemaphore {
public:
    explicit GrBackendSemaphore(GrGLsync sync)
            : fBackend(kOpenGL_GrBackend), fGLSync(sync) {}

    explicit GrBackendSemaphore(VkSemaphore semaphore)
            : fBackend(kVulkan_GrBackend), fVkSemaphore(semaphore) {}

    GrBackend backend() const { return fBackend; }

    GrGLsync glSync() {
        if (kOpenGL_GrBackend != fBackend) {
            return 0;
        }
        return fGLSync;
    }

    VkSemaphore vkSemaphore() {
        if (kVulkan_GrBackend != fBackend) {
            return VK_NULL_HANDLE;
        }
        return fVkSemaphore;
    }

private:
    GrBackend fBackend;
    union {
        GrGLsync    fGLSync;
        VkSemaphore fVkSemaphore;
    };
};

#endif
