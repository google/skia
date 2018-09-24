/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"

#include "GrGpu.h"
#include "GrContextPriv.h"
#include "GrTexture.h"
#include "SkConvertPixels.h"
#include "Test.h"
#include "TestUtils.h"

void testing_only_texture_test(skiatest::Reporter* reporter, GrContext* context, SkColorType ct,
                               bool renderTarget, bool doDataUpload, GrMipMapped mipMapped) {
    const int kWidth = 16;
    const int kHeight = 16;
    SkAutoTMalloc<GrColor> srcBuffer;
    if (doDataUpload) {
        srcBuffer.reset(kWidth * kHeight);
        fill_pixel_data(kWidth, kHeight, srcBuffer.get());
    }
    SkAutoTMalloc<GrColor> dstBuffer(kWidth * kHeight);

    GrGpu* gpu = context->contextPriv().getGpu();

    GrColorType grCT = SkColorTypeToGrColorType(ct);
    GrPixelConfig config = GrColorTypeToPixelConfig(grCT, GrSRGBEncoded::kNo);
    if (!gpu->caps()->isConfigTexturable(config)) {
        return;
    }
    if (gpu->caps()->supportedReadPixelsColorType(config, grCT) != grCT) {
        return;
    }

    GrBackendTexture backendTex = gpu->createTestingOnlyBackendTexture(srcBuffer,
                                                                       kWidth,
                                                                       kHeight,
                                                                       ct,
                                                                       renderTarget,
                                                                       mipMapped, 0);
    if (!backendTex.isValid()) {
        return;
    }

    sk_sp<GrTexture> wrappedTex;
    if (renderTarget) {
        wrappedTex = gpu->wrapRenderableBackendTexture(backendTex, 1,
                                                       GrWrapOwnership::kAdopt_GrWrapOwnership);
    } else {
        wrappedTex = gpu->wrapBackendTexture(backendTex,
                                             GrWrapOwnership::kAdopt_GrWrapOwnership);
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
        for (bool renderable: {true, false}) {
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

