
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrTypes.h"

#include "gl/GrGLConfig.h"

#include "GrGpu.h"
#include "gl/GrGLGpu.h"

GrGpu* GrGpu::Create(GrBackend backend, GrBackendContext backendContext, GrContext* context) {

    const GrGLInterface* glInterface = NULL;
    SkAutoTUnref<const GrGLInterface> glInterfaceUnref;

    if (kOpenGL_GrBackend == backend) {
        glInterface = reinterpret_cast<const GrGLInterface*>(backendContext);
        if (NULL == glInterface) {
            glInterface = GrGLDefaultInterface();
            // By calling GrGLDefaultInterface we've taken a ref on the
            // returned object. We only want to hold that ref until after
            // the GrGpu is constructed and has taken ownership.
            glInterfaceUnref.reset(glInterface);
        }
        if (NULL == glInterface) {
#ifdef SK_DEBUG
            SkDebugf("No GL interface provided!\n");
#endif
            return NULL;
        }
        GrGLContext ctx(glInterface);
        if (ctx.isInitialized()) {
            return SkNEW_ARGS(GrGLGpu, (ctx, context));
        }
    }
    return NULL;
}
