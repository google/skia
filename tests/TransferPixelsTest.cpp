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

bool do_buffers_contain_same_values(const GrColor* bufferA,
                                    const GrColor* bufferB,
                                    int width,
                                    int height,
                                    size_t rowBytesA,
                                    size_t rowBytesB,
                                    bool swiz) {
    for (int j = 0; j < height; ++j) {
        for (int i = 0; i < width; ++i) {
            auto colorA = bufferA[i];
            if (swiz) {
                colorA = GrColorPackRGBA(GrColorUnpackB(colorA), GrColorUnpackG(colorA),
                                         GrColorUnpackR(colorA), GrColorUnpackA(colorA));
            }
            if (colorA != bufferB[i]) {
                return false;
            }
        }
        bufferA = reinterpret_cast<const GrColor*>(reinterpret_cast<const char*>(bufferA) +
                                                   rowBytesA);
        bufferB = reinterpret_cast<const GrColor*>(reinterpret_cast<const char*>(bufferB) +
                                                   rowBytesB);
    }
    return true;
}

void basic_transfer_to_test(skiatest::Reporter* reporter, GrContext* context, GrColorType colorType,
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

        sk_sp<GrTexture> tex = resourceProvider->createTexture(
            desc, SkBudgeted::kNo, GrResourceProvider::Flags::kNoPendingIO);
        if (!tex) {
            continue;
        }

        //////////////////////////
        // transfer full data

        bool result;
        result = gpu->transferPixelsTo(tex.get(), 0, 0, kTextureWidth, kTextureHeight, colorType,
                                       buffer.get(), 0, rowBytes);
        REPORTER_ASSERT(reporter, result);

        memset(dstBuffer.get(), 0xCDCD, size);
        result = gpu->readPixels(tex.get(), 0, 0, kTextureWidth, kTextureHeight, colorType,
                                 dstBuffer.get(), rowBytes);
        if (result) {
            REPORTER_ASSERT(reporter, do_buffers_contain_same_values(srcBuffer,
                                                                     dstBuffer,
                                                                     kTextureWidth,
                                                                     kTextureHeight,
                                                                     rowBytes,
                                                                     rowBytes,
                                                                     false));
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
        result = gpu->transferPixelsTo(tex.get(), kLeft, kTop, kWidth, kHeight, colorType,
                                       buffer.get(), offset, rowBytes);
        REPORTER_ASSERT(reporter, result);

        memset(dstBuffer.get(), 0xCDCD, size);
        result = gpu->readPixels(tex.get(), 0, 0, kTextureWidth, kTextureHeight, colorType,
                                 dstBuffer.get(), rowBytes);
        if (result) {
            REPORTER_ASSERT(reporter, do_buffers_contain_same_values(srcBuffer,
                                                                     dstBuffer,
                                                                     kTextureWidth,
                                                                     kTextureHeight,
                                                                     rowBytes,
                                                                     rowBytes,
                                                                     false));
        }
    }
}

