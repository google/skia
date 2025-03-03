/*
 * Copyright 2007 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/encode/SkJpegEncoderImpl.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColorType.h"
#include "include/core/SkData.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "include/core/SkYUVAInfo.h"
#include "include/core/SkYUVAPixmaps.h"
#include "include/encode/SkEncoder.h"
#include "include/encode/SkJpegEncoder.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkNoncopyable.h"
#include "include/private/base/SkTemplates.h"
#include "src/base/SkMSAN.h"
#include "src/codec/SkJpegConstants.h"
#include "src/codec/SkJpegPriv.h"
#include "src/core/SkConvertPixels.h"
#include "src/core/SkImageInfoPriv.h"
#include "src/encode/SkImageEncoderFns.h"
#include "src/encode/SkImageEncoderPriv.h"
#include "src/encode/SkJPEGWriteUtility.h"
#include "src/image/SkImage_Base.h"

#include <csetjmp>
#include <cstdint>
#include <cstring>
#include <memory>
#include <utility>

class GrDirectContext;
class SkColorSpace;
class SkImage;

extern "C" {
#include "jpeglib.h"  // NO_G3_REWRITE
}

class SkJpegEncoderMgr final : SkNoncopyable {
public:
    /*
     * Create the decode manager
     * Does not take ownership of stream.
     */
    static std::unique_ptr<SkJpegEncoderMgr> Make(SkWStream* stream) {
        return std::unique_ptr<SkJpegEncoderMgr>(new SkJpegEncoderMgr(stream));
    }

    bool initializeRGB(const SkImageInfo&,
                       const SkJpegEncoder::Options&,
                       const SkJpegMetadataEncoder::SegmentList&);
    bool initializeYUV(const SkYUVAPixmapInfo&,
                       const SkJpegEncoder::Options&,
                       const SkJpegMetadataEncoder::SegmentList&);

    jpeg_compress_struct* cinfo() { return &fCInfo; }

    skjpeg_error_mgr* errorMgr() { return &fErrMgr; }

    bool shouldUseColorXform() { return fUseColorXform; }
    bool colorTransformProc(void* dst, const void* src, int width);

    ~SkJpegEncoderMgr() { jpeg_destroy_compress(&fCInfo); }

private:
    SkJpegEncoderMgr(SkWStream* stream) : fDstMgr(stream) {
        fCInfo.err = jpeg_std_error(&fErrMgr);
        fErrMgr.error_exit = skjpeg_error_exit;
        jpeg_create_compress(&fCInfo);
        fCInfo.dest = &fDstMgr;
    }
    void initializeCommon(const SkJpegEncoder::Options&, const SkJpegMetadataEncoder::SegmentList&);

    jpeg_compress_struct fCInfo;
    skjpeg_error_mgr fErrMgr;
    skjpeg_destination_mgr fDstMgr;

    std::optional<SkImageInfo> fSrcInfo;
    std::optional<SkImageInfo> fDstInfo;
    bool fUseColorXform = false;
};

// This function should only be called if fUseColorXform is true and thus fSrcInfo
// and fDstInfo have value. fSrcInfo, fDstInfo, and width must all have the same width.
bool SkJpegEncoderMgr::colorTransformProc(void* dst, const void* src, const int width) {
   SkASSERT(fUseColorXform);
   SkASSERT(fSrcInfo && fDstInfo);
   SkASSERT(width == fSrcInfo->width());
   return SkConvertPixels(fDstInfo.value(), dst, fDstInfo->minRowBytes(),
                          fSrcInfo.value(), src, fSrcInfo->minRowBytes());
}

