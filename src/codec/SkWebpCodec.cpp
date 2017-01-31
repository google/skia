/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCodecPriv.h"
#include "SkColorSpaceXform.h"
#include "SkSampler.h"
#include "SkStreamPriv.h"
#include "SkTemplates.h"
#include "SkWebpCodec.h"

// A WebP decoder on top of (subset of) libwebp
// For more information on WebP image format, and libwebp library, see:
//   https://code.google.com/speed/webp/
//   http://www.webmproject.org/code/#libwebp-webp-image-library
//   https://chromium.googlesource.com/webm/libwebp

// If moving libwebp out of skia source tree, path for webp headers must be
// updated accordingly. Here, we enforce using local copy in webp sub-directory.
#include "webp/decode.h"
#include "webp/demux.h"
#include "webp/encode.h"

bool SkWebpCodec::IsWebp(const void* buf, size_t bytesRead) {
    // WEBP starts with the following:
    // RIFFXXXXWEBPVP
    // Where XXXX is unspecified.
    const char* bytes = static_cast<const char*>(buf);
    return bytesRead >= 14 && !memcmp(bytes, "RIFF", 4) && !memcmp(&bytes[8], "WEBPVP", 6);
}

// Parse headers of RIFF container, and check for valid Webp (VP8) content.
// NOTE: This calls peek instead of read, since onGetPixels will need these
// bytes again.
// Returns an SkWebpCodec on success;
SkCodec* SkWebpCodec::NewFromStream(SkStream* stream) {
    std::unique_ptr<SkStream> streamDeleter(stream);

    // Webp demux needs a contiguous data buffer.
    sk_sp<SkData> data = nullptr;
    if (stream->getMemoryBase()) {
        // It is safe to make without copy because we'll hold onto the stream.
        data = SkData::MakeWithoutCopy(stream->getMemoryBase(), stream->getLength());
    } else {
        data = SkCopyStreamToData(stream);

        // If we are forced to copy the stream to a data, we can go ahead and delete the stream.
        streamDeleter.reset(nullptr);
    }

    // It's a little strange that the |demux| will outlive |webpData|, though it needs the
    // pointer in |webpData| to remain valid.  This works because the pointer remains valid
    // until the SkData is freed.
    WebPData webpData = { data->bytes(), data->size() };
    SkAutoTCallVProc<WebPDemuxer, WebPDemuxDelete> demux(WebPDemuxPartial(&webpData, nullptr));
    if (nullptr == demux) {
        return nullptr;
    }

    int width = WebPDemuxGetI(demux, WEBP_FF_CANVAS_WIDTH);
    int height = WebPDemuxGetI(demux, WEBP_FF_CANVAS_HEIGHT);

    // Sanity check for image size that's about to be decoded.
    {
        const int64_t size = sk_64_mul(width, height);
        if (!sk_64_isS32(size)) {
            return nullptr;
        }
        // now check that if we are 4-bytes per pixel, we also don't overflow
        if (sk_64_asS32(size) > (0x7FFFFFFF >> 2)) {
            return nullptr;
        }
    }

    WebPChunkIterator chunkIterator;
    SkAutoTCallVProc<WebPChunkIterator, WebPDemuxReleaseChunkIterator> autoCI(&chunkIterator);
    sk_sp<SkColorSpace> colorSpace = nullptr;
    bool unsupportedICC = false;
    if (WebPDemuxGetChunk(demux, "ICCP", 1, &chunkIterator)) {
        colorSpace = SkColorSpace::MakeICC(chunkIterator.chunk.bytes, chunkIterator.chunk.size);
        if (!colorSpace) {
            unsupportedICC = true;
        }
    }
    if (!colorSpace) {
        colorSpace = SkColorSpace::MakeNamed(SkColorSpace::kSRGB_Named);
    }

    // Get the dimensions and offset of the first frame.  Since we do not yet support animated
    // webp, this is the only frame that we will decode.
    WebPIterator frame;
    SkAutoTCallVProc<WebPIterator, WebPDemuxReleaseIterator> autoFrame(&frame);
    if (!WebPDemuxGetFrame(demux, 1, &frame)) {
        return nullptr;
    }

    WebPBitstreamFeatures features;
    VP8StatusCode status = WebPGetFeatures(frame.fragment.bytes, frame.fragment.size, &features);
    if (VP8_STATUS_OK != status) {
        return nullptr;
    }

    SkEncodedInfo::Color color;
    SkEncodedInfo::Alpha alpha;
    switch (features.format) {
        case 0:
            // This indicates a "mixed" format.  We would see this for
            // animated webps (multiple fragments).
            // I believe that this is a rare case.
            // We could also guess kYUV here, but I think it makes more
            // sense to guess kBGRA which is likely closer to the final
            // output.  Otherwise, we might end up converting
            // BGRA->YUVA->BGRA.
            color = SkEncodedInfo::kBGRA_Color;
            alpha = SkEncodedInfo::kUnpremul_Alpha;
            break;
        case 1:
            // This is the lossy format (YUV).
            if (SkToBool(features.has_alpha) || frame.width != width || frame.height != height) {
                color = SkEncodedInfo::kYUVA_Color;
                alpha = SkEncodedInfo::kUnpremul_Alpha;
            } else {
                color = SkEncodedInfo::kYUV_Color;
                alpha = SkEncodedInfo::kOpaque_Alpha;
            }
            break;
        case 2:
            // This is the lossless format (BGRA).
            color = SkEncodedInfo::kBGRA_Color;
            alpha = SkEncodedInfo::kUnpremul_Alpha;
            break;
        default:
            return nullptr;
    }

    SkEncodedInfo info = SkEncodedInfo::Make(color, alpha, 8);
    SkWebpCodec* codecOut = new SkWebpCodec(width, height, info, std::move(colorSpace),
                                            streamDeleter.release(), demux.release(),
                                            std::move(data));
    codecOut->setUnsupportedICC(unsupportedICC);
    return codecOut;
}

