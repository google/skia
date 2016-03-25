/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCodec.h"
#include "SkMSAN.h"
#include "SkJpegCodec.h"
#include "SkJpegDecoderMgr.h"
#include "SkCodecPriv.h"
#include "SkColorPriv.h"
#include "SkStream.h"
#include "SkTemplates.h"
#include "SkTypes.h"

// stdio is needed for libjpeg-turbo
#include <stdio.h>
#include "SkJpegUtility.h"

extern "C" {
    #include "jerror.h"
    #include "jpeglib.h"
}

bool SkJpegCodec::IsJpeg(const void* buffer, size_t bytesRead) {
    static const uint8_t jpegSig[] = { 0xFF, 0xD8, 0xFF };
    return bytesRead >= 3 && !memcmp(buffer, jpegSig, sizeof(jpegSig));
}

static uint32_t get_endian_int(const uint8_t* data, bool littleEndian) {
    if (littleEndian) {
        return (data[3] << 24) | (data[2] << 16) | (data[1] << 8) | (data[0]);
    }

    return (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | (data[3]);
}

const uint32_t kExifHeaderSize = 14;
const uint32_t kICCHeaderSize = 14;
const uint32_t kExifMarker = JPEG_APP0 + 1;
const uint32_t kICCMarker = JPEG_APP0 + 2;

static bool is_orientation_marker(jpeg_marker_struct* marker, SkCodec::Origin* orientation) {
    if (kExifMarker != marker->marker || marker->data_length < kExifHeaderSize) {
        return false;
    }

    const uint8_t* data = marker->data;
    static const uint8_t kExifSig[] { 'E', 'x', 'i', 'f', '\0' };
    if (memcmp(data, kExifSig, sizeof(kExifSig))) {
        return false;
    }

    bool littleEndian;
    if (!is_valid_endian_marker(data + 6, &littleEndian)) {
        return false;
    }

    // Get the offset from the start of the marker.
    // Account for 'E', 'x', 'i', 'f', '\0', '<fill byte>'.
    uint32_t offset = get_endian_int(data + 10, littleEndian);
    offset += sizeof(kExifSig) + 1;

    // Require that the marker is at least large enough to contain the number of entries.
    if (marker->data_length < offset + 2) {
        return false;
    }
    uint32_t numEntries = get_endian_short(data + offset, littleEndian);

    // Tag (2 bytes), Datatype (2 bytes), Number of elements (4 bytes), Data (4 bytes)
    const uint32_t kEntrySize = 12;
    numEntries = SkTMin(numEntries, (marker->data_length - offset - 2) / kEntrySize);

    // Advance the data to the start of the entries.
    data += offset + 2;

    const uint16_t kOriginTag = 0x112;
    const uint16_t kOriginType = 3;
    for (uint32_t i = 0; i < numEntries; i++, data += kEntrySize) {
        uint16_t tag = get_endian_short(data, littleEndian);
        uint16_t type = get_endian_short(data + 2, littleEndian);
        uint32_t count = get_endian_int(data + 4, littleEndian);
        if (kOriginTag == tag && kOriginType == type && 1 == count) {
            uint16_t val = get_endian_short(data + 8, littleEndian);
            if (0 < val && val <= SkCodec::kLast_Origin) {
                *orientation = (SkCodec::Origin) val;
                return true;
            }
        }
    }

    return false;
}

static SkCodec::Origin get_exif_orientation(jpeg_decompress_struct* dinfo) {
    SkCodec::Origin orientation;
    for (jpeg_marker_struct* marker = dinfo->marker_list; marker; marker = marker->next) {
        if (is_orientation_marker(marker, &orientation)) {
            return orientation;
        }
    }

    return SkCodec::kDefault_Origin;
}

static bool is_icc_marker(jpeg_marker_struct* marker) {
    if (kICCMarker != marker->marker || marker->data_length < kICCHeaderSize) {
        return false;
    }

    static const uint8_t kICCSig[] { 'I', 'C', 'C', '_', 'P', 'R', 'O', 'F', 'I', 'L', 'E', '\0' };
    return !memcmp(marker->data, kICCSig, sizeof(kICCSig));
}

/*
 * ICC profiles may be stored using a sequence of multiple markers.  We obtain the ICC profile
 * in two steps:
 *     (1) Discover all ICC profile markers and verify that they are numbered properly.
 *     (2) Copy the data from each marker into a contiguous ICC profile.
 */
static sk_sp<SkColorSpace> get_icc_profile(jpeg_decompress_struct* dinfo) {
    // Note that 256 will be enough storage space since each markerIndex is stored in 8-bits.
    jpeg_marker_struct* markerSequence[256];
    memset(markerSequence, 0, sizeof(markerSequence));
    uint8_t numMarkers = 0;
    size_t totalBytes = 0;

    // Discover any ICC markers and verify that they are numbered properly.
    for (jpeg_marker_struct* marker = dinfo->marker_list; marker; marker = marker->next) {
        if (is_icc_marker(marker)) {
            // Verify that numMarkers is valid and consistent.
            if (0 == numMarkers) {
                numMarkers = marker->data[13];
                if (0 == numMarkers) {
                    SkCodecPrintf("ICC Profile Error: numMarkers must be greater than zero.\n");
                    return nullptr;
                }
            } else if (numMarkers != marker->data[13]) {
                SkCodecPrintf("ICC Profile Error: numMarkers must be consistent.\n");
                return nullptr;
            }

            // Verify that the markerIndex is valid and unique.  Note that zero is not
            // a valid index.
            uint8_t markerIndex = marker->data[12];
            if (markerIndex == 0 || markerIndex > numMarkers) {
                SkCodecPrintf("ICC Profile Error: markerIndex is invalid.\n");
                return nullptr;
            }
            if (markerSequence[markerIndex]) {
                SkCodecPrintf("ICC Profile Error: Duplicate value of markerIndex.\n");
                return nullptr;
            }
            markerSequence[markerIndex] = marker;
            SkASSERT(marker->data_length >= kICCHeaderSize);
            totalBytes += marker->data_length - kICCHeaderSize;
        }
    }

    if (0 == totalBytes) {
        // No non-empty ICC profile markers were found.
        return nullptr;
    }

    // Combine the ICC marker data into a contiguous profile.
    SkAutoMalloc iccData(totalBytes);
    void* dst = iccData.get();
    for (uint32_t i = 1; i <= numMarkers; i++) {
        jpeg_marker_struct* marker = markerSequence[i];
        if (!marker) {
            SkCodecPrintf("ICC Profile Error: Missing marker %d of %d.\n", i, numMarkers);
            return nullptr;
        }

        void* src = SkTAddOffset<void>(marker->data, kICCHeaderSize);
        size_t bytes = marker->data_length - kICCHeaderSize;
        memcpy(dst, src, bytes);
        dst = SkTAddOffset<void>(dst, bytes);
    }

    return SkColorSpace::NewICC(iccData.get(), totalBytes);
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

    // Instruct jpeg library to save the markers that we care about.  Since
    // the orientation and color profile will not change, we can skip this
    // step on rewinds.
    if (codecOut) {
        jpeg_save_markers(decoderMgr->dinfo(), kExifMarker, 0xFFFF);
        jpeg_save_markers(decoderMgr->dinfo(), kICCMarker, 0xFFFF);
    }

    // Read the jpeg header
    if (JPEG_HEADER_OK != jpeg_read_header(decoderMgr->dinfo(), true)) {
        return decoderMgr->returnFalse("read_header");
    }

    if (codecOut) {
        // Recommend the color type to decode to
        const SkColorType colorType = decoderMgr->getColorType();

        // Create image info object and the codec
        const SkImageInfo& imageInfo = SkImageInfo::Make(decoderMgr->dinfo()->image_width,
                decoderMgr->dinfo()->image_height, colorType, kOpaque_SkAlphaType);

        Origin orientation = get_exif_orientation(decoderMgr->dinfo());
        sk_sp<SkColorSpace> colorSpace = get_icc_profile(decoderMgr->dinfo());

        *codecOut = new SkJpegCodec(imageInfo, stream, decoderMgr.release(), colorSpace,
                orientation);
    } else {
        SkASSERT(nullptr != decoderMgrOut);
        *decoderMgrOut = decoderMgr.release();
    }
    return true;
}

SkCodec* SkJpegCodec::NewFromStream(SkStream* stream) {
    SkAutoTDelete<SkStream> streamDeleter(stream);
    SkCodec* codec = nullptr;
    if (ReadHeader(stream,  &codec, nullptr)) {
        // Codec has taken ownership of the stream, we do not need to delete it
        SkASSERT(codec);
        streamDeleter.release();
        return codec;
    }
    return nullptr;
}

SkJpegCodec::SkJpegCodec(const SkImageInfo& srcInfo, SkStream* stream,
        JpegDecoderMgr* decoderMgr, sk_sp<SkColorSpace> colorSpace, Origin origin)
    : INHERITED(srcInfo, stream, colorSpace, origin)
    , fDecoderMgr(decoderMgr)
    , fReadyState(decoderMgr->dinfo()->global_state)
    , fSwizzlerSubset(SkIRect::MakeEmpty())
{}

/*
 * Return the row bytes of a particular image type and width
 */
static size_t get_row_bytes(const j_decompress_ptr dinfo) {
#ifdef TURBO_HAS_565
    const size_t colorBytes = (dinfo->out_color_space == JCS_RGB565) ? 2 :
            dinfo->out_color_components;
#else
    const size_t colorBytes = dinfo->out_color_components;
#endif
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
    if (kUnknown_SkAlphaType == dst.alphaType()) {
        return false;
    }

    if (kOpaque_SkAlphaType != dst.alphaType()) {
        SkCodecPrintf("Warning: an opaque image should be decoded as opaque "
                      "- it is being decoded as non-opaque, which will draw slower\n");
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
#ifdef LIBJPEG_TURBO_VERSION
            // Check the byte ordering of the RGBA color space for the
            // current platform
    #ifdef SK_PMCOLOR_IS_RGBA
            fDecoderMgr->dinfo()->out_color_space = JCS_EXT_RGBA;
    #else
            fDecoderMgr->dinfo()->out_color_space = JCS_EXT_BGRA;
    #endif
#else
            fDecoderMgr->dinfo()->out_color_space = JCS_RGB;
#endif
            }
            return true;
        case kRGB_565_SkColorType:
            if (isCMYK) {
                fDecoderMgr->dinfo()->out_color_space = JCS_CMYK;
            } else {
#ifdef TURBO_HAS_565
                fDecoderMgr->dinfo()->dither_mode = JDITHER_NONE;
                fDecoderMgr->dinfo()->out_color_space = JCS_RGB565;
#else
                fDecoderMgr->dinfo()->out_color_space = JCS_RGB;
#endif
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

    J_COLOR_SPACE colorSpace = dinfo->out_color_space;
    if (JCS_CMYK == colorSpace || JCS_RGB == colorSpace) {
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
        sk_msan_mark_initialized(dstRow, dstRow + dstRowBytes, "skbug.com/4550");

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
        // If the out_color_space is not CMYK, the only reason we would need a swizzler is
        // for sampling and/or subsetting.
        switch (dstInfo.colorType()) {
            case kGray_8_SkColorType:
                srcConfig = SkSwizzler::kNoOp8;
                break;
            case kN32_SkColorType:
                srcConfig = SkSwizzler::kNoOp32;
                break;
            case kRGB_565_SkColorType:
                srcConfig = SkSwizzler::kNoOp16;
                break;
            default:
                // This function should only be called if the colorType is supported by jpeg
                SkASSERT(false);
        }
    }

    if (JCS_RGB == fDecoderMgr->dinfo()->out_color_space) {
        srcConfig = SkSwizzler::kRGB;
    }

    Options swizzlerOptions = options;
    if (options.fSubset) {
        // Use fSwizzlerSubset if this is a subset decode.  This is necessary in the case
        // where libjpeg-turbo provides a subset and then we need to subset it further.
        // Also, verify that fSwizzlerSubset is initialized and valid.
        SkASSERT(!fSwizzlerSubset.isEmpty() && fSwizzlerSubset.x() <= options.fSubset->x() &&
                fSwizzlerSubset.width() == options.fSubset->width());
        swizzlerOptions.fSubset = &fSwizzlerSubset;
    }
    fSwizzler.reset(SkSwizzler::CreateSwizzler(srcConfig, nullptr, dstInfo, swizzlerOptions));
    SkASSERT(fSwizzler);
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
    fStorage.reset();

    // Now, given valid output dimensions, we can start the decompress
    if (!jpeg_start_decompress(fDecoderMgr->dinfo())) {
        SkCodecPrintf("start decompress failed\n");
        return kInvalidInput;
    }

    if (options.fSubset) {
        fSwizzlerSubset = *options.fSubset;
    }

#ifdef TURBO_HAS_CROP
    if (options.fSubset) {
        uint32_t startX = options.fSubset->x();
        uint32_t width = options.fSubset->width();

        // libjpeg-turbo may need to align startX to a multiple of the IDCT
        // block size.  If this is the case, it will decrease the value of
        // startX to the appropriate alignment and also increase the value
        // of width so that the right edge of the requested subset remains
        // the same.
        jpeg_crop_scanline(fDecoderMgr->dinfo(), &startX, &width);

        SkASSERT(startX <= (uint32_t) options.fSubset->x());
        SkASSERT(width >= (uint32_t) options.fSubset->width());
        SkASSERT(startX + width >= (uint32_t) options.fSubset->right());

        // Instruct the swizzler (if it is necessary) to further subset the
        // output provided by libjpeg-turbo.
        //
        // We set this here (rather than in the if statement below), so that
        // if (1) we don't need a swizzler for the subset, and (2) we need a
        // swizzler for CMYK, the swizzler will still use the proper subset
        // dimensions.
        //
        // Note that the swizzler will ignore the y and height parameters of
        // the subset.  Since the scanline decoder (and the swizzler) handle
        // one row at a time, only the subsetting in the x-dimension matters.
        fSwizzlerSubset.setXYWH(options.fSubset->x() - startX, 0,
                options.fSubset->width(), options.fSubset->height());

        // We will need a swizzler if libjpeg-turbo cannot provide the exact
        // subset that we request.
        if (startX != (uint32_t) options.fSubset->x() ||
                width != (uint32_t) options.fSubset->width()) {
            this->initializeSwizzler(dstInfo, options);
        }
    }

    // Make sure we have a swizzler if we are converting from CMYK.
    if (!fSwizzler && JCS_CMYK == fDecoderMgr->dinfo()->out_color_space) {
        this->initializeSwizzler(dstInfo, options);
    }
#else
    // We will need a swizzler if we are performing a subset decode or
    // converting from CMYK.
    J_COLOR_SPACE colorSpace = fDecoderMgr->dinfo()->out_color_space;
    if (options.fSubset || JCS_CMYK == colorSpace || JCS_RGB == colorSpace) {
        this->initializeSwizzler(dstInfo, options);
    }
#endif

    return kSuccess;
}

int SkJpegCodec::onGetScanlines(void* dst, int count, size_t dstRowBytes) {
    // Set the jump location for libjpeg errors
    if (setjmp(fDecoderMgr->getJmpBuf())) {
        return fDecoderMgr->returnFailure("setjmp", kInvalidInput);
    }
    // Read rows one at a time
    JSAMPLE* dstRow;
    size_t srcRowBytes = get_row_bytes(fDecoderMgr->dinfo());
    if (fSwizzler) {
        // write data to storage row, then sample using swizzler
        dstRow = fSrcRow;
    } else {
        // write data directly to dst
        SkASSERT(count == 1 || dstRowBytes >= srcRowBytes);
        dstRow = (JSAMPLE*) dst;
    }

    for (int y = 0; y < count; y++) {
        // Read row of the image
        uint32_t rowsDecoded = jpeg_read_scanlines(fDecoderMgr->dinfo(), &dstRow, 1);
        sk_msan_mark_initialized(dstRow, dstRow + srcRowBytes, "skbug.com/4550");
        if (rowsDecoded != 1) {
            fDecoderMgr->dinfo()->output_scanline = this->dstInfo().height();
            return y;
        }

        if (fSwizzler) {
            // use swizzler to sample row
            fSwizzler->swizzle(dst, dstRow);
            dst = SkTAddOffset<JSAMPLE>(dst, dstRowBytes);
        } else {
            dstRow = SkTAddOffset<JSAMPLE>(dstRow, dstRowBytes);
        }
    }
    return count;
}

bool SkJpegCodec::onSkipScanlines(int count) {
    // Set the jump location for libjpeg errors
    if (setjmp(fDecoderMgr->getJmpBuf())) {
        return fDecoderMgr->returnFalse("setjmp");
    }

#ifdef TURBO_HAS_SKIP
    return (uint32_t) count == jpeg_skip_scanlines(fDecoderMgr->dinfo(), count);
#else
    if (!fSrcRow) {
        fStorage.reset(get_row_bytes(fDecoderMgr->dinfo()));
        fSrcRow = fStorage.get();
    }

    for (int y = 0; y < count; y++) {
        if (1 != jpeg_read_scanlines(fDecoderMgr->dinfo(), &fSrcRow, 1)) {
            return false;
        }
    }
    return true;
#endif
}

static bool is_yuv_supported(jpeg_decompress_struct* dinfo) {
    // Scaling is not supported in raw data mode.
    SkASSERT(dinfo->scale_num == dinfo->scale_denom);

    // I can't imagine that this would ever change, but we do depend on it.
    static_assert(8 == DCTSIZE, "DCTSIZE (defined in jpeg library) should always be 8.");

    if (JCS_YCbCr != dinfo->jpeg_color_space) {
        return false;
    }

    SkASSERT(3 == dinfo->num_components);
    SkASSERT(dinfo->comp_info);

    // It is possible to perform a YUV decode for any combination of
    // horizontal and vertical sampling that is supported by
    // libjpeg/libjpeg-turbo.  However, we will start by supporting only the
    // common cases (where U and V have samp_factors of one).
    //
    // The definition of samp_factor is kind of the opposite of what SkCodec
    // thinks of as a sampling factor.  samp_factor is essentially a
    // multiplier, and the larger the samp_factor is, the more samples that
    // there will be.  Ex:
    //     U_plane_width = image_width * (U_h_samp_factor / max_h_samp_factor)
    //
    // Supporting cases where the samp_factors for U or V were larger than
    // that of Y would be an extremely difficult change, given that clients
    // allocate memory as if the size of the Y plane is always the size of the
    // image.  However, this case is very, very rare.
    if  ((1 != dinfo->comp_info[1].h_samp_factor) ||
         (1 != dinfo->comp_info[1].v_samp_factor) ||
         (1 != dinfo->comp_info[2].h_samp_factor) ||
         (1 != dinfo->comp_info[2].v_samp_factor))
    {
        return false;
    }

    // Support all common cases of Y samp_factors.
    // TODO (msarett): As mentioned above, it would be possible to support
    //                 more combinations of samp_factors.  The issues are:
    //                 (1) Are there actually any images that are not covered
    //                     by these cases?
    //                 (2) How much complexity would be added to the
    //                     implementation in order to support these rare
    //                     cases?
    int hSampY = dinfo->comp_info[0].h_samp_factor;
    int vSampY = dinfo->comp_info[0].v_samp_factor;
    return (1 == hSampY && 1 == vSampY) ||
           (2 == hSampY && 1 == vSampY) ||
           (2 == hSampY && 2 == vSampY) ||
           (1 == hSampY && 2 == vSampY) ||
           (4 == hSampY && 1 == vSampY) ||
           (4 == hSampY && 2 == vSampY);
}

bool SkJpegCodec::onQueryYUV8(SkYUVSizeInfo* sizeInfo, SkYUVColorSpace* colorSpace) const {
    jpeg_decompress_struct* dinfo = fDecoderMgr->dinfo();
    if (!is_yuv_supported(dinfo)) {
        return false;
    }

    sizeInfo->fSizes[SkYUVSizeInfo::kY].set(dinfo->comp_info[0].downsampled_width,
                                           dinfo->comp_info[0].downsampled_height);
    sizeInfo->fSizes[SkYUVSizeInfo::kU].set(dinfo->comp_info[1].downsampled_width,
                                           dinfo->comp_info[1].downsampled_height);
    sizeInfo->fSizes[SkYUVSizeInfo::kV].set(dinfo->comp_info[2].downsampled_width,
                                           dinfo->comp_info[2].downsampled_height);
    sizeInfo->fWidthBytes[SkYUVSizeInfo::kY] = dinfo->comp_info[0].width_in_blocks * DCTSIZE;
    sizeInfo->fWidthBytes[SkYUVSizeInfo::kU] = dinfo->comp_info[1].width_in_blocks * DCTSIZE;
    sizeInfo->fWidthBytes[SkYUVSizeInfo::kV] = dinfo->comp_info[2].width_in_blocks * DCTSIZE;

    if (colorSpace) {
        *colorSpace = kJPEG_SkYUVColorSpace;
    }

    return true;
}

SkCodec::Result SkJpegCodec::onGetYUV8Planes(const SkYUVSizeInfo& sizeInfo, void* planes[3]) {
    SkYUVSizeInfo defaultInfo;

    // This will check is_yuv_supported(), so we don't need to here.
    bool supportsYUV = this->onQueryYUV8(&defaultInfo, nullptr);
    if (!supportsYUV ||
            sizeInfo.fSizes[SkYUVSizeInfo::kY] != defaultInfo.fSizes[SkYUVSizeInfo::kY] ||
            sizeInfo.fSizes[SkYUVSizeInfo::kU] != defaultInfo.fSizes[SkYUVSizeInfo::kU] ||
            sizeInfo.fSizes[SkYUVSizeInfo::kV] != defaultInfo.fSizes[SkYUVSizeInfo::kV] ||
            sizeInfo.fWidthBytes[SkYUVSizeInfo::kY] < defaultInfo.fWidthBytes[SkYUVSizeInfo::kY] ||
            sizeInfo.fWidthBytes[SkYUVSizeInfo::kU] < defaultInfo.fWidthBytes[SkYUVSizeInfo::kU] ||
            sizeInfo.fWidthBytes[SkYUVSizeInfo::kV] < defaultInfo.fWidthBytes[SkYUVSizeInfo::kV]) {
        return fDecoderMgr->returnFailure("onGetYUV8Planes", kInvalidInput);
    }

    // Set the jump location for libjpeg errors
    if (setjmp(fDecoderMgr->getJmpBuf())) {
        return fDecoderMgr->returnFailure("setjmp", kInvalidInput);
    }

    // Get a pointer to the decompress info since we will use it quite frequently
    jpeg_decompress_struct* dinfo = fDecoderMgr->dinfo();

    dinfo->raw_data_out = TRUE;
    if (!jpeg_start_decompress(dinfo)) {
        return fDecoderMgr->returnFailure("startDecompress", kInvalidInput);
    }

    // A previous implementation claims that the return value of is_yuv_supported()
    // may change after calling jpeg_start_decompress().  It looks to me like this
    // was caused by a bug in the old code, but we'll be safe and check here.
    SkASSERT(is_yuv_supported(dinfo));

    // Currently, we require that the Y plane dimensions match the image dimensions
    // and that the U and V planes are the same dimensions.
    SkASSERT(sizeInfo.fSizes[SkYUVSizeInfo::kU] == sizeInfo.fSizes[SkYUVSizeInfo::kV]);
    SkASSERT((uint32_t) sizeInfo.fSizes[SkYUVSizeInfo::kY].width() == dinfo->output_width &&
            (uint32_t) sizeInfo.fSizes[SkYUVSizeInfo::kY].height() == dinfo->output_height);

    // Build a JSAMPIMAGE to handle output from libjpeg-turbo.  A JSAMPIMAGE has
    // a 2-D array of pixels for each of the components (Y, U, V) in the image.
    // Cheat Sheet:
    //     JSAMPIMAGE == JSAMPLEARRAY* == JSAMPROW** == JSAMPLE***
    JSAMPARRAY yuv[3];

    // Set aside enough space for pointers to rows of Y, U, and V.
    JSAMPROW rowptrs[2 * DCTSIZE + DCTSIZE + DCTSIZE];
    yuv[0] = &rowptrs[0];           // Y rows (DCTSIZE or 2 * DCTSIZE)
    yuv[1] = &rowptrs[2 * DCTSIZE]; // U rows (DCTSIZE)
    yuv[2] = &rowptrs[3 * DCTSIZE]; // V rows (DCTSIZE)

    // Initialize rowptrs.
    int numYRowsPerBlock = DCTSIZE * dinfo->comp_info[0].v_samp_factor;
    for (int i = 0; i < numYRowsPerBlock; i++) {
        rowptrs[i] = SkTAddOffset<JSAMPLE>(planes[SkYUVSizeInfo::kY],
                i * sizeInfo.fWidthBytes[SkYUVSizeInfo::kY]);
    }
    for (int i = 0; i < DCTSIZE; i++) {
        rowptrs[i + 2 * DCTSIZE] = SkTAddOffset<JSAMPLE>(planes[SkYUVSizeInfo::kU],
                i * sizeInfo.fWidthBytes[SkYUVSizeInfo::kU]);
        rowptrs[i + 3 * DCTSIZE] = SkTAddOffset<JSAMPLE>(planes[SkYUVSizeInfo::kV],
                i * sizeInfo.fWidthBytes[SkYUVSizeInfo::kV]);
    }

    // After each loop iteration, we will increment pointers to Y, U, and V.
    size_t blockIncrementY = numYRowsPerBlock * sizeInfo.fWidthBytes[SkYUVSizeInfo::kY];
    size_t blockIncrementU = DCTSIZE * sizeInfo.fWidthBytes[SkYUVSizeInfo::kU];
    size_t blockIncrementV = DCTSIZE * sizeInfo.fWidthBytes[SkYUVSizeInfo::kV];

    uint32_t numRowsPerBlock = numYRowsPerBlock;

    // We intentionally round down here, as this first loop will only handle
    // full block rows.  As a special case at the end, we will handle any
    // remaining rows that do not make up a full block.
    const int numIters = dinfo->output_height / numRowsPerBlock;
    for (int i = 0; i < numIters; i++) {
        JDIMENSION linesRead = jpeg_read_raw_data(dinfo, yuv, numRowsPerBlock);
        if (linesRead < numRowsPerBlock) {
            // FIXME: Handle incomplete YUV decodes without signalling an error.
            return kInvalidInput;
        }

        // Update rowptrs.
        for (int i = 0; i < numYRowsPerBlock; i++) {
            rowptrs[i] += blockIncrementY;
        }
        for (int i = 0; i < DCTSIZE; i++) {
            rowptrs[i + 2 * DCTSIZE] += blockIncrementU;
            rowptrs[i + 3 * DCTSIZE] += blockIncrementV;
        }
    }

    uint32_t remainingRows = dinfo->output_height - dinfo->output_scanline;
    SkASSERT(remainingRows == dinfo->output_height % numRowsPerBlock);
    SkASSERT(dinfo->output_scanline == numIters * numRowsPerBlock);
    if (remainingRows > 0) {
        // libjpeg-turbo needs memory to be padded by the block sizes.  We will fulfill
        // this requirement using a dummy row buffer.
        // FIXME: Should SkCodec have an extra memory buffer that can be shared among
        //        all of the implementations that use temporary/garbage memory?
        SkAutoTMalloc<JSAMPLE> dummyRow(sizeInfo.fWidthBytes[SkYUVSizeInfo::kY]);
        for (int i = remainingRows; i < numYRowsPerBlock; i++) {
            rowptrs[i] = dummyRow.get();
        }
        int remainingUVRows = dinfo->comp_info[1].downsampled_height - DCTSIZE * numIters;
        for (int i = remainingUVRows; i < DCTSIZE; i++) {
            rowptrs[i + 2 * DCTSIZE] = dummyRow.get();
            rowptrs[i + 3 * DCTSIZE] = dummyRow.get();
        }

        JDIMENSION linesRead = jpeg_read_raw_data(dinfo, yuv, numRowsPerBlock);
        if (linesRead < remainingRows) {
            // FIXME: Handle incomplete YUV decodes without signalling an error.
            return kInvalidInput;
        }
    }

    return kSuccess;
}
