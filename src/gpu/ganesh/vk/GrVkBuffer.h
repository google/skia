/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkBuffer_DEFINED
#define GrVkBuffer_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/vk/VulkanTypes.h"
#include "include/private/base/SkTo.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "include/private/gpu/vk/SkiaVulkan.h"
#include "src/gpu/ganesh/GrGpuBuffer.h"

#include <cstddef>
#include <string_view>

class GrVkDescriptorSet;
class GrVkGpu;

class GrVkBuffer : public GrGpuBuffer {
public:
    static sk_sp<GrVkBuffer> Make(GrVkGpu* gpu,
                                  size_t size,
                                  GrGpuBufferType bufferType,
                                  GrAccessPattern accessPattern);

    VkBuffer vkBuffer() const { return fBuffer; }

    void addMemoryBarrier(VkAccessFlags srcAccessMask,
                          VkAccessFlags dstAccesMask,
                          VkPipelineStageFlags srcStageMask,
                          VkPipelineStageFlags dstStageMask,
                          bool byRegion) const;

    // If the buffer is a uniform buffer, return the descriptor set for that buffer. It is not valid
    // to call this on non uniform buffers.
    const VkDescriptorSet* uniformDescriptorSet() const;

private:
    GrVkBuffer(GrVkGpu* gpu,
               size_t sizeInBytes,
               GrGpuBufferType bufferType,
               GrAccessPattern accessPattern,
               VkBuffer buffer,
               const skgpu::VulkanAlloc& alloc,
               const GrVkDescriptorSet* uniformDescriptorSet,
               std::string_view label);

    bool isVkMappable() const { return fAlloc.fFlags & skgpu::VulkanAlloc::kMappable_Flag; }

    bool vkIsMapped() const { return SkToBool(fMapPtr); }
    void vkMap(size_t readOffset, size_t readSize);
    void vkUnmap(size_t flushOffset, size_t flushSize);
    void copyCpuDataToGpuBuffer(const void* srcData, size_t offset, size_t size);

    void onMap(MapType) override;
    void onUnmap(MapType) override;
    bool onClearToZero() override;
    bool onUpdateData(const void* src, size_t offset, size_t size, bool preserve) override;

    void vkRelease();

    void onAbandon() override;
    void onRelease() override;

    GrVkGpu* getVkGpu() const;

    VkBuffer fBuffer;
    skgpu::VulkanAlloc fAlloc;

    const GrVkDescriptorSet* fUniformDescriptorSet;

    using INHERITED = GrGpuBuffer;
};

#endif
