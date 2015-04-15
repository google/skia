/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCodec.h"
#include "SkJpegCodec.h"
#include "SkJpegDecoderMgr.h"
#include "SkJpegUtility.h"
#include "SkCodecPriv.h"
#include "SkColorPriv.h"
#include "SkStream.h"
#include "SkTemplates.h"
#include "SkTypes.h"

// stdio is needed for jpeglib
#include <stdio.h>

extern "C" {
    #include "jerror.h"
    #include "jmorecfg.h"
    #include "jpegint.h"
    #include "jpeglib.h"
}

// ANDROID_RGB
// If this is defined in the jpeg headers it indicates that jpeg offers
// support for two additional formats: JCS_RGBA_8888 and JCS_RGB_565.

/*
 * Get the source configuarion for the swizzler
 */
SkSwizzler::SrcConfig get_src_config(const jpeg_decompress_struct& dinfo) {
    if (JCS_CMYK == dinfo.out_color_space) {
        // We will need to perform a manual conversion
        return  SkSwizzler::kRGBX;
    }
    if (3 == dinfo.out_color_components && JCS_RGB == dinfo.out_color_space) {
        return SkSwizzler::kRGB;
    }
#ifdef ANDROID_RGB
    if (JCS_RGBA_8888 == dinfo.out_color_space) {
        return SkSwizzler::kRGBX;
    }

    if (JCS_RGB_565 == dinfo.out_color_space) {
        return SkSwizzler::kRGB_565;
    }
#endif
    if (1 == dinfo.out_color_components && JCS_GRAYSCALE == dinfo.out_color_space) {
        return SkSwizzler::kGray;
    }
    return SkSwizzler::kUnknown;
}

/*
 * Convert a row of CMYK samples to RGBX in place.
 * Note that this method moves the row pointer.
 * @param width the number of pixels in the row that is being converted
 *              CMYK is stored as four bytes per pixel
 */
static void convert_CMYK_to_RGB(uint8_t* row, uint32_t width) {
    // We will implement a crude conversion from CMYK -> RGB using formulas
    // from easyrgb.com.
    //
    // CMYK -> CMY
    // C = C * (1 - K) + K
    // M = M * (1 - K) + K
    // Y = Y * (1 - K) + K
    //
    // libjpeg actually gives us inverted CMYK, so we must subtract the
    // original terms from 1.
    // CMYK -> CMY
    // C = (1 - C) * (1 - (1 - K)) + (1 - K)
    // M = (1 - M) * (1 - (1 - K)) + (1 - K)
    // Y = (1 - Y) * (1 - (1 - K)) + (1 - K)
    //
    // Simplifying the above expression.
    // CMYK -> CMY
    // C = 1 - CK
    // M = 1 - MK
    // Y = 1 - YK
    //
    // CMY -> RGB
    // R = (1 - C) * 255
    // G = (1 - M) * 255
    // B = (1 - Y) * 255
    //
    // Therefore the full conversion is below.  This can be verified at
    // www.rapidtables.com (assuming inverted CMYK).
    // CMYK -> RGB
    // R = C * K * 255
    // G = M * K * 255
    // B = Y * K * 255
    //
    // As a final note, we have treated the CMYK values as if they were on
    // a scale from 0-1, when in fact they are 8-bit ints scaling from 0-255.
    // We must divide each CMYK component by 255 to obtain the true conversion
    // we should perform.
    // CMYK -> RGB
    // R = C * K / 255
    // G = M * K / 255
    // B = Y * K / 255
    for (uint32_t x = 0; x < width; x++, row += 4) {
        row[0] = SkMulDiv255Round(row[0], row[3]);
        row[1] = SkMulDiv255Round(row[1], row[3]);
        row[2] = SkMulDiv255Round(row[2], row[3]);
        row[3] = 0xFF;
    }
}

bool SkJpegCodec::IsJpeg(SkStream* stream) {
    static const uint8_t jpegSig[] = { 0xFF, 0xD8, 0xFF };
    char buffer[sizeof(jpegSig)];
    return stream->read(buffer, sizeof(jpegSig)) == sizeof(jpegSig) &&
            !memcmp(buffer, jpegSig, sizeof(jpegSig));
}

