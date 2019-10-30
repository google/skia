/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/codec/SkJpegCodec.h"

#include "include/codec/SkCodec.h"
#include "include/core/SkStream.h"
#include "include/core/SkTypes.h"
#include "include/private/SkColorData.h"
#include "include/private/SkTemplates.h"
#include "include/private/SkTo.h"
#include "src/codec/SkCodecPriv.h"
#include "src/codec/SkJpegDecoderMgr.h"
#include "src/codec/SkParseEncodedOrigin.h"
#include "src/pdf/SkJpegInfo.h"

// stdio is needed for libjpeg-turbo
#include <stdio.h>
#include "src/codec/SkJpegUtility.h"

// This warning triggers false postives way too often in here.
#if defined(__GNUC__) && !defined(__clang__)
    #pragma GCC diagnostic ignored "-Wclobbered"
#endif

extern "C" {
    #include "jerror.h"
    #include "jpeglib.h"
}

bool SkJpegCodec::IsJpeg(const void* buffer, size_t bytesRead) {
    constexpr uint8_t jpegSig[] = { 0xFF, 0xD8, 0xFF };
    return bytesRead >= 3 && !memcmp(buffer, jpegSig, sizeof(jpegSig));
}

const uint32_t kExifHeaderSize = 14;
const uint32_t kExifMarker = JPEG_APP0 + 1;

static bool is_orientation_marker(jpeg_marker_struct* marker, SkEncodedOrigin* orientation) {
    if (kExifMarker != marker->marker || marker->data_length < kExifHeaderSize) {
        return false;
    }

    constexpr uint8_t kExifSig[] { 'E', 'x', 'i', 'f', '\0' };
    if (memcmp(marker->data, kExifSig, sizeof(kExifSig))) {
        return false;
    }

    // Account for 'E', 'x', 'i', 'f', '\0', '<fill byte>'.
    constexpr size_t kOffset = 6;
    return SkParseEncodedOrigin(marker->data + kOffset, marker->data_length - kOffset,
            orientation);
}

static SkEncodedOrigin get_exif_orientation(jpeg_decompress_struct* dinfo) {
    SkEncodedOrigin orientation;
    for (jpeg_marker_struct* marker = dinfo->marker_list; marker; marker = marker->next) {
        if (is_orientation_marker(marker, &orientation)) {
            return orientation;
        }
    }

    return kDefault_SkEncodedOrigin;
}

static bool is_icc_marker(jpeg_marker_struct* marker) {
    if (kICCMarker != marker->marker || marker->data_length < kICCMarkerHeaderSize) {
        return false;
    }

    return !memcmp(marker->data, kICCSig, sizeof(kICCSig));
}

/*
 * ICC profiles may be stored using a sequence of multiple markers.  We obtain the ICC profile
 * in two steps:
 *     (1) Discover all ICC profile markers and verify that they are numbered properly.
 *     (2) Copy the data from each marker into a contiguous ICC profile.
 */
static std::unique_ptr<SkEncodedInfo::ICCProfile> read_color_profile(jpeg_decompress_struct* dinfo)
{
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
            SkASSERT(marker->data_length >= kICCMarkerHeaderSize);
            totalBytes += marker->data_length - kICCMarkerHeaderSize;
        }
    }

    if (0 == totalBytes) {
        // No non-empty ICC profile markers were found.
        return nullptr;
    }

    // Combine the ICC marker data into a contiguous profile.
    sk_sp<SkData> iccData = SkData::MakeUninitialized(totalBytes);
    void* dst = iccData->writable_data();
    for (uint32_t i = 1; i <= numMarkers; i++) {
        jpeg_marker_struct* marker = markerSequence[i];
        if (!marker) {
            SkCodecPrintf("ICC Profile Error: Missing marker %d of %d.\n", i, numMarkers);
            return nullptr;
        }

        void* src = SkTAddOffset<void>(marker->data, kICCMarkerHeaderSize);
        size_t bytes = marker->data_length - kICCMarkerHeaderSize;
        memcpy(dst, src, bytes);
        dst = SkTAddOffset<void>(dst, bytes);
    }

    return SkEncodedInfo::ICCProfile::Make(std::move(iccData));
}

