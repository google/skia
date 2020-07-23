/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"

#if SK_SUPPORT_GPU && defined(SK_VULKAN)

#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "tests/Test.h"
#include "tools/gpu/vk/VkTestHelper.h"
#include "tools/gpu/vk/VkYcbcrSamplerHelper.h"

const size_t kImageWidth = 8;
const size_t kImageHeight = 8;

static int round_and_clamp(float x) {
    int r = static_cast<int>(round(x));
    if (r > 255) return 255;
    if (r < 0) return 0;
    return r;
}

DEF_GPUTEST(VkYCbcrSampler_DrawImageWithYcbcrSampler, reporter, options) {
    VkTestHelper testHelper(false);
    if (!testHelper.init()) {
        ERRORF(reporter, "VkTestHelper initialization failed.");
        return;
    }

    VkYcbcrSamplerHelper ycbcrHelper(testHelper.directContext());
    if (!ycbcrHelper.isYCbCrSupported()) {
        return;
    }

    if (!ycbcrHelper.createBackendTexture(kImageWidth, kImageHeight)) {
        ERRORF(reporter, "Failed to create I420 backend texture");
        return;
    }

    sk_sp<SkImage> srcImage = SkImage::MakeFromTexture(testHelper.directContext(),
                                                       ycbcrHelper.backendTexture(),
                                                       kTopLeft_GrSurfaceOrigin,
                                                       kRGB_888x_SkColorType,
                                                       kPremul_SkAlphaType,
                                                       nullptr);
    if (!srcImage) {
        ERRORF(reporter, "Failed to create I420 image");
        return;
    }

    sk_sp<SkSurface> surface = SkSurface::MakeRenderTarget(
            testHelper.directContext(), SkBudgeted::kNo,
            SkImageInfo::Make(kImageWidth, kImageHeight, kN32_SkColorType, kPremul_SkAlphaType));
    if (!surface) {
        ERRORF(reporter, "Failed to create target SkSurface");
        return;
    }
    surface->getCanvas()->drawImage(srcImage, 0, 0);
    surface->flushAndSubmit();

    std::vector<uint8_t> readbackData(kImageWidth * kImageHeight * 4);
    if (!surface->readPixels(SkImageInfo::Make(kImageWidth, kImageHeight, kRGBA_8888_SkColorType,
                                               kOpaque_SkAlphaType),
                             readbackData.data(), kImageWidth * 4, 0, 0)) {
        ERRORF(reporter, "Readback failed");
        return;
    }

    // Allow resulting color to be off by 1 in each channel as some Vulkan implementations do not
    // round YCbCr sampler result properly.
    const int kColorTolerance = 1;

    // Verify results only for pixels with even coordinates, since others use
    // interpolated U & V channels.
    for (size_t y = 0; y < kImageHeight; y += 2) {
        for (size_t x = 0; x < kImageWidth; x += 2) {
            auto y2 = VkYcbcrSamplerHelper::GetExpectedY(x, y, kImageWidth, kImageHeight);
            auto [u, v] = VkYcbcrSamplerHelper::GetExpectedUV(x, y, kImageWidth, kImageHeight);

            // createI420Image() initializes the image with VK_SAMPLER_YCBCR_RANGE_ITU_NARROW.
            float yChannel = (static_cast<float>(y2) - 16.0) / 219.0;
            float uChannel = (static_cast<float>(u) - 128.0) / 224.0;
            float vChannel = (static_cast<float>(v) - 128.0) / 224.0;

            // BR.709 conversion as specified in
            // https://www.khronos.org/registry/DataFormat/specs/1.2/dataformat.1.2.html#MODEL_YUV
            int expectedR = round_and_clamp((yChannel + 1.5748f * vChannel) * 255.0);
            int expectedG = round_and_clamp((yChannel - 0.13397432f / 0.7152f * uChannel -
                                             0.33480248f / 0.7152f * vChannel) *
                                            255.0);
            int expectedB = round_and_clamp((yChannel + 1.8556f * uChannel) * 255.0);

            int r = readbackData[(y * kImageWidth + x) * 4];
            if (abs(r - expectedR) > kColorTolerance) {
                ERRORF(reporter, "R should be %d, but is %d at (%d, %d)", expectedR, r, x, y);
            }

            int g = readbackData[(y * kImageWidth + x) * 4 + 1];
            if (abs(g - expectedG) > kColorTolerance) {
                ERRORF(reporter, "G should be %d, but is %d at (%d, %d)", expectedG, g, x, y);
            }

            int b = readbackData[(y * kImageWidth + x) * 4 + 2];
            if (abs(b - expectedB) > kColorTolerance) {
                ERRORF(reporter, "B should be %d, but is %d at (%d, %d)", expectedB, b, x, y);
            }
        }
    }
}

// Verifies that it's not possible to allocate Ycbcr texture directly.
DEF_GPUTEST(VkYCbcrSampler_NoYcbcrSurface, reporter, options) {
    VkTestHelper testHelper(false);
    if (!testHelper.init()) {
        ERRORF(reporter, "VkTestHelper initialization failed.");
        return;
    }

    VkYcbcrSamplerHelper ycbcrHelper(testHelper.directContext());
    if (!ycbcrHelper.isYCbCrSupported()) {
        return;
    }

    GrBackendTexture texture = testHelper.directContext()->createBackendTexture(
            kImageWidth, kImageHeight, GrBackendFormat::MakeVk(VK_FORMAT_G8_B8R8_2PLANE_420_UNORM),
            GrMipmapped::kNo, GrRenderable::kNo, GrProtected::kNo);
    if (texture.isValid()) {
        ERRORF(reporter,
               "GrDirectContext::createBackendTexture() didn't fail as expected for Ycbcr format.");
    }
}

#endif  // SK_SUPPORT_GPU && defined(SK_VULKAN)
