/*
* Copyright 2023 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef skgpu_graphite_VulkanFramebuffer_DEFINED
#define skgpu_graphite_VulkanFramebuffer_DEFINED

#include "src/gpu/graphite/Resource.h"

#include "src/gpu/graphite/vk/VulkanGraphiteUtilsPriv.h"

namespace skgpu::graphite {

class VulkanSharedContext;

/**
 * Resource wrapper for VkFramebuffer
*/
class VulkanFramebuffer : public Resource {
public:
    static sk_sp<VulkanFramebuffer> Make(const VulkanSharedContext*,
                                         const VkFramebufferCreateInfo&);

    VkFramebuffer framebuffer() {
        return fFramebuffer;
    }

    const char* getResourceType() const override { return "Vulkan Framebuffer"; }

private:
    VulkanFramebuffer(const VulkanSharedContext*, VkFramebuffer);
    void freeGpuData() override;

    const VulkanSharedContext* fSharedContext;
    VkFramebuffer fFramebuffer;
};
} // namespace skgpu::graphite

#endif // skgpu_graphite_VulkanFramebuffer_DEFINED
