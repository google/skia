/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a GPU-backend specific test. It relies on static initializers to work

#include "include/core/SkTypes.h"

#if defined(SK_VULKAN)
#include "include/core/SkAlphaType.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImage.h"
#include "include/core/SkRefCnt.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrTypes.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "include/gpu/ganesh/vk/GrVkBackendSurface.h"
#include "include/gpu/vk/GrVkTypes.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrSurface.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/gpu/ganesh/GrTextureProxy.h"
#include "src/gpu/ganesh/image/GrImageUtils.h"
#include "src/gpu/ganesh/vk/GrVkCaps.h"
#include "src/gpu/ganesh/vk/GrVkImage.h"
#include "src/gpu/ganesh/vk/GrVkTexture.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"
#include "tools/gpu/ManagedBackendTexture.h"
#include "tools/gpu/ProxyUtils.h"

#include <vulkan/vulkan_core.h>

class GrTexture;
struct GrContextOptions;

DEF_GANESH_TEST_FOR_VULKAN_CONTEXT(VkDRMModifierTest, reporter, ctxInfo, CtsEnforcement::kNever) {
    using namespace skgpu;

    auto dContext = ctxInfo.directContext();

    const GrVkCaps* vkCaps = static_cast<const GrVkCaps*>(dContext->priv().caps());
    if (!vkCaps->supportsDRMFormatModifiers()) {
        return;
    }

    Protected isProtected = Protected(vkCaps->supportsProtectedContent());

    // First make a normal backend texture with DRM
    auto mbet = sk_gpu_test::ManagedBackendTexture::MakeWithoutData(
            dContext, 1, 1, kRGBA_8888_SkColorType, Mipmapped::kNo, GrRenderable::kNo, isProtected);
    if (!mbet) {
        ERRORF(reporter, "Could not create backend texture.");
        return;
    }

    GrVkImageInfo info;
    REPORTER_ASSERT(reporter, GrBackendTextures::GetVkImageInfo(mbet->texture(), &info));

    // Next we will use the same VkImageInfo but lie to say tiling is a DRM modifier. This should
    // cause us to think the resource is eternal/read only internally. Though since we don't
    // explicitly pass in the tiling to anything, this shouldn't cause us to do anything illegal.
    info.fImageTiling = VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT;

    GrBackendTexture drmBETex = GrBackendTextures::MakeVk(1, 1, info);
    GrBackendFormat drmFormat = GrBackendFormats::MakeVk(info.fFormat, true);
    REPORTER_ASSERT(reporter, drmFormat == drmBETex.getBackendFormat());
    REPORTER_ASSERT(reporter, drmBETex.textureType() == GrTextureType::kExternal);

    // Now wrap the texture in an SkImage and make sure we have the required read only properties
    sk_sp<SkImage> drmImage = SkImages::BorrowTextureFrom(dContext,
                                                          drmBETex,
                                                          kTopLeft_GrSurfaceOrigin,
                                                          kRGBA_8888_SkColorType,
                                                          kPremul_SkAlphaType,
                                                          nullptr);
    REPORTER_ASSERT(reporter, drmImage);

    GrBackendTexture actual;
    bool ok = SkImages::GetBackendTextureFromImage(drmImage, &actual, false);
    REPORTER_ASSERT(reporter, ok);
    REPORTER_ASSERT(reporter, GrBackendTexture::TestingOnly_Equals(actual, drmBETex));

    auto [view, _] = skgpu::ganesh::AsView(dContext, drmImage, Mipmapped::kNo);
    REPORTER_ASSERT(reporter, view);
    const GrSurfaceProxy* proxy = view.proxy();
    REPORTER_ASSERT(reporter, proxy);

    REPORTER_ASSERT(reporter, proxy->readOnly());

    const GrSurface* surf = proxy->peekSurface();
    REPORTER_ASSERT(reporter, surf);
    REPORTER_ASSERT(reporter, surf->readOnly());

    drmImage.reset();
}