SkISize SkWebpCodec::onGetScaledDimensions(float desiredScale) const {
    SkISize dim = this->getInfo().dimensions();
    // SkCodec treats zero dimensional images as errors, so the minimum size
    // that we will recommend is 1x1.
    dim.fWidth = SkTMax(1, SkScalarRoundToInt(desiredScale * dim.fWidth));
    dim.fHeight = SkTMax(1, SkScalarRoundToInt(desiredScale * dim.fHeight));
    return dim;
}

bool SkWebpCodec::onDimensionsSupported(const SkISize& dim) {
    const SkImageInfo& info = this->getInfo();
    return dim.width() >= 1 && dim.width() <= info.width()
            && dim.height() >= 1 && dim.height() <= info.height();
}

static WEBP_CSP_MODE webp_decode_mode(SkColorType ct, bool premultiply) {
    switch (ct) {
        case kBGRA_8888_SkColorType:
            return premultiply ? MODE_bgrA : MODE_BGRA;
        case kRGBA_8888_SkColorType:
            return premultiply ? MODE_rgbA : MODE_RGBA;
        case kRGB_565_SkColorType:
            return MODE_RGB_565;
        default:
            return MODE_LAST;
    }
}

bool SkWebpCodec::onGetValidSubset(SkIRect* desiredSubset) const {
    if (!desiredSubset) {
        return false;
    }

    SkIRect dimensions  = SkIRect::MakeSize(this->getInfo().dimensions());
    if (!dimensions.contains(*desiredSubset)) {
        return false;
    }

    // As stated below, libwebp snaps to even left and top. Make sure top and left are even, so we
    // decode this exact subset.
    // Leave right and bottom unmodified, so we suggest a slightly larger subset than requested.
    desiredSubset->fLeft = (desiredSubset->fLeft >> 1) << 1;
    desiredSubset->fTop = (desiredSubset->fTop >> 1) << 1;
    return true;
}

