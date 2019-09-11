/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/images/SkImageEncoderPriv.h"

#ifdef SK_HAS_PNG_LIBRARY

#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/encode/SkPngEncoder.h"
#include "include/private/SkImageInfoPriv.h"
#include "src/codec/SkColorTable.h"
#include "src/codec/SkPngPriv.h"
#include "src/images/SkImageEncoderFns.h"
#include <vector>

#include "png.h"

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
    bool setColorSpace(const SkImageInfo& info);
    bool writeInfo(const SkImageInfo& srcInfo);
    void chooseProc(const SkImageInfo& srcInfo);

    png_structp pngPtr() { return fPngPtr; }
    png_infop infoPtr() { return fInfoPtr; }
    int pngBytesPerPixel() const { return fPngBytesPerPixel; }
    transform_scanline_proc proc() const { return fProc; }

    ~SkPngEncoderMgr() {
        png_destroy_write_struct(&fPngPtr, &fInfoPtr);
    }

private:

    SkPngEncoderMgr(png_structp pngPtr, png_infop infoPtr)
        : fPngPtr(pngPtr)
        , fInfoPtr(infoPtr)
    {}

    png_structp             fPngPtr;
    png_infop               fInfoPtr;
    int                     fPngBytesPerPixel;
    transform_scanline_proc fProc;
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

    int pngColorType;
    png_color_8 sigBit;
    int bitDepth = 8;
    switch (srcInfo.colorType()) {
        case kRGBA_F16Norm_SkColorType:
        case kRGBA_F16_SkColorType:
        case kRGBA_F32_SkColorType:
            sigBit.red = 16;
            sigBit.green = 16;
            sigBit.blue = 16;
            sigBit.alpha = 16;
            bitDepth = 16;
            pngColorType = srcInfo.isOpaque() ? PNG_COLOR_TYPE_RGB : PNG_COLOR_TYPE_RGB_ALPHA;
            fPngBytesPerPixel = 8;
            break;
        case kGray_8_SkColorType:
            sigBit.gray = 8;
            pngColorType = PNG_COLOR_TYPE_GRAY;
            fPngBytesPerPixel = 1;
            SkASSERT(srcInfo.isOpaque());
            break;
        case kRGBA_8888_SkColorType:
        case kBGRA_8888_SkColorType:
            sigBit.red = 8;
            sigBit.green = 8;
            sigBit.blue = 8;
            sigBit.alpha = 8;
            pngColorType = srcInfo.isOpaque() ? PNG_COLOR_TYPE_RGB : PNG_COLOR_TYPE_RGB_ALPHA;
            fPngBytesPerPixel = srcInfo.isOpaque() ? 3 : 4;
            break;
        case kRGB_888x_SkColorType:
            sigBit.red   = 8;
            sigBit.green = 8;
            sigBit.blue  = 8;
            pngColorType = PNG_COLOR_TYPE_RGB;
            fPngBytesPerPixel = 3;
            SkASSERT(srcInfo.isOpaque());
            break;
        case kARGB_4444_SkColorType:
            if (kUnpremul_SkAlphaType == srcInfo.alphaType()) {
                return false;
            }

            sigBit.red = 4;
            sigBit.green = 4;
            sigBit.blue = 4;
            sigBit.alpha = 4;
            pngColorType = srcInfo.isOpaque() ? PNG_COLOR_TYPE_RGB : PNG_COLOR_TYPE_RGB_ALPHA;
            fPngBytesPerPixel = srcInfo.isOpaque() ? 3 : 4;
            break;
        case kRGB_565_SkColorType:
            sigBit.red = 5;
            sigBit.green = 6;
            sigBit.blue = 5;
            pngColorType = PNG_COLOR_TYPE_RGB;
            fPngBytesPerPixel = 3;
            SkASSERT(srcInfo.isOpaque());
            break;
        case kAlpha_8_SkColorType:  // store as gray+alpha, but ignore gray
            sigBit.gray = kGraySigBit_GrayAlphaIsJustAlpha;
            sigBit.alpha = 8;
            pngColorType = PNG_COLOR_TYPE_GRAY_ALPHA;
            fPngBytesPerPixel = 2;
            break;
        case kRGBA_1010102_SkColorType:
            bitDepth     = 16;
            sigBit.red   = 10;
            sigBit.green = 10;
            sigBit.blue  = 10;
            sigBit.alpha = 2;
            pngColorType = srcInfo.isOpaque() ? PNG_COLOR_TYPE_RGB : PNG_COLOR_TYPE_RGB_ALPHA;
            fPngBytesPerPixel = 8;
            break;
        case kRGB_101010x_SkColorType:
            bitDepth     = 16;
            sigBit.red   = 10;
            sigBit.green = 10;
            sigBit.blue  = 10;
            pngColorType = PNG_COLOR_TYPE_RGB;
            fPngBytesPerPixel = 6;
            break;
        default:
            return false;
    }

    png_set_IHDR(fPngPtr, fInfoPtr, srcInfo.width(), srcInfo.height(),
                 bitDepth, pngColorType,
                 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE,
                 PNG_FILTER_TYPE_BASE);
    png_set_sBIT(fPngPtr, fInfoPtr, &sigBit);

    int filters = (int)options.fFilterFlags & (int)SkPngEncoder::FilterFlag::kAll;
    SkASSERT(filters == (int)options.fFilterFlags);
    png_set_filter(fPngPtr, PNG_FILTER_TYPE_BASE, filters);

    int zlibLevel = SkTMin(SkTMax(0, options.fZLibLevel), 9);
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
            png_texts[i].key = (png_charp)keyword;
            png_texts[i].text = (png_charp)text;
        }
        png_set_text(fPngPtr, fInfoPtr, png_texts.data(), png_texts.size());
    }

    return true;
}

