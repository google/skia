/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrGpuFactory.h"
#include "gl/GrGLGpu.h"
#include "mock/GrMockGpu.h"
#ifdef SK_VULKAN
#include "vk/GrVkGpu.h"
#endif

GrGpu* GrGpu::Create(GrBackend backend,
                     GrBackendContext backendContext,
                     const GrContextOptions& options,
                     GrContext* context) {
    switch (backend) {
        case kOpenGL_GrBackend:
            return GrGLGpu::Create(backendContext, options, context);
#ifdef SK_VULKAN
        case kVulkan_GrBackend:
            return GrVkGpu::Create(backendContext, options, context);
#endif
        case kMock_GrBackend:
            return GrMockGpu::Create(backendContext, options, context);
        default:
            return nullptr;
    }
}
