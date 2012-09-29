
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrTypes.h"

#include "gl/GrGLConfig.h"

#include "GrGpu.h"
#include "gl/GrGpuGL.h"

GrGpu* GrGpu::Create(GrEngine engine, GrPlatform3DContext context3D) {

    const GrGLInterface* glInterface = NULL;
    SkAutoTUnref<const GrGLInterface> glInterfaceUnref;

    if (kOpenGL_Shaders_GrEngine == engine) {
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
        GrGLContextInfo ctxInfo(glInterface);
        if (ctxInfo.isInitialized()) {
            return SkNEW_ARGS(GrGpuGL, (ctxInfo));
        }
    }
    return NULL;
}