static transform_scanline_proc choose_proc(const SkImageInfo& info) {
    switch (info.colorType()) {
        case kUnknown_SkColorType:
            break;

        case kRGBA_8888_SkColorType:
            switch (info.alphaType()) {
                case kOpaque_SkAlphaType:
                    return transform_scanline_RGBX;
                case kUnpremul_SkAlphaType:
                    return transform_scanline_memcpy;
                case kPremul_SkAlphaType:
                    return transform_scanline_rgbA;
                default:
                    SkASSERT(false);
                    return nullptr;
            }
        case kBGRA_8888_SkColorType:
            switch (info.alphaType()) {
                case kOpaque_SkAlphaType:
                    return transform_scanline_BGRX;
                case kUnpremul_SkAlphaType:
                    return transform_scanline_BGRA;
                case kPremul_SkAlphaType:
                    return transform_scanline_bgrA;
                default:
                    SkASSERT(false);
                    return nullptr;
            }
        case kRGB_565_SkColorType:
            return transform_scanline_565;
        case kRGB_888x_SkColorType:
            return transform_scanline_RGBX;
        case kARGB_4444_SkColorType:
            switch (info.alphaType()) {
                case kOpaque_SkAlphaType:
                    return transform_scanline_444;
                case kPremul_SkAlphaType:
                    return transform_scanline_4444;
                default:
                    SkASSERT(false);
                    return nullptr;
            }
        case kGray_8_SkColorType:
            return transform_scanline_memcpy;

        case kRGBA_F16Norm_SkColorType:
        case kRGBA_F16_SkColorType:
            switch (info.alphaType()) {
                case kOpaque_SkAlphaType:
                case kUnpremul_SkAlphaType:
                    return transform_scanline_F16;
                case kPremul_SkAlphaType:
                    return transform_scanline_F16_premul;
                default:
                    SkASSERT(false);
                    return nullptr;
            }
        case kRGBA_F32_SkColorType:
            switch (info.alphaType()) {
                case kOpaque_SkAlphaType:
                case kUnpremul_SkAlphaType:
                    return transform_scanline_F32;
                case kPremul_SkAlphaType:
                    return transform_scanline_F32_premul;
                default:
                    SkASSERT(false);
                    return nullptr;
            }
        case kRGBA_1010102_SkColorType:
            switch (info.alphaType()) {
                case kOpaque_SkAlphaType:
                case kUnpremul_SkAlphaType:
                    return transform_scanline_1010102;
                case kPremul_SkAlphaType:
                    return transform_scanline_1010102_premul;
                default:
                    SkASSERT(false);
                    return nullptr;
            }
        case kRGB_101010x_SkColorType:
            return transform_scanline_101010x;
        case kAlpha_8_SkColorType:
            return transform_scanline_A8_to_GrayAlpha;
        case kRG_88_SkColorType:
        case kRG_1616_SkColorType:
        case kAlpha_16_SkColorType:
            return nullptr;
    }
    SkASSERT(false);
    return nullptr;
}