SkCodec::Result SkJpegCodec::ReadHeader(SkStream* stream, SkCodec** codecOut,
        JpegDecoderMgr** decoderMgrOut,
        std::unique_ptr<SkEncodedInfo::ICCProfile> defaultColorProfile) {

    // Create a JpegDecoderMgr to own all of the decompress information
    std::unique_ptr<JpegDecoderMgr> decoderMgr(new JpegDecoderMgr(stream));

    // libjpeg errors will be caught and reported here
    skjpeg_error_mgr::AutoPushJmpBuf jmp(decoderMgr->errorMgr());
    if (setjmp(jmp)) {
        return decoderMgr->returnFailure("ReadHeader", kInvalidInput);
    }

    // Initialize the decompress info and the source manager
    decoderMgr->init();
    auto* dinfo = decoderMgr->dinfo();

    // Instruct jpeg library to save the markers that we care about.  Since
    // the orientation and color profile will not change, we can skip this
    // step on rewinds.
    if (codecOut) {
        jpeg_save_markers(dinfo, kExifMarker, 0xFFFF);
        jpeg_save_markers(dinfo, kICCMarker, 0xFFFF);
    }

    // Read the jpeg header
    switch (jpeg_read_header(dinfo, true)) {
        case JPEG_HEADER_OK:
            break;
        case JPEG_SUSPENDED:
            return decoderMgr->returnFailure("ReadHeader", kIncompleteInput);
        default:
            return decoderMgr->returnFailure("ReadHeader", kInvalidInput);
    }

    if (codecOut) {
        // Get the encoded color type
        SkEncodedInfo::Color color;
        if (!decoderMgr->getEncodedColor(&color)) {
            return kInvalidInput;
        }

        SkEncodedOrigin orientation = get_exif_orientation(dinfo);
        auto profile = read_color_profile(dinfo);
        if (profile) {
            auto type = profile->profile()->data_color_space;
            switch (decoderMgr->dinfo()->jpeg_color_space) {
                case JCS_CMYK:
                case JCS_YCCK:
                    if (type != skcms_Signature_CMYK) {
                        profile = nullptr;
                    }
                    break;
                case JCS_GRAYSCALE:
                    if (type != skcms_Signature_Gray &&
                        type != skcms_Signature_RGB)
                    {
                        profile = nullptr;
                    }
                    break;
                default:
                    if (type != skcms_Signature_RGB) {
                        profile = nullptr;
                    }
                    break;
            }
        }
        if (!profile) {
            profile = std::move(defaultColorProfile);
        }

        SkEncodedInfo info = SkEncodedInfo::Make(dinfo->image_width, dinfo->image_height,
                                                 color, SkEncodedInfo::kOpaque_Alpha, 8,
                                                 std::move(profile));

        SkJpegCodec* codec = new SkJpegCodec(std::move(info), std::unique_ptr<SkStream>(stream),
                                             decoderMgr.release(), orientation);
        *codecOut = codec;
    } else {
        SkASSERT(nullptr != decoderMgrOut);
        *decoderMgrOut = decoderMgr.release();
    }
    return kSuccess;
}

std::unique_ptr<SkCodec> SkJpegCodec::MakeFromStream(std::unique_ptr<SkStream> stream,
                                                     Result* result) {
    return SkJpegCodec::MakeFromStream(std::move(stream), result, nullptr);
}

std::unique_ptr<SkCodec> SkJpegCodec::MakeFromStream(std::unique_ptr<SkStream> stream,
        Result* result, std::unique_ptr<SkEncodedInfo::ICCProfile> defaultColorProfile) {
    SkCodec* codec = nullptr;
    *result = ReadHeader(stream.get(), &codec, nullptr, std::move(defaultColorProfile));
    if (kSuccess == *result) {
        // Codec has taken ownership of the stream, we do not need to delete it
        SkASSERT(codec);
        stream.release();
        return std::unique_ptr<SkCodec>(codec);
    }
    return nullptr;
}

