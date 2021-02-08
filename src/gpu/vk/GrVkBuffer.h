/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkBuffer_DEFINED
#define GrVkBuffer_DEFINED

#include "include/gpu/vk/GrVkTypes.h"
#include "src/gpu/GrGpuBuffer.h"

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
                const GrVkAlloc& alloc,
                const GrVkDescriptorSet* uniformDescriptorSet);

    bool isVkMappable() const { return fAlloc.fFlags & GrVkAlloc::kMappable_Flag; }

    bool vkIsMapped() const { return SkToBool(fMapPtr); }
    void vkMap(size_t size);
    void vkUnmap(size_t size);
    void copyCpuDataToGpuBuffer(const void* srcData, size_t size);


    void onMap() override;
    void onUnmap() override;
    bool onUpdateData(const void* src, size_t srcSizeInBytes) override;

    void vkRelease();

    void onAbandon() override;
    void onRelease() override;

    GrVkGpu* getVkGpu() const;

    VkBuffer fBuffer;
    GrVkAlloc fAlloc;

    const GrVkDescriptorSet* fUniformDescriptorSet;

    using INHERITED = GrGpuBuffer;
};

#endif
