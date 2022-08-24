/*
 * Copyright 2022 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_VulkanTypes_DEFINED
#define skgpu_VulkanTypes_DEFINED

#include "include/core/SkTypes.h"
#include "include/private/gpu/vk/SkiaVulkan.h"

#include <functional>

#ifndef VK_VERSION_1_1
#error Skia requires the use of Vulkan 1.1 headers
#endif

namespace skgpu {

using VulkanGetProc = std::function<PFN_vkVoidFunction(
        const char*, // function name
        VkInstance,  // instance or VK_NULL_HANDLE
        VkDevice     // device or VK_NULL_HANDLE
        )>;

} // namespace skgpu

#endif // skgpu_VulkanTypes_DEFINED