SkJpegCodec::SkJpegCodec(SkEncodedInfo&& info, std::unique_ptr<SkStream> stream,
                         JpegDecoderMgr* decoderMgr, SkEncodedOrigin origin)
    : INHERITED(std::move(info), skcms_PixelFormat_RGBA_8888, std::move(stream), origin)
    , fDecoderMgr(decoderMgr)
    , fReadyState(decoderMgr->dinfo()->global_state)
    , fSwizzleSrcRow(nullptr)
    , fColorXformSrcRow(nullptr)
    , fSwizzlerSubset(SkIRect::MakeEmpty())
{}

/*
 * Return the row bytes of a particular image type and width
 */
static size_t get_row_bytes(const j_decompress_ptr dinfo) {
    const size_t colorBytes = (dinfo->out_color_space == JCS_RGB565) ? 2 :
            dinfo->out_color_components;
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
    dinfo.image_width = this->dimensions().width();
    dinfo.image_height = this->dimensions().height();
    dinfo.global_state = fReadyState;
    calc_output_dimensions(&dinfo, num, denom);

    // Return the calculated output dimensions for the given scale
    return SkISize::Make(dinfo.output_width, dinfo.output_height);
}

bool SkJpegCodec::onRewind() {
    JpegDecoderMgr* decoderMgr = nullptr;
    if (kSuccess != ReadHeader(this->stream(), nullptr, &decoderMgr, nullptr)) {
        return fDecoderMgr->returnFalse("onRewind");
    }
    SkASSERT(nullptr != decoderMgr);
    fDecoderMgr.reset(decoderMgr);

    fSwizzler.reset(nullptr);
    fSwizzleSrcRow = nullptr;
    fColorXformSrcRow = nullptr;
    fStorage.reset();

    return true;
}

bool SkJpegCodec::conversionSupported(const SkImageInfo& dstInfo, bool srcIsOpaque,
                                      bool needsColorXform) {
    SkASSERT(srcIsOpaque);

    if (kUnknown_SkAlphaType == dstInfo.alphaType()) {
        return false;
    }

    if (kOpaque_SkAlphaType != dstInfo.alphaType()) {
        SkCodecPrintf("Warning: an opaque image should be decoded as opaque "
                      "- it is being decoded as non-opaque, which will draw slower\n");
    }

    J_COLOR_SPACE encodedColorType = fDecoderMgr->dinfo()->jpeg_color_space;

    // Check for valid color types and set the output color space
    switch (dstInfo.colorType()) {
        case kRGBA_8888_SkColorType:
            fDecoderMgr->dinfo()->out_color_space = JCS_EXT_RGBA;
            break;
        case kBGRA_8888_SkColorType:
            if (needsColorXform) {
                // Always using RGBA as the input format for color xforms makes the
                // implementation a little simpler.
                fDecoderMgr->dinfo()->out_color_space = JCS_EXT_RGBA;
            } else {
                fDecoderMgr->dinfo()->out_color_space = JCS_EXT_BGRA;
            }
            break;
        case kRGB_565_SkColorType:
            if (needsColorXform) {
                fDecoderMgr->dinfo()->out_color_space = JCS_EXT_RGBA;
            } else {
                fDecoderMgr->dinfo()->dither_mode = JDITHER_NONE;
                fDecoderMgr->dinfo()->out_color_space = JCS_RGB565;
            }
            break;
        case kGray_8_SkColorType:
            if (JCS_GRAYSCALE != encodedColorType) {
                return false;
            }

            if (needsColorXform) {
                fDecoderMgr->dinfo()->out_color_space = JCS_EXT_RGBA;
            } else {
                fDecoderMgr->dinfo()->out_color_space = JCS_GRAYSCALE;
            }
            break;
        case kRGBA_F16_SkColorType:
            SkASSERT(needsColorXform);
            fDecoderMgr->dinfo()->out_color_space = JCS_EXT_RGBA;
            break;
        default:
            return false;
    }

    // Check if we will decode to CMYK.  libjpeg-turbo does not convert CMYK to RGBA, so
    // we must do it ourselves.
    if (JCS_CMYK == encodedColorType || JCS_YCCK == encodedColorType) {
        fDecoderMgr->dinfo()->out_color_space = JCS_CMYK;
    }

    return true;
}

/*
 * Checks if we can natively scale to the requested dimensions and natively scales the
 * dimensions if possible
 */
