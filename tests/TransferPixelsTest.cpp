/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a GPU-backend specific test. It relies on static intializers to work

#include "SkTypes.h"

#if SK_SUPPORT_GPU

#include "GrContextFactory.h"
#include "GrContextPriv.h"
#include "GrGpu.h"
#include "GrResourceProvider.h"
#include "GrSurfaceProxy.h"
#include "GrTexture.h"
#include "GrTest.h"
#include "SkGr.h"
#include "SkSurface.h"
#include "Test.h"

using sk_gpu_test::GrContextFactory;

void fill_transfer_data(int left, int top, int width, int height, int bufferWidth,
                        GrColor* data) {

    // build red-green gradient
    for (int j = top; j < top + height; ++j) {
        for (int i = left; i < left + width; ++i) {
            unsigned int red = (unsigned int)(256.f*((i - left) / (float)width));
            unsigned int green = (unsigned int)(256.f*((j - top) / (float)height));
            data[i + j*bufferWidth] = GrColorPackRGBA(red - (red>>8),
                                                      green - (green>>8), 0xff, 0xff);
        }
    }
}

bool does_full_buffer_contain_correct_values(GrColor* srcBuffer,
                                             GrColor* dstBuffer,
                                             int width,
                                             int height,
                                             int bufferWidth,
                                             int bufferHeight,
                                             GrSurfaceOrigin origin) {
    GrColor* srcPtr = srcBuffer;
    bool bottomUp = SkToBool(kBottomLeft_GrSurfaceOrigin == origin);
    GrColor* dstPtr = bottomUp ? dstBuffer + bufferWidth*(bufferHeight-1) : dstBuffer;
    int dstIncrement = bottomUp ? -bufferWidth : +bufferWidth;

    for (int j = 0; j < height; ++j) {
        for (int i = 0; i < width; ++i) {
            if (srcPtr[i] != dstPtr[i]) {
                return false;
            }
        }
        srcPtr += bufferWidth;
        dstPtr += dstIncrement;
    }
    return true;
}

void basic_transfer_test(skiatest::Reporter* reporter, GrContext* context, GrPixelConfig config,
                         GrSurfaceOrigin origin, bool renderTarget) {
    // set up the data
    const int kTextureWidth = 16;
    const int kTextureHeight = 16;
    const int kBufferWidth = 20;
    const int kBufferHeight = 16;
    size_t rowBytes = kBufferWidth * sizeof(GrColor);
    SkAutoTMalloc<GrColor> srcBuffer(kBufferWidth*kBufferHeight);
    SkAutoTMalloc<GrColor> dstBuffer(kBufferWidth*kBufferHeight);

    fill_transfer_data(0, 0, kTextureWidth, kTextureHeight, kBufferWidth, srcBuffer.get());

    // create and fill transfer buffer
    size_t size = rowBytes*kBufferHeight;
    uint32_t bufferFlags = GrResourceProvider::kNoPendingIO_Flag;
    sk_sp<GrBuffer> buffer(context->resourceProvider()->createBuffer(size,
                                                                     kXferCpuToGpu_GrBufferType,
                                                                     kDynamic_GrAccessPattern,
                                                                     bufferFlags));
    if (!buffer) {
        return;
    }

    void* data = buffer->map();
    memcpy(data, srcBuffer.get(), size);
    buffer->unmap();

    // create texture
    GrSurfaceDesc desc;
    desc.fConfig = config;
    desc.fFlags = renderTarget ? kRenderTarget_GrSurfaceFlag : kNone_GrSurfaceFlags;
    desc.fOrigin = origin;
    desc.fWidth = kTextureWidth;
    desc.fHeight = kTextureHeight;
    desc.fSampleCnt = 0;
    sk_sp<GrTexture> tex = context->resourceProvider()->createTexture(desc, SkBudgeted::kNo);

    //////////////////////////
    // transfer full data

    bool result;
    result = context->getGpu()->transferPixels(tex.get(), 0, 0, kTextureWidth, kTextureHeight,
                                               config, buffer.get(), 0, rowBytes);
    REPORTER_ASSERT(reporter, result);

    memset(dstBuffer.get(), 0xCDCD, size);
    result = context->getGpu()->readPixels(tex.get(), 0, 0, kTextureWidth, kTextureHeight, config,
                                           dstBuffer.get(), rowBytes);
    if (result) {
        REPORTER_ASSERT(reporter, does_full_buffer_contain_correct_values(srcBuffer,
                                                                          dstBuffer,
                                                                          kTextureWidth,
                                                                          kTextureHeight,
                                                                          kBufferWidth,
                                                                          kBufferHeight,
                                                                          origin));
    }
    
    //////////////////////////
    // transfer partial data

    const int kLeft = 2;
    const int kTop = 10;
    const int kWidth = 10;
    const int kHeight = 2;

    // change color of subrectangle
    fill_transfer_data(kLeft, kTop, kWidth, kHeight, kBufferWidth, srcBuffer.get());
    data = buffer->map();
    memcpy(data, srcBuffer.get(), size);
    buffer->unmap();

    size_t offset = sizeof(GrColor)*(kTop*kBufferWidth + kLeft);
    result = context->getGpu()->transferPixels(tex.get(), kLeft, kTop, kWidth, kHeight, config,
                                               buffer.get(), offset, rowBytes);
    REPORTER_ASSERT(reporter, result);

    memset(dstBuffer.get(), 0xCDCD, size);
    result = context->getGpu()->readPixels(tex.get(), 0, 0, kTextureWidth, kTextureHeight, config,
                                           dstBuffer.get(), rowBytes);
    if (result) {
        REPORTER_ASSERT(reporter, does_full_buffer_contain_correct_values(srcBuffer,
                                                                          dstBuffer,
                                                                          kTextureWidth,
                                                                          kTextureHeight,
                                                                          kBufferWidth,
                                                                          kBufferHeight,
                                                                          origin));
    }
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(TransferPixelsTest, reporter, ctxInfo) {
    // RGBA
    basic_transfer_test(reporter, ctxInfo.grContext(), kRGBA_8888_GrPixelConfig,
                        kTopLeft_GrSurfaceOrigin, false);
    basic_transfer_test(reporter, ctxInfo.grContext(), kRGBA_8888_GrPixelConfig,
                        kTopLeft_GrSurfaceOrigin, true);
    basic_transfer_test(reporter, ctxInfo.grContext(), kRGBA_8888_GrPixelConfig,
                        kBottomLeft_GrSurfaceOrigin, false);
    basic_transfer_test(reporter, ctxInfo.grContext(), kRGBA_8888_GrPixelConfig,
                        kBottomLeft_GrSurfaceOrigin, true);

    // BGRA
    basic_transfer_test(reporter, ctxInfo.grContext(), kBGRA_8888_GrPixelConfig,
                        kTopLeft_GrSurfaceOrigin, false);
    basic_transfer_test(reporter, ctxInfo.grContext(), kBGRA_8888_GrPixelConfig,
                        kTopLeft_GrSurfaceOrigin, true);
    basic_transfer_test(reporter, ctxInfo.grContext(), kBGRA_8888_GrPixelConfig,
                        kBottomLeft_GrSurfaceOrigin, false);
    basic_transfer_test(reporter, ctxInfo.grContext(), kBGRA_8888_GrPixelConfig,
                        kBottomLeft_GrSurfaceOrigin, true);
}

#endif
