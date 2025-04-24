/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/encode/SkPngEncoderImpl.h"

#include <optional>

#include "include/core/SkBitmap.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkData.h"
#include "include/core/SkDataTable.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSpan.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/encode/SkEncoder.h"
#include "include/encode/SkPngEncoder.h"
#include "include/private/SkEncodedInfo.h"
#include "include/private/SkGainmapInfo.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkNoncopyable.h"
#include "modules/skcms/skcms.h"
#include "src/codec/SkPngPriv.h"
#include "src/encode/SkImageEncoderFns.h"
#include "src/encode/SkImageEncoderPriv.h"
#include "src/encode/SkPngEncoderBase.h"
#include "src/image/SkImage_Base.h"

#include <algorithm>
#include <array>
#include <csetjmp>
#include <cstdint>
#include <cstring>
#include <memory>
#include <utility>
#include <vector>

#include <png.h>
#include <pngconf.h>

class GrDirectContext;
class SkImage;

static_assert(PNG_FILTER_NONE  == (int)SkPngEncoder::FilterFlag::kNone,  "Skia libpng filter err.");
static_assert(PNG_FILTER_SUB   == (int)SkPngEncoder::FilterFlag::kSub,   "Skia libpng filter err.");
static_assert(PNG_FILTER_UP    == (int)SkPngEncoder::FilterFlag::kUp,    "Skia libpng filter err.");
static_assert(PNG_FILTER_AVG   == (int)SkPngEncoder::FilterFlag::kAvg,   "Skia libpng filter err.");
static_assert(PNG_FILTER_PAETH == (int)SkPngEncoder::FilterFlag::kPaeth, "Skia libpng filter err.");
static_assert(PNG_ALL_FILTERS  == (int)SkPngEncoder::FilterFlag::kAll,   "Skia libpng filter err.");

static constexpr bool kSuppressPngEncodeWarnings = false;

static void sk_error_fn(png_structp png_ptr, png_const_charp msg) {
    if (!kSuppressPngEncodeWarnings) {
        SkDebugf("libpng encode error: %s\n", msg);
    }

    longjmp(png_jmpbuf(png_ptr), 1);
}

static void sk_write_fn(png_structp png_ptr, png_bytep data, png_size_t len) {
    SkWStream* stream = (SkWStream*)png_get_io_ptr(png_ptr);
    if (!stream->write(data, len)) {
        png_error(png_ptr, "sk_write_fn cannot write to stream");
    }
}

class SkPngEncoderMgr final : SkNoncopyable {
public:
    /*
     * Create the decode manager
     * Does not take ownership of stream
     */
    static std::unique_ptr<SkPngEncoderMgr> Make(SkWStream* stream);
    bool setHeader(const SkPngEncoderBase::TargetInfo& targetInfo,
                   const SkImageInfo& srcInfo,
                   const SkPngEncoder::Options& options);
    bool setColorSpace(const SkImageInfo& info, const SkPngEncoder::Options& options);
    bool setV0Gainmap(const SkPngEncoder::Options& options);
    bool writeInfo(const SkImageInfo& srcInfo,const SkPngEncoderBase::TargetInfo& targetInfo);

    png_structp pngPtr() { return fPngPtr; }
    png_infop infoPtr() { return fInfoPtr; }
    transform_scanline_proc proc() const { return fProc; }

    ~SkPngEncoderMgr() { png_destroy_write_struct(&fPngPtr, &fInfoPtr); }

private:
    SkPngEncoderMgr(png_structp pngPtr, png_infop infoPtr) : fPngPtr(pngPtr), fInfoPtr(infoPtr) {}

    png_structp fPngPtr;
    png_infop fInfoPtr;
    transform_scanline_proc fProc = nullptr;
};

std::unique_ptr<SkPngEncoderMgr> SkPngEncoderMgr::Make(SkWStream* stream) {
    png_structp pngPtr =
            png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, sk_error_fn, nullptr);
    if (!pngPtr) {
        return nullptr;
    }

    png_infop infoPtr = png_create_info_struct(pngPtr);
    if (!infoPtr) {
        png_destroy_write_struct(&pngPtr, nullptr);
        return nullptr;
    }

    png_set_write_fn(pngPtr, (void*)stream, sk_write_fn, nullptr);
    return std::unique_ptr<SkPngEncoderMgr>(new SkPngEncoderMgr(pngPtr, infoPtr));
}

