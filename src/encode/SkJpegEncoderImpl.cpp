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
#include "include/private/base/SkNoncopyable.h"
#include "include/private/base/SkTemplates.h"
#include "src/base/SkMSAN.h"
#include "src/codec/SkJpegConstants.h"
#include "src/codec/SkJpegPriv.h"
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

    bool setParams(const SkImageInfo& srcInfo, const SkJpegEncoder::Options& options);
    bool setParams(const SkYUVAPixmapInfo& srcInfo, const SkJpegEncoder::Options& options);

    jpeg_compress_struct* cinfo() { return &fCInfo; }

    skjpeg_error_mgr* errorMgr() { return &fErrMgr; }

    transform_scanline_proc proc() const { return fProc; }

    ~SkJpegEncoderMgr() { jpeg_destroy_compress(&fCInfo); }

private:
    SkJpegEncoderMgr(SkWStream* stream) : fDstMgr(stream), fProc(nullptr) {
        fCInfo.err = jpeg_std_error(&fErrMgr);
        fErrMgr.error_exit = skjpeg_error_exit;
        jpeg_create_compress(&fCInfo);
        fCInfo.dest = &fDstMgr;
    }

    jpeg_compress_struct fCInfo;
    skjpeg_error_mgr fErrMgr;
    skjpeg_destination_mgr fDstMgr;
    transform_scanline_proc fProc;
};

