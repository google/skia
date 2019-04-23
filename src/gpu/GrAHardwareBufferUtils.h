/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GrAHardwareBufferUtils_DEFINED
#define GrAHardwareBufferUtils_DEFINED

#include "include/core/SkTypes.h"

#if defined(SK_BUILD_FOR_ANDROID) && __ANDROID_API__ >= 26

#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrTypes.h"

class GrContext;

extern "C" {
    typedef struct AHardwareBuffer AHardwareBuffer;
}

namespace GrAHardwareBufferUtils {

SkColorType GetSkColorTypeFromBufferFormat(uint32_t bufferFormat);

GrBackendFormat GetBackendFormat(GrContext* context, AHardwareBuffer* hardwareBuffer,
                                 uint32_t bufferFormat, bool requireKnownFormat);

typedef void* DeleteImageCtx;
typedef void (*DeleteImageProc)(DeleteImageCtx);

GrBackendTexture MakeBackendTexture(GrContext* context, AHardwareBuffer* hardwareBuffer,
                                    int width, int height,
                                    DeleteImageProc* deleteProc,
                                    DeleteImageCtx* deleteCtx,
                                    bool isProtectedContent,
                                    const GrBackendFormat& backendFormat,
                                    bool isRenderable);

} // GrAHardwareBufferUtils


#endif
#endif
