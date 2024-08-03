/*
 * Copyright 2024 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_VulkanMemoryAllocatorUtil_DEFINED
#define skgpu_VulkanMemoryAllocatorUtil_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/GpuTypes.h"
#include "include/private/base/SkAPI.h"
#include "include/private/gpu/vk/SkiaVulkan.h"

#include <optional>

namespace skgpu {

struct VulkanBackendContext;
class VulkanExtensions;
class VulkanMemoryAllocator;
enum class ThreadSafe : bool;

namespace VulkanMemoryAllocators {
// Returns a concrete implementation of a memory allocator. Because this has settings
// which are done at compile time, we cannot really expose this to clients in a meaningful way.
sk_sp<VulkanMemoryAllocator> Make(const skgpu::VulkanBackendContext&,
                                  ThreadSafe,
                                  std::optional<VkDeviceSize> blockSize);

}  // namespace VulkanMemoryAllocators
}  // namespace skgpu

#endif  // skgpu_VulkanMemoryAllocatorUtil_DEFINED
