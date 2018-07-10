/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"

#ifdef SK_METAL

#include "GrContextPriv.h"
#include "GrTexture.h"
#include "SkConvertPixels.h"
#include "Test.h"
#include "mtl/MtlTestGpu.h"

void fill_pixel_data_metal(int width, int height, GrColor* data) {
    // build red-green gradient
    for (int j = 0; j < height; ++j) {
        for (int i = 0; i < width; ++i) {
            unsigned int red = (unsigned int)(256.f * (i / (float)width));
            unsigned int green = (unsigned int)(256.f * (j / (float)height));
            data[i + j * width] = GrColorPackRGBA(red - (red >> 8), green - (green >> 8),
                                                  0xff, 0xff);
        }
    }
}

bool does_full_buffer_contain_correct_color_metal(GrColor* srcBuffer,
                                                  GrColor* dstBuffer,
                                                  int width,
                                                  int height) {
    GrColor* srcPtr = srcBuffer;
    GrColor* dstPtr = dstBuffer;
    for (int j = 0; j < height; ++j) {
        for (int i = 0; i < width; ++i) {
            if (srcPtr[i] != dstPtr[i]) {
                return false;
            }
        }
        srcPtr += width;
        dstPtr += width;
    }
    return true;
}

void basic_texture_test_metal(skiatest::Reporter* reporter, GrContext* context, GrColorType ct,
                              bool renderTarget) {
    const int kWidth = 16;
    const int kHeight = 16;
    SkAutoTMalloc<GrColor> srcBuffer(kWidth * kHeight);
    SkAutoTMalloc<GrColor> dstBuffer(kWidth * kHeight);

    fill_pixel_data_metal(kWidth, kHeight, srcBuffer.get());
    MtlTestGpu testGpu(context->contextPriv().getGpu());

    GrPixelConfig config = GrColorTypeToPixelConfig(ct, GrSRGBEncoded::kNo);
    GrBackendTexture backendTex = testGpu.createTestingOnlyBackendTexture(srcBuffer,
                                                                          kWidth,
                                                                          kHeight,
                                                                          config,
                                                                          renderTarget,
                                                                          GrMipMapped::kNo);
    sk_sp<GrTexture> wrappedTex;
    if (renderTarget) {
        wrappedTex = testGpu.wrapRenderableBackendTexture(backendTex, 1,
                                                          GrWrapOwnership::kAdopt_GrWrapOwnership);
    } else {
        wrappedTex = testGpu.wrapBackendTexture(backendTex,
                                                GrWrapOwnership::kAdopt_GrWrapOwnership);
    }
    SkASSERT(wrappedTex);

    int rowBytes = GrColorTypeBytesPerPixel(ct) * kWidth;
    bool result = testGpu.readPixels(wrappedTex.get(), 0, 0, kWidth,
                                     kHeight, ct, dstBuffer, rowBytes);

    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, does_full_buffer_contain_correct_color_metal(srcBuffer,
                                                                           dstBuffer,
                                                                           16,
                                                                           16));
}

DEF_GPUTEST_FOR_METAL_CONTEXT(MtlUploadPixelsTests, reporter, ctxInfo) {
    // RGBA
    basic_texture_test_metal(reporter, ctxInfo.grContext(), GrColorType::kRGBA_8888, false);
    basic_texture_test_metal(reporter, ctxInfo.grContext(), GrColorType::kRGBA_8888, true);

    // BGRA
    basic_texture_test_metal(reporter, ctxInfo.grContext(), GrColorType::kBGRA_8888, false);
    basic_texture_test_metal(reporter, ctxInfo.grContext(), GrColorType::kBGRA_8888, true);
}

#endif