void basic_transfer_from_test(skiatest::Reporter* reporter, const sk_gpu_test::ContextInfo& ctxInfo,
                              GrColorType colorType, bool renderTarget) {
    auto context = ctxInfo.grContext();
    if (GrCaps::kNone_MapFlags == context->priv().caps()->mapBufferFlags()) {
        return;
    }

    // On OpenGL ES it may not be possible to read back in to BGRA becagse GL_RGBA/GL_UNSIGNED_BYTE
    // may be the only allowed format/type params to glReadPixels. So read back into GL_RGBA.
    // TODO(bsalomon): Make this work in GrGLGpu.
    auto readColorType = colorType;
    if (GrColorType::kBGRA_8888 == colorType &&
        ctxInfo.type() == sk_gpu_test::GrContextFactory::kGLES_ContextType) {
        readColorType = GrColorType::kRGBA_8888;
    }

    auto resourceProvider = context->priv().resourceProvider();
    GrGpu* gpu = context->priv().getGpu();

    const int kTextureWidth = 16;
    const int kTextureHeight = 16;

    // We'll do a full texture read into the buffer followed by a partial read. These values
    // describe the partial read subrect.
    const int kPartialLeft = 2;
    const int kPartialTop = 10;
    const int kPartialWidth = 10;
    const int kPartialHeight = 2;

    size_t fullBufferRowBytes;
    size_t partialBufferRowBytes;
    size_t fullBufferOffsetAlignment;
    size_t partialBufferOffsetAlignment;

    SkAssertResult(context->priv().caps()->transferFromBufferRequirements(
            colorType, kTextureWidth, &fullBufferRowBytes, &fullBufferOffsetAlignment));
    SkAssertResult(context->priv().caps()->transferFromBufferRequirements(
            colorType, kPartialWidth, &partialBufferRowBytes, &partialBufferOffsetAlignment));

    size_t bufferSize = fullBufferRowBytes * kTextureHeight;
    // Arbitrary starting offset for the partial read.
    size_t partialReadOffset = GrSizeAlignUp(11, partialBufferOffsetAlignment);
    bufferSize =
            SkTMax(bufferSize, partialReadOffset + partialBufferRowBytes * partialBufferRowBytes);

    sk_sp<GrGpuBuffer> buffer(resourceProvider->createBuffer(
            bufferSize, GrGpuBufferType::kXferGpuToCpu, kDynamic_GrAccessPattern));
    REPORTER_ASSERT(reporter, buffer);
    if (!buffer) {
        return;
    }

    int expectedTransferCnt = 0;
    gpu->stats()->reset();
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

        SkAutoTMalloc<GrColor> textureData(kTextureWidth * kTextureHeight);
        size_t textureDataRowBytes = kTextureWidth * sizeof(GrColor);
        fill_transfer_data(0, 0, kTextureWidth, kTextureHeight, kTextureWidth, textureData.get());
        GrMipLevel data;
        data.fPixels = textureData.get();
        data.fRowBytes = kTextureWidth * sizeof(GrColor);
        sk_sp<GrTexture> tex = resourceProvider->createTexture(desc, SkBudgeted::kNo, &data, 1);
        if (!tex) {
            continue;
        }

        //////////////////////////
        // transfer full data
        auto bufferRowBytes = gpu->transferPixelsFrom(
                tex.get(), 0, 0, kTextureWidth, kTextureHeight, readColorType, buffer.get(), 0);
        REPORTER_ASSERT(reporter, bufferRowBytes = fullBufferRowBytes);
        if (!bufferRowBytes) {
            continue;
        }
        ++expectedTransferCnt;

        // TODO(bsalomon): caps to know if the map() is synchronous and skip the flush if so.
        gpu->finishFlush(nullptr, SkSurface::BackendSurfaceAccess::kNoAccess,
                         kSyncCpu_GrFlushFlag, 0, nullptr, nullptr, nullptr);

        const auto* map = reinterpret_cast<const GrColor*>(buffer->map());
        REPORTER_ASSERT(reporter, map);
        if (!map) {
            continue;
        }
        REPORTER_ASSERT(reporter, do_buffers_contain_same_values(textureData.get(),
                                                                 map,
                                                                 kTextureWidth,
                                                                 kTextureHeight,
                                                                 textureDataRowBytes,
                                                                 bufferRowBytes,
                                                                 readColorType != colorType));
        buffer->unmap();

        ///////////////////////
        // Now test a partial read at an offset into the buffer.
        bufferRowBytes = gpu->transferPixelsFrom(tex.get(), kPartialLeft, kPartialTop,
                                                 kPartialWidth, kPartialHeight, readColorType,
                                                 buffer.get(), partialReadOffset);
        REPORTER_ASSERT(reporter, bufferRowBytes = partialBufferRowBytes);
        if (!bufferRowBytes) {
            continue;
        }
        ++expectedTransferCnt;

        // TODO(bsalomon): caps to know if the map() is synchronous and skip the flush if so.
        gpu->finishFlush(nullptr, SkSurface::BackendSurfaceAccess::kNoAccess,
                         kSyncCpu_GrFlushFlag, 0, nullptr, nullptr, nullptr);

        map = reinterpret_cast<const GrColor*>(buffer->map());
        REPORTER_ASSERT(reporter, map);
        if (!map) {
            continue;
        }
        const GrColor* textureDataStart = reinterpret_cast<const GrColor*>(
                reinterpret_cast<const char*>(textureData.get()) +
                textureDataRowBytes * kPartialTop + sizeof(GrColor) * kPartialLeft);
        const GrColor* bufferStart = reinterpret_cast<const GrColor*>(
                reinterpret_cast<const char*>(map) + partialReadOffset);
        REPORTER_ASSERT(reporter, do_buffers_contain_same_values(textureDataStart,
                                                                 bufferStart,
                                                                 kPartialWidth,
                                                                 kPartialHeight,
                                                                 textureDataRowBytes,
                                                                 bufferRowBytes,
                                                                 readColorType != colorType));
        buffer->unmap();
    }
#if GR_GPU_STATS
    REPORTER_ASSERT(reporter, gpu->stats()->transfersFromSurface() == expectedTransferCnt);
#else
    (void)expectedTransferCnt;
#endif
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(TransferPixelsToTest, reporter, ctxInfo) {
    if (!ctxInfo.grContext()->priv().caps()->transferBufferSupport()) {
        return;
    }
    // RGBA
    basic_transfer_to_test(reporter, ctxInfo.grContext(), GrColorType::kRGBA_8888, false);
    basic_transfer_to_test(reporter, ctxInfo.grContext(), GrColorType::kRGBA_8888, true);

    // BGRA
    basic_transfer_to_test(reporter, ctxInfo.grContext(), GrColorType::kBGRA_8888, false);
    basic_transfer_to_test(reporter, ctxInfo.grContext(), GrColorType::kBGRA_8888, true);
}

// TODO(bsalomon): Metal
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(TransferPixelsFromTest, reporter, ctxInfo) {
    if (!ctxInfo.grContext()->priv().caps()->transferBufferSupport()) {
        return;
    }
    // RGBA
    basic_transfer_from_test(reporter, ctxInfo, GrColorType::kRGBA_8888, false);
    basic_transfer_from_test(reporter, ctxInfo, GrColorType::kRGBA_8888, true);

    // BGRA
    basic_transfer_from_test(reporter, ctxInfo, GrColorType::kBGRA_8888, false);
    basic_transfer_from_test(reporter, ctxInfo, GrColorType::kBGRA_8888, true);
}
