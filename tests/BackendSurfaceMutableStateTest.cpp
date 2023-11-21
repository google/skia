/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "tests/Test.h"

#ifdef SK_VULKAN
#include "include/core/SkAlphaType.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImage.h"
#include "include/core/SkRefCnt.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrTypes.h"
#include "include/gpu/MutableTextureState.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "include/gpu/ganesh/vk/GrVkBackendSurface.h"
#include "include/gpu/vk/GrVkTypes.h"
#include "include/gpu/vk/VulkanMutableTextureState.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/gpu/ganesh/GrTextureProxy.h"
#include "src/gpu/ganesh/vk/GrVkCaps.h"
#include "src/gpu/ganesh/vk/GrVkGpu.h"
#include "src/gpu/ganesh/vk/GrVkImage.h"
#include "src/gpu/ganesh/vk/GrVkTexture.h"
#include "tests/CtsEnforcement.h"
#include "tools/gpu/ProxyUtils.h"

#include <vulkan/vulkan_core.h>
#include <cstdint>

class GrTexture;
struct GrContextOptions;

using skgpu::MutableTextureStates::GetVkImageLayout;
using skgpu::MutableTextureStates::GetVkQueueFamilyIndex;

DEF_GANESH_TEST_FOR_VULKAN_CONTEXT(VkBackendSurfaceMutableStateTest,
                                   reporter,
                                   ctxInfo,
                                   CtsEnforcement::kApiLevel_T) {
    auto dContext = ctxInfo.directContext();

    GrBackendFormat format = GrBackendFormats::MakeVk(VK_FORMAT_R8G8B8A8_UNORM);
    GrBackendTexture backendTex = dContext->createBackendTexture(
            32, 32, format, skgpu::Mipmapped::kNo, GrRenderable::kNo, GrProtected::kNo);

    REPORTER_ASSERT(reporter, backendTex.isValid());

    GrVkImageInfo info;
    REPORTER_ASSERT(reporter, GrBackendTextures::GetVkImageInfo(backendTex, &info));
    VkImageLayout initLayout = info.fImageLayout;
    uint32_t initQueue = info.fCurrentQueueFamily;
    skgpu::MutableTextureState initState(initLayout, initQueue);

    // Verify that setting that state via a copy of a backendTexture is reflected in all the
    // backendTextures.
    GrBackendTexture backendTexCopy = backendTex;
    REPORTER_ASSERT(reporter, GrBackendTextures::GetVkImageInfo(backendTexCopy, &info));
    REPORTER_ASSERT(reporter, initLayout == info.fImageLayout);
    REPORTER_ASSERT(reporter, initQueue == info.fCurrentQueueFamily);

    skgpu::MutableTextureState newState(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                        VK_QUEUE_FAMILY_IGNORED);
    backendTexCopy.setMutableState(newState);

    REPORTER_ASSERT(reporter, GrBackendTextures::GetVkImageInfo(backendTex, &info));
    REPORTER_ASSERT(reporter, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL == info.fImageLayout);
    REPORTER_ASSERT(reporter, VK_QUEUE_FAMILY_IGNORED == info.fCurrentQueueFamily);

    REPORTER_ASSERT(reporter, GrBackendTextures::GetVkImageInfo(backendTexCopy, &info));
    REPORTER_ASSERT(reporter, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL == info.fImageLayout);
    REPORTER_ASSERT(reporter, VK_QUEUE_FAMILY_IGNORED == info.fCurrentQueueFamily);

    // Setting back to the init state since we didn't actually change it
    backendTex.setMutableState(initState);

    sk_sp<SkImage> wrappedImage = SkImages::BorrowTextureFrom(dContext,
                                                              backendTex,
                                                              kTopLeft_GrSurfaceOrigin,
                                                              kRGBA_8888_SkColorType,
                                                              kPremul_SkAlphaType,
                                                              nullptr);

    GrSurfaceProxy* proxy = sk_gpu_test::GetTextureImageProxy(wrappedImage.get(), dContext);

    REPORTER_ASSERT(reporter, proxy);
    REPORTER_ASSERT(reporter, proxy->isInstantiated());
    GrTexture* texture = proxy->peekTexture();
    REPORTER_ASSERT(reporter, texture);

    // Verify that modifying the layout via the GrVkTexture is reflected in the GrBackendTexture
    GrVkImage* vkTexture = static_cast<GrVkTexture*>(texture)->textureImage();
    REPORTER_ASSERT(reporter, initLayout == vkTexture->currentLayout());
    REPORTER_ASSERT(reporter, initQueue == vkTexture->currentQueueFamilyIndex());
    vkTexture->updateImageLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    REPORTER_ASSERT(reporter, GrBackendTextures::GetVkImageInfo(backendTex, &info));
    REPORTER_ASSERT(reporter, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL == info.fImageLayout);
    REPORTER_ASSERT(reporter, initQueue == info.fCurrentQueueFamily);

    GrBackendTexture backendTexImage;
    bool ok = SkImages::GetBackendTextureFromImage(wrappedImage, &backendTexImage, false);
    REPORTER_ASSERT(reporter, ok);
    REPORTER_ASSERT(reporter, GrBackendTextures::GetVkImageInfo(backendTexImage, &info));
    REPORTER_ASSERT(reporter, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL == info.fImageLayout);
    REPORTER_ASSERT(reporter, initQueue == info.fCurrentQueueFamily);

    // Verify that modifying the layout via the GrBackendTexutre is reflected in the GrVkTexture
    backendTexImage.setMutableState(newState);
    REPORTER_ASSERT(reporter,
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL == vkTexture->currentLayout());
    REPORTER_ASSERT(reporter, VK_QUEUE_FAMILY_IGNORED == info.fCurrentQueueFamily);

    vkTexture->setQueueFamilyIndex(initQueue);
    vkTexture->updateImageLayout(initLayout);

    REPORTER_ASSERT(reporter, GrBackendTextures::GetVkImageInfo(backendTex, &info));
    REPORTER_ASSERT(reporter, initLayout == info.fImageLayout);
    REPORTER_ASSERT(reporter, initQueue == info.fCurrentQueueFamily);

    REPORTER_ASSERT(reporter, GrBackendTextures::GetVkImageInfo(backendTexCopy, &info));
    REPORTER_ASSERT(reporter, initLayout == info.fImageLayout);
    REPORTER_ASSERT(reporter, initQueue == info.fCurrentQueueFamily);

    REPORTER_ASSERT(reporter, GrBackendTextures::GetVkImageInfo(backendTexImage, &info));
    REPORTER_ASSERT(reporter, initLayout == info.fImageLayout);
    REPORTER_ASSERT(reporter, initQueue == info.fCurrentQueueFamily);

    // Test using the setBackendTextureStateAPI. Unlike the previous test this will actually add
    // real transitions to the image so we need to be careful about doing actual valid transitions.
    GrVkGpu* gpu = static_cast<GrVkGpu*>(dContext->priv().getGpu());

    skgpu::MutableTextureState previousState;

    dContext->setBackendTextureState(backendTex, newState, &previousState);

    REPORTER_ASSERT(reporter, GrBackendTextures::GetVkImageInfo(backendTex, &info));
    REPORTER_ASSERT(reporter, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL == info.fImageLayout);
    REPORTER_ASSERT(reporter, gpu->queueIndex() == info.fCurrentQueueFamily);

    REPORTER_ASSERT(reporter, previousState.isValid());
    REPORTER_ASSERT(reporter, previousState.backend() == skgpu::BackendApi::kVulkan);
    REPORTER_ASSERT(reporter, GetVkImageLayout(previousState) == initLayout);
    REPORTER_ASSERT(reporter, GetVkQueueFamilyIndex(previousState) == initQueue);

    // Make sure passing in VK_IMAGE_LAYOUT_UNDEFINED does not change the layout
    skgpu::MutableTextureState noopState(VK_IMAGE_LAYOUT_UNDEFINED, VK_QUEUE_FAMILY_IGNORED);
    dContext->setBackendTextureState(backendTex, noopState, &previousState);
    REPORTER_ASSERT(reporter, GrBackendTextures::GetVkImageInfo(backendTex, &info));
    REPORTER_ASSERT(reporter, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL == info.fImageLayout);
    REPORTER_ASSERT(reporter, gpu->queueIndex() == info.fCurrentQueueFamily);

    REPORTER_ASSERT(reporter, previousState.isValid());
    REPORTER_ASSERT(reporter, previousState.backend() == skgpu::BackendApi::kVulkan);
    REPORTER_ASSERT(reporter,
                    GetVkImageLayout(previousState) == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    REPORTER_ASSERT(reporter, GetVkQueueFamilyIndex(previousState) == gpu->queueIndex());

    // To test queue transitions, we don't have any other valid queue available so instead we try
    // to transition to external queue.
    if (gpu->vkCaps().supportsExternalMemory()) {
        skgpu::MutableTextureState externalState(VK_IMAGE_LAYOUT_GENERAL, VK_QUEUE_FAMILY_EXTERNAL);

        dContext->setBackendTextureState(backendTex, externalState, &previousState);

        REPORTER_ASSERT(reporter, GrBackendTextures::GetVkImageInfo(backendTex, &info));
        REPORTER_ASSERT(reporter, VK_IMAGE_LAYOUT_GENERAL == info.fImageLayout);
        REPORTER_ASSERT(reporter, VK_QUEUE_FAMILY_EXTERNAL == info.fCurrentQueueFamily);

        REPORTER_ASSERT(reporter, previousState.isValid());
        REPORTER_ASSERT(reporter, previousState.backend() == skgpu::BackendApi::kVulkan);
        REPORTER_ASSERT(reporter,
                GetVkImageLayout(previousState) == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        REPORTER_ASSERT(reporter, GetVkQueueFamilyIndex(previousState) == gpu->queueIndex());

        dContext->submit();

        // Go back to the initial queue. Also we should stay in VK_IMAGE_LAYOUT_GENERAL since we
        // are passing in VK_IMAGE_LAYOUT_UNDEFINED
        skgpu::MutableTextureState externalState2(VK_IMAGE_LAYOUT_UNDEFINED, initQueue);
        dContext->setBackendTextureState(backendTex, externalState2, &previousState);

        REPORTER_ASSERT(reporter, GrBackendTextures::GetVkImageInfo(backendTex, &info));
        REPORTER_ASSERT(reporter, VK_IMAGE_LAYOUT_GENERAL == info.fImageLayout);
        REPORTER_ASSERT(reporter, gpu->queueIndex() == info.fCurrentQueueFamily);

        REPORTER_ASSERT(reporter, previousState.isValid());
        REPORTER_ASSERT(reporter, previousState.backend() == skgpu::BackendApi::kVulkan);
        REPORTER_ASSERT(reporter, GetVkImageLayout(previousState) == VK_IMAGE_LAYOUT_GENERAL);
        REPORTER_ASSERT(reporter, GetVkQueueFamilyIndex(previousState) == VK_QUEUE_FAMILY_EXTERNAL);
    }

    // We must submit this work before we try to delete the backend texture.
    dContext->submit(GrSyncCpu::kYes);

    dContext->deleteBackendTexture(backendTex);
}

#endif
