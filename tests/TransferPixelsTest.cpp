/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a GPU-backend specific test. It relies on static intializers to work

#include "SkTypes.h"

#if SK_SUPPORT_GPU && SK_ALLOW_STATIC_GLOBAL_INITIALIZERS

#include "GrContextFactory.h"
#include "GrContextPriv.h"
#include "GrResourceProvider.h"
#include "GrSurfaceProxy.h"
#include "GrTest.h"
#include "SkGr.h"
#include "SkSurface.h"
#include "Test.h"
#include "vk/GrVkGpu.h"

using sk_gpu_test::GrContextFactory;

void fill_transfer_data(int width, int height, GrColor* data) {

    // build red-green gradient
    for (int j = 0; j < height; ++j) {
        for (int i = 0; i < width; ++i) {
            unsigned int red = (unsigned int)(256.f*(i / (float)width));
            unsigned int green = (unsigned int)(256.f*(j / (float)height));
            data[i + j*width] = GrColorPackRGBA(red - (red>>8), green - (green>>8), 0xff, 0xff);
        }
    }
}

bool does_full_buffer_contain_correct_values(GrColor* srcBuffer,
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

void basic_transfer_test(skiatest::Reporter* reporter, GrContext* context, GrPixelConfig config,
                         bool renderTarget) {
    // set up the data
    const int kWidth = 16;
    const int kHeight = 16;
    SkAutoTMalloc<GrColor> srcBuffer(kWidth*kHeight);
    SkAutoTMalloc<GrColor> dstBuffer(kWidth*kHeight);

    fill_transfer_data(kWidth, kHeight, srcBuffer.get());

    // create and fill transfer buffer
    size_t size = GrBytesPerPixel(config)*kWidth*kHeight;
    uint32_t bufferFlags = GrResourceProvider::kNoPendingIO_Flag;
    GrBuffer* buffer = context->resourceProvider()->createBuffer(size,
                                                                 kXferCpuToGpu_GrBufferType,
                                                                 kDynamic_GrAccessPattern,
                                                                 bufferFlags);
    void* data = buffer->map();
    memcpy(data, srcBuffer.get(), size);
    buffer->unmap();

    // create texture
    GrSurfaceDesc desc;
    desc.fConfig = config;
    desc.fFlags = renderTarget ? kRenderTarget_GrSurfaceFlag : kNone_GrSurfaceFlags;
    desc.fWidth = kWidth;
    desc.fHeight = kHeight;
    desc.fSampleCnt = 0;
    sk_sp<GrSurface> tex = context->resourceProvider()->createTexture(desc, SkBudgeted::kNo);

    // transfer full data
    bool result;
    GrFence fence = 0;
    result = context->getGpu()->transferPixels(tex.get(), 0, 0, kWidth, kHeight, config, buffer,
                                               0, 0, &fence);
    REPORTER_ASSERT(reporter, result);
//    context->getGpu()->waitFence(fence);

    result = context->getGpu()->readPixels(tex.get(), 0, 0, kWidth, kHeight, config,
                                           dstBuffer.get(), 0);
    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, does_full_buffer_contain_correct_values(srcBuffer,
                                                                      dstBuffer,
                                                                      kWidth,
                                                                      kHeight));

    // transfer partial data
    result = context->getGpu()->transferPixels(tex.get(), 2, 10, 10, 2, config, buffer, 0, 0,
                                               &fence);
    REPORTER_ASSERT(reporter, result);
//    context->getGpu()->waitFence(fence);

    memset(dstBuffer, 0, kWidth*kHeight*sizeof(GrColor));

    result = context->getGpu()->readPixels(tex.get(), 0, 0, kWidth, kHeight, config,
                                           dstBuffer.get(), 0);
    REPORTER_ASSERT(reporter, result);

    REPORTER_ASSERT(reporter, does_full_buffer_contain_correct_values(srcBuffer,
                                                                      dstBuffer,
                                                                      10,
                                                                      2));
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(TransferPixelsTest, reporter, ctxInfo) {
    // RGBA
    basic_transfer_test(reporter, ctxInfo.grContext(), kRGBA_8888_GrPixelConfig, false);
    basic_transfer_test(reporter, ctxInfo.grContext(), kRGBA_8888_GrPixelConfig, true);

    // BGRA
    basic_transfer_test(reporter, ctxInfo.grContext(), kBGRA_8888_GrPixelConfig, false);
    basic_transfer_test(reporter, ctxInfo.grContext(), kBGRA_8888_GrPixelConfig, true);
}

#endif
