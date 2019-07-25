/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a GPU-backend specific test. It relies on static intializers to work

#include "include/core/SkTypes.h"

#include "include/core/SkSurface.h"
#include "include/gpu/GrTexture.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrSurfaceProxy.h"
#include "src/gpu/SkGr.h"
#include "tests/Test.h"
#include "tests/TestUtils.h"
#include "tools/gpu/GrContextFactory.h"

using sk_gpu_test::GrContextFactory;

void fill_transfer_data(int left, int top, int width, int height, int bufferWidth,
                        GrColorType dstType, char* dst) {
    size_t dstBpp = GrColorTypeBytesPerPixel(dstType);
    auto dstLocation = [dst, dstBpp, bufferWidth](int x, int y) {
        return dst + y * dstBpp * bufferWidth + x * dstBpp;
    };
    // build red-green gradient
    for (int j = top; j < top + height; ++j) {
        for (int i = left; i < left + width; ++i) {
            unsigned int red = (unsigned int)(256.f*((i - left) / (float)width));
            unsigned int green = (unsigned int)(256.f*((j - top) / (float)height));
            uint32_t srcPixel = GrColorPackRGBA(red - (red>>8),
                                                green - (green>>8), 0xff, 0xff);
            GrPixelInfo srcInfo(GrColorType::kRGBA_8888, kPremul_SkAlphaType, nullptr, 1, 1);
            GrPixelInfo dstInfo(dstType, kPremul_SkAlphaType, nullptr, 1, 1);
            GrConvertPixels(dstInfo, dstLocation(i, j), dstBpp, srcInfo, &srcPixel, 4);
        }
    }
}

bool read_pixels_from_texture(GrTexture* texture, GrColorType dstColorType, char* dst) {
    auto* context = texture->getContext();
    auto* gpu = context->priv().getGpu();
    auto* caps = context->priv().caps();

    int w = texture->width();
    int h = texture->height();
    size_t rowBytes = GrColorTypeBytesPerPixel(dstColorType) * w;

    GrColorType srcCT = GrPixelConfigToColorType(texture->config());

    GrCaps::SupportedRead supportedRead = caps->supportedReadPixelsColorType(
            srcCT, texture->backendFormat(), dstColorType);
    if (supportedRead.fColorType != dstColorType || supportedRead.fSwizzle != GrSwizzle("rgba")) {
        size_t tmpRowBytes = GrColorTypeBytesPerPixel(supportedRead.fColorType) * w;
        std::unique_ptr<char[]> tmpPixels(new char[tmpRowBytes * h]);
        if (!gpu->readPixels(texture, 0, 0, w, h,
                             supportedRead.fColorType, tmpPixels.get(), tmpRowBytes)) {
            return false;
        }
        GrPixelInfo tmpInfo(supportedRead.fColorType, kPremul_SkAlphaType, nullptr, w, h);
        GrPixelInfo dstInfo(dstColorType,             kPremul_SkAlphaType, nullptr, w, h);
        return GrConvertPixels(dstInfo, dst, rowBytes, tmpInfo, tmpPixels.get(), tmpRowBytes, false,
                               supportedRead.fSwizzle);
    }
    return gpu->readPixels(texture, 0, 0, w, h, supportedRead.fColorType, dst, rowBytes);
}