bool SkJpegCodec::onDimensionsSupported(const SkISize& size) {
    skjpeg_error_mgr::AutoPushJmpBuf jmp(fDecoderMgr->errorMgr());
    if (setjmp(jmp)) {
        return fDecoderMgr->returnFalse("onDimensionsSupported");
    }

    const unsigned int dstWidth = size.width();
    const unsigned int dstHeight = size.height();

    // Set up a fake decompress struct in order to use libjpeg to calculate output dimensions
    // FIXME: Why is this necessary?
    jpeg_decompress_struct dinfo;
    sk_bzero(&dinfo, sizeof(dinfo));
    dinfo.image_width = this->dimensions().width();
    dinfo.image_height = this->dimensions().height();
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

int SkJpegCodec::readRows(const SkImageInfo& dstInfo, void* dst, size_t rowBytes, int count,
                          const Options& opts) {
    // Set the jump location for libjpeg-turbo errors
    skjpeg_error_mgr::AutoPushJmpBuf jmp(fDecoderMgr->errorMgr());
    if (setjmp(jmp)) {
        return 0;
    }

    // When fSwizzleSrcRow is non-null, it means that we need to swizzle.  In this case,
    // we will always decode into fSwizzlerSrcRow before swizzling into the next buffer.
    // We can never swizzle "in place" because the swizzler may perform sampling and/or
    // subsetting.
    // When fColorXformSrcRow is non-null, it means that we need to color xform and that
    // we cannot color xform "in place" (many times we can, but not when the src and dst
    // are different sizes).
    // In this case, we will color xform from fColorXformSrcRow into the dst.
    JSAMPLE* decodeDst = (JSAMPLE*) dst;
    uint32_t* swizzleDst = (uint32_t*) dst;
    size_t decodeDstRowBytes = rowBytes;
    size_t swizzleDstRowBytes = rowBytes;
    int dstWidth = opts.fSubset ? opts.fSubset->width() : dstInfo.width();
    if (fSwizzleSrcRow && fColorXformSrcRow) {
        decodeDst = (JSAMPLE*) fSwizzleSrcRow;
        swizzleDst = fColorXformSrcRow;
        decodeDstRowBytes = 0;
        swizzleDstRowBytes = 0;
        dstWidth = fSwizzler->swizzleWidth();
    } else if (fColorXformSrcRow) {
        decodeDst = (JSAMPLE*) fColorXformSrcRow;
        swizzleDst = fColorXformSrcRow;
        decodeDstRowBytes = 0;
        swizzleDstRowBytes = 0;
    } else if (fSwizzleSrcRow) {
        decodeDst = (JSAMPLE*) fSwizzleSrcRow;
        decodeDstRowBytes = 0;
        dstWidth = fSwizzler->swizzleWidth();
    }

    for (int y = 0; y < count; y++) {
        uint32_t lines = jpeg_read_scanlines(fDecoderMgr->dinfo(), &decodeDst, 1);
        if (0 == lines) {
            return y;
        }

        if (fSwizzler) {
            fSwizzler->swizzle(swizzleDst, decodeDst);
        }

        if (this->colorXform()) {
            this->applyColorXform(dst, swizzleDst, dstWidth);
            dst = SkTAddOffset<void>(dst, rowBytes);
        }

        decodeDst = SkTAddOffset<JSAMPLE>(decodeDst, decodeDstRowBytes);
        swizzleDst = SkTAddOffset<uint32_t>(swizzleDst, swizzleDstRowBytes);
    }

    return count;
}

/*
 * This is a bit tricky.  We only need the swizzler to do format conversion if the jpeg is
 * encoded as CMYK.
 * And even then we still may not need it.  If the jpeg has a CMYK color profile and a color
 * xform, the color xform will handle the CMYK->RGB conversion.
 */
static inline bool needs_swizzler_to_convert_from_cmyk(J_COLOR_SPACE jpegColorType,
                                                       const skcms_ICCProfile* srcProfile,
                                                       bool hasColorSpaceXform) {
    if (JCS_CMYK != jpegColorType) {
        return false;
    }

    bool hasCMYKColorSpace = srcProfile && srcProfile->data_color_space == skcms_Signature_CMYK;
    return !hasCMYKColorSpace || !hasColorSpaceXform;
}

/*
 * Performs the jpeg decode
 */
SkCodec::Result SkJpegCodec::onGetPixels(const SkImageInfo& dstInfo,
                                         void* dst, size_t dstRowBytes,
                                         const Options& options,
                                         int* rowsDecoded) {
    if (options.fSubset) {
        // Subsets are not supported.
        return kUnimplemented;
    }

    // Get a pointer to the decompress info since we will use it quite frequently
    jpeg_decompress_struct* dinfo = fDecoderMgr->dinfo();

    // Set the jump location for libjpeg errors
    skjpeg_error_mgr::AutoPushJmpBuf jmp(fDecoderMgr->errorMgr());
    if (setjmp(jmp)) {
        return fDecoderMgr->returnFailure("setjmp", kInvalidInput);
    }

    if (!jpeg_start_decompress(dinfo)) {
        return fDecoderMgr->returnFailure("startDecompress", kInvalidInput);
    }

    // The recommended output buffer height should always be 1 in high quality modes.
    // If it's not, we want to know because it means our strategy is not optimal.
    SkASSERT(1 == dinfo->rec_outbuf_height);

    if (needs_swizzler_to_convert_from_cmyk(dinfo->out_color_space,
                                            this->getEncodedInfo().profile(), this->colorXform())) {
        this->initializeSwizzler(dstInfo, options, true);
    }

    this->allocateStorage(dstInfo);

    int rows = this->readRows(dstInfo, dst, dstRowBytes, dstInfo.height(), options);
    if (rows < dstInfo.height()) {
        *rowsDecoded = rows;
        return fDecoderMgr->returnFailure("Incomplete image data", kIncompleteInput);
    }

    return kSuccess;
}

void SkJpegCodec::allocateStorage(const SkImageInfo& dstInfo) {
    int dstWidth = dstInfo.width();

    size_t swizzleBytes = 0;
    if (fSwizzler) {
        swizzleBytes = get_row_bytes(fDecoderMgr->dinfo());
        dstWidth = fSwizzler->swizzleWidth();
        SkASSERT(!this->colorXform() || SkIsAlign4(swizzleBytes));
    }

    size_t xformBytes = 0;

    if (this->colorXform() && sizeof(uint32_t) != dstInfo.bytesPerPixel()) {
        xformBytes = dstWidth * sizeof(uint32_t);
    }

    size_t totalBytes = swizzleBytes + xformBytes;
    if (totalBytes > 0) {
        fStorage.reset(totalBytes);
        fSwizzleSrcRow = (swizzleBytes > 0) ? fStorage.get() : nullptr;
        fColorXformSrcRow = (xformBytes > 0) ?
                SkTAddOffset<uint32_t>(fStorage.get(), swizzleBytes) : nullptr;
    }
}

void SkJpegCodec::initializeSwizzler(const SkImageInfo& dstInfo, const Options& options,
        bool needsCMYKToRGB) {
    Options swizzlerOptions = options;
    if (options.fSubset) {
        // Use fSwizzlerSubset if this is a subset decode.  This is necessary in the case
        // where libjpeg-turbo provides a subset and then we need to subset it further.
        // Also, verify that fSwizzlerSubset is initialized and valid.
        SkASSERT(!fSwizzlerSubset.isEmpty() && fSwizzlerSubset.x() <= options.fSubset->x() &&
                fSwizzlerSubset.width() == options.fSubset->width());
        swizzlerOptions.fSubset = &fSwizzlerSubset;
    }

    SkImageInfo swizzlerDstInfo = dstInfo;
    if (this->colorXform()) {
        // The color xform will be expecting RGBA 8888 input.
        swizzlerDstInfo = swizzlerDstInfo.makeColorType(kRGBA_8888_SkColorType);
    }

    if (needsCMYKToRGB) {
        // The swizzler is used to convert to from CMYK.
        // The swizzler does not use the width or height on SkEncodedInfo.
        auto swizzlerInfo = SkEncodedInfo::Make(0, 0, SkEncodedInfo::kInvertedCMYK_Color,
                                                SkEncodedInfo::kOpaque_Alpha, 8);
        fSwizzler = SkSwizzler::Make(swizzlerInfo, nullptr, swizzlerDstInfo, swizzlerOptions);
    } else {
        int srcBPP = 0;
        switch (fDecoderMgr->dinfo()->out_color_space) {
            case JCS_EXT_RGBA:
            case JCS_EXT_BGRA:
            case JCS_CMYK:
                srcBPP = 4;
                break;
            case JCS_RGB565:
                srcBPP = 2;
                break;
            case JCS_GRAYSCALE:
                srcBPP = 1;
                break;
            default:
                SkASSERT(false);
                break;
        }
        fSwizzler = SkSwizzler::MakeSimple(srcBPP, swizzlerDstInfo, swizzlerOptions);
    }
    SkASSERT(fSwizzler);
}

SkSampler* SkJpegCodec::getSampler(bool createIfNecessary) {
    if (!createIfNecessary || fSwizzler) {
        SkASSERT(!fSwizzler || (fSwizzleSrcRow && fStorage.get() == fSwizzleSrcRow));
        return fSwizzler.get();
    }

    bool needsCMYKToRGB = needs_swizzler_to_convert_from_cmyk(
            fDecoderMgr->dinfo()->out_color_space, this->getEncodedInfo().profile(),
            this->colorXform());
    this->initializeSwizzler(this->dstInfo(), this->options(), needsCMYKToRGB);
    this->allocateStorage(this->dstInfo());
    return fSwizzler.get();
}

SkCodec::Result SkJpegCodec::onStartScanlineDecode(const SkImageInfo& dstInfo,
        const Options& options) {
    // Set the jump location for libjpeg errors
    skjpeg_error_mgr::AutoPushJmpBuf jmp(fDecoderMgr->errorMgr());
    if (setjmp(jmp)) {
        SkCodecPrintf("setjmp: Error from libjpeg\n");
        return kInvalidInput;
    }

    if (!jpeg_start_decompress(fDecoderMgr->dinfo())) {
        SkCodecPrintf("start decompress failed\n");
        return kInvalidInput;
    }

    bool needsCMYKToRGB = needs_swizzler_to_convert_from_cmyk(
            fDecoderMgr->dinfo()->out_color_space, this->getEncodedInfo().profile(),
            this->colorXform());
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
            this->initializeSwizzler(dstInfo, options, needsCMYKToRGB);
        }
    }

    // Make sure we have a swizzler if we are converting from CMYK.
    if (!fSwizzler && needsCMYKToRGB) {
        this->initializeSwizzler(dstInfo, options, true);
    }

    this->allocateStorage(dstInfo);

    return kSuccess;
}