bool SkJpegEncoderMgr::setParams(const SkImageInfo& srcInfo,
                                 const SkJpegEncoder::Options& options) {
    auto chooseProc8888 = [&]() {
        if (kUnpremul_SkAlphaType == srcInfo.alphaType() &&
            options.fAlphaOption == SkJpegEncoder::AlphaOption::kBlendOnBlack) {
            return transform_scanline_to_premul_legacy;
        }
        return (transform_scanline_proc) nullptr;
    };

    J_COLOR_SPACE jpegColorType = JCS_EXT_RGBA;
    int numComponents = 0;
    switch (srcInfo.colorType()) {
        case kRGBA_8888_SkColorType:
            fProc = chooseProc8888();
            jpegColorType = JCS_EXT_RGBA;
            numComponents = 4;
            break;
        case kBGRA_8888_SkColorType:
            fProc = chooseProc8888();
            jpegColorType = JCS_EXT_BGRA;
            numComponents = 4;
            break;
        case kRGB_565_SkColorType:
            fProc = transform_scanline_565;
            jpegColorType = JCS_RGB;
            numComponents = 3;
            break;
        case kARGB_4444_SkColorType:
            if (SkJpegEncoder::AlphaOption::kBlendOnBlack == options.fAlphaOption) {
                return false;
            }

            fProc = transform_scanline_444;
            jpegColorType = JCS_RGB;
            numComponents = 3;
            break;
        case kGray_8_SkColorType:
        case kAlpha_8_SkColorType:
        case kR8_unorm_SkColorType:
            jpegColorType = JCS_GRAYSCALE;
            numComponents = 1;
            break;
        case kRGBA_F16_SkColorType:
            if (kUnpremul_SkAlphaType == srcInfo.alphaType() &&
                options.fAlphaOption == SkJpegEncoder::AlphaOption::kBlendOnBlack) {
                fProc = transform_scanline_F16_to_premul_8888;
            } else {
                fProc = transform_scanline_F16_to_8888;
            }
            jpegColorType = JCS_EXT_RGBA;
            numComponents = 4;
            break;
        default:
            return false;
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

    // Tells libjpeg-turbo to compute optimal Huffman coding tables
    // for the image.  This improves compression at the cost of
    // slower encode performance.
    fCInfo.optimize_coding = TRUE;
    return true;
}

// Convert a row of an SkYUVAPixmaps to a row of Y,U,V triples.
// TODO(ccameron): This is horribly inefficient.
static void yuva_copy_row(const SkYUVAPixmaps* src, int row, uint8_t* dst) {
    int width = src->plane(0).width();
    switch (src->yuvaInfo().planeConfig()) {
        case SkYUVAInfo::PlaneConfig::kY_U_V: {
            auto [ssWidthU, ssHeightU] = src->yuvaInfo().planeSubsamplingFactors(1);
            auto [ssWidthV, ssHeightV] = src->yuvaInfo().planeSubsamplingFactors(2);
            const uint8_t* srcY = reinterpret_cast<const uint8_t*>(src->plane(0).addr(0, row));
            const uint8_t* srcU =
                    reinterpret_cast<const uint8_t*>(src->plane(1).addr(0, row / ssHeightU));
            const uint8_t* srcV =
                    reinterpret_cast<const uint8_t*>(src->plane(2).addr(0, row / ssHeightV));
            for (int col = 0; col < width; ++col) {
                dst[3 * col + 0] = srcY[col];
                dst[3 * col + 1] = srcU[col / ssWidthU];
                dst[3 * col + 2] = srcV[col / ssWidthV];
            }
            break;
        }
        case SkYUVAInfo::PlaneConfig::kY_UV: {
            auto [ssWidthUV, ssHeightUV] = src->yuvaInfo().planeSubsamplingFactors(1);
            const uint8_t* srcY = reinterpret_cast<const uint8_t*>(src->plane(0).addr(0, row));
            const uint8_t* srcUV =
                    reinterpret_cast<const uint8_t*>(src->plane(1).addr(0, row / ssHeightUV));
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

bool SkJpegEncoderMgr::setParams(const SkYUVAPixmapInfo& srcInfo,
                                 const SkJpegEncoder::Options& options) {
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

    fCInfo.optimize_coding = TRUE;
    return true;
}

static std::unique_ptr<SkEncoder> Make(SkWStream* dst,
                                       const SkPixmap* src,
                                       const SkYUVAPixmaps* srcYUVA,
                                       const SkColorSpace* srcYUVAColorSpace,
                                       const SkJpegEncoder::Options& options) {
    // Exactly one of |src| or |srcYUVA| should be specified.
    if (srcYUVA) {
        SkASSERT(!src);
        if (!srcYUVA->isValid()) {
            return nullptr;
        }
    } else {
        SkASSERT(src);
        if (!src || !SkPixmapIsValid(*src)) {
            return nullptr;
        }
    }

    std::unique_ptr<SkJpegEncoderMgr> encoderMgr = SkJpegEncoderMgr::Make(dst);

    skjpeg_error_mgr::AutoPushJmpBuf jmp(encoderMgr->errorMgr());
    if (setjmp(jmp)) {
        return nullptr;
    }

    if (srcYUVA) {
        if (!encoderMgr->setParams(srcYUVA->pixmapsInfo(), options)) {
            return nullptr;
        }
    } else {
        if (!encoderMgr->setParams(src->info(), options)) {
            return nullptr;
        }
    }

    jpeg_set_quality(encoderMgr->cinfo(), options.fQuality, TRUE);
    jpeg_start_compress(encoderMgr->cinfo(), TRUE);

    // Write XMP metadata. This will only write the standard XMP segment.
    // TODO(ccameron): Split this into a standard and extended XMP segment if needed.
    if (options.xmpMetadata) {
        SkDynamicMemoryWStream s;
        s.write(kXMPStandardSig, sizeof(kXMPStandardSig));
        s.write(options.xmpMetadata->data(), options.xmpMetadata->size());
        auto data = s.detachAsData();
        jpeg_write_marker(encoderMgr->cinfo(), kXMPMarker, data->bytes(), data->size());
    }

    // Write the ICC profile.
    // TODO(ccameron): This limits ICC profile size to a single segment's parameters (less than
    // 64k). Split larger profiles into more segments.
    sk_sp<SkData> icc = icc_from_color_space(srcYUVA ? srcYUVAColorSpace : src->colorSpace(),
                                             options.fICCProfile,
                                             options.fICCProfileDescription);
    if (icc) {
        // Create a contiguous block of memory with the icc signature followed by the profile.
        sk_sp<SkData> markerData = SkData::MakeUninitialized(kICCMarkerHeaderSize + icc->size());
        uint8_t* ptr = (uint8_t*)markerData->writable_data();
        memcpy(ptr, kICCSig, sizeof(kICCSig));
        ptr += sizeof(kICCSig);
        *ptr++ = 1;  // This is the first marker.
        *ptr++ = 1;  // Out of one total markers.
        memcpy(ptr, icc->data(), icc->size());

        jpeg_write_marker(encoderMgr->cinfo(), kICCMarker, markerData->bytes(), markerData->size());
    }

    if (srcYUVA) {
        return std::make_unique<SkJpegEncoderImpl>(std::move(encoderMgr), srcYUVA);
    }
    return std::make_unique<SkJpegEncoderImpl>(std::move(encoderMgr), *src);
}

SkJpegEncoderImpl::SkJpegEncoderImpl(std::unique_ptr<SkJpegEncoderMgr> encoderMgr,
                                     const SkPixmap& src)
        : SkEncoder(src,
                    encoderMgr->proc() ? encoderMgr->cinfo()->input_components * src.width() : 0)
        , fEncoderMgr(std::move(encoderMgr)) {}

SkJpegEncoderImpl::SkJpegEncoderImpl(std::unique_ptr<SkJpegEncoderMgr> encoderMgr,
                                     const SkYUVAPixmaps* src)
        : SkEncoder(src->plane(0), encoderMgr->cinfo()->input_components * src->yuvaInfo().width())
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
            yuva_copy_row(fSrcYUVA, fCurrRow + i, fStorage.get());
            JSAMPLE* jpegSrcRow = fStorage.get();
            jpeg_write_scanlines(fEncoderMgr->cinfo(), &jpegSrcRow, 1);
        }
    } else {
        const size_t srcBytes = SkColorTypeBytesPerPixel(fSrc.colorType()) * fSrc.width();
        const size_t jpegSrcBytes = fEncoderMgr->cinfo()->input_components * fSrc.width();
        const void* srcRow = fSrc.addr(0, fCurrRow);
        for (int i = 0; i < numRows; i++) {
            JSAMPLE* jpegSrcRow = (JSAMPLE*)(const_cast<void*>(srcRow));
            if (fEncoderMgr->proc()) {
                sk_msan_assert_initialized(srcRow, SkTAddOffset<const void>(srcRow, srcBytes));
                fEncoderMgr->proc()((char*)fStorage.get(),
                                    (const char*)srcRow,
                                    fSrc.width(),
                                    fEncoderMgr->cinfo()->input_components);
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
    return Make(dst, &src, nullptr, nullptr, options);
}

std::unique_ptr<SkEncoder> Make(SkWStream* dst,
                                const SkYUVAPixmaps& src,
                                const SkColorSpace* srcColorSpace,
                                const Options& options) {
    return Make(dst, nullptr, &src, srcColorSpace, options);
}

}  // namespace SkJpegEncoder
