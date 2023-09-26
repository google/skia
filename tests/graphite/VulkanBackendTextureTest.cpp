/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/vk/VulkanGraphiteTypes.h"

using namespace skgpu::graphite;

namespace {
const SkISize kSize = {16, 16};
}

DEF_GRAPHITE_TEST_FOR_VULKAN_CONTEXT(VulkanBackendTextureSimpleCreationTest, reporter, context,
                                     CtsEnforcement::kNextRelease) {
    auto recorder = context->makeRecorder();

    VulkanTextureInfo textureInfo;
    textureInfo.fSampleCount = 1;
    textureInfo.fMipmapped = skgpu::Mipmapped::kNo;
    textureInfo.fFlags = 0;
    textureInfo.fFormat = VK_FORMAT_R8G8B8A8_UNORM;
    textureInfo.fImageTiling = VK_IMAGE_TILING_OPTIMAL;
    textureInfo.fImageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT;
    textureInfo.fSharingMode = VK_SHARING_MODE_EXCLUSIVE;

    auto beTexture = recorder->createBackendTexture(kSize, textureInfo);
    REPORTER_ASSERT(reporter, beTexture.isValid());
    recorder->deleteBackendTexture(beTexture);

    // It should also pass if we set the usage to be a render target
    textureInfo.fImageUsageFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    beTexture = recorder->createBackendTexture(kSize, textureInfo);
    REPORTER_ASSERT(reporter, beTexture.isValid());
    recorder->deleteBackendTexture(beTexture);
}

// Test that copying BackendTexture variables works.
DEF_GRAPHITE_TEST_FOR_VULKAN_CONTEXT(VulkanBackendTextureCopyVariableTest, reporter, context,
                                     CtsEnforcement::kNextRelease) {
    auto recorder = context->makeRecorder();

    VulkanTextureInfo textureInfo;
    textureInfo.fSampleCount = 1;
    textureInfo.fMipmapped = skgpu::Mipmapped::kNo;
    textureInfo.fFlags = 0;
    textureInfo.fFormat = VK_FORMAT_R8G8B8A8_UNORM;
    textureInfo.fImageTiling = VK_IMAGE_TILING_OPTIMAL;
    textureInfo.fImageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT;
    textureInfo.fSharingMode = VK_SHARING_MODE_EXCLUSIVE;

    BackendTexture beTexture = recorder->createBackendTexture(kSize, textureInfo);
    REPORTER_ASSERT(reporter, beTexture.isValid());

    BackendTexture beTexture2;
    REPORTER_ASSERT(reporter, beTexture2 != beTexture);
    REPORTER_ASSERT(reporter, beTexture2.getVkImage() == VK_NULL_HANDLE);
    REPORTER_ASSERT(reporter, beTexture2.getVkImageLayout() == VK_IMAGE_LAYOUT_UNDEFINED);

    beTexture2 = beTexture;
    REPORTER_ASSERT(reporter, beTexture2 == beTexture);
    REPORTER_ASSERT(reporter, beTexture2.getVkImage() == beTexture.getVkImage());
    REPORTER_ASSERT(reporter, beTexture2.getVkImageLayout() == beTexture.getVkImageLayout());
    REPORTER_ASSERT(
            reporter, beTexture2.getVkQueueFamilyIndex() == beTexture.getVkQueueFamilyIndex());

    recorder->deleteBackendTexture(beTexture);
    // The backend memory of beTexture2 == that of beTexture, so only call delete once.
}
