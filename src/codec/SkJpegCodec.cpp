/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/codec/SkJpegCodec.h"

#include "include/codec/SkCodec.h"
#include "include/codec/SkJpegDecoder.h"
#include "include/core/SkAlphaType.h"
#include "include/core/SkColorType.h"
#include "include/core/SkData.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "include/core/SkTypes.h"
#include "include/core/SkYUVAInfo.h"
#include "include/private/base/SkAlign.h"
#include "include/private/base/SkTemplates.h"
#include "modules/skcms/skcms.h"
#include "src/codec/SkCodecPriv.h"
#include "src/codec/SkJpegConstants.h"
#include "src/codec/SkJpegDecoderMgr.h"
#include "src/codec/SkJpegMetadataDecoderImpl.h"
#include "src/codec/SkJpegPriv.h"
#include "src/codec/SkParseEncodedOrigin.h"
#include "src/codec/SkSwizzler.h"

#ifdef SK_CODEC_DECODES_JPEG_GAINMAPS
#include "include/private/SkGainmapInfo.h"
#endif  // SK_CODEC_DECODES_JPEG_GAINMAPS

#include <array>
#include <csetjmp>
#include <cstring>
#include <utility>

using namespace skia_private;

class SkSampler;
struct SkGainmapInfo;

// This warning triggers false postives way too often in here.
#if defined(__GNUC__) && !defined(__clang__)
    #pragma GCC diagnostic ignored "-Wclobbered"
#endif

extern "C" {
    #include "jpeglib.h"  // NO_G3_REWRITE
}

bool SkJpegCodec::IsJpeg(const void* buffer, size_t bytesRead) {
    return bytesRead >= sizeof(kJpegSig) && !memcmp(buffer, kJpegSig, sizeof(kJpegSig));
}

SkJpegMarkerList get_sk_marker_list(jpeg_decompress_struct* dinfo) {
    SkJpegMarkerList markerList;
    for (auto* marker = dinfo->marker_list; marker; marker = marker->next) {
        markerList.emplace_back(marker->marker,
                                SkData::MakeWithoutCopy(marker->data, marker->data_length));
    }
    return markerList;
}

static SkEncodedOrigin get_exif_orientation(sk_sp<SkData> exifData) {
    SkEncodedOrigin origin = kDefault_SkEncodedOrigin;
    if (exifData && SkParseEncodedOrigin(exifData->bytes(), exifData->size(), &origin)) {
        return origin;
    }
    return kDefault_SkEncodedOrigin;
}

SkCodec::Result SkJpegCodec::ReadHeader(
        SkStream* stream,
        SkCodec** codecOut,
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
        jpeg_save_markers(dinfo, kMpfMarker, 0xFFFF);
    }

    // Read the jpeg header
    switch (jpeg_read_header(dinfo, TRUE)) {
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

        auto metadataDecoder =
                std::make_unique<SkJpegMetadataDecoderImpl>(get_sk_marker_list(dinfo));

        SkEncodedOrigin orientation =
                get_exif_orientation(metadataDecoder->getExifMetadata(/*copyData=*/false));

        std::unique_ptr<SkEncodedInfo::ICCProfile> profile;
        if (auto iccProfileData = metadataDecoder->getICCProfileData(/*copyData=*/true)) {
            profile = SkEncodedInfo::ICCProfile::Make(std::move(iccProfileData));
        }
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

        SkJpegCodec* codec = new SkJpegCodec(std::move(info),
                                             std::unique_ptr<SkStream>(stream),
                                             decoderMgr.release(),
                                             orientation);
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
    SkASSERT(result);
    if (!stream) {
        *result = SkCodec::kInvalidInput;
        return nullptr;
    }
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

SkJpegCodec::SkJpegCodec(SkEncodedInfo&& info,
                         std::unique_ptr<SkStream> stream,
                         JpegDecoderMgr* decoderMgr,
                         SkEncodedOrigin origin)
        : INHERITED(std::move(info), skcms_PixelFormat_RGBA_8888, std::move(stream), origin)
        , fDecoderMgr(decoderMgr)
        , fReadyState(decoderMgr->dinfo()->global_state) {}
SkJpegCodec::~SkJpegCodec() = default;

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

    // Set up a fake decompress struct in order to use libjpeg to calculate output dimensions.
    // This isn't conventional use of libjpeg-turbo but initializing the decompress struct with
    // jpeg_create_decompress allows for less violation of the API regardless of the version.
    jpeg_decompress_struct dinfo;
    jpeg_create_decompress(&dinfo);
    dinfo.image_width = this->dimensions().width();
    dinfo.image_height = this->dimensions().height();
    dinfo.global_state = fReadyState;
    calc_output_dimensions(&dinfo, num, denom);
    SkISize outputDimensions = SkISize::Make(dinfo.output_width, dinfo.output_height);
    jpeg_destroy_decompress(&dinfo);

    return outputDimensions;
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
        case kBGRA_10101010_XR_SkColorType:
        case kBGR_101010x_XR_SkColorType:
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
    // This isn't conventional use of libjpeg-turbo but initializing the decompress struct with
    // jpeg_create_decompress allows for less violation of the API regardless of the version.
    // FIXME: Why is this necessary?
    jpeg_decompress_struct dinfo;
    jpeg_create_decompress(&dinfo);
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
            jpeg_destroy_decompress(&dinfo);
            return false;
        }

        // Try the next scale
        num -= 1;
        calc_output_dimensions(&dinfo, num, denom);
    }
    jpeg_destroy_decompress(&dinfo);

    fDecoderMgr->dinfo()->scale_num = num;
    fDecoderMgr->dinfo()->scale_denom = denom;
    return true;
}