bool SkJpegEncoderMgr::initializeRGB(const SkImageInfo& srcInfo,
                                     const SkJpegEncoder::Options& options,
                                     const SkJpegMetadataEncoder::SegmentList& metadataSegments) {
    J_COLOR_SPACE jpegColorType = JCS_EXT_RGBA;
    int numComponents = 0;
    SkImageInfo dstInfo;
    fUseColorXform = false;

    SkColorType srcCT = srcInfo.colorType();
    const bool applyPremul = SkJpegEncoder::AlphaOption::kBlendOnBlack == options.fAlphaOption
                              && srcInfo.alphaType() == kUnpremul_SkAlphaType;
    if (srcCT == kRGB_888x_SkColorType) {
        jpegColorType = JCS_EXT_RGBX;
        numComponents = 4;
    } else if (!applyPremul && srcCT == kRGBA_8888_SkColorType){
        jpegColorType = JCS_EXT_RGBA;
        numComponents = 4;
    } else if (!applyPremul && srcCT == kBGRA_8888_SkColorType) {
        jpegColorType = JCS_EXT_BGRA;
        numComponents = 4;
    } else {
      // Color type conversion is needed.
      switch(SkColorTypeNumChannels(srcCT)) {
        case 1:
          // We support encoding kAlpha_8_SkColorType pixmaps as JCS_GRAYSCALE as
          // this come up often. Otherwise we have no sensible way to encode alpha
          // images.
          if (SkColorTypeIsAlphaOnly(srcCT) && srcCT != kAlpha_8_SkColorType) {
            return false;
          }
          jpegColorType = JCS_GRAYSCALE;
          numComponents = 1;
          break;
        case 3:
          jpegColorType = JCS_EXT_RGBX;
          numComponents = 4;
          dstInfo = SkImageInfo::Make(srcInfo.width(), 1, kRGB_888x_SkColorType, kUnpremul_SkAlphaType);
          fUseColorXform = true;
          break;
        case 4: {
            SkAlphaType dstAT = applyPremul ? kPremul_SkAlphaType : srcInfo.alphaType();
            jpegColorType = JCS_EXT_RGBA;
            numComponents = 4;
            dstInfo = SkImageInfo::Make(srcInfo.width(), 1, kRGBA_8888_SkColorType, dstAT);
            fUseColorXform = true;
            break;
        }
        default:
            return false;
      }
    }
    SkASSERT(numComponents != 0);

    if (fUseColorXform) {
        fSrcInfo = srcInfo.makeWH(srcInfo.width(), 1);
        fDstInfo = dstInfo;
    }

    fCInfo.image_width = srcInfo.width();
    fCInfo.image_height = srcInfo.height();
    fCInfo.in_color_space = jpegColorType;
    fCInfo.input_components = numComponents;
    jpeg_set_defaults(&fCInfo);

    if (numComponents != 1) {
        switch (options.fDownsample) {
            case SkJpegEncoder::Downsample::k420:
                SkASSERT(2 == fCInfo.comp_info[0].h_samp_factor);
                SkASSERT(2 == fCInfo.comp_info[0].v_samp_factor);
                SkASSERT(1 == fCInfo.comp_info[1].h_samp_factor);
                SkASSERT(1 == fCInfo.comp_info[1].v_samp_factor);
                SkASSERT(1 == fCInfo.comp_info[2].h_samp_factor);
                SkASSERT(1 == fCInfo.comp_info[2].v_samp_factor);
                break;
            case SkJpegEncoder::Downsample::k422:
                fCInfo.comp_info[0].h_samp_factor = 2;
                fCInfo.comp_info[0].v_samp_factor = 1;
                SkASSERT(1 == fCInfo.comp_info[1].h_samp_factor);
                SkASSERT(1 == fCInfo.comp_info[1].v_samp_factor);
                SkASSERT(1 == fCInfo.comp_info[2].h_samp_factor);
                SkASSERT(1 == fCInfo.comp_info[2].v_samp_factor);
                break;
            case SkJpegEncoder::Downsample::k444:
                fCInfo.comp_info[0].h_samp_factor = 1;
                fCInfo.comp_info[0].v_samp_factor = 1;
                SkASSERT(1 == fCInfo.comp_info[1].h_samp_factor);
                SkASSERT(1 == fCInfo.comp_info[1].v_samp_factor);
                SkASSERT(1 == fCInfo.comp_info[2].h_samp_factor);
                SkASSERT(1 == fCInfo.comp_info[2].v_samp_factor);
                break;
        }
    }

    initializeCommon(options, metadataSegments);
    return true;
}

