/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a GPU-backend specific test. It relies on static intializers to work

#include "include/core/SkTypes.h"

#include "include/core/SkSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrImageInfo.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrSurfaceProxy.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/SkGr.h"
#include "tests/Test.h"
#include "tests/TestUtils.h"
#include "tools/gpu/GrContextFactory.h"

using sk_gpu_test::GrContextFactory;

void fill_transfer_data(SkIRect subset, GrPixmap dst) {
    size_t dstBpp = GrColorTypeBytesPerPixel(dst.colorType());
    auto dstLocation = [dst, dstBpp](int x, int y) {
        return static_cast<char*>(dst.addr()) + y*dst.rowBytes() + x*dstBpp;
    };
    // build red-green gradient
    for (int j = subset.top(); j < subset.bottom(); ++j) {
        for (int i = subset.left(); i < subset.right(); ++i) {
            auto r = (unsigned int)(256.f*((i - subset.left())/(float)subset.width()));
            auto g = (unsigned int)(256.f*((j - subset.top()) /(float)subset.height()));
            r -= (r >> 8);
            g -= (g >> 8);
            // set b and a channels to be inverse of r and g just to have interesting values to
            // test.
            uint32_t srcPixel = GrColorPackRGBA(r, g, 0xff - r, 0xff - g);
            GrImageInfo srcInfo(GrColorType::kRGBA_8888, kUnpremul_SkAlphaType, nullptr, 1, 1);
            GrImageInfo dstInfo = dst.info().makeWH(1, 1);
            GrConvertPixels(GrPixmap(dstInfo, dstLocation(i, j), dstBpp),
                            GrPixmap(srcInfo,         &srcPixel,      4));
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

bool read_pixels_from_texture(GrTexture* texture, GrPixmap pixmap, float tolerances[4]) {
    auto* context = texture->getContext();
    auto* gpu = context->priv().getGpu();
    auto* caps = context->priv().caps();

    int w = texture->width();
    int h = texture->height();

    GrCaps::SupportedRead supportedRead =
            caps->supportedReadPixelsColorType(pixmap.colorType(),
                                               texture->backendFormat(),
                                               pixmap.colorType());
    std::fill_n(tolerances, 4, 0);
    if (supportedRead.fColorType != pixmap.colorType()) {
        GrPixmap temp = GrPixmap::Allocate(pixmap.info().makeColorType(supportedRead.fColorType));
        if (!gpu->readPixels(texture,
                             SkIRect::MakeWH(w, h),
                             pixmap.colorType(),
                             supportedRead.fColorType,
                             temp.addr(),
                             temp.rowBytes())) {
            return false;
        }
        GrImageInfo tmpInfo(supportedRead.fColorType, kUnpremul_SkAlphaType, nullptr, w, h);
        determine_tolerances(tmpInfo.colorType(), pixmap.colorType(), tolerances);
        return GrConvertPixels(pixmap, temp);
    }
    return gpu->readPixels(texture,
                           SkIRect::MakeWH(w, h),
                           pixmap.colorType(),
                           supportedRead.fColorType,
                           pixmap.addr(),
                           pixmap.rowBytes());
}

void basic_transfer_to_test(skiatest::Reporter* reporter,
                            GrDirectContext* dContext,
                            GrColorType colorType,
                            GrRenderable renderable) {
    if (GrCaps::kNone_MapFlags == dContext->priv().caps()->mapBufferFlags()) {
        return;
    }

    auto* caps = dContext->priv().caps();

    auto backendFormat = caps->getDefaultBackendFormat(colorType, renderable);
    if (!backendFormat.isValid()) {
        return;
    }

    auto resourceProvider = dContext->priv().resourceProvider();
    GrGpu* gpu = dContext->priv().getGpu();

    static constexpr SkISize kTexDims = {16, 16};

    sk_sp<GrTexture> tex =
            resourceProvider->createTexture(kTexDims, backendFormat, renderable, 1,
                                            GrMipmapped::kNo, SkBudgeted::kNo, GrProtected::kNo);
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
    if (!caps->isFormatAsColorTypeRenderable(colorType, tex->backendFormat())) {
        return;
    }

    // The caps tell us what color type we are allowed to upload and read back from this texture,
    // either of which may differ from 'colorType'.
    GrCaps::SupportedWrite allowedSrc = caps->supportedWritePixelsColorType(colorType,
                                                                            tex->backendFormat(),
                                                                            colorType);
    SkISize srcDims = kTexDims;
    if (caps->writePixelsRowBytesSupport()) {
        srcDims.fWidth += 4;
    }
    size_t srcRowBytes =
            GrAlignTo(GrColorTypeBytesPerPixel(allowedSrc.fColorType)*srcDims.width(),
                      caps->transferBufferAlignment());
    GrImageInfo srcInfo(allowedSrc.fColorType, kUnpremul_SkAlphaType, nullptr, srcDims);
    GrPixmap src = GrPixmap::Allocate(srcInfo, srcRowBytes);
    fill_transfer_data(SkIRect::MakeSize(kTexDims), src);

    // create and fill transfer buffer
    sk_sp<GrGpuBuffer> buffer = resourceProvider->createBuffer(
            srcInfo.minRowBytes()*srcInfo.height(),
            GrGpuBufferType::kXferCpuToGpu,
            kDynamic_GrAccessPattern);
    if (!buffer) {
        ERRORF(reporter, "Could not create buffer.");
        return;
    }
    void* data = buffer->map();
    if (!buffer) {
        ERRORF(reporter, "Could not map buffer");
        return;
    }
    memcpy(data, src.addr(), srcInfo.minRowBytes()*srcInfo.height());
    buffer->unmap();

    //////////////////////////
    // transfer full data

    if (!gpu->transferPixelsTo(tex.get(),
                               SkIRect::MakeSize(kTexDims),
                               colorType,
                               src.colorType(),
                               buffer,
                               0,
                               src.rowBytes())) {
        ERRORF(reporter,
               "Could not do full transfer to color type %s.",
               GrColorTypeToStr(colorType));
    }

    GrPixmap read = GrPixmap::Allocate(srcInfo.makeColorType(colorType).makeDimensions(kTexDims));
    float compareTolerances[4] = {};
    if (!read_pixels_from_texture(tex.get(), read, compareTolerances)) {
        ERRORF(reporter,
               "Could not read pixels from texture, color type: %s",
               GrColorType(colorType));
        return;
    }

    auto error = std::function<ComparePixmapsErrorReporter>(
            [reporter, colorType](int x, int y, const float diffs[4]) {
                ERRORF(reporter,
                       "Error at (%d %d) in transfer, color type: %s, diffs: (%f, %f, %f, %f)",
                       x, y, GrColorTypeToStr(colorType),
                       diffs[0], diffs[1], diffs[2], diffs[3]);
            });
    GrImageInfo dstInfo(colorType, kUnpremul_SkAlphaType, nullptr, tex->dimensions());
    ComparePixels(src.extractSubset(SkIRect::MakeSize(kTexDims)), read, compareTolerances, error);

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
    auto subset = SkIRect::MakeXYWH(2, 10, 10, 2);
    size_t offset = subset.top()*src.rowBytes() +
                    subset.left()*GrColorTypeBytesPerPixel(allowedSrc.fColorType);
    while (offset % allowedSrc.fOffsetAlignmentForTransferBuffer) {
        offset += GrColorTypeBytesPerPixel(allowedSrc.fColorType);
        ++subset.fLeft;
        // In most cases we assume that the required alignment is 1 or a small multiple of the bpp,
        // which it is for color types across all current backends except Direct3D. To correct for
        // Direct3D's large alignment requirement we may adjust the top location as well.
        if (subset.right() > tex->width()) {
            subset.fLeft = 0;
            ++subset.fTop;
            offset = subset.fTop * srcRowBytes;
        }
        SkASSERT(subset.right() <= tex->width());
        SkASSERT(subset.bottom() <= tex->height());
    }

    // change color of subrectangle, but re-fill the whole buffer. We'll pick out the subrect
    // from the buffer in the transfer.
    fill_transfer_data(subset, src);
    data = buffer->map();
    memcpy(data, src.addr(), src.rowBytes()*src.height());
    buffer->unmap();

    if (!gpu->transferPixelsTo(tex.get(),
                               subset,
                               colorType,
                               allowedSrc.fColorType,
                               buffer,
                               offset,
                               src.rowBytes())) {
        ERRORF(reporter,
               "Could not do partial transfer to color type %s.",
               GrColorTypeToStr(colorType));
        return;
    }

    if (!read_pixels_from_texture(tex.get(), read, compareTolerances)) {
        ERRORF(reporter,
               "Could not read pixels from texture, color type: %s",
               GrColorTypeToStr(colorType));
        return;
    }
    ComparePixels(src.extractSubset(SkIRect::MakeSize(kTexDims)), read, compareTolerances, error);
}

void basic_transfer_from_test(skiatest::Reporter* reporter, const sk_gpu_test::ContextInfo& ctxInfo,
                              GrColorType colorType, GrRenderable renderable) {
    auto context = ctxInfo.directContext();
    auto caps = context->priv().caps();
    if (GrCaps::kNone_MapFlags == caps->mapBufferFlags()) {
        return;
    }

    auto resourceProvider = context->priv().resourceProvider();
    GrGpu* gpu = context->priv().getGpu();

    static constexpr SkISize kTexDims = {16, 16};

    // We'll do a full texture read into the buffer followed by a partial read. These values
    // describe the partial read subrect.
    const auto kPartialSubset = SkIRect::MakeXYWH(2, 10, 10, 2);

    // create texture
    auto format = context->priv().caps()->getDefaultBackendFormat(colorType, renderable);
    if (!format.isValid()) {
        return;
    }

    GrPixmap data = GrPixmap::Allocate(GrImageInfo(colorType,
                                                   kUnpremul_SkAlphaType,
                                                   nullptr,
                                                   kTexDims));
    fill_transfer_data(SkIRect::MakeSize(kTexDims), data);
    GrMipLevel level{data.addr(), data.rowBytes(), data.pixelStorage()};
    sk_sp<GrTexture> tex = resourceProvider->createTexture(data.dimensions(),
                                                           format,
                                                           data.colorType(),
                                                           renderable,
                                                           1,
                                                           SkBudgeted::kNo,
                                                           GrMipmapped::kNo,
                                                           GrProtected::kNo,
                                                           &level);
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
    if (!caps->isFormatAsColorTypeRenderable(colorType, tex->backendFormat())) {
        return;
    }

    // Create the transfer buffer.
    auto allowedRead =
            caps->supportedReadPixelsColorType(colorType, tex->backendFormat(), colorType);
    GrImageInfo readInfo(allowedRead.fColorType, data.alphaType(), nullptr, kTexDims);

    size_t bpp = GrColorTypeBytesPerPixel(allowedRead.fColorType);
    size_t fullBufferRowBytes = GrAlignTo(kTexDims.fWidth * bpp, caps->transferBufferAlignment());
    size_t partialBufferRowBytes =
            GrAlignTo(kPartialSubset.width() * bpp, caps->transferBufferAlignment());
    size_t offsetAlignment = allowedRead.fOffsetAlignmentForTransferBuffer;
    SkASSERT(offsetAlignment);

    size_t bufferSize = fullBufferRowBytes * kTexDims.fHeight;
    // Arbitrary starting offset for the partial read.
    static constexpr size_t kStartingOffset = 11;
    size_t partialReadOffset = kStartingOffset +
                               (offsetAlignment - kStartingOffset%offsetAlignment)%offsetAlignment;
    bufferSize = std::max(bufferSize,
                          partialReadOffset + partialBufferRowBytes*kPartialSubset.height());

    sk_sp<GrGpuBuffer> buffer = resourceProvider->createBuffer(bufferSize,
                                                               GrGpuBufferType::kXferGpuToCpu,
                                                               kDynamic_GrAccessPattern);
    REPORTER_ASSERT(reporter, buffer);
    if (!buffer) {
        return;
    }

    int expectedTransferCnt = 0;
    gpu->stats()->reset();

    //////////////////////////
    // transfer full data
    if (!gpu->transferPixelsFrom(tex.get(),
                                 SkIRect::MakeSize(kTexDims),
                                 colorType,
                                 allowedRead.fColorType,
                                 buffer,
                                 0)) {
        ERRORF(reporter,
               "Full transferPixelsFrom failed with color type %s.",
               GrColorTypeToStr(colorType));
        return;
    }
    ++expectedTransferCnt;

    if (context->priv().caps()->mapBufferFlags() & GrCaps::kAsyncRead_MapFlag) {
        gpu->submitToGpu(true);
    }

    // Copy the transfer buffer contents to a temporary so we can manipulate it.
    const auto* map = reinterpret_cast<const char*>(buffer->map());
    REPORTER_ASSERT(reporter, map);
    if (!map) {
        ERRORF(reporter, "Failed to map transfer buffer.");
        return;
    }
    GrCPixmap result(data.info().makeColorType(allowedRead.fColorType), map, fullBufferRowBytes);

    float tol[4];
    determine_tolerances(result.colorType(), data.colorType(), tol);
    auto error = std::function<ComparePixmapsErrorReporter>(
            [reporter, colorType](int x, int y, const float diffs[4]) {
                ERRORF(reporter,
                       "Error at (%d %d) in transfer, color type: %s, diffs: (%f, %f, %f, %f)",
                       x, y, GrColorTypeToStr(colorType),
                       diffs[0], diffs[1], diffs[2], diffs[3]);
            });
    ComparePixels(data, result, tol, error);
    buffer->unmap();

    ///////////////////////
    // Now test a partial read at an offset into the buffer.
    if (!gpu->transferPixelsFrom(tex.get(),
                                 kPartialSubset,
                                 colorType,
                                 allowedRead.fColorType,
                                 buffer,
                                 partialReadOffset)) {
        ERRORF(reporter,
               "Partial transferPixelsFrom failed with color type %s.",
               GrColorTypeToStr(colorType));
        return;
    }
    ++expectedTransferCnt;

    if (context->priv().caps()->mapBufferFlags() & GrCaps::kAsyncRead_MapFlag) {
        gpu->submitToGpu(true);
    }

    map = reinterpret_cast<const char*>(buffer->map());
    if (!map) {
        ERRORF(reporter, "Failed to map transfer buffer.");
        return;
    }
    GrImageInfo resultInfo = data.info().makeColorType(allowedRead.fColorType)
                                        .makeDimensions(kPartialSubset.size());
    result = GrCPixmap(resultInfo, map + partialReadOffset, partialBufferRowBytes);
    ComparePixels(data.extractSubset(kPartialSubset), result, tol, error);
    buffer->unmap();

#if GR_GPU_STATS
    REPORTER_ASSERT(reporter, gpu->stats()->transfersFromSurface() == expectedTransferCnt);
#else
    (void)expectedTransferCnt;
#endif
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(TransferPixelsToTextureTest, reporter, ctxInfo) {
    if (!ctxInfo.directContext()->priv().caps()->transferFromBufferToTextureSupport()) {
        return;
    }
    for (auto renderable : {GrRenderable::kNo, GrRenderable::kYes}) {
        for (auto colorType : {
                     GrColorType::kAlpha_8,
                     GrColorType::kBGR_565,
                     GrColorType::kABGR_4444,
                     GrColorType::kRGBA_8888,
                     GrColorType::kRGBA_8888_SRGB,
                     GrColorType::kRGB_888x,
                     GrColorType::kRG_88,
                     GrColorType::kBGRA_8888,
                     GrColorType::kRGBA_1010102,
                     GrColorType::kBGRA_1010102,
                     GrColorType::kGray_8,
                     GrColorType::kAlpha_F16,
                     GrColorType::kRGBA_F16,
                     GrColorType::kRGBA_F16_Clamped,
                     GrColorType::kRGBA_F32,
                     GrColorType::kAlpha_16,
                     GrColorType::kRG_1616,
                     GrColorType::kRGBA_16161616,
                     GrColorType::kRG_F16,
             }) {
            basic_transfer_to_test(reporter, ctxInfo.directContext(), colorType, renderable);
        }
    }
}

// TODO(bsalomon): Metal
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(TransferPixelsFromTextureTest, reporter, ctxInfo) {
    if (!ctxInfo.directContext()->priv().caps()->transferFromSurfaceToBufferSupport()) {
        return;
    }
    for (auto renderable : {GrRenderable::kNo, GrRenderable::kYes}) {
        for (auto colorType : {
                     GrColorType::kAlpha_8,
                     GrColorType::kAlpha_16,
                     GrColorType::kBGR_565,
                     GrColorType::kABGR_4444,
                     GrColorType::kRGBA_8888,
                     GrColorType::kRGBA_8888_SRGB,
                     GrColorType::kRGB_888x,
                     GrColorType::kRG_88,
                     GrColorType::kBGRA_8888,
                     GrColorType::kRGBA_1010102,
                     GrColorType::kBGRA_1010102,
                     GrColorType::kGray_8,
                     GrColorType::kAlpha_F16,
                     GrColorType::kRGBA_F16,
                     GrColorType::kRGBA_F16_Clamped,
                     GrColorType::kRGBA_F32,
                     GrColorType::kRG_1616,
                     GrColorType::kRGBA_16161616,
                     GrColorType::kRG_F16,
             }) {
            basic_transfer_from_test(reporter, ctxInfo, colorType, renderable);
        }
    }
}
