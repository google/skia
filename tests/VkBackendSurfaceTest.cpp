/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a GPU-backend specific test. It relies on static intializers to work

#include "SkTypes.h"

#if defined(SK_VULKAN)

#include "vk/GrVkVulkan.h"

#include "Test.h"

#include "GrBackendSurface.h"
#include "GrContextPriv.h"
#include "GrTextureProxy.h"
#include "GrTexture.h"
#include "SkImage.h"
#include "SkImage_Base.h"
#include "vk/GrVkGpu.h"
#include "vk/GrVkImageLayout.h"
#include "vk/GrVkTexture.h"
#include "vk/GrVkTypes.h"

DEF_GPUTEST_FOR_VULKAN_CONTEXT(VkImageLayoutTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    GrVkGpu* gpu = static_cast<GrVkGpu*>(context->priv().getGpu());

    GrBackendTexture backendTex = gpu->createTestingOnlyBackendTexture(nullptr, 1, 1,
                                                                       GrColorType::kRGBA_8888,
                                                                       false,
                                                                       GrMipMapped::kNo);
    REPORTER_ASSERT(reporter, backendTex.isValid());

    GrVkImageInfo info;
    REPORTER_ASSERT(reporter, backendTex.getVkImageInfo(&info));
    VkImageLayout initLayout = info.fImageLayout;

    // Verify that setting that layout via a copy of a backendTexture is reflected in all the
    // backendTextures.
    GrBackendTexture backendTexCopy = backendTex;
    REPORTER_ASSERT(reporter, backendTexCopy.getVkImageInfo(&info));
    REPORTER_ASSERT(reporter, initLayout == info.fImageLayout);

    backendTexCopy.setVkImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    REPORTER_ASSERT(reporter, backendTex.getVkImageInfo(&info));
    REPORTER_ASSERT(reporter, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL == info.fImageLayout);

    REPORTER_ASSERT(reporter, backendTexCopy.getVkImageInfo(&info));
    REPORTER_ASSERT(reporter, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL == info.fImageLayout);

    // Setting back the layout since we didn't actually change it
    backendTex.setVkImageLayout(initLayout);

    sk_sp<SkImage> wrappedImage = SkImage::MakeFromTexture(context, backendTex,
                                                           kTopLeft_GrSurfaceOrigin,
                                                           kRGBA_8888_SkColorType,
                                                           kPremul_SkAlphaType, nullptr);
    REPORTER_ASSERT(reporter, wrappedImage.get());

    sk_sp<GrTextureProxy> texProxy = as_IB(wrappedImage)->asTextureProxyRef();
    REPORTER_ASSERT(reporter, texProxy.get());
    REPORTER_ASSERT(reporter, texProxy->isInstantiated());
    GrTexture* texture = texProxy->peekTexture();
    REPORTER_ASSERT(reporter, texture);

    // Verify that modifying the layout via the GrVkTexture is reflected in the GrBackendTexture
    GrVkTexture* vkTexture = static_cast<GrVkTexture*>(texture);
    REPORTER_ASSERT(reporter, initLayout == vkTexture->currentLayout());
    vkTexture->updateImageLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    REPORTER_ASSERT(reporter, backendTex.getVkImageInfo(&info));
    REPORTER_ASSERT(reporter, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL == info.fImageLayout);

    GrBackendTexture backendTexImage = wrappedImage->getBackendTexture(false);
    REPORTER_ASSERT(reporter, backendTexImage.getVkImageInfo(&info));
    REPORTER_ASSERT(reporter, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL == info.fImageLayout);

    // Verify that modifying the layout via the GrBackendTexutre is reflected in the GrVkTexture
    backendTexImage.setVkImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    REPORTER_ASSERT(reporter, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL == vkTexture->currentLayout());

    vkTexture->updateImageLayout(initLayout);

    REPORTER_ASSERT(reporter, backendTex.getVkImageInfo(&info));
    REPORTER_ASSERT(reporter, initLayout == info.fImageLayout);

    REPORTER_ASSERT(reporter, backendTexCopy.getVkImageInfo(&info));
    REPORTER_ASSERT(reporter, initLayout == info.fImageLayout);

    REPORTER_ASSERT(reporter, backendTexImage.getVkImageInfo(&info));
    REPORTER_ASSERT(reporter, initLayout == info.fImageLayout);

    // Check that we can do things like assigning the backend texture to invalid one, assign an
    // invalid one, assin a backend texture to inself etc. Success here is that we don't hit any of
    // our ref counting asserts.
    REPORTER_ASSERT(reporter, GrBackendTexture::TestingOnly_Equals(backendTex, backendTexCopy));

    GrBackendTexture invalidTexture;
    REPORTER_ASSERT(reporter, !invalidTexture.isValid());
    REPORTER_ASSERT(reporter, !GrBackendTexture::TestingOnly_Equals(invalidTexture, backendTexCopy));

    backendTexCopy = invalidTexture;
    REPORTER_ASSERT(reporter, !backendTexCopy.isValid());
    REPORTER_ASSERT(reporter, !GrBackendTexture::TestingOnly_Equals(invalidTexture, backendTexCopy));

    invalidTexture = backendTex;
    REPORTER_ASSERT(reporter, invalidTexture.isValid());
    REPORTER_ASSERT(reporter, GrBackendTexture::TestingOnly_Equals(invalidTexture, backendTex));

    invalidTexture = static_cast<decltype(invalidTexture)&>(invalidTexture);
    REPORTER_ASSERT(reporter, invalidTexture.isValid());
    REPORTER_ASSERT(reporter, GrBackendTexture::TestingOnly_Equals(invalidTexture, invalidTexture));

    gpu->deleteTestingOnlyBackendTexture(backendTex);
}

