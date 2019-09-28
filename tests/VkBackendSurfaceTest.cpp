/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a GPU-backend specific test. It relies on static intializers to work

#include "include/core/SkTypes.h"

#if defined(SK_VULKAN)

#include "include/gpu/vk/GrVkVulkan.h"

#include "tests/Test.h"

#include "include/core/SkImage.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrTexture.h"
#include "include/gpu/vk/GrVkTypes.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrTextureProxy.h"
#include "src/gpu/SkGpuDevice.h"
#include "src/gpu/vk/GrVkGpu.h"
#include "src/gpu/vk/GrVkImageLayout.h"
#include "src/gpu/vk/GrVkTexture.h"
#include "src/image/SkImage_Base.h"
#include "src/image/SkSurface_Gpu.h"

DEF_GPUTEST_FOR_VULKAN_CONTEXT(VkImageLayoutTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();

    GrBackendTexture backendTex = context->createBackendTexture(1, 1,
                                                                kRGBA_8888_SkColorType,
                                                                SkColors::kTransparent,
                                                                GrMipMapped::kNo,
                                                                GrRenderable::kNo,
                                                                GrProtected::kNo);
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

    sk_sp<GrTextureProxy> texProxy = as_IB(wrappedImage)->asTextureProxyRef(context);
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

    context->deleteBackendTexture(backendTex);
}

static void testing_release_proc(void* ctx) {
    int* count = (int*)ctx;
    *count += 1;
}

// Test to make sure we don't call our release proc on an image until we've transferred it back to
// its original queue family.
DEF_GPUTEST_FOR_VULKAN_CONTEXT(VkReleaseExternalQueueTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    GrGpu* gpu = context->priv().getGpu();
    GrVkGpu* vkGpu = static_cast<GrVkGpu*>(gpu);
    if (!vkGpu->vkCaps().supportsExternalMemory()) {
        return;
    }

    for (bool useExternal : {false, true}) {
        GrBackendTexture backendTex = context->createBackendTexture(1, 1,
                                                                    kRGBA_8888_SkColorType,
                                                                    SkColors::kTransparent,
                                                                    GrMipMapped::kNo,
                                                                    GrRenderable::kNo,
                                                                    GrProtected::kNo);
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
            vkTex->setCurrentQueueFamilyToGraphicsQueue(vkGpu);
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

        context->deleteBackendTexture(backendTex);
    }
}