// Convert a row of an SkYUVAPixmaps to a row of Y,U,V triples.
// TODO(ccameron): This is horribly inefficient.
static void yuva_copy_row(const SkYUVAPixmaps& src, int row, uint8_t* dst) {
    int width = src.plane(0).width();
    switch (src.yuvaInfo().planeConfig()) {
        case SkYUVAInfo::PlaneConfig::kY_U_V: {
            auto [ssWidthU, ssHeightU] = src.yuvaInfo().planeSubsamplingFactors(1);
            auto [ssWidthV, ssHeightV] = src.yuvaInfo().planeSubsamplingFactors(2);
            const uint8_t* srcY = reinterpret_cast<const uint8_t*>(src.plane(0).addr(0, row));
            const uint8_t* srcU =
                    reinterpret_cast<const uint8_t*>(src.plane(1).addr(0, row / ssHeightU));
            const uint8_t* srcV =
                    reinterpret_cast<const uint8_t*>(src.plane(2).addr(0, row / ssHeightV));
            for (int col = 0; col < width; ++col) {
                dst[3 * col + 0] = srcY[col];
                dst[3 * col + 1] = srcU[col / ssWidthU];
                dst[3 * col + 2] = srcV[col / ssWidthV];
            }
            break;
        }
        case SkYUVAInfo::PlaneConfig::kY_UV: {
            auto [ssWidthUV, ssHeightUV] = src.yuvaInfo().planeSubsamplingFactors(1);
            const uint8_t* srcY = reinterpret_cast<const uint8_t*>(src.plane(0).addr(0, row));
            const uint8_t* srcUV =
                    reinterpret_cast<const uint8_t*>(src.plane(1).addr(0, row / ssHeightUV));
            for (int col = 0; col < width; ++col) {
                dst[3 * col + 0] = srcY[col];
                dst[3 * col + 1] = srcUV[2 * (col / ssWidthUV) + 0];
                dst[3 * col + 2] = srcUV[2 * (col / ssWidthUV) + 1];
            }
            break;
        }
        default:
            break;
    }
}

bool SkJpegEncoderMgr::initializeYUV(const SkYUVAPixmapInfo& srcInfo,
                                     const SkJpegEncoder::Options& options,
                                     const SkJpegMetadataEncoder::SegmentList& metadataSegments) {
    fCInfo.image_width = srcInfo.yuvaInfo().width();
    fCInfo.image_height = srcInfo.yuvaInfo().height();
    fCInfo.in_color_space = JCS_YCbCr;
    fCInfo.input_components = 3;
    jpeg_set_defaults(&fCInfo);

    // Support no color space conversion.
    if (srcInfo.yuvColorSpace() != kJPEG_Full_SkYUVColorSpace) {
        return false;
    }

    // Support only 8-bit data.
    switch (srcInfo.dataType()) {
        case SkYUVAPixmapInfo::DataType::kUnorm8:
            break;
        default:
            return false;
    }

    // Support only Y,U,V and Y,UV configurations (they are the only ones supported by
    // yuva_copy_row).
    switch (srcInfo.yuvaInfo().planeConfig()) {
        case SkYUVAInfo::PlaneConfig::kY_U_V:
        case SkYUVAInfo::PlaneConfig::kY_UV:
            break;
        default:
            return false;
    }

    // Specify to the encoder to use the same subsampling as the input image. The U and V planes
    // always have a sampling factor of 1.
    auto [ssHoriz, ssVert] = SkYUVAInfo::SubsamplingFactors(srcInfo.yuvaInfo().subsampling());
    fCInfo.comp_info[0].h_samp_factor = ssHoriz;
    fCInfo.comp_info[0].v_samp_factor = ssVert;

    initializeCommon(options, metadataSegments);
    return true;
}

void SkJpegEncoderMgr::initializeCommon(
        const SkJpegEncoder::Options& options,
        const SkJpegMetadataEncoder::SegmentList& metadataSegments) {
    // Tells libjpeg-turbo to compute optimal Huffman coding tables
    // for the image.  This improves compression at the cost of
    // slower encode performance.
    fCInfo.optimize_coding = TRUE;

    jpeg_set_quality(&fCInfo, options.fQuality, TRUE);
    jpeg_start_compress(&fCInfo, TRUE);

    for (const auto& segment : metadataSegments) {
        jpeg_write_marker(&fCInfo,
                          segment.fMarker,
                          segment.fParameters->bytes(),
                          segment.fParameters->size());
    }
}

std::unique_ptr<SkEncoder> SkJpegEncoderImpl::MakeYUV(
        SkWStream* dst,
        const SkYUVAPixmaps& srcYUVA,
        const SkColorSpace* srcYUVAColorSpace,
        const SkJpegEncoder::Options& options,
        const SkJpegMetadataEncoder::SegmentList& metadataSegments) {
    if (!srcYUVA.isValid()) {
        return nullptr;
    }
    std::unique_ptr<SkJpegEncoderMgr> encoderMgr = SkJpegEncoderMgr::Make(dst);
    skjpeg_error_mgr::AutoPushJmpBuf jmp(encoderMgr->errorMgr());
    if (setjmp(jmp)) {
        return nullptr;
    }

    if (!encoderMgr->initializeYUV(srcYUVA.pixmapsInfo(), options, metadataSegments)) {
        return nullptr;
    }
    return std::unique_ptr<SkJpegEncoderImpl>(
            new SkJpegEncoderImpl(std::move(encoderMgr), srcYUVA));
}

