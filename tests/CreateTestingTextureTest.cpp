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

void fill_gradient_data(int width, int height, GrColor* data) {
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

bool do_buffers_match(GrColor* srcBuffer, GrColor* dstBuffer,  int width, int height) {
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

void testing_only_texture_test(skiatest::Reporter* reporter, GrContext* context, GrColorType ct,
                               bool renderTarget) {
    const int kWidth = 16;
    const int kHeight = 16;
    SkAutoTMalloc<GrColor> srcBuffer(kWidth * kHeight);
    SkAutoTMalloc<GrColor> dstBuffer(kWidth * kHeight);

    fill_gradient_data(kWidth, kHeight, srcBuffer.get());
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
    REPORTER_ASSERT(reporter, do_buffers_match(srcBuffer, dstBuffer, kWidth, kHeight));
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(CreateTestingTextureTest, reporter, ctxInfo) {
    // RGBA
    testing_only_texture_test(reporter, ctxInfo.grContext(), GrColorType::kRGBA_8888, false);
    testing_only_texture_test(reporter, ctxInfo.grContext(), GrColorType::kRGBA_8888, true);

    // BGRA
    testing_only_texture_test(reporter, ctxInfo.grContext(), GrColorType::kBGRA_8888, false);
    testing_only_texture_test(reporter, ctxInfo.grContext(), GrColorType::kBGRA_8888, true);
}

