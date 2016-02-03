/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCodec.h"
#include "SkJpegCodec.h"
#include "SkJpegDecoderMgr.h"
#include "SkJpegUtility_codec.h"
#include "SkCodecPriv.h"
#include "SkColorPriv.h"
#include "SkStream.h"
#include "SkTemplates.h"
#include "SkTypes.h"

// stdio is needed for libjpeg-turbo
#include <stdio.h>

extern "C" {
    #include "jerror.h"
    #include "jpeglib.h"
}

bool SkJpegCodec::IsJpeg(const void* buffer, size_t bytesRead) {
    static const uint8_t jpegSig[] = { 0xFF, 0xD8, 0xFF };
    return bytesRead >= 3 && !memcmp(buffer, jpegSig, sizeof(jpegSig));
}

bool SkJpegCodec::ReadHeader(SkStream* stream, SkCodec** codecOut,
        JpegDecoderMgr** decoderMgrOut) {

    // Create a JpegDecoderMgr to own all of the decompress information
    SkAutoTDelete<JpegDecoderMgr> decoderMgr(new JpegDecoderMgr(stream));

    // libjpeg errors will be caught and reported here
    if (setjmp(decoderMgr->getJmpBuf())) {
        return decoderMgr->returnFalse("setjmp");
    }

    // Initialize the decompress info and the source manager
    decoderMgr->init();

    // Read the jpeg header
    if (JPEG_HEADER_OK != jpeg_read_header(decoderMgr->dinfo(), true)) {
        return decoderMgr->returnFalse("read_header");
    }

    if (nullptr != codecOut) {
        // Recommend the color type to decode to
        const SkColorType colorType = decoderMgr->getColorType();

        // Create image info object and the codec
        const SkImageInfo& imageInfo = SkImageInfo::Make(decoderMgr->dinfo()->image_width,
                decoderMgr->dinfo()->image_height, colorType, kOpaque_SkAlphaType);
        *codecOut = new SkJpegCodec(imageInfo, stream, decoderMgr.detach());
    } else {
        SkASSERT(nullptr != decoderMgrOut);
        *decoderMgrOut = decoderMgr.detach();
    }
    return true;
}

SkCodec* SkJpegCodec::NewFromStream(SkStream* stream) {
    SkAutoTDelete<SkStream> streamDeleter(stream);
    SkCodec* codec = nullptr;
    if (ReadHeader(stream,  &codec, nullptr)) {
        // Codec has taken ownership of the stream, we do not need to delete it
        SkASSERT(codec);
        streamDeleter.detach();
        return codec;
    }
    return nullptr;
}

SkJpegCodec::SkJpegCodec(const SkImageInfo& srcInfo, SkStream* stream,
        JpegDecoderMgr* decoderMgr)
    : INHERITED(srcInfo, stream)
    , fDecoderMgr(decoderMgr)
    , fReadyState(decoderMgr->dinfo()->global_state)
{}

/*
 * Return the row bytes of a particular image type and width
 */
static int get_row_bytes(const j_decompress_ptr dinfo) {
    int colorBytes = (dinfo->out_color_space == JCS_RGB565) ? 2 : dinfo->out_color_components;
    return dinfo->output_width * colorBytes;

}

/*
 *  Calculate output dimensions based on the provided factors.
 *
 *  Not to be used on the actual jpeg_decompress_struct used for decoding, since it will
 *  incorrectly modify num_components.
 */
void calc_output_dimensions(jpeg_decompress_struct* dinfo, unsigned int num, unsigned int denom) {
    dinfo->num_components = 0;
    dinfo->scale_num = num;
    dinfo->scale_denom = denom;
    jpeg_calc_output_dimensions(dinfo);
}

/*
 * Return a valid set of output dimensions for this decoder, given an input scale
 */
