/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/graphite/TextureFormatTest.h"

#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/vk/VulkanGraphiteTypes.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/vk/VulkanGraphiteUtils.h"

namespace skgpu::graphite {

static TextureInfo create_vulkan_texture_info(TextureFormat format) {
        VulkanTextureInfo info;
        info.fFormat = TextureFormatToVkFormat(format);
        info.fImageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT;
        if (TextureFormatIsDepthOrStencil(format)) {
            info.fImageUsageFlags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        } else {
            info.fImageUsageFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                                     VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
        }

        return TextureInfos::MakeVulkan(info);
}

DEF_GRAPHITE_TEST_FOR_VULKAN_CONTEXT(TextureFormatTest_Vulkan, r, ctx, CtsEnforcement::kNextRelease) {
    for (int i = 0; i < kTextureFormatCount; ++i) {
        RunTextureFormatTest(r, ctx->priv().caps(),
                             static_cast<TextureFormat>(i),
                             create_vulkan_texture_info);
    }
}

} // namespace skgpu::graphite
