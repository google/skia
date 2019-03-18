/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a GPU-backend specific test. It relies on static intializers to work

#include "SkTypes.h"

#include "GrContextFactory.h"
#include "GrContextPriv.h"
#include "GrGpu.h"
#include "GrResourceProvider.h"
#include "GrSurfaceProxy.h"
#include "GrTexture.h"
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
                                             int bufferHeight) {
    GrColor* srcPtr = srcBuffer;
    GrColor* dstPtr = dstBuffer;

    for (int j = 0; j < height; ++j) {
        for (int i = 0; i < width; ++i) {
            if (srcPtr[i] != dstPtr[i]) {
                return false;
            }
        }
        srcPtr += bufferWidth;
        dstPtr += bufferWidth;
    }
    return true;
}

void basic_transfer_test(skiatest::Reporter* reporter, GrContext* context, GrColorType colorType,
                         bool renderTarget) {
    if (GrCaps::kNone_MapFlags == context->priv().caps()->mapBufferFlags()) {
        return;
    }

    auto resourceProvider = context->priv().resourceProvider();
    GrGpu* gpu = context->priv().getGpu();

    // set up the data
    const int kTextureWidth = 16;
    const int kTextureHeight = 16;
#ifdef SK_BUILD_FOR_IOS
    // UNPACK_ROW_LENGTH is broken on iOS so rowBytes needs to match data width
    const int kBufferWidth = GrBackendApi::kOpenGL == context->backend() ? 16 : 20;
#else
    const int kBufferWidth = 20;
#endif
    const int kBufferHeight = 16;
    size_t rowBytes = kBufferWidth * sizeof(GrColor);
    SkAutoTMalloc<GrColor> srcBuffer(kBufferWidth*kBufferHeight);
    SkAutoTMalloc<GrColor> dstBuffer(kBufferWidth*kBufferHeight);

    fill_transfer_data(0, 0, kTextureWidth, kTextureHeight, kBufferWidth, srcBuffer.get());

    // create and fill transfer buffer
    size_t size = rowBytes*kBufferHeight;
    sk_sp<GrGpuBuffer> buffer(resourceProvider->createBuffer(size, GrGpuBufferType::kXferCpuToGpu,
                                                             kDynamic_GrAccessPattern));
    if (!buffer) {
        return;
    }

    void* data = buffer->map();
    memcpy(data, srcBuffer.get(), size);
    buffer->unmap();

    for (auto srgbEncoding : {GrSRGBEncoded::kNo, GrSRGBEncoded::kYes}) {
        // create texture
        GrSurfaceDesc desc;
        desc.fFlags = renderTarget ? kRenderTarget_GrSurfaceFlag : kNone_GrSurfaceFlags;
        desc.fWidth = kTextureWidth;
        desc.fHeight = kTextureHeight;
        desc.fConfig = GrColorTypeToPixelConfig(colorType, srgbEncoding);
        desc.fSampleCnt = 1;

        if (kUnknown_GrPixelConfig == desc.fConfig) {
            SkASSERT(GrSRGBEncoded::kYes == srgbEncoding);
            continue;
        }

        if (!context->priv().caps()->isConfigTexturable(desc.fConfig) ||
            (renderTarget && !context->priv().caps()->isConfigRenderable(desc.fConfig))) {
            continue;
        }

        sk_sp<GrTexture> tex = resourceProvider->createTexture(desc, SkBudgeted::kNo);
        if (!tex) {
            continue;
        }

        //////////////////////////
        // transfer full data

        bool result;
        result = gpu->transferPixels(tex.get(), 0, 0, kTextureWidth, kTextureHeight, colorType,
                                     buffer.get(), 0, rowBytes);
        REPORTER_ASSERT(reporter, result);

        memset(dstBuffer.get(), 0xCDCD, size);
        result = gpu->readPixels(tex.get(), 0, 0, kTextureWidth, kTextureHeight, colorType,
                                 dstBuffer.get(), rowBytes);
        if (result) {
            REPORTER_ASSERT(reporter, does_full_buffer_contain_correct_values(srcBuffer,
                                                                              dstBuffer,
                                                                              kTextureWidth,
                                                                              kTextureHeight,
                                                                              kBufferWidth,
                                                                              kBufferHeight));
        }

        //////////////////////////
        // transfer partial data
#ifdef SK_BUILD_FOR_IOS
        // UNPACK_ROW_LENGTH is broken on iOS so we can't do partial transfers
        if (GrBackendApi::kOpenGL == context->backend()) {
            continue;
        }
#endif
        const int kLeft = 2;
        const int kTop = 10;
        const int kWidth = 10;
        const int kHeight = 2;

        // change color of subrectangle
        fill_transfer_data(kLeft, kTop, kWidth, kHeight, kBufferWidth, srcBuffer.get());
        data = buffer->map();
        memcpy(data, srcBuffer.get(), size);
        buffer->unmap();

        size_t offset = sizeof(GrColor) * (kTop * kBufferWidth + kLeft);
        result = gpu->transferPixels(tex.get(), kLeft, kTop, kWidth, kHeight, colorType,
                                     buffer.get(), offset, rowBytes);
        REPORTER_ASSERT(reporter, result);

        memset(dstBuffer.get(), 0xCDCD, size);
        result = gpu->readPixels(tex.get(), 0, 0, kTextureWidth, kTextureHeight, colorType,
                                 dstBuffer.get(), rowBytes);
        if (result) {
            REPORTER_ASSERT(reporter, does_full_buffer_contain_correct_values(srcBuffer,
                                                                              dstBuffer,
                                                                              kTextureWidth,
                                                                              kTextureHeight,
                                                                              kBufferWidth,
                                                                              kBufferHeight));
        }
    }
}

#ifndef SK_METAL

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(TransferPixelsTest, reporter, ctxInfo) {
    // RGBA
    basic_transfer_test(reporter, ctxInfo.grContext(), GrColorType::kRGBA_8888, false);
    basic_transfer_test(reporter, ctxInfo.grContext(), GrColorType::kRGBA_8888, true);

    // BGRA
    basic_transfer_test(reporter, ctxInfo.grContext(), GrColorType::kBGRA_8888, false);
    basic_transfer_test(reporter, ctxInfo.grContext(), GrColorType::kBGRA_8888, true);
}

#endif