std::unique_ptr<SkEncoder> SkJpegEncoderImpl::MakeRGB(
        SkWStream* dst,
        const SkPixmap& src,
        const SkJpegEncoder::Options& options,
        const SkJpegMetadataEncoder::SegmentList& metadataSegments) {
    if (!SkPixmapIsValid(src)) {
        return nullptr;
    }
    std::unique_ptr<SkJpegEncoderMgr> encoderMgr = SkJpegEncoderMgr::Make(dst);
    skjpeg_error_mgr::AutoPushJmpBuf jmp(encoderMgr->errorMgr());
    if (setjmp(jmp)) {
        return nullptr;
    }

    if (!encoderMgr->initializeRGB(src.info(), options, metadataSegments)) {
        return nullptr;
    }
    return std::unique_ptr<SkJpegEncoderImpl>(new SkJpegEncoderImpl(std::move(encoderMgr), src));
}

SkJpegEncoderImpl::SkJpegEncoderImpl(std::unique_ptr<SkJpegEncoderMgr> encoderMgr,
                                     const SkPixmap& src)
        : SkEncoder(src,
                    encoderMgr->shouldUseColorXform() ? encoderMgr->cinfo()->input_components * src.width() : 0)
        , fEncoderMgr(std::move(encoderMgr)) {}

SkJpegEncoderImpl::SkJpegEncoderImpl(std::unique_ptr<SkJpegEncoderMgr> encoderMgr,
                                     const SkYUVAPixmaps& src)
        : SkEncoder(src.plane(0), encoderMgr->cinfo()->input_components * src.yuvaInfo().width())
        , fEncoderMgr(std::move(encoderMgr))
        , fSrcYUVA(src) {}

SkJpegEncoderImpl::~SkJpegEncoderImpl() {}

bool SkJpegEncoderImpl::onEncodeRows(int numRows) {
    skjpeg_error_mgr::AutoPushJmpBuf jmp(fEncoderMgr->errorMgr());
    if (setjmp(jmp)) {
        return false;
    }

    if (fSrcYUVA) {
        // TODO(ccameron): Consider using jpeg_write_raw_data, to avoid having to re-pack the data.
        for (int i = 0; i < numRows; i++) {
            yuva_copy_row(*fSrcYUVA, fCurrRow + i, fStorage.get());
            JSAMPLE* jpegSrcRow = fStorage.get();
            jpeg_write_scanlines(fEncoderMgr->cinfo(), &jpegSrcRow, 1);
        }
    } else {
        const size_t srcBytes = SkColorTypeBytesPerPixel(fSrc.colorType()) * fSrc.width();
        const size_t jpegSrcBytes = fEncoderMgr->cinfo()->input_components * fSrc.width();
        const void* srcRow = fSrc.addr(0, fCurrRow);
        for (int i = 0; i < numRows; i++) {
            JSAMPLE* jpegSrcRow = (JSAMPLE*)(const_cast<void*>(srcRow));
            if (fEncoderMgr->shouldUseColorXform()) {
                sk_msan_assert_initialized(srcRow, SkTAddOffset<const void>(srcRow, srcBytes));
                if (!fEncoderMgr->colorTransformProc((void*)fStorage.get(), srcRow, fSrc.width())) {
                    return false;
                }
                jpegSrcRow = fStorage.get();
                sk_msan_assert_initialized(jpegSrcRow,
                                           SkTAddOffset<const void>(jpegSrcRow, jpegSrcBytes));
            } else {
                // Same as above, but this repetition allows determining whether a
                // proc was used when msan asserts.
                sk_msan_assert_initialized(jpegSrcRow,
                                           SkTAddOffset<const void>(jpegSrcRow, jpegSrcBytes));
            }

            jpeg_write_scanlines(fEncoderMgr->cinfo(), &jpegSrcRow, 1);
            srcRow = SkTAddOffset<const void>(srcRow, fSrc.rowBytes());
        }
    }

    fCurrRow += numRows;
    if (fCurrRow == fSrc.height()) {
        jpeg_finish_compress(fEncoderMgr->cinfo());
    }

    return true;
}

