/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkColorSpace.h"
#include "include/core/SkSurface.h"
#include "include/gpu/MutableTextureState.h"
#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Image.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Surface.h"
#include "include/gpu/vk/VulkanTypes.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/ResourceTypes.h"

using namespace skgpu;
using namespace skgpu::graphite;

namespace {
    const SkISize kSize = {16, 16};
}

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(BackendTextureTest, reporter, context,
                                   CtsEnforcement::kNextRelease) {
    // TODO: Remove this check once Vulkan supports creating default TexutreInfo from caps and we
    // implement createBackendTexture.
    if (context->backend() == BackendApi::kVulkan) {
        return;
    }

    auto caps = context->priv().caps();
    auto recorder = context->makeRecorder();

    TextureInfo info = caps->getDefaultSampledTextureInfo(kRGBA_8888_SkColorType,
                                                          /*mipmapped=*/Mipmapped::kNo,
                                                          Protected::kNo,
                                                          Renderable::kNo);
    REPORTER_ASSERT(reporter, info.isValid());

    auto texture1 = recorder->createBackendTexture(kSize, info);
    REPORTER_ASSERT(reporter, texture1.isValid());

    // We make a copy to do the remaining tests so we still have texture1 to safely delete the
    // backend object.
    auto texture1Copy = texture1;
    REPORTER_ASSERT(reporter, texture1Copy.isValid());
    REPORTER_ASSERT(reporter, texture1 == texture1Copy);

    auto texture2 = recorder->createBackendTexture(kSize, info);
    REPORTER_ASSERT(reporter, texture2.isValid());

    REPORTER_ASSERT(reporter, texture1Copy != texture2);

    // Test state after assignment
    texture1Copy = texture2;
    REPORTER_ASSERT(reporter, texture1Copy.isValid());
    REPORTER_ASSERT(reporter, texture1Copy == texture2);

    BackendTexture invalidTexture;
    REPORTER_ASSERT(reporter, !invalidTexture.isValid());

    texture1Copy = invalidTexture;
    REPORTER_ASSERT(reporter, !texture1Copy.isValid());

    texture1Copy = texture1;
    REPORTER_ASSERT(reporter, texture1Copy.isValid());
    REPORTER_ASSERT(reporter, texture1 == texture1Copy);

    recorder->deleteBackendTexture(texture1);
    recorder->deleteBackendTexture(texture2);

    // Test that deleting is safe from the Context or a different Recorder.
    texture1 = recorder->createBackendTexture(kSize, info);
    context->deleteBackendTexture(texture1);

    auto recorder2 = context->makeRecorder();
    texture1 = recorder->createBackendTexture(kSize, info);
    recorder2->deleteBackendTexture(texture1);
}

// Tests the wrapping of a BackendTexture in an SkSurface
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(SurfaceBackendTextureTest, reporter, context,
                                   CtsEnforcement::kNextRelease) {
    // TODO: Right now this just tests very basic combinations of surfaces. This should be expanded
    // to cover a much broader set of things once we add more support in Graphite for different
    // formats, color types, etc.

    // TODO: Remove this check once Vulkan supports creating default TexutreInfo from caps and we
    // implement createBackendTexture.
    if (context->backend() == BackendApi::kVulkan) {
        return;
    }

    auto caps = context->priv().caps();
    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    TextureInfo info = caps->getDefaultSampledTextureInfo(kRGBA_8888_SkColorType,
                                                          /*mipmapped=*/Mipmapped::kNo,
                                                          Protected::kNo,
                                                          Renderable::kYes);

    auto texture = recorder->createBackendTexture(kSize, info);
    REPORTER_ASSERT(reporter, texture.isValid());

    sk_sp<SkSurface> surface = SkSurfaces::WrapBackendTexture(recorder.get(),
                                                              texture,
                                                              kRGBA_8888_SkColorType,
                                                              /*colorSpace=*/nullptr,
                                                              /*props=*/nullptr);
    REPORTER_ASSERT(reporter, surface);

    surface.reset();

    // We should fail when trying to wrap the same texture in a surface with a non-compatible
    // color type.
    surface = SkSurfaces::WrapBackendTexture(recorder.get(),
                                             texture,
                                             kAlpha_8_SkColorType,
                                             /*colorSpace=*/nullptr,
                                             /*props=*/nullptr);
    REPORTER_ASSERT(reporter, !surface);

    recorder->deleteBackendTexture(texture);

    // We should fail to wrap a non-renderable texture in a surface.
    info = caps->getDefaultSampledTextureInfo(kRGBA_8888_SkColorType,
                                              /*mipmapped=*/Mipmapped::kNo,
                                              Protected::kNo,
                                              Renderable::kNo);
    texture = recorder->createBackendTexture(kSize, info);
    REPORTER_ASSERT(reporter, texture.isValid());

    surface = SkSurfaces::WrapBackendTexture(recorder.get(),
                                             texture,
                                             kRGBA_8888_SkColorType,
                                             /*colorSpace=*/nullptr,
                                             /*props=*/nullptr);

    REPORTER_ASSERT(reporter, !surface);
    recorder->deleteBackendTexture(texture);
}

