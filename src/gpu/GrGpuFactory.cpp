
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

    const GrGLInterface* glInterface = NULL;
    SkAutoTUnref<const GrGLInterface> glInterfaceUnref;

    if (kOpenGL_Shaders_GrEngine == engine ||
        kOpenGL_Fixed_GrEngine == engine) {
        glInterface = reinterpret_cast<const GrGLInterface*>(context3D);
        if (NULL == glInterface) {
            glInterface = GrGLDefaultInterface();
            // By calling GrGLDefaultInterface we've taken a ref on the
            // returned object. We only want to hold that ref until after
            // the GrGpu is constructed and has taken ownership.
            glInterfaceUnref.reset(glInterface);
        }
        if (NULL == glInterface) {
#if GR_DEBUG
            GrPrintf("No GL interface provided!\n");
#endif
            return NULL;
        }
        if (!glInterface->validate(engine)) {
#if GR_DEBUG
            GrPrintf("Failed GL interface validation!\n");
#endif
            return NULL;
        }
    }

    GrGpu* gpu = NULL;

    switch (engine) {
        case kOpenGL_Shaders_GrEngine:
            GrAssert(NULL != glInterface);
            gpu = new GrGpuGLShaders(glInterface);
            break;
        case kOpenGL_Fixed_GrEngine:
            GrAssert(NULL != glInterface);
            gpu = new GrGpuGLFixed(glInterface);
            break;
        default:
            GrAssert(!"unknown engine");
            break;
    }

    return gpu;
}