namespace SkJpegEncoder {

bool Encode(SkWStream* dst, const SkPixmap& src, const Options& options) {
    auto encoder = Make(dst, src, options);
    return encoder.get() && encoder->encodeRows(src.height());
}

bool Encode(SkWStream* dst,
            const SkYUVAPixmaps& src,
            const SkColorSpace* srcColorSpace,
            const Options& options) {
    auto encoder = Make(dst, src, srcColorSpace, options);
    return encoder.get() && encoder->encodeRows(src.yuvaInfo().height());
}

sk_sp<SkData> Encode(GrDirectContext* ctx, const SkImage* img, const Options& options) {
    if (!img) {
        return nullptr;
    }
    SkBitmap bm;
    if (!as_IB(img)->getROPixels(ctx, &bm)) {
        return nullptr;
    }
    SkDynamicMemoryWStream stream;
    if (Encode(&stream, bm.pixmap(), options)) {
        return stream.detachAsData();
    }
    return nullptr;
}

std::unique_ptr<SkEncoder> Make(SkWStream* dst, const SkPixmap& src, const Options& options) {
    SkJpegMetadataEncoder::SegmentList metadataSegments;
    SkJpegMetadataEncoder::AppendXMPStandard(metadataSegments, options.xmpMetadata);
    SkJpegMetadataEncoder::AppendICC(metadataSegments, options, src.colorSpace());
    if (options.fOrigin.has_value()) {
      SkJpegMetadataEncoder::AppendOrigin(metadataSegments, options.fOrigin.value());
    }
    return SkJpegEncoderImpl::MakeRGB(dst, src, options, metadataSegments);
}

std::unique_ptr<SkEncoder> Make(SkWStream* dst,
                                const SkYUVAPixmaps& src,
                                const SkColorSpace* srcColorSpace,
                                const Options& options) {
    SkJpegMetadataEncoder::SegmentList metadataSegments;
    SkJpegMetadataEncoder::AppendXMPStandard(metadataSegments, options.xmpMetadata);
    SkJpegMetadataEncoder::AppendICC(metadataSegments, options, srcColorSpace);
    if (options.fOrigin.has_value()) {
      SkJpegMetadataEncoder::AppendOrigin(metadataSegments, options.fOrigin.value());
    }
    return SkJpegEncoderImpl::MakeYUV(dst, src, srcColorSpace, options, metadataSegments);
}

}  // namespace SkJpegEncoder

namespace SkJpegMetadataEncoder {

void AppendICC(SegmentList& segmentList,
               const SkJpegEncoder::Options& options,
               const SkColorSpace* colorSpace) {
    sk_sp<SkData> icc =
            icc_from_color_space(colorSpace, options.fICCProfile, options.fICCProfileDescription);
    if (!icc) {
        return;
    }

    // TODO(ccameron): This limits ICC profile size to a single segment's parameters (less than
    // 64k). Split larger profiles into more segments.
    SkDynamicMemoryWStream s;
    s.write(kICCSig, sizeof(kICCSig));
    s.write8(1);  // This is the first marker.
    s.write8(1);  // Out of one total markers.
    s.write(icc->data(), icc->size());
    segmentList.emplace_back(kICCMarker, s.detachAsData());
}

void AppendXMPStandard(SegmentList& segmentList, const SkData* xmpMetadata) {
    if (!xmpMetadata) {
        return;
    }

    // TODO(ccameron): Split this into a standard and extended XMP segment if needed.
    SkDynamicMemoryWStream s;
    s.write(kXMPStandardSig, sizeof(kXMPStandardSig));
    s.write(xmpMetadata->data(), xmpMetadata->size());
    segmentList.emplace_back(kXMPMarker, s.detachAsData());
}

void AppendOrigin(SegmentList& segmentList, SkEncodedOrigin origin) {
    if (origin < kDefault_SkEncodedOrigin || origin > kLast_SkEncodedOrigin) {
      SkDebugf("Origin is not a valid value.\n");
      return;
    }
    sk_sp<SkData> exif = exif_from_origin(origin);
    if (!exif) {
      return;
    }
    SkDynamicMemoryWStream s;
    s.write(kExifSig, sizeof(kExifSig));
    s.write8(0);
    s.write(exif->data(), exif->size());
    segmentList.emplace_back(kExifMarker, s.detachAsData());
}

}  // namespace SkJpegMetadataEncoder