SkCodec::Result SkJpegCodec::readRows(const SkImageInfo& dstInfo, void* dst, size_t rowBytes, int count,
                          const Options& opts, int* rowsDecoded) {
    // Set the jump location for libjpeg-turbo errors
    skjpeg_error_mgr::AutoPushJmpBuf jmp(fDecoderMgr->errorMgr());
    if (setjmp(jmp)) {
        *rowsDecoded = 0;
        return kInvalidInput;
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
            *rowsDecoded = y;
            return kSuccess;
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

    *rowsDecoded = count;
    return kSuccess;
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

    const bool isProgressive = dinfo->progressive_mode;
    if (isProgressive) {
       dinfo->buffered_image = TRUE;
       jpeg_start_decompress(dinfo);
    } else if (!jpeg_start_decompress(dinfo)) {
        return fDecoderMgr->returnFailure("startDecompress", kInvalidInput);
    }

    // The recommended output buffer height should always be 1 in high quality modes.
    // If it's not, we want to know because it means our strategy is not optimal.
    SkASSERT(1 == dinfo->rec_outbuf_height);

    if (needs_swizzler_to_convert_from_cmyk(dinfo->out_color_space,
                                            this->getEncodedInfo().profile(), this->colorXform())) {
        this->initializeSwizzler(dstInfo, options, true);
    }

    if (!this->allocateStorage(dstInfo)) {
        return kInternalError;
    }

    if (isProgressive) {
      // Keep consuming input until we can't anymore, and only output/read scanlines
      // if there is at least one valid output.
      int last_scan_completed = 0;
      while (!jpeg_input_complete(dinfo)) {
        // Call the progress monitor hook if present, to prevent decoder from hanging.
        if (dinfo->progress) {
           dinfo->progress->progress_monitor((j_common_ptr)dinfo);
        }
        const int res = jpeg_consume_input(dinfo);
        if (res == JPEG_SUSPENDED) {
           break;
        }
        if (res == JPEG_SCAN_COMPLETED) {
           last_scan_completed = dinfo->input_scan_number;
        }
      }
      if (last_scan_completed >  0) {
        jpeg_start_output(dinfo, last_scan_completed);
        int rows = 0;
         SkCodec::Result readResult = this->readRows(dstInfo, dst, dstRowBytes,
                                                     dstInfo.height(), options, &rows);
         // Checks if scan was called too many times to not stall the decoder.
         jpeg_finish_output(dinfo);
         if (readResult != kSuccess) {
            return fDecoderMgr->returnFailure("readRows", readResult);
         }
         if (rows < dstInfo.height()) {
             *rowsDecoded = rows;
             return fDecoderMgr->returnFailure("Incomplete image data", kIncompleteInput);
         }
      } else {
           return fDecoderMgr->returnFailure("Incomplete image data", kIncompleteInput);
      }
    } else {
      // Baseline image
      int rows = 0;
      this->readRows(dstInfo, dst, dstRowBytes, dstInfo.height(), options, &rows);
      if (rows < dstInfo.height()) {
          *rowsDecoded = rows;
          return fDecoderMgr->returnFailure("Incomplete image data", kIncompleteInput);
      }
  }

    return kSuccess;
}

bool SkJpegCodec::allocateStorage(const SkImageInfo& dstInfo) {
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
        if (!fStorage.reset(totalBytes)) {
            return false;
        }
        fSwizzleSrcRow = (swizzleBytes > 0) ? fStorage.get() : nullptr;
        fColorXformSrcRow = (xformBytes > 0) ?
                SkTAddOffset<uint32_t>(fStorage.get(), swizzleBytes) : nullptr;
    }
    return true;
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
    if (!this->allocateStorage(this->dstInfo())) {
        return nullptr;
    }
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

    if (!this->allocateStorage(dstInfo)) {
        return kInternalError;
    }

    return kSuccess;
}