bool SkPngEncoderMgr::setHeader(const SkPngEncoderBase::TargetInfo& targetInfo,
                                const SkImageInfo& srcInfo,
                                const SkPngEncoder::Options& options) {
    if (setjmp(png_jmpbuf(fPngPtr))) {
        return false;
    }

    const SkEncodedInfo& dstInfo = targetInfo.fDstInfo;
    const std::optional<SkImageInfo>& dstRowInfo = targetInfo.fDstRowInfo;

    int pngColorType;
    switch (dstInfo.color()) {
        case SkEncodedInfo::kRGB_Color:
            pngColorType = PNG_COLOR_TYPE_RGB;
            break;
        case SkEncodedInfo::kRGBA_Color:
            SkASSERT(dstRowInfo);
            pngColorType = dstRowInfo->isOpaque() ? PNG_COLOR_TYPE_RGB
                                : PNG_COLOR_TYPE_RGB_ALPHA;
            break;
        case SkEncodedInfo::kGray_Color:
            pngColorType = PNG_COLOR_TYPE_GRAY;
            break;
        case SkEncodedInfo::kGrayAlpha_Color:
            pngColorType = PNG_COLOR_TYPE_GRAY_ALPHA;
            break;
        default:
            SkDEBUGFAIL("`getTargetInfo` returned unexpected `SkEncodedInfo::Color`");
            return false;
    }

    png_color_8 sigBit;
    switch (srcInfo.colorType()) {
        case kRGBA_F16Norm_SkColorType:
        case kRGBA_F16_SkColorType:
        case kRGBA_F32_SkColorType:
            sigBit.red = 16;
            sigBit.green = 16;
            sigBit.blue = 16;
            sigBit.alpha = 16;
            break;
        case kRGB_F16F16F16x_SkColorType:
            sigBit.red = 16;
            sigBit.green = 16;
            sigBit.blue = 16;
            break;
        case kGray_8_SkColorType:
            sigBit.gray = 8;
            break;
        case kRGB_888x_SkColorType:
            sigBit.red = 8;
            sigBit.green = 8;
            sigBit.blue = 8;
            break;
        case kARGB_4444_SkColorType:
            sigBit.red = 4;
            sigBit.green = 4;
            sigBit.blue = 4;
            sigBit.alpha = 4;
            break;
        case kRGB_565_SkColorType:
            sigBit.red = 5;
            sigBit.green = 6;
            sigBit.blue = 5;
            break;
        case kAlpha_8_SkColorType:  // store as gray+alpha, but ignore gray
            sigBit.gray = kGraySigBit_GrayAlphaIsJustAlpha;
            sigBit.alpha = 8;
            break;
        case kRGBA_1010102_SkColorType:
            sigBit.red = 10;
            sigBit.green = 10;
            sigBit.blue = 10;
            sigBit.alpha = 2;
            break;
        case kBGR_101010x_XR_SkColorType:
            case kRGB_101010x_SkColorType:
            sigBit.red = 10;
            sigBit.green = 10;
            sigBit.blue = 10;
            break;
        case kBGRA_10101010_XR_SkColorType:
            sigBit.red = 10;
            sigBit.green = 10;
            sigBit.blue = 10;
            sigBit.alpha = 10;
            break;
        case kRGBA_8888_SkColorType:
        case kBGRA_8888_SkColorType:
            default:
            sigBit.red = 8;
            sigBit.green = 8;
            sigBit.blue = 8;
            sigBit.alpha = 8;
            break;
    }

    png_set_IHDR(fPngPtr,
                 fInfoPtr,
                 srcInfo.width(),
                 srcInfo.height(),
                 dstInfo.bitsPerComponent(),
                 pngColorType,
                 PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE,
                 PNG_FILTER_TYPE_BASE);
    png_set_sBIT(fPngPtr, fInfoPtr, &sigBit);

    int filters = (int)options.fFilterFlags & (int)SkPngEncoder::FilterFlag::kAll;
    SkASSERT(filters == (int)options.fFilterFlags);
    png_set_filter(fPngPtr, PNG_FILTER_TYPE_BASE, filters);

    int zlibLevel = std::min(std::max(0, options.fZLibLevel), 9);
    SkASSERT(zlibLevel == options.fZLibLevel);
    png_set_compression_level(fPngPtr, zlibLevel);

    // Set comments in tEXt chunk
    const sk_sp<SkDataTable>& comments = options.fComments;
    if (comments != nullptr) {
        if (comments->count() % 2 != 0) {
            return false;
        }

        std::vector<png_text> png_texts(comments->count());
        std::vector<SkString> clippedKeys;
        for (int i = 0; i < comments->count() / 2; ++i) {
            const char* keyword;
            const char* originalKeyword = comments->atStr(2 * i);
            const char* text = comments->atStr(2 * i + 1);
            if (strlen(originalKeyword) <= PNG_KEYWORD_MAX_LENGTH) {
                keyword = originalKeyword;
            } else {
                SkDEBUGFAILF("PNG tEXt keyword should be no longer than %d.",
                             PNG_KEYWORD_MAX_LENGTH);
                clippedKeys.emplace_back(originalKeyword, PNG_KEYWORD_MAX_LENGTH);
                keyword = clippedKeys.back().c_str();
            }
            // It seems safe to convert png_const_charp to png_charp for key/text,
            // and we don't have to provide text_length and other fields as we're providing
            // 0-terminated c_str with PNG_TEXT_COMPRESSION_NONE (no compression, no itxt).
            png_texts[i].compression = PNG_TEXT_COMPRESSION_NONE;
            png_texts[i].key = const_cast<png_charp>(keyword);
            png_texts[i].text = const_cast<png_charp>(text);
        }
        png_set_text(fPngPtr, fInfoPtr, png_texts.data(), png_texts.size());
    }

    return true;
}