SkISize SkJpegCodec::onGetScaledDimensions(float desiredScale) const {
    // libjpeg-turbo supports scaling by 1/8, 1/4, 3/8, 1/2, 5/8, 3/4, 7/8, and 1/1, so we will
    // support these as well
    unsigned int num;
    unsigned int denom = 8;
    if (desiredScale >= 0.9375) {
        num = 8;
    } else if (desiredScale >= 0.8125) {
        num = 7;
    } else if (desiredScale >= 0.6875f) {
        num = 6;
    } else if (desiredScale >= 0.5625f) {
        num = 5;
    } else if (desiredScale >= 0.4375f) {
        num = 4;
    } else if (desiredScale >= 0.3125f) {
        num = 3;
    } else if (desiredScale >= 0.1875f) {
        num = 2;
    } else {
        num = 1;
    }

    // Set up a fake decompress struct in order to use libjpeg to calculate output dimensions
    jpeg_decompress_struct dinfo;
    sk_bzero(&dinfo, sizeof(dinfo));
    dinfo.image_width = this->getInfo().width();
    dinfo.image_height = this->getInfo().height();
    dinfo.global_state = fReadyState;
    calc_output_dimensions(&dinfo, num, denom);

    // Return the calculated output dimensions for the given scale
    return SkISize::Make(dinfo.output_width, dinfo.output_height);
}

bool SkJpegCodec::onRewind() {
    JpegDecoderMgr* decoderMgr = nullptr;
    if (!ReadHeader(this->stream(), nullptr, &decoderMgr)) {
        return fDecoderMgr->returnFalse("could not rewind");
    }
    SkASSERT(nullptr != decoderMgr);
    fDecoderMgr.reset(decoderMgr);
    return true;
}

/*
 * Checks if the conversion between the input image and the requested output
 * image has been implemented
 * Sets the output color space
 */
bool SkJpegCodec::setOutputColorSpace(const SkImageInfo& dst) {
    const SkImageInfo& src = this->getInfo();

    // Ensure that the profile type is unchanged
    if (dst.profileType() != src.profileType()) {
        return false;
    }

    // Ensure that the alpha type is opaque
    if (kOpaque_SkAlphaType != dst.alphaType()) {
        return false;
    }

    // Check if we will decode to CMYK because a conversion to RGBA is not supported
    J_COLOR_SPACE colorSpace = fDecoderMgr->dinfo()->jpeg_color_space;
    bool isCMYK = JCS_CMYK == colorSpace || JCS_YCCK == colorSpace;

    // Check for valid color types and set the output color space
    switch (dst.colorType()) {
        case kN32_SkColorType:
            if (isCMYK) {
                fDecoderMgr->dinfo()->out_color_space = JCS_CMYK;
            } else {
                // Check the byte ordering of the RGBA color space for the
                // current platform
#if defined(SK_PMCOLOR_IS_RGBA)
                fDecoderMgr->dinfo()->out_color_space = JCS_EXT_RGBA;
#else
                fDecoderMgr->dinfo()->out_color_space = JCS_EXT_BGRA;
#endif
            }
            return true;
        case kRGB_565_SkColorType:
            if (isCMYK) {
                fDecoderMgr->dinfo()->out_color_space = JCS_CMYK;
            } else {
                fDecoderMgr->dinfo()->dither_mode = JDITHER_NONE;
                fDecoderMgr->dinfo()->out_color_space = JCS_RGB565;
            }
            return true;
        case kGray_8_SkColorType:
            if (isCMYK) {
                return false;
            } else {
                // We will enable decodes to gray even if the image is color because this is
                // much faster than decoding to color and then converting
                fDecoderMgr->dinfo()->out_color_space = JCS_GRAYSCALE;
            }
            return true;
        default:
            return false;
    }
}

/*
 * Checks if we can natively scale to the requested dimensions and natively scales the 
 * dimensions if possible
 */