int SkJpegCodec::onGetScanlines(void* dst, int count, size_t dstRowBytes) {
    int rows = 0;
    this->readRows(this->dstInfo(), dst, dstRowBytes, count, this->options(), &rows);
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

static bool is_yuv_supported(const jpeg_decompress_struct* dinfo,
                             const SkJpegCodec& codec,
                             const SkYUVAPixmapInfo::SupportedDataTypes* supportedDataTypes,
                             SkYUVAPixmapInfo* yuvaPixmapInfo) {
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
    SkASSERT(hSampY == dinfo->max_h_samp_factor);
    SkASSERT(vSampY == dinfo->max_v_samp_factor);

    SkYUVAInfo::Subsampling tempSubsampling;
    if        (1 == hSampY && 1 == vSampY) {
        tempSubsampling = SkYUVAInfo::Subsampling::k444;
    } else if (2 == hSampY && 1 == vSampY) {
        tempSubsampling = SkYUVAInfo::Subsampling::k422;
    } else if (2 == hSampY && 2 == vSampY) {
        tempSubsampling = SkYUVAInfo::Subsampling::k420;
    } else if (1 == hSampY && 2 == vSampY) {
        tempSubsampling = SkYUVAInfo::Subsampling::k440;
    } else if (4 == hSampY && 1 == vSampY) {
        tempSubsampling = SkYUVAInfo::Subsampling::k411;
    } else if (4 == hSampY && 2 == vSampY) {
        tempSubsampling = SkYUVAInfo::Subsampling::k410;
    } else {
        return false;
    }
    if (supportedDataTypes &&
        !supportedDataTypes->supported(SkYUVAInfo::PlaneConfig::kY_U_V,
                                       SkYUVAPixmapInfo::DataType::kUnorm8)) {
        return false;
    }
    if (yuvaPixmapInfo) {
        SkColorType colorTypes[SkYUVAPixmapInfo::kMaxPlanes];
        size_t rowBytes[SkYUVAPixmapInfo::kMaxPlanes];
        for (int i = 0; i < 3; ++i) {
            colorTypes[i] = kAlpha_8_SkColorType;
            rowBytes[i] = dinfo->comp_info[i].width_in_blocks * DCTSIZE;
        }
        SkYUVAInfo yuvaInfo(codec.dimensions(),
                            SkYUVAInfo::PlaneConfig::kY_U_V,
                            tempSubsampling,
                            kJPEG_Full_SkYUVColorSpace,
                            codec.getOrigin(),
                            SkYUVAInfo::Siting::kCentered,
                            SkYUVAInfo::Siting::kCentered);
        *yuvaPixmapInfo = SkYUVAPixmapInfo(yuvaInfo, colorTypes, rowBytes);
    }
    return true;
}

bool SkJpegCodec::onQueryYUVAInfo(const SkYUVAPixmapInfo::SupportedDataTypes& supportedDataTypes,
                                  SkYUVAPixmapInfo* yuvaPixmapInfo) const {
    jpeg_decompress_struct* dinfo = fDecoderMgr->dinfo();
    return is_yuv_supported(dinfo, *this, &supportedDataTypes, yuvaPixmapInfo);
}

SkCodec::Result SkJpegCodec::onGetYUVAPlanes(const SkYUVAPixmaps& yuvaPixmaps) {
    // Get a pointer to the decompress info since we will use it quite frequently
    jpeg_decompress_struct* dinfo = fDecoderMgr->dinfo();
    if (!is_yuv_supported(dinfo, *this, nullptr, nullptr)) {
        return fDecoderMgr->returnFailure("onGetYUVAPlanes", kInvalidInput);
    }
    // Set the jump location for libjpeg errors
    skjpeg_error_mgr::AutoPushJmpBuf jmp(fDecoderMgr->errorMgr());
    if (setjmp(jmp)) {
        return fDecoderMgr->returnFailure("setjmp", kInvalidInput);
    }

    dinfo->raw_data_out = TRUE;
    if (!jpeg_start_decompress(dinfo)) {
        return fDecoderMgr->returnFailure("startDecompress", kInvalidInput);
    }

    const std::array<SkPixmap, SkYUVAPixmaps::kMaxPlanes>& planes = yuvaPixmaps.planes();

#ifdef SK_DEBUG
    {
        // A previous implementation claims that the return value of is_yuv_supported()
        // may change after calling jpeg_start_decompress().  It looks to me like this
        // was caused by a bug in the old code, but we'll be safe and check here.
        // Also check that pixmap properties agree with expectations.
        SkYUVAPixmapInfo info;
        SkASSERT(is_yuv_supported(dinfo, *this, nullptr, &info));
        SkASSERT(info.yuvaInfo() == yuvaPixmaps.yuvaInfo());
        for (int i = 0; i < info.numPlanes(); ++i) {
            SkASSERT(planes[i].colorType() == kAlpha_8_SkColorType);
            SkASSERT(info.planeInfo(i) == planes[i].info());
        }
    }
#endif

    // Build a JSAMPIMAGE to handle output from libjpeg-turbo.  A JSAMPIMAGE has
    // a 2-D array of pixels for each of the components (Y, U, V) in the image.
    // Cheat Sheet:
    //     JSAMPIMAGE == JSAMPLEARRAY* == JSAMPROW** == JSAMPLE***
    JSAMPARRAY yuv[3];

    // Set aside enough space for pointers to rows of Y, U, and V.
    JSAMPROW rowptrs[2 * DCTSIZE + DCTSIZE + DCTSIZE];
    yuv[0] = &rowptrs[0];            // Y rows (DCTSIZE or 2 * DCTSIZE)
    yuv[1] = &rowptrs[2 * DCTSIZE];  // U rows (DCTSIZE)
    yuv[2] = &rowptrs[3 * DCTSIZE];  // V rows (DCTSIZE)

    // Initialize rowptrs.
    int numYRowsPerBlock = DCTSIZE * dinfo->comp_info[0].v_samp_factor;
    static_assert(sizeof(JSAMPLE) == 1);
    for (int i = 0; i < numYRowsPerBlock; i++) {
        rowptrs[i] = static_cast<JSAMPLE*>(planes[0].writable_addr()) + i* planes[0].rowBytes();
    }
    for (int i = 0; i < DCTSIZE; i++) {
        rowptrs[i + 2 * DCTSIZE] =
                static_cast<JSAMPLE*>(planes[1].writable_addr()) + i* planes[1].rowBytes();
        rowptrs[i + 3 * DCTSIZE] =
                static_cast<JSAMPLE*>(planes[2].writable_addr()) + i* planes[2].rowBytes();
    }

    // After each loop iteration, we will increment pointers to Y, U, and V.
    size_t blockIncrementY = numYRowsPerBlock * planes[0].rowBytes();
    size_t blockIncrementU = DCTSIZE * planes[1].rowBytes();
    size_t blockIncrementV = DCTSIZE * planes[2].rowBytes();

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
        for (int j = 0; j < numYRowsPerBlock; j++) {
            rowptrs[j] += blockIncrementY;
        }
        for (int j = 0; j < DCTSIZE; j++) {
            rowptrs[j + 2 * DCTSIZE] += blockIncrementU;
            rowptrs[j + 3 * DCTSIZE] += blockIncrementV;
        }
    }

    uint32_t remainingRows = dinfo->output_height - dinfo->output_scanline;
    SkASSERT(remainingRows == dinfo->output_height % numRowsPerBlock);
    SkASSERT(dinfo->output_scanline == numIters * numRowsPerBlock);
    if (remainingRows > 0) {
        // libjpeg-turbo needs memory to be padded by the block sizes.  We will fulfill
        // this requirement using an extra row buffer.
        // FIXME: Should SkCodec have an extra memory buffer that can be shared among
        //        all of the implementations that use temporary/garbage memory?
        AutoTMalloc<JSAMPLE> extraRow(planes[0].rowBytes());
        for (int i = remainingRows; i < numYRowsPerBlock; i++) {
            rowptrs[i] = extraRow.get();
        }
        int remainingUVRows = dinfo->comp_info[1].downsampled_height - DCTSIZE * numIters;
        for (int i = remainingUVRows; i < DCTSIZE; i++) {
            rowptrs[i + 2 * DCTSIZE] = extraRow.get();
            rowptrs[i + 3 * DCTSIZE] = extraRow.get();
        }

        JDIMENSION linesRead = jpeg_read_raw_data(dinfo, yuv, numRowsPerBlock);
        if (linesRead < remainingRows) {
            // FIXME: Handle incomplete YUV decodes without signalling an error.
            return kInvalidInput;
        }
    }

    return kSuccess;
}