// Tests the wrapping of a BackendTexture in an SkImage
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(ImageBackendTextureTest, reporter, context,
                                   CtsEnforcement::kNextRelease) {
    // TODO: Right now this just tests very basic combinations of images. This should be expanded
    // to cover a much broader set of things once we add more support in Graphite for different
    // formats, color types, etc.

    // TODO: Remove this check once Vulkan supports creating default TexutreInfo from caps and we
    // implement createBackendTexture.
    if (context->backend() == BackendApi::kVulkan) {
        return;
    }

    const Caps* caps = context->priv().caps();
    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    for (Mipmapped mipmapped : { Mipmapped::kYes, Mipmapped::kNo }) {
        for (Renderable renderable : { Renderable::kYes, Renderable::kNo }) {

            TextureInfo info = caps->getDefaultSampledTextureInfo(kRGBA_8888_SkColorType,
                                                                  mipmapped,
                                                                  Protected::kNo,
                                                                  renderable);

            BackendTexture texture = recorder->createBackendTexture(kSize, info);
            REPORTER_ASSERT(reporter, texture.isValid());

            sk_sp<SkImage> image = SkImages::WrapTexture(recorder.get(),
                                                         texture,
                                                         kRGBA_8888_SkColorType,
                                                         kPremul_SkAlphaType,
                                                         /*colorSpace=*/nullptr);
            REPORTER_ASSERT(reporter, image);
            REPORTER_ASSERT(reporter, image->hasMipmaps() == (mipmapped == Mipmapped::kYes));

            image.reset();

            // We should fail when trying to wrap the same texture in an image with a non-compatible
            // color type.
            image = SkImages::WrapTexture(recorder.get(),
                                          texture,
                                          kAlpha_8_SkColorType,
                                          kPremul_SkAlphaType,
                                          /* colorSpace= */ nullptr);
            REPORTER_ASSERT(reporter, !image);

            recorder->deleteBackendTexture(texture);
        }
    }
}

#ifdef SK_VULKAN
DEF_GRAPHITE_TEST_FOR_VULKAN_CONTEXT(VulkanBackendTextureMutableStateTest, reporter, context,
                                     CtsEnforcement::kNextRelease) {
    VulkanTextureInfo info(/*sampleCount=*/1,
                           /*mipmapped=*/Mipmapped::kNo,
                           /*flags=*/0,
                           VK_FORMAT_R8G8B8A8_UNORM,
                           VK_IMAGE_TILING_OPTIMAL,
                           VK_IMAGE_USAGE_SAMPLED_BIT,
                           VK_SHARING_MODE_EXCLUSIVE,
                           VK_IMAGE_ASPECT_COLOR_BIT,
                           /*ycbcrConversionInfo*/{});

    BackendTexture texture({16, 16},
                           info,
                           VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                           /*queueFamilyIndex=*/1,
                           VK_NULL_HANDLE,
                           skgpu::VulkanAlloc());

    REPORTER_ASSERT(reporter, texture.isValid());
    REPORTER_ASSERT(reporter,
                    texture.getVkImageLayout() == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    REPORTER_ASSERT(reporter, texture.getVkQueueFamilyIndex() == 1);

    skgpu::MutableTextureState newState(VK_IMAGE_LAYOUT_GENERAL, 0);
    texture.setMutableState(newState);

    REPORTER_ASSERT(reporter,
                    texture.getVkImageLayout() == VK_IMAGE_LAYOUT_GENERAL);
    REPORTER_ASSERT(reporter, texture.getVkQueueFamilyIndex() == 0);

    // TODO: Add to this test to check that the setMutableState calls also update values we see in
    // wrapped VulkanTextures once we have them. Also check that updates in VulkanTexture are also
    // visible in the getters of BackendTexture. We will need a real VkImage to do these tests.
}
#endif // SK_VULKAN