static void set_icc(png_structp png_ptr, png_infop info_ptr, const SkImageInfo& info) {
    sk_sp<SkData> icc = icc_from_color_space(info);
    if (!icc) {
        return;
    }

#if PNG_LIBPNG_VER_MAJOR > 1 || (PNG_LIBPNG_VER_MAJOR == 1 && PNG_LIBPNG_VER_MINOR >= 5)
    const char* name = "Skia";
    png_const_bytep iccPtr = icc->bytes();
#else
    SkString str("Skia");
    char* name = str.writable_str();
    png_charp iccPtr = (png_charp) icc->writable_data();
#endif
    png_set_iCCP(png_ptr, info_ptr, name, 0, iccPtr, icc->size());
}

bool SkPngEncoderMgr::setColorSpace(const SkImageInfo& info) {
    if (setjmp(png_jmpbuf(fPngPtr))) {
        return false;
    }

    if (info.colorSpace() && info.colorSpace()->isSRGB()) {
        png_set_sRGB(fPngPtr, fInfoPtr, PNG_sRGB_INTENT_PERCEPTUAL);
    } else {
        set_icc(fPngPtr, fInfoPtr, info);
    }

    return true;
}

bool SkPngEncoderMgr::writeInfo(const SkImageInfo& srcInfo) {
    if (setjmp(png_jmpbuf(fPngPtr))) {
        return false;
    }

    png_write_info(fPngPtr, fInfoPtr);
    if (kRGBA_F16_SkColorType == srcInfo.colorType() &&
        kOpaque_SkAlphaType == srcInfo.alphaType())
    {
        // For kOpaque, kRGBA_F16, we will keep the row as RGBA and tell libpng
        // to skip the alpha channel.
        png_set_filler(fPngPtr, 0, PNG_FILLER_AFTER);
    }

    return true;
}

void SkPngEncoderMgr::chooseProc(const SkImageInfo& srcInfo) {
    fProc = choose_proc(srcInfo);
}

std::unique_ptr<SkEncoder> SkPngEncoder::Make(SkWStream* dst, const SkPixmap& src,
                                              const Options& options) {
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

    if (!encoderMgr->setColorSpace(src.info())) {
        return nullptr;
    }

    if (!encoderMgr->writeInfo(src.info())) {
        return nullptr;
    }

    encoderMgr->chooseProc(src.info());

    return std::unique_ptr<SkPngEncoder>(new SkPngEncoder(std::move(encoderMgr), src));
}

SkPngEncoder::SkPngEncoder(std::unique_ptr<SkPngEncoderMgr> encoderMgr, const SkPixmap& src)
    : INHERITED(src, encoderMgr->pngBytesPerPixel() * src.width())
    , fEncoderMgr(std::move(encoderMgr))
{}

SkPngEncoder::~SkPngEncoder() {}

bool SkPngEncoder::onEncodeRows(int numRows) {
    if (setjmp(png_jmpbuf(fEncoderMgr->pngPtr()))) {
        return false;
    }

    const void* srcRow = fSrc.addr(0, fCurrRow);
    for (int y = 0; y < numRows; y++) {
        fEncoderMgr->proc()((char*)fStorage.get(),
                            (const char*)srcRow,
                            fSrc.width(),
                            SkColorTypeBytesPerPixel(fSrc.colorType()));

        png_bytep rowPtr = (png_bytep) fStorage.get();
        png_write_rows(fEncoderMgr->pngPtr(), &rowPtr, 1);
        srcRow = SkTAddOffset<const void>(srcRow, fSrc.rowBytes());
    }

    fCurrRow += numRows;
    if (fCurrRow == fSrc.height()) {
        png_write_end(fEncoderMgr->pngPtr(), fEncoderMgr->infoPtr());
    }

    return true;
}

bool SkPngEncoder::Encode(SkWStream* dst, const SkPixmap& src, const Options& options) {
    auto encoder = SkPngEncoder::Make(dst, src, options);
    return encoder.get() && encoder->encodeRows(src.height());
}

#endif