bool SkJpegCodec::onGetGainmapCodec(SkGainmapInfo* info, std::unique_ptr<SkCodec>* gainmapCodec) {
    std::unique_ptr<SkStream> stream;
    if (!this->onGetGainmapInfo(info, &stream)) {
        return false;
    }
    if (gainmapCodec) {
        Result result;
        *gainmapCodec = MakeFromStream(std::move(stream), &result);
        if (!*gainmapCodec) {
            return false;
        }
    }
    return true;
}

bool SkJpegCodec::onGetGainmapInfo(SkGainmapInfo* info,
                                   std::unique_ptr<SkStream>* gainmapImageStream) {
#ifdef SK_CODEC_DECODES_JPEG_GAINMAPS
    sk_sp<SkData> gainmap_data;
    SkGainmapInfo gainmap_info;

    auto metadataDecoder =
            std::make_unique<SkJpegMetadataDecoderImpl>(get_sk_marker_list(fDecoderMgr->dinfo()));
    if (!metadataDecoder->findGainmapImage(
                fDecoderMgr->getSourceMgr(), gainmap_data, gainmap_info)) {
        return false;
    }

    *info = gainmap_info;
    *gainmapImageStream = SkMemoryStream::Make(gainmap_data);
    return true;
#else
    return false;
#endif  // SK_CODEC_DECODES_JPEG_GAINMAPS
}

namespace SkJpegDecoder {
bool IsJpeg(const void* data, size_t len) {
    return SkJpegCodec::IsJpeg(data, len);
}

std::unique_ptr<SkCodec> Decode(std::unique_ptr<SkStream> stream,
                                SkCodec::Result* outResult,
                                SkCodecs::DecodeContext) {
    SkCodec::Result resultStorage;
    if (!outResult) {
        outResult = &resultStorage;
    }
    return SkJpegCodec::MakeFromStream(std::move(stream), outResult);
}

std::unique_ptr<SkCodec> Decode(sk_sp<SkData> data,
                                SkCodec::Result* outResult,
                                SkCodecs::DecodeContext) {
    if (!data) {
        if (outResult) {
            *outResult = SkCodec::kInvalidInput;
        }
        return nullptr;
    }
    return Decode(SkMemoryStream::Make(std::move(data)), outResult, nullptr);
}

}  // namespace SkJpegDecoder