void basic_transfer_to_test(skiatest::Reporter* reporter, GrContext* context, GrColorType colorType,
                            GrRenderable renderable) {
    if (GrCaps::kNone_MapFlags == context->priv().caps()->mapBufferFlags()) {
        return;
    }

    auto* caps = context->priv().caps();

    auto backendFormat = caps->getBackendFormatFromColorType(colorType);
    if (!caps->isFormatTexturable(colorType, backendFormat)) {
        return;
    }

    if ((renderable == GrRenderable::kYes && !caps->isFormatRenderable(colorType, backendFormat))) {
        return;
    }

    auto resourceProvider = context->priv().resourceProvider();
    GrGpu* gpu = context->priv().getGpu();

    const int kTextureWidth = 16;
    const int kTextureHeight = 16;
    int srcBufferWidth = caps->writePixelsRowBytesSupport() ? 20 : 16;
    const int kBufferHeight = 16;

    GrSurfaceDesc desc;
    desc.fWidth = kTextureWidth;
    desc.fHeight = kTextureHeight;
    desc.fConfig = GrColorTypeToPixelConfig(colorType);

    sk_sp<GrTexture> tex =
            resourceProvider->createTexture(desc, renderable, 1, SkBudgeted::kNo, GrProtected::kNo,
                                            GrResourceProvider::Flags::kNoPendingIO);
    if (!tex) {
        ERRORF(reporter, "Could not create texture");
        return;
    }

    // The caps tell us what color type we are allowed to upload and read back from this texture,
    // either of which may differ from 'colorType'.
    GrColorType allowedSrc = caps->supportedWritePixelsColorType(desc.fConfig, colorType);
    size_t srcRowBytes = GrColorTypeBytesPerPixel(allowedSrc) * srcBufferWidth;
    std::unique_ptr<char[]> srcData(new char[kTextureHeight * srcRowBytes]);

    fill_transfer_data(0, 0, kTextureWidth, kTextureHeight, srcBufferWidth, allowedSrc,
                       srcData.get());

    // create and fill transfer buffer
    size_t size = srcRowBytes * kBufferHeight;
    sk_sp<GrGpuBuffer> buffer(resourceProvider->createBuffer(size, GrGpuBufferType::kXferCpuToGpu,
                                                             kDynamic_GrAccessPattern));
    if (!buffer) {
        return;
    }
    void* data = buffer->map();
    if (!buffer) {
        ERRORF(reporter, "Could not map buffer");
        return;
    }
    memcpy(data, srcData.get(), size);
    buffer->unmap();

    //////////////////////////
    // transfer full data

    bool result;
    result = gpu->transferPixelsTo(tex.get(), 0, 0, kTextureWidth, kTextureHeight, allowedSrc,
                                   buffer.get(), 0, srcRowBytes);
    REPORTER_ASSERT(reporter, result);

    size_t dstRowBytes = GrColorTypeBytesPerPixel(colorType) * kTextureWidth;
    std::unique_ptr<char[]> dstBuffer(new char[dstRowBytes * kTextureHeight]());

    result = read_pixels_from_texture(tex.get(), colorType, dstBuffer.get());
    if (!result) {
        ERRORF(reporter, "Could not read pixels from texture");
        return;
    }

    static constexpr float kTol[4] = {};
    auto error = std::function<ComparePixmapsErrorReporter>(
            [reporter, colorType](int x, int y, const float diffs[4]) {
                ERRORF(reporter, "Error at (%d %d) in transfer, color type: %d", x, y, colorType);
            });
    GrPixelInfo srcInfo(allowedSrc, kPremul_SkAlphaType, nullptr, tex->width(), tex->height());
    GrPixelInfo dstInfo(colorType, kPremul_SkAlphaType, nullptr, tex->width(), tex->height());

    compare_pixels(srcInfo, srcData.get(), srcRowBytes, dstInfo, dstBuffer.get(), dstRowBytes, kTol,
                   error);

    //////////////////////////
    // transfer partial data

    // We're relying on this cap to write partial texture data
    if (!caps->writePixelsRowBytesSupport()) {
        return;
    }
    const int kLeft = 2;
    const int kTop = 10;
    const int kWidth = 10;
    const int kHeight = 2;

    // change color of subrectangle
    fill_transfer_data(kLeft, kTop, kWidth, kHeight, srcBufferWidth, allowedSrc, srcData.get());
    data = buffer->map();
    memcpy(data, srcData.get(), size);
    buffer->unmap();

    size_t offset = kTop * srcRowBytes + kLeft * GrColorTypeBytesPerPixel(allowedSrc);
    result = gpu->transferPixelsTo(tex.get(), kLeft, kTop, kWidth, kHeight, allowedSrc,
                                   buffer.get(), offset, srcRowBytes);
    REPORTER_ASSERT(reporter, result);

    result = read_pixels_from_texture(tex.get(), colorType, dstBuffer.get());
    if (!result) {
        ERRORF(reporter, "Could not read pixels from texture");
        return;
    }
    compare_pixels(srcInfo, srcData.get(), srcRowBytes, dstInfo, dstBuffer.get(), dstRowBytes, kTol,
                   error);
}

