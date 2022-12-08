/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_VulkanBuffer_DEFINED
#define skgpu_graphite_VulkanBuffer_DEFINED

#include "include/gpu/vk/VulkanMemoryAllocator.h"
#include "src/gpu/graphite/Buffer.h"

namespace skgpu::graphite {

class VulkanSharedContext;

class VulkanBuffer final : public Buffer {
public:
    static sk_sp<Buffer> Make(const VulkanSharedContext*, size_t, BufferType, PrioritizeGpuReads);
    void freeGpuData() override {}

private:
    VulkanBuffer(const VulkanSharedContext*, size_t, BufferType, PrioritizeGpuReads, VkBuffer);

    void onMap() override {}
    void onUnmap() override {}

    VkBuffer fBuffer;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_VulkanBuffer_DEFINED

