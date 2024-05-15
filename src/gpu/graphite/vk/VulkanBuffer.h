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
#include "src/gpu/graphite/vk/VulkanSharedContext.h"

namespace skgpu::graphite {

class VulkanCommandBuffer;

class VulkanBuffer final : public Buffer {
public:
    static sk_sp<Buffer> Make(const VulkanSharedContext*,
                              size_t,
                              BufferType,
                              AccessPattern,
                              std::string_view label);
    void freeGpuData() override;
    VkBuffer vkBuffer() const { return fBuffer; }
    VkBufferUsageFlags bufferUsageFlags() const { return fBufferUsageFlags; }

    void setBufferAccess(VulkanCommandBuffer* buffer,
                         VkAccessFlags dstAccessMask,
                         VkPipelineStageFlags dstStageMask) const;

private:
    VulkanBuffer(const VulkanSharedContext*,
                 size_t,
                 BufferType,
                 AccessPattern,
                 VkBuffer,
                 const skgpu::VulkanAlloc&,
                 VkBufferUsageFlags,
                 std::string_view label);

    void onMap() override;
    void onUnmap() override;

    void internalMap(size_t readOffset, size_t readSize);
    void internalUnmap(size_t flushOffset, size_t flushSize);

    bool isMappable() const { return fAlloc.fFlags & skgpu::VulkanAlloc::kMappable_Flag; }

    const VulkanSharedContext* vulkanSharedContext() const {
        return static_cast<const VulkanSharedContext*>(this->sharedContext());
    }

    static VkPipelineStageFlags AccessMaskToPipelineSrcStageFlags(const VkAccessFlags accessFlags);

    VkBuffer fBuffer;
    skgpu::VulkanAlloc fAlloc;
    const VkBufferUsageFlags fBufferUsageFlags;
    mutable VkAccessFlags fCurrentAccessMask = 0;

    /**
     * Buffers can either be mapped for:
     * 1) Reading from the CPU (The effect of writing would be undefined)
     * 2) Writing from the CPU (The existing contents are discarded. Even in the case where the
     *    initial contents are overwritten, CPU reads should be avoided for performance reasons as
     *    the memory may not be cached).
     */
    bool fBufferUsedForCPURead = false;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_VulkanBuffer_DEFINED

