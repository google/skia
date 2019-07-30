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
            auto r = (unsigned int)(256.f*((i - left) / (float)width));
            auto g = (unsigned int)(256.f*((j - top) / (float)height));
            r -= (r >> 8);
            g -= (g >> 8);
            // set b and a channels to be inverse of r and g just to have interesting values to
            // test.
            uint32_t srcPixel = GrColorPackRGBA(r, g, 0xff - r, 0xff - g);
            GrPixelInfo srcInfo(GrColorType::kRGBA_8888, kUnpremul_SkAlphaType, nullptr, 1, 1);
            GrPixelInfo dstInfo(dstType, kUnpremul_SkAlphaType, nullptr, 1, 1);
            GrConvertPixels(dstInfo, dstLocation(i, j), dstBpp, srcInfo, &srcPixel, 4);
        }
    }
}

void determine_tolerances(GrColorType a, GrColorType b, float tolerances[4]) {
    std::fill_n(tolerances, 4, 0);

    auto descA = GrGetColorTypeDesc(a);
    auto descB = GrGetColorTypeDesc(b);
    // For each channel x set the tolerance to 1 / (2^min(bits_in_a, bits_in_b) - 1) unless
    // one color type is missing the channel. In that case leave it at 0. If the other color
    // has the channel then it better be exactly 1 for alpha or 0 for rgb.
    for (int i = 0; i < 4; ++i) {
        if (descA[i] != descB[i]) {
            auto m = std::min(descA[i], descB[i]);
            if (m) {
                tolerances[i] = 1.f / (m - 1);
            }
        }
    }
}

