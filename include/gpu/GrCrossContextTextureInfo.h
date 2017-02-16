/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrCrossContextTextureInfo_DEFINED
#define GrCrossContextTextureInfo_DEFINED

#include "GrTypes.h"
#include "gl/GrGLTypes.h"
#ifdef SK_VULKAN
#include "vk/GrVkTypes.h"
#endif

struct GrCrossContextTextureInfo {
    GrCrossContextTextureInfo(const GrGLTextureInfo& info)
        : fBackend(kOpenGL_GrBackend)
        , fGLBackendInfo(info) {}

#if SK_VULKAN
    GrCrossContextTextureInfo(const GrVkImageInfo& info)
        : fBackend(kVulkan_GrBackend)
        , fVkBackendInfo(info) {}
#endif

    GrBackendObject getBackendObject() const {
        if (kOpenGL_GrBackend == fBackend) {
            return reinterpret_cast<GrBackendObject>(&fGLBackendInfo);
        } else if (kVulkan_GrBackend == fBackend) {
#if SK_VULKAN
            return reinterpret_cast<GrBackendObject>(&fVkBackendInfo);
#endif
        }
        return 0;
    }

    GrBackend fBackend;
    GrGLTextureInfo fGLBackendInfo;     // For GL backed images
#if SK_VULKAN
    GrVkImageInfo fVkBackendInfo;       // For Vulkan backed images
#endif
};

#endif