bool SkJpegCodec::onDimensionsSupported(const SkISize& size) {
    if (setjmp(fDecoderMgr->getJmpBuf())) {
        return fDecoderMgr->returnFalse("onDimensionsSupported/setjmp");
    }

    const unsigned int dstWidth = size.width();
    const unsigned int dstHeight = size.height();

    // Set up a fake decompress struct in order to use libjpeg to calculate output dimensions
    // FIXME: Why is this necessary?
    jpeg_decompress_struct dinfo;
    sk_bzero(&dinfo, sizeof(dinfo));
    dinfo.image_width = this->getInfo().width();
    dinfo.image_height = this->getInfo().height();
    dinfo.global_state = fReadyState;

    // libjpeg-turbo can scale to 1/8, 1/4, 3/8, 1/2, 5/8, 3/4, 7/8, and 1/1
    unsigned int num = 8;
    const unsigned int denom = 8;
    calc_output_dimensions(&dinfo, num, denom);
    while (dinfo.output_width != dstWidth || dinfo.output_height != dstHeight) {

        // Return a failure if we have tried all of the possible scales
        if (1 == num || dstWidth > dinfo.output_width || dstHeight > dinfo.output_height) {
            return false;
        }

        // Try the next scale
        num -= 1;
        calc_output_dimensions(&dinfo, num, denom);
    }

    fDecoderMgr->dinfo()->scale_num = num;
    fDecoderMgr->dinfo()->scale_denom = denom;
    return true;
}

/*
 * Performs the jpeg decode
 */
SkCodec::Result SkJpegCodec::onGetPixels(const SkImageInfo& dstInfo,
                                         void* dst, size_t dstRowBytes,
                                         const Options& options, SkPMColor*, int*,
                                         int* rowsDecoded) {
    if (options.fSubset) {
        // Subsets are not supported.
        return kUnimplemented;
    }

    // Get a pointer to the decompress info since we will use it quite frequently
    jpeg_decompress_struct* dinfo = fDecoderMgr->dinfo();

    // Set the jump location for libjpeg errors
    if (setjmp(fDecoderMgr->getJmpBuf())) {
        return fDecoderMgr->returnFailure("setjmp", kInvalidInput);
    }

    // Check if we can decode to the requested destination and set the output color space
    if (!this->setOutputColorSpace(dstInfo)) {
        return fDecoderMgr->returnFailure("conversion_possible", kInvalidConversion);
    }

    // Now, given valid output dimensions, we can start the decompress
    if (!jpeg_start_decompress(dinfo)) {
        return fDecoderMgr->returnFailure("startDecompress", kInvalidInput);
    }

    // The recommended output buffer height should always be 1 in high quality modes.
    // If it's not, we want to know because it means our strategy is not optimal.
    SkASSERT(1 == dinfo->rec_outbuf_height);

    if (JCS_CMYK == dinfo->out_color_space) {
        this->initializeSwizzler(dstInfo, options);
    }

    // Perform the decode a single row at a time
    uint32_t dstHeight = dstInfo.height();

    JSAMPLE* dstRow;
    if (fSwizzler) {
        // write data to storage row, then sample using swizzler
        dstRow = fSrcRow;
    } else {
        // write data directly to dst
        dstRow = (JSAMPLE*) dst;
    }

    for (uint32_t y = 0; y < dstHeight; y++) {
        // Read rows of the image
        uint32_t lines = jpeg_read_scanlines(dinfo, &dstRow, 1);

        // If we cannot read enough rows, assume the input is incomplete
        if (lines != 1) {
            *rowsDecoded = y;

            return fDecoderMgr->returnFailure("Incomplete image data", kIncompleteInput);
        }

        if (fSwizzler) {
            // use swizzler to sample row
            fSwizzler->swizzle(dst, dstRow);
            dst = SkTAddOffset<JSAMPLE>(dst, dstRowBytes);
        } else {
            dstRow = SkTAddOffset<JSAMPLE>(dstRow, dstRowBytes);
        }
    }

    return kSuccess;
}

void SkJpegCodec::initializeSwizzler(const SkImageInfo& dstInfo, const Options& options) {
    SkSwizzler::SrcConfig srcConfig = SkSwizzler::kUnknown;
    if (JCS_CMYK == fDecoderMgr->dinfo()->out_color_space) {
        srcConfig = SkSwizzler::kCMYK;
    } else {
        switch (dstInfo.colorType()) {
            case kGray_8_SkColorType:
                srcConfig = SkSwizzler::kGray;
                break;
            case kRGBA_8888_SkColorType:
                srcConfig = SkSwizzler::kRGBX;
                break;
            case kBGRA_8888_SkColorType:
                srcConfig = SkSwizzler::kBGRX;
                break;
            case kRGB_565_SkColorType:
                srcConfig = SkSwizzler::kRGB_565;
                break;
            default:
                // This function should only be called if the colorType is supported by jpeg
                SkASSERT(false);
        }
    }

    fSwizzler.reset(SkSwizzler::CreateSwizzler(srcConfig, nullptr, dstInfo, options));
    fStorage.reset(get_row_bytes(fDecoderMgr->dinfo()));
    fSrcRow = fStorage.get();
}