int SkJpegCodec::onGetScanlines(void* dst, int count, size_t dstRowBytes) {
    int rows = this->readRows(this->dstInfo(), dst, dstRowBytes, count, this->options());
    if (rows < count) {
        // This allows us to skip calling jpeg_finish_decompress().
        fDecoderMgr->dinfo()->output_scanline = this->dstInfo().height();
    }

    return rows;
}

bool SkJpegCodec::onSkipScanlines(int count) {
    // Set the jump location for libjpeg errors
    skjpeg_error_mgr::AutoPushJmpBuf jmp(fDecoderMgr->errorMgr());
    if (setjmp(jmp)) {
        return fDecoderMgr->returnFalse("onSkipScanlines");
    }

    return (uint32_t) count == jpeg_skip_scanlines(fDecoderMgr->dinfo(), count);
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

bool SkJpegCodec::onQueryYUV8(SkYUVASizeInfo* sizeInfo, SkYUVColorSpace* colorSpace) const {
    jpeg_decompress_struct* dinfo = fDecoderMgr->dinfo();
    if (!is_yuv_supported(dinfo)) {
        return false;
    }

    jpeg_component_info * comp_info = dinfo->comp_info;
    for (int i = 0; i < 3; ++i) {
        sizeInfo->fSizes[i].set(comp_info[i].downsampled_width, comp_info[i].downsampled_height);
        sizeInfo->fWidthBytes[i] = comp_info[i].width_in_blocks * DCTSIZE;
    }

    // JPEG never has an alpha channel
    sizeInfo->fSizes[3].fHeight = sizeInfo->fSizes[3].fWidth = sizeInfo->fWidthBytes[3] = 0;

    sizeInfo->fOrigin = this->getOrigin();

    if (colorSpace) {
        *colorSpace = kJPEG_SkYUVColorSpace;
    }

    return true;
}

SkCodec::Result SkJpegCodec::onGetYUV8Planes(const SkYUVASizeInfo& sizeInfo,
                                             void* planes[SkYUVASizeInfo::kMaxCount]) {
    SkYUVASizeInfo defaultInfo;

    // This will check is_yuv_supported(), so we don't need to here.
    bool supportsYUV = this->onQueryYUV8(&defaultInfo, nullptr);
    if (!supportsYUV ||
            sizeInfo.fSizes[0] != defaultInfo.fSizes[0] ||
            sizeInfo.fSizes[1] != defaultInfo.fSizes[1] ||
            sizeInfo.fSizes[2] != defaultInfo.fSizes[2] ||
            sizeInfo.fWidthBytes[0] < defaultInfo.fWidthBytes[0] ||
            sizeInfo.fWidthBytes[1] < defaultInfo.fWidthBytes[1] ||
            sizeInfo.fWidthBytes[2] < defaultInfo.fWidthBytes[2]) {
        return fDecoderMgr->returnFailure("onGetYUV8Planes", kInvalidInput);
    }

    // Set the jump location for libjpeg errors
    skjpeg_error_mgr::AutoPushJmpBuf jmp(fDecoderMgr->errorMgr());
    if (setjmp(jmp)) {
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
    SkASSERT(sizeInfo.fSizes[1] == sizeInfo.fSizes[2]);
    SkASSERT((uint32_t) sizeInfo.fSizes[0].width() == dinfo->output_width &&
             (uint32_t) sizeInfo.fSizes[0].height() == dinfo->output_height);

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
        rowptrs[i] = SkTAddOffset<JSAMPLE>(planes[0], i * sizeInfo.fWidthBytes[0]);
    }
    for (int i = 0; i < DCTSIZE; i++) {
        rowptrs[i + 2 * DCTSIZE] =
            SkTAddOffset<JSAMPLE>(planes[1], i * sizeInfo.fWidthBytes[1]);
        rowptrs[i + 3 * DCTSIZE] =
            SkTAddOffset<JSAMPLE>(planes[2], i * sizeInfo.fWidthBytes[2]);
    }

    // After each loop iteration, we will increment pointers to Y, U, and V.
    size_t blockIncrementY = numYRowsPerBlock * sizeInfo.fWidthBytes[0];
    size_t blockIncrementU = DCTSIZE * sizeInfo.fWidthBytes[1];
    size_t blockIncrementV = DCTSIZE * sizeInfo.fWidthBytes[2];

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
        SkAutoTMalloc<JSAMPLE> dummyRow(sizeInfo.fWidthBytes[0]);
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

// This function is declared in SkJpegInfo.h, used by SkPDF.
bool SkGetJpegInfo(const void* data, size_t len,
                   SkISize* size,
                   SkEncodedInfo::Color* colorType,
                   SkEncodedOrigin* orientation) {
    if (!SkJpegCodec::IsJpeg(data, len)) {
        return false;
    }

    SkMemoryStream stream(data, len);
    JpegDecoderMgr decoderMgr(&stream);
    // libjpeg errors will be caught and reported here
    skjpeg_error_mgr::AutoPushJmpBuf jmp(decoderMgr.errorMgr());
    if (setjmp(jmp)) {
        return false;
    }
    decoderMgr.init();
    jpeg_decompress_struct* dinfo = decoderMgr.dinfo();
    jpeg_save_markers(dinfo, kExifMarker, 0xFFFF);
    jpeg_save_markers(dinfo, kICCMarker, 0xFFFF);
    if (JPEG_HEADER_OK != jpeg_read_header(dinfo, true)) {
        return false;
    }
    SkEncodedInfo::Color encodedColorType;
    if (!decoderMgr.getEncodedColor(&encodedColorType)) {
        return false;  // Unable to interpret the color channels as colors.
    }
    if (colorType) {
        *colorType = encodedColorType;
    }
    if (orientation) {
        *orientation = get_exif_orientation(dinfo);
    }
    if (size) {
        *size = {SkToS32(dinfo->image_width), SkToS32(dinfo->image_height)};
    }
    return true;
}
