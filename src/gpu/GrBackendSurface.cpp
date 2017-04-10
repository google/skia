/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrBackendSurface.h"

#include "vk/GrVkTypes.h"
#include "vk/GrVkUtil.h"

GrBackendSurface::GrBackendSurface(int width,
                                    int height,
                                    GrVkImageInfo* vkInfo)
        : fWidth(width)
        , fHeight(height)
        , fConfig(GrVkFormatToPixelConfig(vkInfo->fFormat))
        , fBackend(kVulkan_GrBackend)
        , fVkInfo(vkInfo) {}

GrBackendSurface::GrBackendSurface(int width,
                                   int height,
                                   GrPixelConfig config,
                                   GrGLTextureInfo* glInfo)
        : fWidth(width)
        , fHeight(height)
        , fConfig(config)
        , fBackend(kOpenGL_GrBackend)
        , fGLInfo(glInfo) {}

GrBackendSurface::GrBackendSurface(int width,
                                   int height,
                                   GrPixelConfig config,
                                   GrBackend backend,
                                   GrBackendObject handle)
        : fWidth(width)
        , fHeight(height)
        , fConfig(kVulkan_GrBackend == backend
                  ? GrVkFormatToPixelConfig(((GrVkImageInfo*)handle)->fFormat)
                  : config)
        , fBackend(backend)
        , fHandle(handle) {}

GrVkImageInfo* GrBackendSurface::getVkImageInfo() {
    if (kVulkan_GrBackend == fBackend) {
        return fVkInfo;
    }
    return nullptr;
}

GrGLTextureInfo* GrBackendSurface::getGLTextureInfo() {
    if (kOpenGL_GrBackend == fBackend) {
        return fGLInfo;
    }
    return nullptr;
}