SkSampler* SkJpegCodec::getSampler(bool createIfNecessary) {
    if (!createIfNecessary || fSwizzler) {
        SkASSERT(!fSwizzler || (fSrcRow && fStorage.get() == fSrcRow));
        return fSwizzler;
    }

    this->initializeSwizzler(this->dstInfo(), this->options());
    return fSwizzler;
}

SkCodec::Result SkJpegCodec::onStartScanlineDecode(const SkImageInfo& dstInfo,
        const Options& options, SkPMColor ctable[], int* ctableCount) {
    // Set the jump location for libjpeg errors
    if (setjmp(fDecoderMgr->getJmpBuf())) {
        SkCodecPrintf("setjmp: Error from libjpeg\n");
        return kInvalidInput;
    }

    // Check if we can decode to the requested destination and set the output color space
    if (!this->setOutputColorSpace(dstInfo)) {
        return kInvalidConversion;
    }

    // Remove objects used for sampling.
    fSwizzler.reset(nullptr);
    fSrcRow = nullptr;
    fStorage.free();

    // Now, given valid output dimensions, we can start the decompress
    if (!jpeg_start_decompress(fDecoderMgr->dinfo())) {
        SkCodecPrintf("start decompress failed\n");
        return kInvalidInput;
    }

    // We will need a swizzler if we are performing a subset decode or
    // converting from CMYK.
    if (options.fSubset || JCS_CMYK == fDecoderMgr->dinfo()->out_color_space) {
        this->initializeSwizzler(dstInfo, options);
    }

    return kSuccess;
}

int SkJpegCodec::onGetScanlines(void* dst, int count, size_t rowBytes) {
    // Set the jump location for libjpeg errors
    if (setjmp(fDecoderMgr->getJmpBuf())) {
        return fDecoderMgr->returnFailure("setjmp", kInvalidInput);
    }
    // Read rows one at a time
    JSAMPLE* dstRow;
    if (fSwizzler) {
        // write data to storage row, then sample using swizzler
        dstRow = fSrcRow;
    } else {
        // write data directly to dst
        dstRow = (JSAMPLE*) dst;
    }

    for (int y = 0; y < count; y++) {
        // Read row of the image
        uint32_t rowsDecoded = jpeg_read_scanlines(fDecoderMgr->dinfo(), &dstRow, 1);
        if (rowsDecoded != 1) {
            fDecoderMgr->dinfo()->output_scanline = this->dstInfo().height();
            return y;
        }

        if (fSwizzler) {
            // use swizzler to sample row
            fSwizzler->swizzle(dst, dstRow);
            dst = SkTAddOffset<JSAMPLE>(dst, rowBytes);
        } else {
            dstRow = SkTAddOffset<JSAMPLE>(dstRow, rowBytes);
        }
    }
    return count;
}

#ifndef TURBO_HAS_SKIP
// TODO (msarett): Avoid reallocating the memory buffer on each call to skip.
static uint32_t jpeg_skip_scanlines(jpeg_decompress_struct* dinfo, int count) {
    SkAutoTMalloc<uint8_t> storage(get_row_bytes(dinfo));
    uint8_t* storagePtr = storage.get();
    for (int y = 0; y < count; y++) {
        if (1 != jpeg_read_scanlines(dinfo, &storagePtr, 1)) {
            return y;
        }
    }
    return count;
}
#endif

bool SkJpegCodec::onSkipScanlines(int count) {
    // Set the jump location for libjpeg errors
    if (setjmp(fDecoderMgr->getJmpBuf())) {
        return fDecoderMgr->returnFalse("setjmp");
    }

    return (uint32_t) count == jpeg_skip_scanlines(fDecoderMgr->dinfo(), count);
}
