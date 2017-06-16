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

/**
 * Wrapper class for passing into and receiving data from Ganesh about a backend semaphore object.
 */
class GrBackendSemaphore {
public:
    // For convenience we just set the backend here to OpenGL. The GrBackendSemaphore cannot be used
    // until either initGL or initVulkan are called which will set the appropriate GrBackend.
    GrBackendSemaphore() : fBackend(kOpenGL_GrBackend), fGLSync(0), fIsInitialized(false) {}

    void initGL(GrGLsync sync) {
        fBackend = kOpenGL_GrBackend;
        fGLSync = sync;
        fIsInitialized = true;
    }

#ifdef SK_VULKAN
    void initVulkan(VkSemaphore semaphore) {
        fBackend = kVulkan_GrBackend;
        fVkSemaphore = semaphore;
        fIsInitialized = true;
    }
#endif

    GrGLsync glSync() const {
        if (!fIsInitialized || kOpenGL_GrBackend != fBackend) {
            return 0;
        }
        return fGLSync;
    }

#ifdef SK_VULKAN
    VkSemaphore vkSemaphore() const {
        if (!fIsInitialized || kVulkan_GrBackend != fBackend) {
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
    bool fIsInitialized;
};

#endif
