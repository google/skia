/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImageEncoderPriv.h"

#ifdef SK_HAS_PNG_LIBRARY

#include "SkImageEncoderFns.h"
#include "SkImageInfoPriv.h"
#include "SkStream.h"
#include "SkString.h"
#include "SkPngEncoder.h"

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
    bool setPalette(const SkImageInfo& srcInfo, SkColorTable* colorTable,
                    SkTransferFunctionBehavior);
    bool setColorSpace(SkColorSpace* colorSpace);
    bool writeInfo(const SkImageInfo& srcInfo);
    void chooseProc(const SkImageInfo& srcInfo, SkTransferFunctionBehavior unpremulBehavior);

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
        case kRGBA_F16_SkColorType:
            SkASSERT(srcInfo.colorSpace() && srcInfo.colorSpace()->gammaIsLinear());
            sigBit.red = 16;
            sigBit.green = 16;
            sigBit.blue = 16;
            sigBit.alpha = 16;
            bitDepth = 16;
            pngColorType = srcInfo.isOpaque() ? PNG_COLOR_TYPE_RGB : PNG_COLOR_TYPE_RGB_ALPHA;
            fPngBytesPerPixel = 8;
            break;
        case kIndex_8_SkColorType:
            sigBit.red = 8;
            sigBit.green = 8;
            sigBit.blue = 8;
            sigBit.alpha = 8;
            pngColorType = PNG_COLOR_TYPE_PALETTE;
            fPngBytesPerPixel = 1;
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
    return true;
}

static transform_scanline_proc choose_proc(const SkImageInfo& info,
                                           SkTransferFunctionBehavior unpremulBehavior) {
    const bool isSRGBTransferFn =
            (SkTransferFunctionBehavior::kRespect == unpremulBehavior) && info.gammaCloseToSRGB();
    switch (info.colorType()) {
        case kRGBA_8888_SkColorType:
            switch (info.alphaType()) {
                case kOpaque_SkAlphaType:
                    return transform_scanline_RGBX;
                case kUnpremul_SkAlphaType:
                    return transform_scanline_memcpy;
                case kPremul_SkAlphaType:
                    return isSRGBTransferFn ? transform_scanline_srgbA :
                                              transform_scanline_rgbA;
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
                    return isSRGBTransferFn ? transform_scanline_sbgrA :
                                              transform_scanline_bgrA;
                default:
                    SkASSERT(false);
                    return nullptr;
            }
        case kRGB_565_SkColorType:
            return transform_scanline_565;
        case kARGB_4444_SkColorType:
            switch (info.alphaType()) {
                case kOpaque_SkAlphaType:
                    return transform_scanline_444;
                case kPremul_SkAlphaType:
                    // 4444 is assumed to be legacy premul.
                    return transform_scanline_4444;
                default:
                    SkASSERT(false);
                    return nullptr;
            }
        case kIndex_8_SkColorType:
        case kGray_8_SkColorType:
            return transform_scanline_memcpy;
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
        default:
            SkASSERT(false);
            return nullptr;
    }
}

/*
 * Pack palette[] with the corresponding colors, and if the image has alpha, also
 * pack trans[] and return the number of alphas[] entries written. If the image is
 * opaque, the return value will always be 0.
 */
