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

    const int width = WebPDemuxGetI(demux, WEBP_FF_CANVAS_WIDTH);
    const int height = WebPDemuxGetI(demux, WEBP_FF_CANVAS_HEIGHT);

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
        colorSpace = SkColorSpace::MakeSRGB();
    }

    // Get the first frame and its "features" to determine the color and alpha types.
    // Since we do not yet support animated webp, this is the only frame that we will
    // decode.
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
            // This indicates a "mixed" format.  We could see this for
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
    if (!conversion_possible(dstInfo, this->getInfo()) ||
        !this->initializeColorXform(dstInfo, options.fPremulBehavior))
    {
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
    auto frameRect = SkIRect::MakeXYWH(frame.x_offset, frame.y_offset, frame.width, frame.height);
    SkASSERT(this->getInfo().bounds().contains(frameRect));
    bool frameIsSubset = frameRect.size() != this->getInfo().dimensions();
    if (frameIsSubset) {
        SkSampler::Fill(dstInfo, dst, rowBytes, 0, options.fZeroInitialized);
    }

    int dstX = frameRect.x();
    int dstY = frameRect.y();
    int subsetWidth = frameRect.width();
    int subsetHeight = frameRect.height();
    if (options.fSubset) {
        SkIRect subset = *options.fSubset;
        SkASSERT(this->getInfo().bounds().contains(subset));
        SkASSERT(SkIsAlign2(subset.fLeft) && SkIsAlign2(subset.fTop));
        SkASSERT(this->getValidSubset(&subset) && subset == *options.fSubset);

        if (!SkIRect::IntersectsNoEmptyCheck(subset, frameRect)) {
            return kSuccess;
        }

        int minXOffset = SkTMin(dstX, subset.x());
        int minYOffset = SkTMin(dstY, subset.y());
        dstX -= minXOffset;
        dstY -= minYOffset;
        frameRect.offset(-minXOffset, -minYOffset);
        subset.offset(-minXOffset, -minYOffset);

        // Just like we require that the requested subset x and y offset are even, libwebp
        // guarantees that the frame x and y offset are even (it's actually impossible to specify
        // an odd frame offset).  So we can still guarantee that the adjusted offsets are even.
        SkASSERT(SkIsAlign2(subset.fLeft) && SkIsAlign2(subset.fTop));

        SkIRect intersection;
        SkAssertResult(intersection.intersect(frameRect, subset));
        subsetWidth = intersection.width();
        subsetHeight = intersection.height();

        config.options.use_cropping = 1;
        config.options.crop_left = subset.x();
        config.options.crop_top = subset.y();
        config.options.crop_width = subsetWidth;
        config.options.crop_height = subsetHeight;
    }

    // Ignore the frame size and offset when determining if scaling is necessary.
    int scaledWidth = subsetWidth;
    int scaledHeight = subsetHeight;
    SkISize srcSize = options.fSubset ? options.fSubset->size() : this->getInfo().dimensions();
    if (srcSize != dstInfo.dimensions()) {
        config.options.use_scaling = 1;

        if (frameIsSubset) {
            float scaleX = ((float) dstInfo.width()) / srcSize.width();
            float scaleY = ((float) dstInfo.height()) / srcSize.height();

            // We need to be conservative here and floor rather than round.
            // Otherwise, we may find ourselves decoding off the end of memory.
            dstX = scaleX * dstX;
            scaledWidth = scaleX * scaledWidth;
            dstY = scaleY * dstY;
            scaledHeight = scaleY * scaledHeight;
            if (0 == scaledWidth || 0 == scaledHeight) {
                return kSuccess;
            }
        } else {
            scaledWidth = dstInfo.width();
            scaledHeight = dstInfo.height();
        }

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
    // does not provide a row-by-row API.  This is a shame particularly when we do not want
    // 8888, since we will need to create another image sized buffer.
    SkAutoTMalloc<uint32_t> pixels;
    bool needsCopy = this->colorXform() && kRGBA_8888_SkColorType != dstInfo.colorType() &&
                                           kBGRA_8888_SkColorType != dstInfo.colorType();
    void* webpDst = needsCopy ? pixels.reset(dstInfo.width() * dstInfo.height()) : dst;
    size_t webpRowBytes = needsCopy ? dstInfo.width() * sizeof(uint32_t) : rowBytes;
    size_t totalBytes = needsCopy ? webpRowBytes * dstInfo.height()
                                  : dstInfo.getSafeSize(webpRowBytes);
    size_t dstBpp = SkColorTypeBytesPerPixel(dstInfo.colorType());
    size_t webpBpp = needsCopy ? sizeof(uint32_t) : dstBpp;

    size_t offset = dstX * webpBpp + dstY * webpRowBytes;
    config.output.u.RGBA.rgba = SkTAddOffset<uint8_t>(webpDst, offset);
    config.output.u.RGBA.stride = (int) webpRowBytes;
    config.output.u.RGBA.size = totalBytes - offset;

    SkAutoTCallVProc<WebPIDecoder, WebPIDelete> idec(WebPIDecode(nullptr, 0, &config));
    if (!idec) {
        return kInvalidInput;
    }

    int rowsDecoded;
    SkCodec::Result result;
    switch (WebPIUpdate(idec, frame.fragment.bytes, frame.fragment.size)) {
        case VP8_STATUS_OK:
            rowsDecoded = scaledHeight;
            result = kSuccess;
            break;
        case VP8_STATUS_SUSPENDED:
            WebPIDecGetRGB(idec, &rowsDecoded, nullptr, nullptr, nullptr);
            *rowsDecodedPtr = rowsDecoded + dstY;
            result = kIncompleteInput;
            break;
        default:
            return kInvalidInput;
    }

    if (this->colorXform()) {
        SkColorSpaceXform::ColorFormat dstColorFormat = select_xform_format(dstInfo.colorType());
        SkAlphaType xformAlphaType = select_xform_alpha(dstInfo.alphaType(),
                                                        this->getInfo().alphaType());

        uint32_t* xformSrc = (uint32_t*) config.output.u.RGBA.rgba;
        void* xformDst = SkTAddOffset<void>(dst, dstBpp * dstX + rowBytes * dstY);
        size_t srcRowBytes = config.output.u.RGBA.stride;
        for (int y = 0; y < rowsDecoded; y++) {
            SkAssertResult(this->colorXform()->apply(dstColorFormat, xformDst,
                    SkColorSpaceXform::kBGRA_8888_ColorFormat, xformSrc, scaledWidth,
                    xformAlphaType));
            xformDst = SkTAddOffset<void>(xformDst, rowBytes);
            xformSrc = SkTAddOffset<uint32_t>(xformSrc, srcRowBytes);
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
