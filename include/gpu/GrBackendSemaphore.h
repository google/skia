/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBackendSemaphore_DEFINED
#define GrBackendSemaphore_DEFINED

#include "include/gpu/GrTypes.h"

#include "include/gpu/gl/GrGLTypes.h"
#include "include/gpu/mtl/GrMtlTypes.h"
#include "include/gpu/vk/GrVkTypes.h"

/**
 * Wrapper class for passing into and receiving data from Ganesh about a backend semaphore object.
 */
class GrBackendSemaphore {
public:
    // For convenience we just set the backend here to OpenGL. The GrBackendSemaphore cannot be used
    // until either initGL or initVulkan are called which will set the appropriate GrBackend.
    GrBackendSemaphore() : fBackend(GrBackendApi::kOpenGL), fGLSync(0), fIsInitialized(false) {}

    void initGL(GrGLsync sync) {
        fBackend = GrBackendApi::kOpenGL;
        fGLSync = sync;
        fIsInitialized = true;
    }

    void initVulkan(VkSemaphore semaphore) {
        fBackend = GrBackendApi::kVulkan;
        fVkSemaphore = semaphore;
#ifdef SK_VULKAN
        fIsInitialized = true;
#else
        fIsInitialized = false;
#endif
    }

    // It is the creator's responsibility to ref the MTLEvent passed in here, via __bridge_retained.
    // The other end will wrap this BackendSemaphore and take the ref, via __bridge_transfer.
    void initMetal(GrMTLHandle event, uint64_t value) {
        fBackend = GrBackendApi::kMetal;
        fMtlEvent = event;
        fMtlValue = value;
#ifdef SK_METAL
        fIsInitialized = true;
#else
        fIsInitialized = false;
#endif
    }

    bool isInitialized() const { return fIsInitialized; }

    GrGLsync glSync() const {
        if (!fIsInitialized || GrBackendApi::kOpenGL != fBackend) {
            return 0;
        }
        return fGLSync;
    }

    VkSemaphore vkSemaphore() const {
        if (!fIsInitialized || GrBackendApi::kVulkan != fBackend) {
            return VK_NULL_HANDLE;
        }
        return fVkSemaphore;
    }

    GrMTLHandle mtlSemaphore() const {
        if (!fIsInitialized || GrBackendApi::kMetal != fBackend) {
            return nullptr;
        }
        return fMtlEvent;
    }

    uint64_t mtlValue() const {
        if (!fIsInitialized || GrBackendApi::kMetal != fBackend) {
            return 0;
        }
        return fMtlValue;
    }

private:
    GrBackendApi fBackend;
    union {
        GrGLsync    fGLSync;
        VkSemaphore fVkSemaphore;
        GrMTLHandle fMtlEvent;    // Expected to be an id<MTLEvent>
    };
    uint64_t fMtlValue;
    bool fIsInitialized;
};

#endif