bool SkJpegCodec::ReadHeader(SkStream* stream, SkCodec** codecOut,
        JpegDecoderMgr** decoderMgrOut) {

    // Create a JpegDecoderMgr to own all of the decompress information
    SkAutoTDelete<JpegDecoderMgr> decoderMgr(SkNEW_ARGS(JpegDecoderMgr, (stream)));

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

    if (NULL != codecOut) {
        // Recommend the color type to decode to
        const SkColorType colorType = decoderMgr->getColorType();

        // Create image info object and the codec
        const SkImageInfo& imageInfo = SkImageInfo::Make(decoderMgr->dinfo()->image_width,
                decoderMgr->dinfo()->image_height, colorType, kOpaque_SkAlphaType);
        *codecOut = SkNEW_ARGS(SkJpegCodec, (imageInfo, stream, decoderMgr.detach()));
    } else {
        SkASSERT(NULL != decoderMgrOut);
        *decoderMgrOut = decoderMgr.detach();
    }
    return true;
}

SkCodec* SkJpegCodec::NewFromStream(SkStream* stream) {
    SkAutoTDelete<SkStream> streamDeleter(stream);
    SkCodec* codec = NULL;
    if (ReadHeader(stream,  &codec, NULL)) {
        // Codec has taken ownership of the stream, we do not need to delete it
        SkASSERT(codec);
        streamDeleter.detach();
        return codec;
    }
    return NULL;
}

SkJpegCodec::SkJpegCodec(const SkImageInfo& srcInfo, SkStream* stream,
        JpegDecoderMgr* decoderMgr)
    : INHERITED(srcInfo, stream)
    , fDecoderMgr(decoderMgr)
{}

/*
 * Return a valid set of output dimensions for this decoder, given an input scale
 */
SkISize SkJpegCodec::onGetScaledDimensions(float desiredScale) const {
    // libjpeg supports scaling by 1/1, 1/2, 1/4, and 1/8, so we will support these as well
    long scale;
    if (desiredScale > 0.75f) {
        scale = 1;
    } else if (desiredScale > 0.375f) {
        scale = 2;
    } else if (desiredScale > 0.1875f) {
        scale = 4;
    } else {
        scale = 8;
    }

    // Set up a fake decompress struct in order to use libjpeg to calculate output dimensions
    jpeg_decompress_struct dinfo;
    dinfo.image_width = this->getInfo().width();
    dinfo.image_height = this->getInfo().height();
    dinfo.global_state = DSTATE_READY;
    dinfo.num_components = 0;
    dinfo.scale_num = 1;
    dinfo.scale_denom = scale;
    jpeg_calc_output_dimensions(&dinfo);

    // Return the calculated output dimensions for the given scale
    return SkISize::Make(dinfo.output_width, dinfo.output_height);
}

/*
 * Checks if the conversion between the input image and the requested output
 * image has been implemented
 */
static bool conversion_possible(const SkImageInfo& dst,
                                const SkImageInfo& src) {
    // Ensure that the profile type is unchanged
    if (dst.profileType() != src.profileType()) {
        return false;
    }

    // Ensure that the alpha type is opaque
    if (kOpaque_SkAlphaType != dst.alphaType()) {
        return false;
    }

    // Always allow kN32 as the color type
    if (kN32_SkColorType == dst.colorType()) {
        return true;
    }

    // Otherwise require that the destination color type match our recommendation
    return dst.colorType() == src.colorType();
}

/*
 * Performs the jpeg decode
 */
