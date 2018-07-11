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

void testing_only_texture_test(skiatest::Reporter* reporter, GrContext* context, GrColorType ct,
                               bool renderTarget) {
    const int kWidth = 16;
    const int kHeight = 16;
    SkAutoTMalloc<GrColor> srcBuffer(kWidth * kHeight);
    SkAutoTMalloc<GrColor> dstBuffer(kWidth * kHeight);

    fill_pixel_data(kWidth, kHeight, srcBuffer.get());
    GrGpu* gpu = context->contextPriv().getGpu();

    GrPixelConfig config = GrColorTypeToPixelConfig(ct, GrSRGBEncoded::kNo);
    GrBackendTexture backendTex = gpu->createTestingOnlyBackendTexture(srcBuffer,
                                                                       kWidth,
                                                                       kHeight,
                                                                       config,
                                                                       renderTarget,
                                                                       GrMipMapped::kNo);
    sk_sp<GrTexture> wrappedTex;
    if (renderTarget) {
        wrappedTex = gpu->wrapRenderableBackendTexture(backendTex, 1,
                                                       GrWrapOwnership::kAdopt_GrWrapOwnership);
    } else {
        wrappedTex = gpu->wrapBackendTexture(backendTex,
                                             GrWrapOwnership::kAdopt_GrWrapOwnership);
    }
    SkASSERT(wrappedTex);

    int rowBytes = GrColorTypeBytesPerPixel(ct) * kWidth;
    bool result = gpu->readPixels(wrappedTex.get(), 0, 0, kWidth,
                                  kHeight, ct, dstBuffer, rowBytes);

    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, does_full_buffer_contain_correct_color(srcBuffer, dstBuffer,
                                                                     kWidth, kHeight));
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GpuTestTextureUploadTest, reporter, ctxInfo) {
    // RGBA
    testing_only_texture_test(reporter, ctxInfo.grContext(), GrColorType::kRGBA_8888, false);
    testing_only_texture_test(reporter, ctxInfo.grContext(), GrColorType::kRGBA_8888, true);

    // BGRA
    testing_only_texture_test(reporter, ctxInfo.grContext(), GrColorType::kBGRA_8888, false);
    testing_only_texture_test(reporter, ctxInfo.grContext(), GrColorType::kBGRA_8888, true);
}