static void set_icc(png_structp png_ptr,
                    png_infop info_ptr,
                    const SkImageInfo& info,
                    const skcms_ICCProfile* profile,
                    const char* profile_description) {
    sk_sp<SkData> icc = icc_from_color_space(info, profile, profile_description);
    if (!icc) {
        return;
    }

#if PNG_LIBPNG_VER_MAJOR > 1 || (PNG_LIBPNG_VER_MAJOR == 1 && PNG_LIBPNG_VER_MINOR >= 5)
    const char* name = "Skia";
    png_const_bytep iccPtr = icc->bytes();
#else
    SkString str("Skia");
    char* name = str.data();
    png_charp iccPtr = (png_charp)icc->writable_data();
#endif
    png_set_iCCP(png_ptr, info_ptr, name, 0, iccPtr, icc->size());
}

bool SkPngEncoderMgr::setColorSpace(const SkImageInfo& info, const SkPngEncoder::Options& options) {
    if (setjmp(png_jmpbuf(fPngPtr))) {
        return false;
    }

    if (info.colorSpace() && info.colorSpace()->isSRGB()) {
        png_set_sRGB(fPngPtr, fInfoPtr, PNG_sRGB_INTENT_PERCEPTUAL);
    } else {
        set_icc(fPngPtr, fInfoPtr, info, options.fICCProfile, options.fICCProfileDescription);
    }

    return true;
}

bool SkPngEncoderMgr::setV0Gainmap(const SkPngEncoder::Options& options) {
#ifdef PNG_STORE_UNKNOWN_CHUNKS_SUPPORTED
    if (setjmp(png_jmpbuf(fPngPtr))) {
        return false;
    }

    // We require some gainmap information.
    if (!options.fGainmapInfo) {
        return false;
    }

    if (options.fGainmap) {
        sk_sp<SkData> gainmapVersion = SkGainmapInfo::SerializeVersion();
        SkDynamicMemoryWStream gainmapStream;

        // When we encode the gainmap, we need to remove the gainmap from its
        // own encoding options, so that we don't recurse.
        auto modifiedOptions = options;
        modifiedOptions.fGainmap = nullptr;

        auto gainmapInfo = *(options.fGainmapInfo);
        auto gainmapPixels = *(options.fGainmap);
        auto targetInfo = SkPngEncoderBase::getTargetInfo(gainmapPixels.info());

        if (targetInfo && targetInfo->fDstInfo.color() != SkEncodedInfo::kGray_Color &&
            targetInfo->fDstInfo.color() != SkEncodedInfo::kGrayAlpha_Color) {
            // Encode the alternate image colorspace directly in the gainmap profile,
            // since the ISO gainmap payload does not contain the actual alternative
            // image primaries.
            const auto& gainmapColorSpace = options.fGainmapInfo->fGainmapMathColorSpace;
            gainmapPixels.setColorSpace(gainmapColorSpace);
        } else {
            // Scrub the gainmap colorspace, since grayscale PNGs don't support
            // RGB ICC profiles
            gainmapInfo.fGainmapMathColorSpace = nullptr;
            modifiedOptions.fGainmapInfo = &gainmapInfo;
        }

        bool result = SkPngEncoder::Encode(&gainmapStream, gainmapPixels, modifiedOptions);
        if (!result) {
            return false;
        }

        sk_sp<SkData> gainmapData = gainmapStream.detachAsData();

        // The base image contains chunks for both the gainmap versioning (for possible
        // forward-compat, and as a cheap way to check a gainmap might exist) as
        // well as the gainmap data.
        std::array<png_unknown_chunk, 2> chunks;
        auto& gmapChunk = chunks.at(0);
        std::strcpy(reinterpret_cast<char*>(gmapChunk.name), "gmAP\0");
        gmapChunk.data = reinterpret_cast<png_byte*>(gainmapVersion->writable_data());
        gmapChunk.size = gainmapVersion->size();
        gmapChunk.location = PNG_HAVE_IHDR;

        auto& gdatChunk = chunks.at(1);
        std::strcpy(reinterpret_cast<char*>(gdatChunk.name), "gdAT\0");
        gdatChunk.data = reinterpret_cast<png_byte*>(gainmapData->writable_data());
        gdatChunk.size = gainmapData->size();
        gdatChunk.location = PNG_HAVE_IHDR;

        png_set_keep_unknown_chunks(fPngPtr, PNG_HANDLE_CHUNK_ALWAYS,
                                    (png_const_bytep)"gmAP\0gdAT\0", chunks.size());
        png_set_unknown_chunks(fPngPtr, fInfoPtr, chunks.data(), chunks.size());
    } else {
        // If there is no gainmap provided for encoding, but we have info, then
        // we're currently encoding the gainmap pixels, so we need to encode the
        // gainmap metadata to interpret those pixels.
        sk_sp<SkData> data = options.fGainmapInfo->serialize();
        png_unknown_chunk chunk;
        std::strcpy(reinterpret_cast<char*>(chunk.name), "gmAP\0");
        chunk.data = reinterpret_cast<png_byte*>(data->writable_data());
        chunk.size = data->size();
        chunk.location = PNG_HAVE_IHDR;
        png_set_keep_unknown_chunks(fPngPtr, PNG_HANDLE_CHUNK_ALWAYS,
                                    (png_const_bytep)"gmAP\0", 1);
        png_set_unknown_chunks(fPngPtr, fInfoPtr, &chunk, 1);
    }
#endif
    return true;
}