bool read_pixels_from_texture(GrTexture* texture, GrColorType dstColorType, char* dst,
                              float tolerances[4]) {
    auto* context = texture->getContext();
    auto* gpu = context->priv().getGpu();
    auto* caps = context->priv().caps();

    int w = texture->width();
    int h = texture->height();
    size_t rowBytes = GrColorTypeBytesPerPixel(dstColorType) * w;

    GrColorType srcCT = GrPixelConfigToColorType(texture->config());

    GrCaps::SupportedRead supportedRead = caps->supportedReadPixelsColorType(
            srcCT, texture->backendFormat(), dstColorType);
    std::fill_n(tolerances, 4, 0);
    if (supportedRead.fColorType != dstColorType || supportedRead.fSwizzle != GrSwizzle("rgba")) {
        size_t tmpRowBytes = GrColorTypeBytesPerPixel(supportedRead.fColorType) * w;
        std::unique_ptr<char[]> tmpPixels(new char[tmpRowBytes * h]);
        if (!gpu->readPixels(texture, 0, 0, w, h,
                             supportedRead.fColorType, tmpPixels.get(), tmpRowBytes)) {
            return false;
        }
        GrPixelInfo tmpInfo(supportedRead.fColorType, kUnpremul_SkAlphaType, nullptr, w, h);
        GrPixelInfo dstInfo(dstColorType,             kUnpremul_SkAlphaType, nullptr, w, h);
        determine_tolerances(tmpInfo.colorType(), dstInfo.colorType(), tolerances);
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

    auto backendFormat = caps->getDefaultBackendFormat(colorType, renderable);
    if (!backendFormat.isValid()) {
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

    // We validate the results using GrGpu::readPixels, so exit if this is not supported.
    // TODO: Do this through GrSurfaceContext once it works for all color types or support
    // kCopyToTexture2D here.
    if (GrCaps::SurfaceReadPixelsSupport::kSupported !=
        caps->surfaceSupportsReadPixels(tex.get())) {
        return;
    }
    // GL requires a texture to be framebuffer bindable to call glReadPixels. However, we have not
    // incorporated that test into surfaceSupportsReadPixels(). TODO: Remove this once we handle
    // drawing to a bindable format.
    if (!caps->isFormatRenderable(colorType, tex->backendFormat())) {
        return;
    }

    // The caps tell us what color type we are allowed to upload and read back from this texture,
    // either of which may differ from 'colorType'.
    GrCaps::SupportedWrite allowedSrc =
            caps->supportedWritePixelsColorType(desc.fConfig, colorType);
    size_t srcRowBytes = GrColorTypeBytesPerPixel(allowedSrc.fColorType) * srcBufferWidth;
    std::unique_ptr<char[]> srcData(new char[kTextureHeight * srcRowBytes]);

    fill_transfer_data(0, 0, kTextureWidth, kTextureHeight, srcBufferWidth, allowedSrc.fColorType,
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
    result = gpu->transferPixelsTo(tex.get(), 0, 0, kTextureWidth, kTextureHeight,
                                   allowedSrc.fColorType, buffer.get(), 0, srcRowBytes);
    REPORTER_ASSERT(reporter, result);

    size_t dstRowBytes = GrColorTypeBytesPerPixel(colorType) * kTextureWidth;
    std::unique_ptr<char[]> dstBuffer(new char[dstRowBytes * kTextureHeight]());

    float compareTolerances[4] = {};
    result = read_pixels_from_texture(tex.get(), colorType, dstBuffer.get(), compareTolerances);
    if (!result) {
        ERRORF(reporter, "Could not read pixels from texture, color type: %d",
               static_cast<int>(colorType));
        return;
    }

    auto error = std::function<ComparePixmapsErrorReporter>(
            [reporter, colorType](int x, int y, const float diffs[4]) {
                ERRORF(reporter,
                       "Error at (%d %d) in transfer, color type: %d, diffs: (%f, %f, %f, %f)", x,
                       y, colorType, diffs[0], diffs[1], diffs[2], diffs[3]);
            });
    GrPixelInfo srcInfo(allowedSrc.fColorType, kUnpremul_SkAlphaType, nullptr, tex->width(),
                        tex->height());
    GrPixelInfo dstInfo(colorType, kUnpremul_SkAlphaType, nullptr, tex->width(), tex->height());
    compare_pixels(srcInfo, srcData.get(), srcRowBytes, dstInfo, dstBuffer.get(), dstRowBytes,
                   compareTolerances, error);

    //////////////////////////
    // transfer partial data

    // We're relying on this cap to write partial texture data
    if (!caps->writePixelsRowBytesSupport()) {
        return;
    }
    // We keep a 1 to 1 correspondence between pixels in the buffer and the entire texture. We
    // update the contents of a sub-rect of the buffer and push that rect to the texture. We start
    // with a left sub-rect inset of 2 but may adjust that so we can fulfill the transfer buffer
    // offset alignment requirement.
    int left = 2;
    const int top = 10;
    const int width = 10;
    const int height = 2;
    size_t offset = top * srcRowBytes + left * GrColorTypeBytesPerPixel(allowedSrc.fColorType);
    while (offset % allowedSrc.fOffsetAlignmentForTransferBuffer) {
        offset += GrColorTypeBytesPerPixel(allowedSrc.fColorType);
        ++left;
        // We're assuming that the required alignment is 1 or a small multiple of the bpp, which
        // it is currently for all color types across all backends.
        SkASSERT(left + width <= tex->width());
    }

    // change color of subrectangle
    fill_transfer_data(left, top, width, height, srcBufferWidth, allowedSrc.fColorType,
                       srcData.get());
    data = buffer->map();
    memcpy(data, srcData.get(), size);
    buffer->unmap();

    result = gpu->transferPixelsTo(tex.get(), left, top, width, height, allowedSrc.fColorType,
                                   buffer.get(), offset, srcRowBytes);
    if (!result) {
        gpu->transferPixelsTo(tex.get(), left, top, width, height, allowedSrc.fColorType,
                              buffer.get(), offset, srcRowBytes);
        ERRORF(reporter, "Could not transfer pixels to texture, color type: %d",
               static_cast<int>(colorType));
        return;
    }

    result = read_pixels_from_texture(tex.get(), colorType, dstBuffer.get(), compareTolerances);
    if (!result) {
        ERRORF(reporter, "Could not read pixels from texture, color type: %d",
               static_cast<int>(colorType));
        return;
    }
    compare_pixels(srcInfo, srcData.get(), srcRowBytes, dstInfo, dstBuffer.get(), dstRowBytes,
                   compareTolerances, error);
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

    if (GrCaps::SurfaceReadPixelsSupport::kSupported !=
        caps->surfaceSupportsReadPixels(tex.get())) {
        return;
    }
    // GL requires a texture to be framebuffer bindable to call glReadPixels. However, we have not
    // incorporated that test into surfaceSupportsReadPixels(). TODO: Remove this once we handle
    // drawing to a bindable format.
    if (!caps->isFormatRenderable(colorType, tex->backendFormat())) {
        return;
    }

    // Create the transfer buffer.
    auto allowedRead =
            caps->supportedReadPixelsColorType(colorType, tex->backendFormat(), colorType);
    GrPixelInfo readInfo(allowedRead.fColorType, kUnpremul_SkAlphaType, nullptr, kTextureWidth,
                         kTextureHeight);

    size_t bpp = GrColorTypeBytesPerPixel(allowedRead.fColorType);
    size_t fullBufferRowBytes = kTextureWidth * bpp;
    size_t partialBufferRowBytes = kPartialWidth * bpp;
    size_t offsetAlignment = allowedRead.fOffsetAlignmentForTransferBuffer;
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

    GrPixelInfo transferInfo(allowedRead.fColorType, kUnpremul_SkAlphaType, nullptr, kTextureWidth,
                             kTextureHeight);
    // Caps may indicate that we should swizzle this data before we compare it.
    if (allowedRead.fSwizzle != GrSwizzle("rgba")) {
        GrConvertPixels(transferInfo, transferData.get(), fullBufferRowBytes, transferInfo,
                        transferData.get(), fullBufferRowBytes, false, allowedRead.fSwizzle);
    }

    float tol[4];
    determine_tolerances(allowedRead.fColorType, colorType, tol);
    auto error = std::function<ComparePixmapsErrorReporter>(
            [reporter, colorType](int x, int y, const float diffs[4]) {
                ERRORF(reporter,
                       "Error at (%d %d) in transfer, color type: %d, diffs: (%f, %f, %f, %f)", x,
                       y, colorType, diffs[0], diffs[1], diffs[2], diffs[3]);
            });
    GrPixelInfo textureDataInfo(colorType, kUnpremul_SkAlphaType, nullptr, kTextureWidth,
                                kTextureHeight);
    compare_pixels(textureDataInfo, textureData.get(), textureDataRowBytes, transferInfo,
                   transferData.get(), fullBufferRowBytes, tol, error);

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
        GrConvertPixels(transferInfo, transferData.get(), partialBufferRowBytes, transferInfo,
                        transferData.get(), partialBufferRowBytes, false, allowedRead.fSwizzle);
    }

    const char* textureDataStart =
            textureData.get() + textureDataRowBytes * kPartialTop + textureDataBpp * kPartialLeft;
    textureDataInfo = textureDataInfo.makeWH(kPartialWidth, kPartialHeight);
    compare_pixels(textureDataInfo, textureDataStart, textureDataRowBytes, transferInfo,
                   transferData.get(), partialBufferRowBytes, tol, error);
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
        for (auto colorType : {
                     GrColorType::kAlpha_8,
                     GrColorType::kBGR_565,
                     GrColorType::kABGR_4444,
                     GrColorType::kRGBA_8888,
                     GrColorType::kRGBA_8888_SRGB,
                     //  GrColorType::kRGB_888x, Broken in GL until we have kRGB_888
                     GrColorType::kRG_88,
                     GrColorType::kBGRA_8888,
                     GrColorType::kRGBA_1010102,
                     //  GrColorType::kGray_8, Reading back to kGray is busted.
                     GrColorType::kAlpha_F16,
                     GrColorType::kRGBA_F16,
                     GrColorType::kRGBA_F16_Clamped,
                     GrColorType::kRGBA_F32,
                     GrColorType::kR_16,
                     GrColorType::kRG_1616,
                     GrColorType::kRGBA_16161616,
                     GrColorType::kRG_F16,
             }) {
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
        for (auto colorType : {
                GrColorType::kAlpha_8,
                GrColorType::kBGR_565,
                GrColorType::kABGR_4444,
                GrColorType::kRGBA_8888,
                GrColorType::kRGBA_8888_SRGB,
                //  GrColorType::kRGB_888x, Broken in GL until we have kRGB_888
                GrColorType::kRG_88,
                GrColorType::kBGRA_8888,
                GrColorType::kRGBA_1010102,
                //  GrColorType::kGray_8, Reading back to kGray is busted.
                GrColorType::kAlpha_F16,
                GrColorType::kRGBA_F16,
                GrColorType::kRGBA_F16_Clamped,
                GrColorType::kRGBA_F32,
                GrColorType::kR_16,
                GrColorType::kRG_1616,
                GrColorType::kRGBA_16161616,
                GrColorType::kRG_F16,
        }) {
            basic_transfer_from_test(reporter, ctxInfo, colorType, renderable);
        }
    }
}
