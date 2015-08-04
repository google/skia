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
#include "SkScanlineDecoder.h"
#include "SkStream.h"
#include "SkTemplates.h"
#include "SkTypes.h"

// stdio is needed for libjpeg-turbo
#include <stdio.h>

extern "C" {
    #include "jpeglibmangler.h"
    #include "jerror.h"
    #include "jpegint.h"
    #include "jpeglib.h"
}

/*
 * Convert a row of CMYK samples to RGBA in place.
 * Note that this method moves the row pointer.
 * @param width the number of pixels in the row that is being converted
 *              CMYK is stored as four bytes per pixel
 */
static void convert_CMYK_to_RGBA(uint8_t* row, uint32_t width) {
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
#if defined(SK_PMCOLOR_IS_RGBA)
        row[0] = SkMulDiv255Round(row[0], row[3]);
        row[1] = SkMulDiv255Round(row[1], row[3]);
        row[2] = SkMulDiv255Round(row[2], row[3]);
#else
        uint8_t tmp = row[0];
        row[0] = SkMulDiv255Round(row[2], row[3]);
        row[1] = SkMulDiv255Round(row[1], row[3]);
        row[2] = SkMulDiv255Round(tmp, row[3]);
#endif
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
    if (JPEG_HEADER_OK != turbo_jpeg_read_header(decoderMgr->dinfo(), true)) {
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
    // libjpeg-turbo supports scaling by 1/8, 1/4, 3/8, 1/2, 5/8, 3/4, 7/8, and 1/1, so we will
    // support these as well
    long num;
    long denom = 8;
    if (desiredScale > 0.875f) {
        num = 8;
    } else if (desiredScale > 0.75f) {
        num = 7;
    } else if (desiredScale > 0.625f) {
        num = 6;
    } else if (desiredScale > 0.5f) {
        num = 5;
    } else if (desiredScale > 0.375f) {
        num = 4;
    } else if (desiredScale > 0.25f) {
        num = 3;
    } else if (desiredScale > 0.125f) {
        num = 2;
    } else {
        num = 1;
    }

    // Set up a fake decompress struct in order to use libjpeg to calculate output dimensions
    jpeg_decompress_struct dinfo;
    sk_bzero(&dinfo, sizeof(dinfo));
    dinfo.image_width = this->getInfo().width();
    dinfo.image_height = this->getInfo().height();
    dinfo.global_state = DSTATE_READY;
    dinfo.num_components = 0;
    dinfo.scale_num = num;
    dinfo.scale_denom = denom;
    turbo_jpeg_calc_output_dimensions(&dinfo);

    // Return the calculated output dimensions for the given scale
    return SkISize::Make(dinfo.output_width, dinfo.output_height);
}

/*
 * Handles rewinding the input stream if it is necessary
 */
bool SkJpegCodec::handleRewind() {
    switch(this->rewindIfNeeded()) {
        case kCouldNotRewind_RewindState:
            return fDecoderMgr->returnFalse("could not rewind");
        case kRewound_RewindState: {
            JpegDecoderMgr* decoderMgr = NULL;
            if (!ReadHeader(this->stream(), NULL, &decoderMgr)) {
                return fDecoderMgr->returnFalse("could not rewind");
            }
            SkASSERT(NULL != decoderMgr);
            fDecoderMgr.reset(decoderMgr);
            return true;
        }
        case kNoRewindNecessary_RewindState:
            return true;
        default:
            SkASSERT(false);
            return false;
    }
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
                return false;
            } else {
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
 * Checks if we can scale to the requested dimensions and scales the dimensions
 * if possible
 */
bool SkJpegCodec::scaleToDimensions(uint32_t dstWidth, uint32_t dstHeight) {
    // libjpeg-turbo can scale to 1/8, 1/4, 3/8, 1/2, 5/8, 3/4, 7/8, and 1/1
    fDecoderMgr->dinfo()->scale_denom = 8;
    fDecoderMgr->dinfo()->scale_num = 8;
    turbo_jpeg_calc_output_dimensions(fDecoderMgr->dinfo());
    while (fDecoderMgr->dinfo()->output_width != dstWidth ||
            fDecoderMgr->dinfo()->output_height != dstHeight) {

        // Return a failure if we have tried all of the possible scales
        if (1 == fDecoderMgr->dinfo()->scale_num ||
                dstWidth > fDecoderMgr->dinfo()->output_width ||
                dstHeight > fDecoderMgr->dinfo()->output_height) {
            return fDecoderMgr->returnFalse("could not scale to requested dimensions");
        }

        // Try the next scale
        fDecoderMgr->dinfo()->scale_num -= 1;
        turbo_jpeg_calc_output_dimensions(fDecoderMgr->dinfo());
    }
    return true;
}

/*
 * Performs the jpeg decode
 */
SkCodec::Result SkJpegCodec::onGetPixels(const SkImageInfo& dstInfo,
                                         void* dst, size_t dstRowBytes,
                                         const Options& options, SkPMColor*, int*) {
    // Rewind the stream if needed
    if (!this->handleRewind()) {
        return fDecoderMgr->returnFailure("could not rewind stream", kCouldNotRewind);
    }

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

    // Perform the necessary scaling
    if (!this->scaleToDimensions(dstInfo.width(), dstInfo.height())) {
        return fDecoderMgr->returnFailure("cannot scale to requested dims", kInvalidScale);
    }

    // Now, given valid output dimensions, we can start the decompress
    if (!turbo_jpeg_start_decompress(dinfo)) {
        return fDecoderMgr->returnFailure("startDecompress", kInvalidInput);
    }

    // The recommended output buffer height should always be 1 in high quality modes.
    // If it's not, we want to know because it means our strategy is not optimal.
    SkASSERT(1 == dinfo->rec_outbuf_height);

    // Perform the decode a single row at a time
    uint32_t dstHeight = dstInfo.height();
    JSAMPLE* dstRow = (JSAMPLE*) dst;
    for (uint32_t y = 0; y < dstHeight; y++) {
        // Read rows of the image
        uint32_t rowsDecoded = turbo_jpeg_read_scanlines(dinfo, &dstRow, 1);

        // If we cannot read enough rows, assume the input is incomplete
        if (rowsDecoded != 1) {
            // Fill the remainder of the image with black. This error handling
            // behavior is unspecified but SkCodec consistently uses black as
            // the fill color for opaque images.  If the destination is kGray,
            // the low 8 bits of SK_ColorBLACK will be used.  Conveniently,
            // these are zeros, which is the representation for black in kGray.
            // If the destination is kRGB_565, the low 16 bits of SK_ColorBLACK
            // will be used.  Conveniently, these are zeros, which is the
            // representation for black in kRGB_565.
            if (kNo_ZeroInitialized == options.fZeroInitialized ||
                    kN32_SkColorType == dstInfo.colorType()) {
                SkSwizzler::Fill(dstRow, dstInfo, dstRowBytes, dstHeight - y,
                        SK_ColorBLACK, NULL);
            }

            // Prevent libjpeg from failing on incomplete decode
            dinfo->output_scanline = dstHeight;

            // Finish the decode and indicate that the input was incomplete.
            turbo_jpeg_finish_decompress(dinfo);
            return fDecoderMgr->returnFailure("Incomplete image data", kIncompleteInput);
        }

        // Convert to RGBA if necessary
        if (JCS_CMYK == dinfo->out_color_space) {
            convert_CMYK_to_RGBA(dstRow, dstInfo.width());
        }

        // Move to the next row
        dstRow = SkTAddOffset<JSAMPLE>(dstRow, dstRowBytes);
    }
    turbo_jpeg_finish_decompress(dinfo);

    return kSuccess;
}

/*
 * Enable scanline decoding for jpegs
 */
class SkJpegScanlineDecoder : public SkScanlineDecoder {
public:
    SkJpegScanlineDecoder(const SkImageInfo& srcInfo, SkJpegCodec* codec)
        : INHERITED(srcInfo)
        , fCodec(codec)
        , fOpts()
    {}

    SkCodec::Result onStart(const SkImageInfo& dstInfo, const SkCodec::Options& options,
                            SkPMColor ctable[], int* ctableCount) override {

        // Rewind the stream if needed
        if (!fCodec->handleRewind()) {
            return SkCodec::kCouldNotRewind;
        }

        // Set the jump location for libjpeg errors
        if (setjmp(fCodec->fDecoderMgr->getJmpBuf())) {
            SkCodecPrintf("setjmp: Error from libjpeg\n");
            return SkCodec::kInvalidInput;
        }

        // Check if we can decode to the requested destination and set the output color space
        if (!fCodec->setOutputColorSpace(dstInfo)) {
            return SkCodec::kInvalidConversion;
        }

        // Perform the necessary scaling
        if (!fCodec->scaleToDimensions(dstInfo.width(), dstInfo.height())) {
            return SkCodec::kInvalidScale;
        }

        // Now, given valid output dimensions, we can start the decompress
        if (!turbo_jpeg_start_decompress(fCodec->fDecoderMgr->dinfo())) {
            SkCodecPrintf("start decompress failed\n");
            return SkCodec::kInvalidInput;
        }

        fOpts = options;

        return SkCodec::kSuccess;
    }

    virtual ~SkJpegScanlineDecoder() {
        if (setjmp(fCodec->fDecoderMgr->getJmpBuf())) {
            SkCodecPrintf("setjmp: Error in libjpeg finish_decompress\n");
            return;
        }

        // We may not have decoded the entire image.  Prevent libjpeg-turbo from failing on a
        // partial decode.
        fCodec->fDecoderMgr->dinfo()->output_scanline = fCodec->getInfo().height();
        turbo_jpeg_finish_decompress(fCodec->fDecoderMgr->dinfo());
    }

    SkCodec::Result onGetScanlines(void* dst, int count, size_t rowBytes) override {
        // Set the jump location for libjpeg errors
        if (setjmp(fCodec->fDecoderMgr->getJmpBuf())) {
            return fCodec->fDecoderMgr->returnFailure("setjmp", SkCodec::kInvalidInput);
        }

        // Read rows one at a time
        JSAMPLE* dstRow = (JSAMPLE*) dst;
        for (int y = 0; y < count; y++) {
            // Read row of the image
            uint32_t rowsDecoded =
                    turbo_jpeg_read_scanlines(fCodec->fDecoderMgr->dinfo(), &dstRow, 1);
            if (rowsDecoded != 1) {
                if (SkCodec::kNo_ZeroInitialized == fOpts.fZeroInitialized ||
                        kN32_SkColorType == this->dstInfo().colorType()) {
                    SkSwizzler::Fill(dstRow, this->dstInfo(), rowBytes,
                            count - y, SK_ColorBLACK, NULL);
                }
                fCodec->fDecoderMgr->dinfo()->output_scanline = this->dstInfo().height();
                return SkCodec::kIncompleteInput;
            }

            // Convert to RGBA if necessary
            if (JCS_CMYK == fCodec->fDecoderMgr->dinfo()->out_color_space) {
                convert_CMYK_to_RGBA(dstRow, this->dstInfo().width());
            }

            // Move to the next row
            dstRow = SkTAddOffset<JSAMPLE>(dstRow, rowBytes);
        }

        return SkCodec::kSuccess;
    }

#ifndef TURBO_HAS_SKIP
#define turbo_jpeg_skip_scanlines(dinfo, count)                              \
    SkAutoMalloc storage(dinfo->output_width * dinfo->out_color_components); \
    uint8_t* storagePtr = static_cast<uint8_t*>(storage.get());              \
    for (int y = 0; y < count; y++) {                                        \
        turbo_jpeg_read_scanlines(dinfo, &storagePtr, 1);                    \
    }
#endif

    SkCodec::Result onSkipScanlines(int count) override {
        // Set the jump location for libjpeg errors
        if (setjmp(fCodec->fDecoderMgr->getJmpBuf())) {
            return fCodec->fDecoderMgr->returnFailure("setjmp", SkCodec::kInvalidInput);
        }

        turbo_jpeg_skip_scanlines(fCodec->fDecoderMgr->dinfo(), count);

        return SkCodec::kSuccess;
    }

private:
    SkAutoTDelete<SkJpegCodec> fCodec;
    SkCodec::Options           fOpts;

    typedef SkScanlineDecoder INHERITED;
};

SkScanlineDecoder* SkJpegCodec::NewSDFromStream(SkStream* stream) {
    SkAutoTDelete<SkJpegCodec> codec(static_cast<SkJpegCodec*>(SkJpegCodec::NewFromStream(stream)));
    if (!codec) {
        return NULL;
    }

    const SkImageInfo& srcInfo = codec->getInfo();
    // Return the new scanline decoder
    return SkNEW_ARGS(SkJpegScanlineDecoder, (srcInfo, codec.detach()));
}
