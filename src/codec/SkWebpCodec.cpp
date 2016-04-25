/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCodecPriv.h"
#include "SkWebpCodec.h"
#include "SkTemplates.h"

// A WebP decoder on top of (subset of) libwebp
// For more information on WebP image format, and libwebp library, see:
//   https://code.google.com/speed/webp/
//   http://www.webmproject.org/code/#libwebp-webp-image-library
//   https://chromium.googlesource.com/webm/libwebp

// If moving libwebp out of skia source tree, path for webp headers must be
// updated accordingly. Here, we enforce using local copy in webp sub-directory.
#include "webp/decode.h"
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
    SkAutoTDelete<SkStream> streamDeleter(stream);

    unsigned char buffer[WEBP_VP8_HEADER_SIZE];
    SkASSERT(WEBP_VP8_HEADER_SIZE <= SkCodec::MinBufferedBytesNeeded());

    const size_t bytesPeeked = stream->peek(buffer, WEBP_VP8_HEADER_SIZE);
    if (bytesPeeked != WEBP_VP8_HEADER_SIZE) {
        // Use read + rewind as a backup
        if (stream->read(buffer, WEBP_VP8_HEADER_SIZE) != WEBP_VP8_HEADER_SIZE
            || !stream->rewind())
        return nullptr;
    }

    WebPBitstreamFeatures features;
    VP8StatusCode status = WebPGetFeatures(buffer, WEBP_VP8_HEADER_SIZE, &features);
    if (VP8_STATUS_OK != status) {
        return nullptr; // Invalid WebP file.
    }

    // sanity check for image size that's about to be decoded.
    {
        const int64_t size = sk_64_mul(features.width, features.height);
        if (!sk_64_isS32(size)) {
            return nullptr;
        }
        // now check that if we are 4-bytes per pixel, we also don't overflow
        if (sk_64_asS32(size) > (0x7FFFFFFF >> 2)) {
            return nullptr;
        }
    }

    SkEncodedInfo::Color color;
    SkEncodedInfo::Alpha alpha;
    switch (features.format) {
        case 0:
            // This indicates a "mixed" format.  We would see this for
            // animated webps or for webps encoded in multiple fragments.
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
            if (SkToBool(features.has_alpha)) {
                color = SkEncodedInfo::kYUVA_Color;
                alpha = SkEncodedInfo::kUnpremul_Alpha;
            } else {
                color = SkEncodedInfo::kYUV_Color;
                alpha = SkEncodedInfo::kOpaque_Alpha;
            }
            break;
        case 2:
            // This is the lossless format (BGRA).
            // FIXME: Should we check the has_alpha flag here?  It looks
            //        like the image is encoded with an alpha channel
            //        regardless of whether or not the alpha flag is set.
            color = SkEncodedInfo::kBGRA_Color;
            alpha = SkEncodedInfo::kUnpremul_Alpha;
            break;
        default:
            return nullptr;
    }

    SkEncodedInfo info = SkEncodedInfo::Make(color, alpha, 8);
    return new SkWebpCodec(features.width, features.height, info, streamDeleter.release());
}

// This version is slightly different from SkCodecPriv's version of conversion_possible. It
// supports both byte orders for 8888.
static bool webp_conversion_possible(const SkImageInfo& dst, const SkImageInfo& src) {
    // FIXME: skbug.com/4895
    // Currently, we ignore the SkColorProfileType on the SkImageInfo.  We
    // will treat the encoded data as linear regardless of what the client
    // requests.

    if (!valid_alpha(dst.alphaType(), src.alphaType())) {
        return false;
    }

    switch (dst.colorType()) {
        // Both byte orders are supported.
        case kBGRA_8888_SkColorType:
        case kRGBA_8888_SkColorType:
            return true;
        case kRGB_565_SkColorType:
            return src.alphaType() == kOpaque_SkAlphaType;
        default:
            return false;
    }
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

// The WebP decoding API allows us to incrementally pass chunks of bytes as we receive them to the
// decoder with WebPIAppend. In order to do so, we need to read chunks from the SkStream. This size
// is arbitrary.
static const size_t BUFFER_SIZE = 4096;

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
                                         int* rowsDecoded) {
    if (!webp_conversion_possible(dstInfo, this->getInfo())) {
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

    SkIRect bounds = SkIRect::MakeSize(this->getInfo().dimensions());
    if (options.fSubset) {
        // Caller is requesting a subset.
        if (!bounds.contains(*options.fSubset)) {
            // The subset is out of bounds.
            return kInvalidParameters;
        }

        bounds = *options.fSubset;

        // This is tricky. libwebp snaps the top and left to even values. We could let libwebp
        // do the snap, and return a subset which is a different one than requested. The problem
        // with that approach is that the caller may try to stitch subsets together, and if we
        // returned different subsets than requested, there would be artifacts at the boundaries.
        // Instead, we report that we cannot support odd values for top and left..
        if (!SkIsAlign2(bounds.fLeft) || !SkIsAlign2(bounds.fTop)) {
            return kInvalidParameters;
        }

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
    }

    SkISize dstDimensions = dstInfo.dimensions();
    if (bounds.size() != dstDimensions) {
        // Caller is requesting scaling.
        config.options.use_scaling = 1;
        config.options.scaled_width = dstDimensions.width();
        config.options.scaled_height = dstDimensions.height();
    }

    config.output.colorspace = webp_decode_mode(dstInfo.colorType(),
            dstInfo.alphaType() == kPremul_SkAlphaType);
    config.output.u.RGBA.rgba = (uint8_t*) dst;
    config.output.u.RGBA.stride = (int) rowBytes;
    config.output.u.RGBA.size = dstInfo.getSafeSize(rowBytes);
    config.output.is_external_memory = 1;

    SkAutoTCallVProc<WebPIDecoder, WebPIDelete> idec(WebPIDecode(nullptr, 0, &config));
    if (!idec) {
        return kInvalidInput;
    }

    SkAutoTMalloc<uint8_t> storage(BUFFER_SIZE);
    uint8_t* buffer = storage.get();
    while (true) {
        const size_t bytesRead = stream()->read(buffer, BUFFER_SIZE);
        if (0 == bytesRead) {
            WebPIDecGetRGB(idec, rowsDecoded, NULL, NULL, NULL);
            return kIncompleteInput;
        }

        switch (WebPIAppend(idec, buffer, bytesRead)) {
            case VP8_STATUS_OK:
                return kSuccess;
            case VP8_STATUS_SUSPENDED:
                // Break out of the switch statement. Continue the loop.
                break;
            default:
                return kInvalidInput;
        }
    }
}

SkWebpCodec::SkWebpCodec(int width, int height, const SkEncodedInfo& info, SkStream* stream)
    // The spec says an unmarked image is sRGB, so we return that space here.
    // TODO: Add support for parsing ICC profiles from webps.
    : INHERITED(width, height, info, stream, SkColorSpace::NewNamed(SkColorSpace::kSRGB_Named)) {}
