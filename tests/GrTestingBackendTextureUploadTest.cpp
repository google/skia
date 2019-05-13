/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"

#include "include/gpu/GrTexture.h"
#include "src/core/SkConvertPixels.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/SkGr.h"
#include "tests/Test.h"
#include "tests/TestUtils.h"

void testing_only_texture_test(skiatest::Reporter* reporter, GrContext* context, SkColorType ct,
                               GrRenderable renderable, bool doDataUpload, GrMipMapped mipMapped) {

    const int kWidth = 16;
    const int kHeight = 16;
    SkAutoTMalloc<GrColor> srcBuffer;
    if (doDataUpload) {
        srcBuffer.reset(kWidth * kHeight);
        fill_pixel_data(kWidth, kHeight, srcBuffer.get());
    }
    SkAutoTMalloc<GrColor> dstBuffer(kWidth * kHeight);

    GrGpu* gpu = context->priv().getGpu();

    GrPixelConfig config = SkColorType2GrPixelConfig(ct);
    if (!gpu->caps()->isConfigTexturable(config)) {
        return;
    }

    GrColorType grCT = SkColorTypeToGrColorType(ct);
    if (GrColorType::kUnknown == grCT) {
        return;
    }

    if (gpu->caps()->supportedReadPixelsColorType(config, grCT) != grCT) {
        return;
    }

    GrBackendTexture backendTex = gpu->createTestingOnlyBackendTexture(
                                        kWidth, kHeight, ct,
                                        mipMapped, renderable, srcBuffer);
    if (!backendTex.isValid()) {
        return;
    }

    sk_sp<GrTexture> wrappedTex;
    if (GrRenderable::kYes == renderable) {
        wrappedTex = gpu->wrapRenderableBackendTexture(
                backendTex, 1, GrWrapOwnership::kAdopt_GrWrapOwnership, GrWrapCacheable::kNo);
    } else {
        wrappedTex = gpu->wrapBackendTexture(backendTex, GrWrapOwnership::kAdopt_GrWrapOwnership,
                                             GrWrapCacheable::kNo, kRead_GrIOType);
    }
    REPORTER_ASSERT(reporter, wrappedTex);

    int rowBytes = GrColorTypeBytesPerPixel(grCT) * kWidth;
    bool result = gpu->readPixels(wrappedTex.get(), 0, 0, kWidth,
                                  kHeight, grCT, dstBuffer, rowBytes);

    if (!doDataUpload) {
        // createTestingOnlyBackendTexture will fill the texture with 0's if no data is provided, so
        // we set the expected result likewise.
        srcBuffer.reset(kWidth * kHeight);
        memset(srcBuffer, 0, kWidth * kHeight * sizeof(GrColor));
    }
    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, does_full_buffer_contain_correct_color(srcBuffer, dstBuffer,
                                                                     kWidth, kHeight));
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrTestingBackendTextureUploadTest, reporter, ctxInfo) {
    for (auto colorType: { kRGBA_8888_SkColorType, kBGRA_8888_SkColorType }) {
        for (auto renderable: { GrRenderable::kYes, GrRenderable::kNo }) {
            for (bool doDataUpload: {true, false}) {
                testing_only_texture_test(reporter, ctxInfo.grContext(), colorType,
                                          renderable, doDataUpload, GrMipMapped::kNo);

                if (!doDataUpload) {
                    testing_only_texture_test(reporter, ctxInfo.grContext(), colorType,
                                              renderable, doDataUpload, GrMipMapped::kYes);
                }
            }
        }
    }
}