SkCodec::Result SkJpegCodec::onGetPixels(const SkImageInfo& dstInfo,
                                         void* dst, size_t dstRowBytes,
                                         const Options& options, SkPMColor*, int*) {
    // Rewind the stream if needed
    SkCodec::RewindState rewindState = this->rewindIfNeeded();
    if (rewindState == kCouldNotRewind_RewindState) {
        return kCouldNotRewind;
    } else if (rewindState == kRewound_RewindState) {
        JpegDecoderMgr* decoderMgr = NULL;
        if (!ReadHeader(this->stream(), NULL, &decoderMgr)) {
            return kCouldNotRewind;
        }
        SkASSERT(NULL != decoderMgr);
        fDecoderMgr.reset(decoderMgr);
    }

    // Get a pointer to the decompress info since we will use it quite frequently
    jpeg_decompress_struct* dinfo = fDecoderMgr->dinfo();

    // Set the jump location for libjpeg errors
    if (setjmp(fDecoderMgr->getJmpBuf())) {
        return fDecoderMgr->returnFailure("setjmp", kInvalidInput);
    }

    // Check if we can decode to the requested destination
    if (!conversion_possible(dstInfo, this->getInfo())) {
        return fDecoderMgr->returnFailure("conversion_possible", kInvalidConversion);
    }
    // Check if we can scale to the requested dimensions
    // libjpeg can scale to 1/1, 1/2, 1/4, and 1/8
    SkASSERT(1 == dinfo->scale_num);
    SkASSERT(1 == dinfo->scale_denom);
    jpeg_calc_output_dimensions(dinfo);
    const uint32_t dstWidth = dstInfo.width();
    const uint32_t dstHeight = dstInfo.height();
    while (dinfo->output_width != dstWidth || dinfo->output_height != dstHeight) {

        // Return a failure if we have tried all of the possible scales
        if (8 == dinfo->scale_denom ||
                dstWidth > dinfo->output_width ||
                dstHeight > dinfo->output_height) {
            return fDecoderMgr->returnFailure("cannot scale to requested dims", kInvalidScale);
        }

        // Try the next scale
        dinfo->scale_denom *= 2;
        jpeg_calc_output_dimensions(dinfo);
    }

    // Now, given valid output dimensions, we can start the decompress
    if (!jpeg_start_decompress(dinfo)) {
        return fDecoderMgr->returnFailure("startDecompress", kInvalidInput);
    }

    // Create the swizzler
    SkSwizzler::SrcConfig srcConfig = get_src_config(*dinfo);
    SkAutoTDelete<SkSwizzler> swizzler(SkSwizzler::CreateSwizzler(srcConfig, NULL, dstInfo, dst,
            dstRowBytes, options.fZeroInitialized));
    if (NULL == swizzler) {
        return fDecoderMgr->returnFailure("getSwizzler", kInvalidInput);
    }
    const uint32_t srcBytesPerPixel = SkSwizzler::BytesPerPixel(srcConfig);

    // This is usually 1, but can also be 2 or 4.
    // If we wanted to always read one row at a time, we could, but we will save space and time
    // by using the recommendation from libjpeg.
    const uint32_t rowsPerDecode = dinfo->rec_outbuf_height;
    SkASSERT(rowsPerDecode <= 4);

    // Create a buffer to contain decoded rows (libjpeg requires a 2D array)
    const uint32_t srcRowBytes = srcBytesPerPixel * dstWidth;
    SkAutoTDeleteArray<uint8_t> srcBuffer(SkNEW_ARRAY(uint8_t, srcRowBytes * rowsPerDecode));
    JSAMPLE* srcRows[4];
    uint8_t* srcPtr = srcBuffer.get();
    for (uint8_t i = 0; i < rowsPerDecode; i++) {
        srcRows[i] = (JSAMPLE*) srcPtr;
        srcPtr += srcRowBytes;
    }

    // Ensure that we loop enough times to decode all of the rows
    // libjpeg will prevent us from reading past the bottom of the image
    for (uint32_t y = 0; y < dstHeight + rowsPerDecode - 1; y += rowsPerDecode) {
        // Read rows of the image
        uint32_t rowsDecoded = jpeg_read_scanlines(dinfo, srcRows, rowsPerDecode);

        // Convert to RGB if necessary
        if (JCS_CMYK == dinfo->out_color_space) {
            convert_CMYK_to_RGB(srcRows[0], dstWidth * rowsDecoded);
        }

        // Swizzle to output destination
        for (uint32_t i = 0; i < rowsDecoded; i++) {
            swizzler->next(srcRows[i]);
        }

        // If we cannot read enough rows, assume the input is incomplete
        if (rowsDecoded < rowsPerDecode && y + rowsDecoded < dstHeight) {
            // Fill the remainder of the image with black. This error handling
            // behavior is unspecified but SkCodec consistently uses black as
            // the fill color for opaque images.  If the destination is kGray,
            // the low 8 bits of SK_ColorBLACK will be used.  Conveniently,
            // these are zeros, which is the representation for black in kGray.
            SkSwizzler::Fill(swizzler->getDstRow(), dstInfo, dstRowBytes,
                    dstHeight - y - rowsDecoded, SK_ColorBLACK, NULL);

            // Prevent libjpeg from failing on incomplete decode
            dinfo->output_scanline = dstHeight;

            // Finish the decode and indicate that the input was incomplete.
            jpeg_finish_decompress(dinfo);
            return fDecoderMgr->returnFailure("Incomplete image data", kIncompleteInput);
        }
    }
    jpeg_finish_decompress(dinfo);

    return kSuccess;
}