static inline int pack_palette(SkColorTable* ctable, png_color* SK_RESTRICT palette,
                               png_byte* SK_RESTRICT alphas, const SkImageInfo& info,
                               SkTransferFunctionBehavior unpremulBehavior) {
    const SkPMColor* colors = ctable->readColors();
    const int count = ctable->count();
    SkPMColor storage[256];
    if (kPremul_SkAlphaType == info.alphaType()) {
        // Unpremultiply the colors.
        const SkImageInfo rgbaInfo = info.makeColorType(kRGBA_8888_SkColorType);
        transform_scanline_proc proc = choose_proc(rgbaInfo, unpremulBehavior);
        proc((char*) storage, (const char*) colors, ctable->count(), 4, nullptr);
        colors = storage;
    }

    int numWithAlpha = 0;
    if (kOpaque_SkAlphaType != info.alphaType()) {
        // PNG requires that all non-opaque colors come first in the palette.  Write these first.
        for (int i = 0; i < count; i++) {
            uint8_t alpha = SkGetPackedA32(colors[i]);
            if (0xFF != alpha) {
                alphas[numWithAlpha] = alpha;
                palette[numWithAlpha].red   = SkGetPackedR32(colors[i]);
                palette[numWithAlpha].green = SkGetPackedG32(colors[i]);
                palette[numWithAlpha].blue  = SkGetPackedB32(colors[i]);
                numWithAlpha++;
            }
        }
    }

    if (0 == numWithAlpha) {
        // All of the entries are opaque.
        for (int i = 0; i < count; i++) {
            SkPMColor c = *colors++;
            palette[i].red   = SkGetPackedR32(c);
            palette[i].green = SkGetPackedG32(c);
            palette[i].blue  = SkGetPackedB32(c);
        }
    } else {
        // We have already written the non-opaque colors.  Now just write the opaque colors.
        int currIndex = numWithAlpha;
        int i = 0;
        while (currIndex != count) {
            uint8_t alpha = SkGetPackedA32(colors[i]);
            if (0xFF == alpha) {
                palette[currIndex].red   = SkGetPackedR32(colors[i]);
                palette[currIndex].green = SkGetPackedG32(colors[i]);
                palette[currIndex].blue  = SkGetPackedB32(colors[i]);
                currIndex++;
            }

            i++;
        }
    }

    return numWithAlpha;
}

bool SkPngEncoderMgr::setPalette(const SkImageInfo& srcInfo, SkColorTable* colorTable,
                                 SkTransferFunctionBehavior unpremulBehavior) {
    if (setjmp(png_jmpbuf(fPngPtr))) {
        return false;
    }

    png_color paletteColors[256];
    png_byte trans[256];
    if (kIndex_8_SkColorType == srcInfo.colorType()) {
        if (!colorTable || colorTable->count() <= 0) {
            return false;
        }

        int numTrans = pack_palette(colorTable, paletteColors, trans, srcInfo, unpremulBehavior);
        png_set_PLTE(fPngPtr, fInfoPtr, paletteColors, colorTable->count());
        if (numTrans > 0) {
            png_set_tRNS(fPngPtr, fInfoPtr, trans, numTrans, nullptr);
        }
    }

    return true;
}

static void set_icc(png_structp png_ptr, png_infop info_ptr, const SkColorSpace& colorSpace) {
    sk_sp<SkData> icc = icc_from_color_space(colorSpace);
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

bool SkPngEncoderMgr::setColorSpace(SkColorSpace* colorSpace) {
    if (setjmp(png_jmpbuf(fPngPtr))) {
        return false;
    }

    if (colorSpace) {
        if (colorSpace->isSRGB()) {
            png_set_sRGB(fPngPtr, fInfoPtr, PNG_sRGB_INTENT_PERCEPTUAL);
        } else {
            set_icc(fPngPtr, fInfoPtr, *colorSpace);
        }
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

void SkPngEncoderMgr::chooseProc(const SkImageInfo& srcInfo,
                                 SkTransferFunctionBehavior unpremulBehavior) {
    fProc = choose_proc(srcInfo, unpremulBehavior);
}

std::unique_ptr<SkEncoder> SkPngEncoder::Make(SkWStream* dst, const SkPixmap& src,
                                              const Options& options) {
    if (!SkPixmapIsValid(src, options.fUnpremulBehavior)) {
        return nullptr;
    }

    std::unique_ptr<SkPngEncoderMgr> encoderMgr = SkPngEncoderMgr::Make(dst);
    if (!encoderMgr) {
        return nullptr;
    }

    if (!encoderMgr->setHeader(src.info(), options)) {
        return nullptr;
    }

    if (!encoderMgr->setPalette(src.info(), src.ctable(), options.fUnpremulBehavior)) {
        return nullptr;
    }

    if (!encoderMgr->setColorSpace(src.colorSpace())) {
        return nullptr;
    }

    if (!encoderMgr->writeInfo(src.info())) {
        return nullptr;
    }

    encoderMgr->chooseProc(src.info(), options.fUnpremulBehavior);

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
        fEncoderMgr->proc()((char*) fStorage.get(), (const char*) srcRow, fSrc.width(),
                            SkColorTypeBytesPerPixel(fSrc.colorType()), nullptr);

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
