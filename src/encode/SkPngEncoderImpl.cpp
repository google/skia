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
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/encode/SkEncoder.h"
#include "include/encode/SkPngEncoder.h"
#include "include/private/SkEncodedInfo.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkNoncopyable.h"
#include "include/private/base/SkTemplates.h"
#include "modules/skcms/skcms.h"
#include "src/base/SkMSAN.h"
#include "src/base/SkSafeMath.h"
#include "src/codec/SkPngPriv.h"
#include "src/encode/SkImageEncoderFns.h"
#include "src/encode/SkImageEncoderPriv.h"
#include "src/encode/SkPngEncoderBase.h"
#include "src/image/SkImage_Base.h"

#include <algorithm>
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

static constexpr bool kSuppressPngEncodeWarnings = true;

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

    bool setHeader(const SkImageInfo& srcInfo, const SkPngEncoder::Options& options);
    bool setColorSpace(const SkImageInfo& info, const SkPngEncoder::Options& options);
    bool writeInfo(const SkImageInfo& srcInfo);

    png_structp pngPtr() { return fPngPtr; }
    png_infop infoPtr() { return fInfoPtr; }
    size_t bytesPerEncodedRow() const { return fBytesPerEncodedRow; }
    transform_scanline_proc proc() const { return fProc; }

    ~SkPngEncoderMgr() { png_destroy_write_struct(&fPngPtr, &fInfoPtr); }

private:
    SkPngEncoderMgr(png_structp pngPtr, png_infop infoPtr) : fPngPtr(pngPtr), fInfoPtr(infoPtr) {}

    png_structp fPngPtr;
    png_infop fInfoPtr;
    size_t fBytesPerEncodedRow = 0;
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

bool SkPngEncoderMgr::setHeader(const SkImageInfo& srcInfo, const SkPngEncoder::Options& options) {
    if (setjmp(png_jmpbuf(fPngPtr))) {
        return false;
    }

    std::optional<std::pair<SkEncodedInfo, transform_scanline_proc>> maybeDstInfo =
            SkPngEncoderBase::getTargetInfo(srcInfo);
    if (!maybeDstInfo.has_value()) {
        return false;
    }
    const SkEncodedInfo& dstInfo = maybeDstInfo->first;
    fProc = maybeDstInfo->second;

    // `static_cast<size_t>`(dstInfo.bitsPerPixel())` uses trustworthy, bounded
    // data as input - no need to use `SkSafeMath` for this part.
    SkASSERT(dstInfo.bitsPerComponent() == 8 || dstInfo.bitsPerComponent() == 16);
    SkASSERT(dstInfo.bitsPerPixel() <= (16 * 4));
    size_t bitsPerPixel = static_cast<size_t>(dstInfo.bitsPerPixel());
    SkASSERT((bitsPerPixel % 8) == 0);
    size_t bytesPerPixel = bitsPerPixel / 8;

    SkSafeMath safe;
    fBytesPerEncodedRow = safe.mul(safe.castTo<size_t>(dstInfo.width()), bytesPerPixel);
    if (!safe.ok()) {
        return false;
    }

    int pngColorType;
    switch (dstInfo.color()) {
        case SkEncodedInfo::kRGB_Color:
            pngColorType = PNG_COLOR_TYPE_RGB;
            break;
        case SkEncodedInfo::kRGBA_Color:
            pngColorType = PNG_COLOR_TYPE_RGB_ALPHA;
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
        case kRGBA_8888_SkColorType:
        case kBGRA_8888_SkColorType:
            sigBit.red = 8;
            sigBit.green = 8;
            sigBit.blue = 8;
            sigBit.alpha = 8;
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
        default:
            return false;
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

bool SkPngEncoderMgr::writeInfo(const SkImageInfo& srcInfo) {
    if (setjmp(png_jmpbuf(fPngPtr))) {
        return false;
    }

    png_write_info(fPngPtr, fInfoPtr);
    return true;
}

SkPngEncoderImpl::SkPngEncoderImpl(std::unique_ptr<SkPngEncoderMgr> encoderMgr, const SkPixmap& src)
        : SkEncoder(src, encoderMgr->bytesPerEncodedRow()), fEncoderMgr(std::move(encoderMgr)) {}

SkPngEncoderImpl::~SkPngEncoderImpl() {}

bool SkPngEncoderImpl::onEncodeRows(int numRows) {
    if (setjmp(png_jmpbuf(fEncoderMgr->pngPtr()))) {
        return false;
    }

    const void* srcRow = fSrc.addr(0, fCurrRow);
    for (int y = 0; y < numRows; y++) {
        sk_msan_assert_initialized(srcRow,
                                   (const uint8_t*)srcRow + (fSrc.width() << fSrc.shiftPerPixel()));
        fEncoderMgr->proc()((char*)fStorage.get(),
                            (const char*)srcRow,
                            fSrc.width(),
                            SkColorTypeBytesPerPixel(fSrc.colorType()));

        png_bytep rowPtr = (png_bytep)fStorage.get();
        png_write_rows(fEncoderMgr->pngPtr(), &rowPtr, 1);
        srcRow = SkTAddOffset<const void>(srcRow, fSrc.rowBytes());
    }

    fCurrRow += numRows;
    if (fCurrRow == fSrc.height()) {
        png_write_end(fEncoderMgr->pngPtr(), fEncoderMgr->infoPtr());
    }

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

    if (!encoderMgr->setHeader(src.info(), options)) {
        return nullptr;
    }

    if (!encoderMgr->setColorSpace(src.info(), options)) {
        return nullptr;
    }

    if (!encoderMgr->writeInfo(src.info())) {
        return nullptr;
    }

    return std::make_unique<SkPngEncoderImpl>(std::move(encoderMgr), src);
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
