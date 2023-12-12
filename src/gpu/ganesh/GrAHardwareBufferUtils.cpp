/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/android/GrAHardwareBufferUtils.h"

#if defined(SK_BUILD_FOR_ANDROID) && __ANDROID_API__ >= 26
#define GL_GLEXT_PROTOTYPES
#define EGL_EGLEXT_PROTOTYPES

#include <android/hardware_buffer.h>

#if !defined(SK_DISABLE_LEGACY_ANDROID_HW_UTILS)
#include "include/gpu/GrDirectContext.h"
#endif

// TODO: remove this once Android is using the AHardwareBufferUtils version of
// GetSkColorTypeFromBufferFormat
#include "include/android/AHardwareBufferUtils.h"

namespace GrAHardwareBufferUtils {

SkColorType GetSkColorTypeFromBufferFormat(uint32_t bufferFormat) {
    return AHardwareBufferUtils::GetSkColorTypeFromBufferFormat(bufferFormat);
}

#if !defined(SK_DISABLE_LEGACY_ANDROID_HW_UTILS)
GrBackendFormat GetBackendFormat(GrDirectContext* dContext, AHardwareBuffer* hardwareBuffer,
                                 uint32_t bufferFormat, bool requireKnownFormat) {
    GrBackendApi backend = dContext->backend();

    if (backend == GrBackendApi::kOpenGL) {
#ifdef SK_GL
        return GetGLBackendFormat(dContext, bufferFormat, requireKnownFormat);
#else // SK_GL
        return GrBackendFormat();
#endif // SK_GL
    } else if (backend == GrBackendApi::kVulkan) {
#ifdef SK_VULKAN
        return GetVulkanBackendFormat(dContext, hardwareBuffer, bufferFormat, requireKnownFormat);
#else // SK_VULKAN
        return GrBackendFormat();
#endif // SK_VULKAN
    }
    return GrBackendFormat();
}

GrBackendTexture MakeBackendTexture(GrDirectContext* dContext,
                                    AHardwareBuffer* hardwareBuffer,
                                    int width, int height,
                                    DeleteImageProc* deleteProc,
                                    UpdateImageProc* updateProc,
                                    TexImageCtx* imageCtx,
                                    bool isProtectedContent,
                                    const GrBackendFormat& backendFormat,
                                    bool isRenderable,
                                    bool fromAndroidWindow) {
    SkASSERT(dContext);
    if (!dContext || dContext->abandoned()) {
        return GrBackendTexture();
    }

    if (GrBackendApi::kOpenGL == dContext->backend()) {
#ifdef SK_GL
        return MakeGLBackendTexture(dContext, hardwareBuffer, width, height, deleteProc,
                                    updateProc, imageCtx, isProtectedContent, backendFormat,
                                    isRenderable);
#else
        return GrBackendTexture();
#endif // SK_GL
    } else {
        SkASSERT(GrBackendApi::kVulkan == dContext->backend());
#ifdef SK_VULKAN
        return MakeVulkanBackendTexture(dContext, hardwareBuffer, width, height, deleteProc,
                                        updateProc, imageCtx, isProtectedContent, backendFormat,
                                        isRenderable, fromAndroidWindow);
#else
        return GrBackendTexture();
#endif // SK_VULKAN
    }
}
#endif

}  // namespace GrAHardwareBufferUtils

#endif
