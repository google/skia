/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GrAHardwareBufferUtils_DEFINED
#define GrAHardwareBufferUtils_DEFINED

#include "include/core/SkTypes.h"

#if defined(SK_BUILD_FOR_ANDROID)

#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrTypes.h"

class GrDirectContext;

extern "C" {
    typedef struct AHardwareBuffer AHardwareBuffer;
    typedef struct AHardwareBuffer_Desc AHardwareBuffer_Desc;
    typedef struct ARect ARect;
}

namespace GrAHardwareBufferUtils {

SkColorType GetSkColorTypeFromBufferFormat(uint32_t bufferFormat);

GrBackendFormat GetBackendFormat(GrDirectContext* context, AHardwareBuffer* hardwareBuffer,
                                 uint32_t bufferFormat, bool requireKnownFormat);

typedef void* TexImageCtx;
typedef void (*DeleteImageProc)(TexImageCtx);
typedef void (*UpdateImageProc)(TexImageCtx, GrDirectContext*);

/**
 * Create a GrBackendTexture from AHardwareBuffer
 *
 * @param   context         GPU context
 * @param   hardwareBuffer  AHB
 * @param   width           texture width
 * @param   height          texture height
 * @param   deleteProc      returns a function that deletes the texture and
 *                          other GPU resources. Must be invoked on the same
 *                          thread as MakeBackendTexture
 * @param   updateProc      returns a function, that needs to be invoked, when
 *                          AHB buffer content has changed. Must be invoked on
 *                          the same thread as MakeBackendTexture
 * @param   imageCtx        returns an opaque image context, that is passed as
 *                          first argument to deleteProc and updateProc
 * @param   isProtectedContent if true, GL backend uses EXT_protected_content
 * @param   backendFormat   backend format, usually created with helper
 *                          function GetBackendFormat
 * @param   isRenderable    true if GrBackendTexture can be used as a color
 *                          attachment
 * @return                  valid GrBackendTexture object on success
 */
GrBackendTexture MakeBackendTexture(GrDirectContext* context, AHardwareBuffer* hardwareBuffer,
                                    int width, int height,
                                    DeleteImageProc* deleteProc,
                                    UpdateImageProc* updateProc,
                                    TexImageCtx* imageCtx,
                                    bool isProtectedContent,
                                    const GrBackendFormat& backendFormat,
                                    bool isRenderable);

/**
 * Initialize AHardwareBuffer_ methods
 */
bool Init();

void Describe(const AHardwareBuffer*, AHardwareBuffer_Desc* desc);

void Acquire(AHardwareBuffer*);

void Release(AHardwareBuffer*);

int Allocate(const AHardwareBuffer_Desc*, AHardwareBuffer**);

int Lock(AHardwareBuffer* buffer, uint64_t usage, int32_t fence,
         const ARect* rect, void** outVirtualAddress);

int Unlock(AHardwareBuffer* buffer, int32_t* fence);

} // GrAHardwareBufferUtils


#endif
#endif