// Test to make sure we transition to the original queue when requests for prepareforexternalio are
// in flush calls
DEF_GPUTEST_FOR_VULKAN_CONTEXT(VkPrepareForExternalIOQueueTransitionTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();

    GrVkGpu* vkGpu = static_cast<GrVkGpu*>(context->priv().getGpu());
    if (!vkGpu->vkCaps().supportsExternalMemory()) {
        return;
    }

    for (bool useSurface : {false, true}) {
        for (bool preparePresent : {false, true}) {
            if (!useSurface && preparePresent) {
                // We don't set textures to present
                continue;
            }
            GrBackendTexture backendTex = context->createBackendTexture(
                    4, 4, kRGBA_8888_SkColorType,
                    SkColors::kTransparent, GrMipMapped::kNo,
                    useSurface ? GrRenderable::kYes : GrRenderable::kNo,
                    GrProtected::kNo);

            // Make a backend texture with an external queue family and general layout.
            GrVkImageInfo vkInfo;
            if (!backendTex.getVkImageInfo(&vkInfo)) {
                return;
            }

            // We can't actually make an external texture in our test. However, we lie and say it is
            // and then will manually go and swap the queue to the graphics queue once we wrap it.
            if (preparePresent) {
                // We don't transition to present to things that are going to external for foreign
                // queues.
                vkInfo.fCurrentQueueFamily = vkGpu->queueIndex();
            } else {
                vkInfo.fCurrentQueueFamily = VK_QUEUE_FAMILY_EXTERNAL;
            }

            GrBackendTexture vkExtTex(1, 1, vkInfo);

            sk_sp<SkImage> image;
            sk_sp<SkSurface> surface;
            GrTexture* texture;
            if (useSurface) {
                surface = SkSurface::MakeFromBackendTexture(context, vkExtTex,
                        kTopLeft_GrSurfaceOrigin, 0, kRGBA_8888_SkColorType, nullptr, nullptr);
                REPORTER_ASSERT(reporter, surface.get());
                if (!surface) {
                    continue;
                }
                SkSurface_Gpu* gpuSurface = static_cast<SkSurface_Gpu*>(surface.get());
                auto* rtc = gpuSurface->getDevice()->accessRenderTargetContext();
                texture = rtc->asTextureProxy()->peekTexture();
            } else {
                image = SkImage::MakeFromTexture(context, vkExtTex, kTopLeft_GrSurfaceOrigin,
                        kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr, nullptr, nullptr);

                REPORTER_ASSERT(reporter, image.get());
                if (!image) {
                    continue;
                }

                texture = image->getTexture();
            }

            REPORTER_ASSERT(reporter, texture);
            GrVkTexture* vkTex = static_cast<GrVkTexture*>(texture);

            // Testing helper so we claim that we don't need to transition from our fake external
            // queue first.
            vkTex->setCurrentQueueFamilyToGraphicsQueue(vkGpu);

            GrBackendTexture newBackendTexture;
            if (useSurface) {
                newBackendTexture = surface->getBackendTexture(
                        SkSurface::kFlushRead_TextureHandleAccess);
            } else {
                newBackendTexture = image->getBackendTexture(false);
            }
            GrVkImageInfo newVkInfo;
            REPORTER_ASSERT(reporter, newBackendTexture.getVkImageInfo(&newVkInfo));
            REPORTER_ASSERT(reporter, newVkInfo.fCurrentQueueFamily == vkGpu->queueIndex());
            VkImageLayout oldLayout = newVkInfo.fImageLayout;

            GrPrepareForExternalIORequests externalRequests;
            SkImage* imagePtr;
            SkSurface* surfacePtr;
            if (useSurface) {
                externalRequests.fNumSurfaces = 1;
                surfacePtr = surface.get();
                externalRequests.fSurfaces = &surfacePtr;
                externalRequests.fPrepareSurfaceForPresent = &preparePresent;
            } else {
                externalRequests.fNumImages = 1;
                imagePtr = image.get();
                externalRequests.fImages = &imagePtr;

            }
            context->flush(GrFlushInfo(), externalRequests);

            if (useSurface) {
                newBackendTexture = surface->getBackendTexture(
                        SkSurface::kFlushRead_TextureHandleAccess);
            } else {
                newBackendTexture = image->getBackendTexture(false);
            }
            REPORTER_ASSERT(reporter, newBackendTexture.getVkImageInfo(&newVkInfo));
            if (preparePresent) {
                REPORTER_ASSERT(reporter, newVkInfo.fCurrentQueueFamily == vkGpu->queueIndex());
                REPORTER_ASSERT(reporter,
                                newVkInfo.fImageLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
            } else {
                REPORTER_ASSERT(reporter, newVkInfo.fCurrentQueueFamily == VK_QUEUE_FAMILY_EXTERNAL);
                REPORTER_ASSERT(reporter, newVkInfo.fImageLayout == oldLayout);
            }

            GrFlushInfo flushInfo;
            flushInfo.fFlags = kSyncCpu_GrFlushFlag;
            context->flush(flushInfo);
            context->deleteBackendTexture(backendTex);
        }
    }
}

// This test is disabled because it executes illegal vulkan calls which cause the validations layers
// to fail and makes us assert. Once fixed to use a valid vulkan call sequence it should be
// renenabled, see skbug.com/8936.
#if 0
// Test to make sure we transition from the EXTERNAL queue even when no layout transition is needed.
DEF_GPUTEST_FOR_VULKAN_CONTEXT(VkTransitionExternalQueueTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    GrGpu* gpu = context->priv().getGpu();
    GrVkGpu* vkGpu = static_cast<GrVkGpu*>(gpu);
    if (!vkGpu->vkCaps().supportsExternalMemory()) {
        return;
    }

    GrBackendTexture backendTex = context->createBackendTexture(
            1, 1, kRGBA_8888_SkColorType,
            SkColors::kTransparent, GrMipMapped::kNo, GrRenderable::kNo);
    sk_sp<SkImage> image;
    // Make a backend texture with an external queue family and general layout.
    GrVkImageInfo vkInfo;
    if (!backendTex.getVkImageInfo(&vkInfo)) {
        return;
    }
    vkInfo.fCurrentQueueFamily = VK_QUEUE_FAMILY_EXTERNAL;
    // Use a read-only layout as these are the ones where we can otherwise skip a transition.
    vkInfo.fImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    GrBackendTexture vkExtTex(1, 1, vkInfo);
    REPORTER_ASSERT(reporter, vkExtTex.isValid());
    image = SkImage::MakeFromTexture(context, vkExtTex, kTopLeft_GrSurfaceOrigin,
                                     kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr, nullptr,
                                     nullptr);

    if (!image) {
        return;
    }

    GrTexture* texture = image->getTexture();
    REPORTER_ASSERT(reporter, texture);
    GrVkTexture* vkTex = static_cast<GrVkTexture*>(texture);

    // Change our backend texture to the internal queue, with the same layout. This should force a
    // queue transition even though the layouts match.
    vkTex->setImageLayout(vkGpu, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0,
                          VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, false, false);

    // Get our image info again and make sure we transitioned queues.
    GrBackendTexture newBackendTexture = image->getBackendTexture(true);
    GrVkImageInfo newVkInfo;
    REPORTER_ASSERT(reporter, newBackendTexture.getVkImageInfo(&newVkInfo));
    REPORTER_ASSERT(reporter, newVkInfo.fCurrentQueueFamily == vkGpu->queueIndex());

    image.reset();
    gpu->testingOnly_flushGpuAndSync();
    context->deleteBackendTexture(backendTex);
}
#endif

#endif