SkCodec::Result SkWebpCodec::onGetPixels(const SkImageInfo& dstInfo, void* dst, size_t rowBytes,
                                         const Options& options, SkPMColor*, int*,
                                         int* rowsDecodedPtr) {
    if (!conversion_possible(dstInfo, this->getInfo())) {
        return kInvalidConversion;
    }

    if (!this->initializeColorXform(dstInfo)) {
        return kInvalidConversion;
    }

    WebPDecoderConfig config;
    if (0 == WebPInitDecoderConfig(&config)) {
        // ABI mismatch.
        // FIXME: New enum for this?
        return kInvalidInput;
    }

    // Free any memory associated with the buffer. Must be called last, so we declare it first.
    SkAutoTCallVProc<WebPDecBuffer, WebPFreeDecBuffer> autoFree(&(config.output));

    WebPIterator frame;
    SkAutoTCallVProc<WebPIterator, WebPDemuxReleaseIterator> autoFrame(&frame);
    // If this succeeded in NewFromStream(), it should succeed again here.
    SkAssertResult(WebPDemuxGetFrame(fDemux, 1, &frame));

    // Get the frameRect.  libwebp will have already signaled an error if this is not fully
    // contained by the canvas.
    SkIRect frameRect =
            SkIRect::MakeXYWH(frame.x_offset, frame.y_offset, frame.width, frame.height);
            printf("frame %d %d %d %d\n", frame.x_offset, frame.y_offset, frame.width, frame.height);
    SkASSERT(SkIRect::MakeWH(this->getInfo().width(), this->getInfo().height())
            .contains(frameRect));
    if (frameRect.width() != this->getInfo().width() ||
        frameRect.height() != this->getInfo().height())
    {
        SkSampler::Fill(dstInfo, dst, rowBytes, 0, options.fZeroInitialized);
    }

    int frameX = frameRect.x();
    int frameY = frameRect.y();
    SkIRect bounds = SkIRect::MakeSize(this->getInfo().dimensions());
    int unscaledWidth = bounds.width();
    int unscaledHeight = bounds.height();
    if (options.fSubset) {
        // Caller is requesting a subset.
        if (!bounds.contains(*options.fSubset)) {
            // The subset is out of bounds.
            return kInvalidParameters;
        }

        bounds = *options.fSubset;
        unscaledWidth = bounds.width();
        unscaledHeight = bounds.height();

        // This is tricky. libwebp snaps the top and left to even values. We could let libwebp
        // do the snap, and return a subset which is a different one than requested. The problem
        // with that approach is that the caller may try to stitch subsets together, and if we
        // returned different subsets than requested, there would be artifacts at the boundaries.
        // Instead, we report that we cannot support odd values for top and left.
        if (!SkIsAlign2(bounds.fLeft) || !SkIsAlign2(bounds.fTop)) {
            return kInvalidParameters;
        }

        if (!SkIRect::IntersectsNoEmptyCheck(bounds, frameRect)) {
            return kSuccess;
        }

        int minXOffset = SkTMin(frameX, bounds.x());
        int minYOffset = SkTMin(frameY, bounds.y());
        printf("frameX %d frameY %d\n", frameX, frameY);
        frameX -= minXOffset;
        frameY -= minYOffset;
        printf("frameX %d frameY %d\n", frameX, frameY);
        bounds.offset(-minXOffset, -minYOffset);
        bounds.fRight = SkTMin(bounds.fRight, frameRect.fRight);
        bounds.fBottom = SkTMin(bounds.fBottom, frameRect.fBottom);
        SkDebugf("l %d t %d w %d h %d\n", bounds.fLeft, bounds.fTop, bounds.width(), bounds.height());
        SkASSERT(SkIsAlign2(bounds.fLeft) && SkIsAlign2(bounds.fTop));

#ifdef SK_DEBUG
        {
            // Make a copy, since getValidSubset can change its input.
            SkIRect subset(bounds);
            // That said, getValidSubset should *not* change its input, in this case; otherwise
            // getValidSubset does not match the actual subsets we can do.
            SkASSERT(this->getValidSubset(&subset) && subset == bounds);
        }
#endif

        config.options.use_cropping = 1;
        config.options.crop_left = bounds.fLeft;
        config.options.crop_top = bounds.fTop;
        config.options.crop_width = bounds.width();
        config.options.crop_height = bounds.height();
        SkDebugf("l %d t %d w %d h %d\n", bounds.fLeft, bounds.fTop, bounds.width(), bounds.height());
    }

    int scaledWidth = bounds.width();
    SkISize dstDimensions = dstInfo.dimensions();
    if (bounds.size() != dstDimensions) {
        // Caller is requesting scaling.
        config.options.use_scaling = 1;

        float scaleX = (float) dstDimensions.width() / (float) unscaledWidth;
        SkDebugf("%d %d %g\n", dstDimensions.width(), bounds.width(), scaleX);
        scaledWidth = SkScalarRoundToInt(scaleX * (float) frameRect.width());
        frameX = (int) (scaleX * (float) frameX);

        float scaleY = (float) dstDimensions.height() / (float) unscaledHeight;
        int scaledHeight = SkScalarRoundToInt(scaleY * (float) frameRect.height());
        frameY = (int) (scaleY * (float) frameY);
        
        config.options.scaled_width = scaledWidth;
        config.options.scaled_height = scaledHeight;
    }

    // Swizzling between RGBA and BGRA is zero cost in a color transform.  So when we have a
    // color transform, we should decode to whatever is easiest for libwebp, and then let the
    // color transform swizzle if necessary.
    // Lossy webp is encoded as YUV (so RGBA and BGRA are the same cost).  Lossless webp is
    // encoded as BGRA. This means decoding to BGRA is either faster or the same cost as RGBA.
    config.output.colorspace = this->colorXform() ? MODE_BGRA :
            webp_decode_mode(dstInfo.colorType(), dstInfo.alphaType() == kPremul_SkAlphaType);
    config.output.is_external_memory = 1;

    // We will decode the entire image and then perform the color transform.  libwebp
    // does not provide a row-by-row API.  This is a shame particularly in the F16 case,
    // where we need to allocate an extra image-sized buffer.
    SkAutoTMalloc<uint32_t> pixels;
    bool isF16 = kRGBA_F16_SkColorType == dstInfo.colorType();
    if (isF16) {
        pixels.reset(dstDimensions.width() * dstDimensions.height());
        size_t tmpRowBytes = dstDimensions.width() * sizeof(uint32_t);
        size_t pixelOffset = frameX * sizeof(uint32_t) + frameY * tmpRowBytes;
        config.output.u.RGBA.rgba = SkTAddOffset<uint8_t>(pixels.get(), pixelOffset);
        config.output.u.RGBA.stride = (int) tmpRowBytes;
        config.output.u.RGBA.size = tmpRowBytes * dstDimensions.height() - pixelOffset;
    } else {
        size_t pixelOffset =
                frameX * SkColorTypeBytesPerPixel(dstInfo.colorType()) + frameY * rowBytes;
        config.output.u.RGBA.rgba = SkTAddOffset<uint8_t>(dst, pixelOffset);
        config.output.u.RGBA.stride = (int) rowBytes;
        config.output.u.RGBA.size = dstInfo.getSafeSize(rowBytes) - pixelOffset;
    }

    SkAutoTCallVProc<WebPIDecoder, WebPIDelete> idec(WebPIDecode(nullptr, 0, &config));
    if (!idec) {
        return kInvalidInput;
    }

    int rowsDecoded;
    SkCodec::Result result;
    switch (WebPIUpdate(idec, frame.fragment.bytes, frame.fragment.size)) {
        case VP8_STATUS_OK:
            rowsDecoded = dstInfo.height();
            result = kSuccess;
            break;
        case VP8_STATUS_SUSPENDED:
            WebPIDecGetRGB(idec, rowsDecodedPtr, nullptr, nullptr, nullptr);
            rowsDecoded = frameY + *rowsDecodedPtr;
            result = kIncompleteInput;
            break;
        default:
        SkDebugf("OR WORSE\n");
            return kInvalidInput;
    }

    if (this->colorXform()) {
        SkColorSpaceXform::ColorFormat dstColorFormat = select_xform_format(dstInfo.colorType());
        SkAlphaType xformAlphaType = select_xform_alpha(dstInfo.alphaType(),
                                                        this->getInfo().alphaType());

        uint32_t* src = (uint32_t*) (isF16 ? pixels.get() : dst);
        size_t srcRowBytes = config.output.u.RGBA.stride;
        for (int y = 0; y < rowsDecoded; y++) {
            SkAssertResult(this->colorXform()->apply(dstColorFormat, dst,
                    SkColorSpaceXform::kBGRA_8888_ColorFormat, src, scaledWidth,
                    xformAlphaType));
            dst = SkTAddOffset<void>(dst, rowBytes);
            src = SkTAddOffset<uint32_t>(src, srcRowBytes);
        }
    }

    return result;
}

SkWebpCodec::SkWebpCodec(int width, int height, const SkEncodedInfo& info,
                         sk_sp<SkColorSpace> colorSpace, SkStream* stream, WebPDemuxer* demux,
                         sk_sp<SkData> data)
    : INHERITED(width, height, info, stream, std::move(colorSpace))
    , fDemux(demux)
    , fData(std::move(data))
{}
