/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/vk/VulkanGraphicsPipeline.h"

#include "src/gpu/graphite/vk/VulkanSharedContext.h"

namespace skgpu::graphite {

sk_sp<VulkanGraphicsPipeline> VulkanGraphicsPipeline::Make(const VulkanSharedContext* sharedContext
                                                           /* TODO: fill in argument list*/) {

    return sk_sp<VulkanGraphicsPipeline>(new VulkanGraphicsPipeline(sharedContext));
}

void VulkanGraphicsPipeline::freeGpuData() {
}

} // namespace skgpu::graphite
