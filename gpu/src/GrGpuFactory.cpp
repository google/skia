
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrTypes.h"

// must be before GrGLConfig.h
#if GR_WIN32_BUILD
//    #include "GrGpuD3D9.h"
#endif

#include "GrGLConfig.h"

#include "GrGpu.h"
#include "GrGpuGLFixed.h"
#include "GrGpuGLShaders.h"

GrGpu* GrGpu::Create(GrEngine engine, GrPlatform3DContext context3D) {

    if (kOpenGL_Shaders_GrEngine == engine ||
        kOpenGL_Fixed_GrEngine == engine) {
        // If no GL bindings have been installed, fall-back to calling the
        // GL functions that have been linked with the executable.
        if (!GrGLGetGLInterface()) {
            GrGLSetDefaultGLInterface();
            // If there is no platform-default then just fail.
            if (!GrGLGetGLInterface()) {
                return NULL;
            }
        }
        if (!GrGLGetGLInterface()->validate(engine)) {
#if GR_DEBUG
            GrPrintf("Failed GL interface validation!");
#endif
            return NULL;
        }
    }

    GrGpu* gpu = NULL;

    switch (engine) {
        case kOpenGL_Shaders_GrEngine:
            GrAssert(NULL == (void*)context3D);
            {
#if 0 // old code path, will be removed soon
                gpu = new GrGpuGLShaders2;
#else
                gpu = new GrGpuGLShaders;
#endif
            }
            break;
        case kOpenGL_Fixed_GrEngine:
            GrAssert(NULL == (void*)context3D);
            gpu = new GrGpuGLFixed;
            break;
        case kDirect3D9_GrEngine:
            GrAssert(NULL != (void*)context3D);
#if GR_WIN32_BUILD
//            gpu = new GrGpuD3D9((IDirect3DDevice9*)context3D);
#endif
            break;
        default:
            GrAssert(!"unknown engine");
            break;
    }

    return gpu;
}