static void testing_release_proc(void* ctx) {
    int* count = (int*)ctx;
    *count += 1;
}

// Test to make sure we don't call our release proc on an image until we've transferred it back to
// its original queue family.
DEF_GPUTEST_FOR_VULKAN_CONTEXT(VkReleaseExternalQueueTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    GrVkGpu* gpu = static_cast<GrVkGpu*>(context->priv().getGpu());
    if (!gpu->vkCaps().supportsExternalMemory()) {
        return;
    }

    for (bool useExternal : {false, true}) {
        GrBackendTexture backendTex = gpu->createTestingOnlyBackendTexture(nullptr, 1, 1,
                                                                           GrColorType::kRGBA_8888,
                                                                           false,
                                                                           GrMipMapped::kNo);
        sk_sp<SkImage> image;
        int count = 0;
        if (useExternal) {
            // Make a backend texture with an external queue family;
            GrVkImageInfo vkInfo;
            if (!backendTex.getVkImageInfo(&vkInfo)) {
                return;
            }
            vkInfo.fCurrentQueueFamily = VK_QUEUE_FAMILY_EXTERNAL;

            GrBackendTexture vkExtTex(1, 1, vkInfo);
            REPORTER_ASSERT(reporter, vkExtTex.isValid());
            image = SkImage::MakeFromTexture(context, vkExtTex,
                                             kTopLeft_GrSurfaceOrigin,
                                             kRGBA_8888_SkColorType,
                                             kPremul_SkAlphaType,
                                             nullptr, testing_release_proc,
                                             (void*)&count);

        } else {
            image = SkImage::MakeFromTexture(context, backendTex,
                                             kTopLeft_GrSurfaceOrigin,
                                             kRGBA_8888_SkColorType,
                                             kPremul_SkAlphaType,
                                             nullptr, testing_release_proc,
                                             (void*)&count);
        }

        if (!image) {
            continue;
        }

        REPORTER_ASSERT(reporter, !count);

        GrTexture* texture = image->getTexture();
        REPORTER_ASSERT(reporter, texture);
        GrVkTexture* vkTex = static_cast<GrVkTexture*>(texture);

        if (useExternal) {
            // Testing helper so we claim that we don't need to transition from our fake external
            // queue first.
            vkTex->setCurrentQueueFamilyToGraphicsQueue(gpu);
        }

        image.reset();

        // Resetting the image should only trigger the release proc if we are not using an external
        // queue. When using an external queue when we free the SkImage and the underlying
        // GrTexture, we submit a queue transition on the command buffer.
        if (useExternal) {
            REPORTER_ASSERT(reporter, !count);
        } else {
            REPORTER_ASSERT(reporter, count == 1);
        }

        gpu->testingOnly_flushGpuAndSync();

        // Now that we flushed and waited the release proc should have be triggered.
        REPORTER_ASSERT(reporter, count == 1);

        gpu->deleteTestingOnlyBackendTexture(backendTex);
    }
}

#endif
