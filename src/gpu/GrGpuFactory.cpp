
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrGpuFactory.h"

#include "gl/GrGLConfig.h"

#include "GrGpu.h"
#include "gl/GrGLGpu.h"

static GrGpu* gl_gpu_create(GrBackendContext backendContext, GrContext* context) {
    const GrGLInterface* glInterface = NULL;
    SkAutoTUnref<const GrGLInterface> glInterfaceUnref;

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
    return NULL;
}

static const int kMaxNumBackends = 4;
static CreateGpuProc gGpuFactories[kMaxNumBackends] = {gl_gpu_create, NULL, NULL, NULL};

GrGpuFactoryRegistrar::GrGpuFactoryRegistrar(int i, CreateGpuProc proc) {
    gGpuFactories[i] = proc;
}

GrGpu* GrGpu::Create(GrBackend backend, GrBackendContext backendContext, GrContext* context) {
    SkASSERT((int)backend < kMaxNumBackends);
    if (!gGpuFactories[backend]) {
        return NULL;
    }
    return (gGpuFactories[backend])(backendContext, context);
}