void basic_transfer_from_test(skiatest::Reporter* reporter, const sk_gpu_test::ContextInfo& ctxInfo,
                              GrColorType colorType, GrRenderable renderable) {
    auto context = ctxInfo.grContext();
    auto caps = context->priv().caps();
    if (GrCaps::kNone_MapFlags == caps->mapBufferFlags()) {
        return;
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

    // create texture
    GrSurfaceDesc desc;
    desc.fWidth = kTextureWidth;
    desc.fHeight = kTextureHeight;
    desc.fConfig = GrColorTypeToPixelConfig(colorType);

    if (!context->priv().caps()->isConfigTexturable(desc.fConfig) ||
        (renderable == GrRenderable::kYes &&
         !context->priv().caps()->isConfigRenderable(desc.fConfig))) {
        return;
    }

    size_t textureDataBpp = GrColorTypeBytesPerPixel(colorType);
    size_t textureDataRowBytes = kTextureWidth * textureDataBpp;
    std::unique_ptr<char[]> textureData(new char[kTextureHeight * textureDataRowBytes]);
    fill_transfer_data(0, 0, kTextureWidth, kTextureHeight, kTextureWidth, colorType,
                       textureData.get());
    GrMipLevel data;
    data.fPixels = textureData.get();
    data.fRowBytes = textureDataRowBytes;
    sk_sp<GrTexture> tex = resourceProvider->createTexture(desc, renderable, 1, SkBudgeted::kNo,
                                                           GrProtected::kNo, &data, 1);
    if (!tex) {
        return;
    }

    // Create the transfer buffer.
    auto allowedRead =
            caps->supportedReadPixelsColorType(colorType, tex->backendFormat(), colorType);
    GrPixelInfo readInfo(allowedRead.fColorType, kPremul_SkAlphaType, nullptr, kTextureWidth,
                         kTextureHeight);

    size_t bpp = GrColorTypeBytesPerPixel(allowedRead.fColorType);
    size_t fullBufferRowBytes = kTextureWidth * bpp;
    size_t partialBufferRowBytes = kPartialWidth * bpp;
    size_t offsetAlignment = caps->transferFromOffsetAlignment(allowedRead.fColorType);
    SkASSERT(offsetAlignment);

    size_t bufferSize = fullBufferRowBytes * kTextureHeight;
    // Arbitrary starting offset for the partial read.
    size_t partialReadOffset = GrSizeAlignUp(11, offsetAlignment);
    bufferSize = SkTMax(bufferSize, partialReadOffset + partialBufferRowBytes * kPartialHeight);

    sk_sp<GrGpuBuffer> buffer(resourceProvider->createBuffer(
            bufferSize, GrGpuBufferType::kXferGpuToCpu, kDynamic_GrAccessPattern));
    REPORTER_ASSERT(reporter, buffer);
    if (!buffer) {
        return;
    }

    int expectedTransferCnt = 0;
    gpu->stats()->reset();

    //////////////////////////
    // transfer full data
    bool result = gpu->transferPixelsFrom(tex.get(), 0, 0, kTextureWidth, kTextureHeight,
                                          allowedRead.fColorType, buffer.get(), 0);
    if (!result) {
        ERRORF(reporter, "transferPixelsFrom failed.");
        return;
    }
    ++expectedTransferCnt;

    GrFlushInfo flushInfo;
    flushInfo.fFlags = kSyncCpu_GrFlushFlag;
    if (context->priv().caps()->mapBufferFlags() & GrCaps::kAsyncRead_MapFlag) {
        gpu->finishFlush(nullptr, 0, SkSurface::BackendSurfaceAccess::kNoAccess, flushInfo,
                         GrPrepareForExternalIORequests());
    }

    // Copy the transfer buffer contents to a temporary so we can manipulate it.
    const auto* map = reinterpret_cast<const char*>(buffer->map());
    REPORTER_ASSERT(reporter, map);
    if (!map) {
        ERRORF(reporter, "Failed to map transfer buffer.");
        return;
    }
    std::unique_ptr<char[]> transferData(new char[kTextureHeight * fullBufferRowBytes]);
    memcpy(transferData.get(), map, fullBufferRowBytes * kTextureHeight);
    buffer->unmap();

    GrPixelInfo transferInfo(allowedRead.fColorType, kPremul_SkAlphaType, nullptr, kTextureWidth,
                             kTextureHeight);
    // Caps may indicate that we should swizzle this data before we compare it.
    if (allowedRead.fSwizzle != GrSwizzle("rgba")) {
        GrConvertPixels(transferInfo, transferData.get(), fullBufferRowBytes, transferInfo,
                        transferData.get(), fullBufferRowBytes, false, allowedRead.fSwizzle);
    }

    static constexpr float kTol[4] = {};
    auto error = std::function<ComparePixmapsErrorReporter>(
            [reporter, colorType](int x, int y, const float diffs[4]) {
                ERRORF(reporter, "Error at (%d %d) in transfer, color type: %d", x, y, colorType);
            });
    GrPixelInfo textureDataInfo(colorType, kPremul_SkAlphaType, nullptr, kTextureWidth,
                                kTextureHeight);
    compare_pixels(textureDataInfo, textureData.get(), textureDataRowBytes, transferInfo,
                   transferData.get(), fullBufferRowBytes, kTol, error);

    ///////////////////////
    // Now test a partial read at an offset into the buffer.
    result = gpu->transferPixelsFrom(tex.get(), kPartialLeft, kPartialTop, kPartialWidth,
                                     kPartialHeight, allowedRead.fColorType, buffer.get(),
                                     partialReadOffset);
    if (!result) {
        ERRORF(reporter, "transferPixelsFrom failed.");
        return;
    }
    ++expectedTransferCnt;

    if (context->priv().caps()->mapBufferFlags() & GrCaps::kAsyncRead_MapFlag) {
        gpu->finishFlush(nullptr, 0, SkSurface::BackendSurfaceAccess::kNoAccess, flushInfo,
                         GrPrepareForExternalIORequests());
    }

    map = reinterpret_cast<const char*>(buffer->map());
    REPORTER_ASSERT(reporter, map);
    if (!map) {
        ERRORF(reporter, "Failed to map transfer buffer.");
        return;
    }
    const char* bufferStart = reinterpret_cast<const char*>(map) + partialReadOffset;
    memcpy(transferData.get(), bufferStart, partialBufferRowBytes * kTextureHeight);
    buffer->unmap();

    transferInfo = transferInfo.makeWH(kPartialWidth, kPartialHeight);
    if (allowedRead.fSwizzle != GrSwizzle("rgba")) {
        GrConvertPixels(transferInfo, transferData.get(), fullBufferRowBytes, transferInfo,
                        transferData.get(), fullBufferRowBytes, false, allowedRead.fSwizzle);
    }

    const char* textureDataStart =
            textureData.get() + textureDataRowBytes * kPartialTop + textureDataBpp * kPartialLeft;
    textureDataInfo = textureDataInfo.makeWH(kPartialWidth, kPartialHeight);
    compare_pixels(textureDataInfo, textureDataStart, textureDataRowBytes, transferInfo,
                   transferData.get(), partialBufferRowBytes, kTol, error);
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
    for (auto renderable : {GrRenderable::kNo, GrRenderable::kYes}) {
        for (auto colorType :
             {GrColorType::kRGBA_8888, GrColorType::kRGBA_8888_SRGB, GrColorType::kBGRA_8888}) {
            basic_transfer_to_test(reporter, ctxInfo.grContext(), colorType, renderable);
        }
    }
}

// TODO(bsalomon): Metal
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(TransferPixelsFromTest, reporter, ctxInfo) {
    if (!ctxInfo.grContext()->priv().caps()->transferBufferSupport()) {
        return;
    }
    for (auto renderable : {GrRenderable::kNo, GrRenderable::kYes}) {
        for (auto colorType :
             {GrColorType::kRGBA_8888, GrColorType::kRGBA_8888_SRGB, GrColorType::kBGRA_8888}) {
            basic_transfer_from_test(reporter, ctxInfo, colorType, renderable);
        }
    }
}
