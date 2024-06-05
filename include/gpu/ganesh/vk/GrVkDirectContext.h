/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkDirectContext_DEFINED
#define GrVkDirectContext_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/base/SkAPI.h"

class GrDirectContext;
struct GrContextOptions;
namespace skgpu {
struct VulkanBackendContext;
}

namespace GrDirectContexts {
/**
 * The Vulkan context (VkQueue, VkDevice, VkInstance) must be kept alive until the returned
 * GrDirectContext is destroyed. This also means that any objects created with this
 * GrDirectContext (e.g. SkSurfaces, SkImages, etc.) must also be released as they may hold
 * refs on the GrDirectContext. Once all these objects and the GrDirectContext are released,
 * then it is safe to delete the vulkan objects.
 */
SK_API sk_sp<GrDirectContext> MakeVulkan(const skgpu::VulkanBackendContext&,
                                         const GrContextOptions&);
SK_API sk_sp<GrDirectContext> MakeVulkan(const skgpu::VulkanBackendContext&);
}

#endif
