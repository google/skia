
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrGpuFactory.h"

#include "GrGpu.h"
#include "gl/GrGLConfig.h"
#include "gl/GrGLGpu.h"

static const int kMaxNumBackends = 4;
static CreateGpuProc gGpuFactories[kMaxNumBackends] = {GrGLGpu::Create, NULL, NULL, NULL};

GrGpuFactoryRegistrar::GrGpuFactoryRegistrar(int i, CreateGpuProc proc) {
    gGpuFactories[i] = proc;
}

GrGpu* GrGpu::Create(GrBackend backend,
                     GrBackendContext backendContext,
                     const GrContextOptions& options,
                     GrContext* context) {
    SkASSERT((int)backend < kMaxNumBackends);
    if (!gGpuFactories[backend]) {
        return NULL;
    }
    return (gGpuFactories[backend])(backendContext, options, context);
}
