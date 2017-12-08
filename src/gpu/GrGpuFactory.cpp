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

sk_sp<GrGpu> GrGpu::Make(GrBackend backend,
                         GrBackendContext backendContext,
                         GrContext* context) {
    switch (backend) {
        case kOpenGL_GrBackend:
            return GrGLGpu::Make(backendContext, context);
#ifdef SK_VULKAN
        case kVulkan_GrBackend:
            return GrVkGpu::Make(backendContext, context);
#endif
        case kMock_GrBackend:
            return GrMockGpu::Make(backendContext, context);
        default:
            return nullptr;
    }
}