DEF_GANESH_TEST_FOR_VULKAN_CONTEXT(VkImageLayoutTest, reporter, ctxInfo, CtsEnforcement::kNever) {
    using namespace skgpu;

    auto dContext = ctxInfo.directContext();

    Protected isProtected = Protected(dContext->priv().caps()->supportsProtectedContent());

    auto mbet = sk_gpu_test::ManagedBackendTexture::MakeWithoutData(
            dContext, 1, 1, kRGBA_8888_SkColorType, Mipmapped::kNo, GrRenderable::kNo, isProtected);
    if (!mbet) {
        ERRORF(reporter, "Could not create backend texture.");
        return;
    }

    GrVkImageInfo info;
    REPORTER_ASSERT(reporter, GrBackendTextures::GetVkImageInfo(mbet->texture(), &info));
    VkImageLayout initLayout = info.fImageLayout;

    // Verify that setting that layout via a copy of a backendTexture is reflected in all the
    // backendTextures.
    GrBackendTexture backendTex1 = mbet->texture();
    GrBackendTexture backendTex2 = backendTex1;
    REPORTER_ASSERT(reporter, GrBackendTextures::GetVkImageInfo(backendTex2, &info));
    REPORTER_ASSERT(reporter, initLayout == info.fImageLayout);

    GrBackendTextures::SetVkImageLayout(&backendTex2, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    REPORTER_ASSERT(reporter, GrBackendTextures::GetVkImageInfo(backendTex1, &info));
    REPORTER_ASSERT(reporter, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL == info.fImageLayout);

    REPORTER_ASSERT(reporter, GrBackendTextures::GetVkImageInfo(backendTex2, &info));
    REPORTER_ASSERT(reporter, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL == info.fImageLayout);

    // Setting back the layout since we didn't actually change it
    GrBackendTextures::SetVkImageLayout(&backendTex1, initLayout);

    sk_sp<SkImage> wrappedImage =
            SkImages::BorrowTextureFrom(dContext,
                                        backendTex1,
                                        kTopLeft_GrSurfaceOrigin,
                                        kRGBA_8888_SkColorType,
                                        kPremul_SkAlphaType,
                                        /*color space*/ nullptr,
                                        sk_gpu_test::ManagedBackendTexture::ReleaseProc,
                                        mbet->releaseContext());
    REPORTER_ASSERT(reporter, wrappedImage.get());

    GrSurfaceProxy* proxy = sk_gpu_test::GetTextureImageProxy(wrappedImage.get(), dContext);
    REPORTER_ASSERT(reporter, proxy);
    REPORTER_ASSERT(reporter, proxy->isInstantiated());
    GrTexture* texture = proxy->peekTexture();
    REPORTER_ASSERT(reporter, texture);

    // Verify that modifying the layout via the GrVkTexture is reflected in the GrBackendTexture
    GrVkImage* vkTexture = static_cast<GrVkTexture*>(texture)->textureImage();
    REPORTER_ASSERT(reporter, initLayout == vkTexture->currentLayout());
    vkTexture->updateImageLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    REPORTER_ASSERT(reporter, GrBackendTextures::GetVkImageInfo(backendTex1, &info));
    REPORTER_ASSERT(reporter, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL == info.fImageLayout);

    GrBackendTexture backendTexImage;
    bool ok = SkImages::GetBackendTextureFromImage(wrappedImage, &backendTexImage, false);
    REPORTER_ASSERT(reporter, ok);
    REPORTER_ASSERT(reporter, GrBackendTextures::GetVkImageInfo(backendTexImage, &info));
    REPORTER_ASSERT(reporter, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL == info.fImageLayout);

    // Verify that modifying the layout via the GrBackendTexutre is reflected in the GrVkTexture
    GrBackendTextures::SetVkImageLayout(&backendTexImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    REPORTER_ASSERT(reporter, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL == vkTexture->currentLayout());

    vkTexture->updateImageLayout(initLayout);

    REPORTER_ASSERT(reporter, GrBackendTextures::GetVkImageInfo(backendTex1, &info));
    REPORTER_ASSERT(reporter, initLayout == info.fImageLayout);

    REPORTER_ASSERT(reporter, GrBackendTextures::GetVkImageInfo(backendTex2, &info));
    REPORTER_ASSERT(reporter, initLayout == info.fImageLayout);

    REPORTER_ASSERT(reporter, GrBackendTextures::GetVkImageInfo(backendTexImage, &info));
    REPORTER_ASSERT(reporter, initLayout == info.fImageLayout);

    // Check that we can do things like assigning the backend texture to invalid one, assign an
    // invalid one, assin a backend texture to inself etc. Success here is that we don't hit any of
    // our ref counting asserts.
    REPORTER_ASSERT(reporter, GrBackendTexture::TestingOnly_Equals(backendTex1, backendTex2));

    GrBackendTexture invalidTexture;
    REPORTER_ASSERT(reporter, !invalidTexture.isValid());
    REPORTER_ASSERT(reporter, !GrBackendTexture::TestingOnly_Equals(invalidTexture, backendTex2));

    backendTex2 = invalidTexture;
    REPORTER_ASSERT(reporter, !backendTex2.isValid());
    REPORTER_ASSERT(reporter, !GrBackendTexture::TestingOnly_Equals(invalidTexture, backendTex2));

    invalidTexture = backendTex1;
    REPORTER_ASSERT(reporter, invalidTexture.isValid());
    REPORTER_ASSERT(reporter, GrBackendTexture::TestingOnly_Equals(invalidTexture, backendTex1));

    invalidTexture = static_cast<decltype(invalidTexture)&>(invalidTexture);
    REPORTER_ASSERT(reporter, invalidTexture.isValid());
    REPORTER_ASSERT(reporter, GrBackendTexture::TestingOnly_Equals(invalidTexture, invalidTexture));
}

// This test is disabled because it executes illegal vulkan calls which cause the validations layers
// to fail and makes us assert. Once fixed to use a valid vulkan call sequence it should be
// renenabled, see skbug.com/8936.
#if 0
// Test to make sure we transition from the EXTERNAL queue even when no layout transition is needed.
DEF_GANESH_TEST_FOR_VULKAN_CONTEXT(VkTransitionExternalQueueTest, reporter, ctxInfo,
                                   CtsEnforcement::kApiLevel_T) {
    auto dContext = ctxInfo.directContext();
    GrGpu* gpu = dContext->priv().getGpu();
    GrVkGpu* vkGpu = static_cast<GrVkGpu*>(gpu);
    if (!vkGpu->vkCaps().supportsExternalMemory()) {
        return;
    }

    GrBackendTexture backendTex = dContext->createBackendTexture(
            1, 1, kRGBA_8888_SkColorType,
            SkColors::kTransparent, skgpu::Mipmapped::kNo, GrRenderable::kNo);
    sk_sp<SkImage> image;
    // Make a backend texture with an external queue family and general layout.
    GrVkImageInfo vkInfo;
    if (!GrBackendTextures::GetVkImageInfo(backendTex, &vkInfo)) {
        return;
    }
    vkInfo.fCurrentQueueFamily = VK_QUEUE_FAMILY_EXTERNAL;
    // Use a read-only layout as these are the ones where we can otherwise skip a transition.
    vkInfo.fImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    GrBackendTexture vkExtTex(1, 1, vkInfo);
    REPORTER_ASSERT(reporter, vkExtTex.isValid());
    image = SkImages::BorrowTextureFrom(dContext, vkExtTex, kTopLeft_GrSurfaceOrigin,
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
    REPORTER_ASSERT(reporter, GrBackendTextures::GetVkImageInfo(newBackendTexture, &newVkInfo));
    REPORTER_ASSERT(reporter, newVkInfo.fCurrentQueueFamily == vkGpu->queueIndex());

    image.reset();
    dContext->submit(GrSyncCpu::kYes);
    dContext->deleteBackendTexture(backendTex);
}
#endif

#endif