bool SkPngEncoderMgr::writeInfo(const SkImageInfo& srcInfo, const SkPngEncoderBase::TargetInfo& targetInfo) {
  if (setjmp(png_jmpbuf(fPngPtr))) {
      return false;
  }
  png_write_info(fPngPtr, fInfoPtr);

  const SkEncodedInfo& dstInfo = targetInfo.fDstInfo;
  const std::optional<SkImageInfo>& dstRowInfo = targetInfo.fDstRowInfo;

  // Strip input data that has 4 or 8 bytes per pixel down to 3 or 6 bytes if we don't want alpha.
  if (dstInfo.color() == SkEncodedInfo::kRGBA_Color) {
      SkASSERT(dstRowInfo);
      if (dstRowInfo->isOpaque()) {
        png_set_filler(fPngPtr, 0, PNG_FILLER_AFTER);
      }
  }
  return true;
}

SkPngEncoderImpl::SkPngEncoderImpl(TargetInfo targetInfo,
                                   std::unique_ptr<SkPngEncoderMgr> encoderMgr,
                                   const SkPixmap& src)
        : SkPngEncoderBase(std::move(targetInfo), src), fEncoderMgr(std::move(encoderMgr)) {}

SkPngEncoderImpl::~SkPngEncoderImpl() {}

bool SkPngEncoderImpl::onEncodeRow(SkSpan<const uint8_t> row) {
    if (setjmp(png_jmpbuf(fEncoderMgr->pngPtr()))) {
        return false;
    }

    // `png_bytep` is `uint8_t*` rather than `const uint8_t*`.
    png_bytep rowPtr = const_cast<png_bytep>(row.data());

    png_write_rows(fEncoderMgr->pngPtr(), &rowPtr, 1);
    return true;
}

bool SkPngEncoderImpl::onFinishEncoding() {
    if (setjmp(png_jmpbuf(fEncoderMgr->pngPtr()))) {
        return false;
    }

    png_write_end(fEncoderMgr->pngPtr(), fEncoderMgr->infoPtr());
    return true;
}

namespace SkPngEncoder {
std::unique_ptr<SkEncoder> Make(SkWStream* dst, const SkPixmap& src, const Options& options) {
    if (!SkPixmapIsValid(src)) {
        return nullptr;
    }

    std::unique_ptr<SkPngEncoderMgr> encoderMgr = SkPngEncoderMgr::Make(dst);
    if (!encoderMgr) {
        return nullptr;
    }

    std::optional<SkPngEncoderBase::TargetInfo> targetInfo =
            SkPngEncoderBase::getTargetInfo(src.info());
    if (!targetInfo.has_value()) {
        return nullptr;
    }

    if (!encoderMgr->setHeader(targetInfo.value(), src.info(), options)) {
      return nullptr;
    }

    if (!encoderMgr->setColorSpace(src.info(), options)) {
        return nullptr;
    }

    if (options.fGainmapInfo && !encoderMgr->setV0Gainmap(options)) {
        return nullptr;
    }

    if (!encoderMgr->writeInfo(src.info(), targetInfo.value())) {
        return nullptr;
    }
    return std::make_unique<SkPngEncoderImpl>(std::move(*targetInfo), std::move(encoderMgr), src);
}

bool Encode(SkWStream* dst, const SkPixmap& src, const Options& options) {
    auto encoder = Make(dst, src, options);
    return encoder.get() && encoder->encodeRows(src.height());
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

}  // namespace SkPngEncoder
