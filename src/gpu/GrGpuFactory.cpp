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
#ifdef SK_VULKAN
#include "vk/GrVkGpu.h"
#endif

static CreateGpuProc gGpuFactories[kBackendCount] = { GrGLGpu::Create, nullptr };

#ifdef SK_VULKAN
GrGpuFactoryRegistrar gVkGpuFactoryProc(kVulkan_GrBackend, GrVkGpu::Create);
#endif

GrGpuFactoryRegistrar::GrGpuFactoryRegistrar(int i, CreateGpuProc proc) {
    gGpuFactories[i] = proc;
}

GrGpu* GrGpu::Create(GrBackend backend,
                     GrBackendContext backendContext,
                     const GrContextOptions& options,
                     GrContext* context) {
    SkASSERT((int)backend < kBackendCount);
    if (!gGpuFactories[backend]) {
        return nullptr;
    }
    return (gGpuFactories[backend])(backendContext, options, context);
}
