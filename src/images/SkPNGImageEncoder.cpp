/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImageEncoderPriv.h"

#ifdef SK_HAS_PNG_LIBRARY

#include "SkColor.h"
#include "SkColorPriv.h"
#include "SkDither.h"
#include "SkMath.h"
#include "SkStream.h"
#include "SkTemplates.h"
#include "SkUnPreMultiply.h"
#include "SkUtils.h"
#include "transform_scanline.h"

#include "png.h"

// Suppress most PNG warnings when calling image decode functions.
static const bool c_suppressPNGImageDecoderWarnings = true;

static void sk_error_fn(png_structp png_ptr, png_const_charp msg) {
    if (!c_suppressPNGImageDecoderWarnings) {
        SkDEBUGF(("------ png error %s\n", msg));
    }
    longjmp(png_jmpbuf(png_ptr), 1);
}

static void sk_write_fn(png_structp png_ptr, png_bytep data, png_size_t len) {
    SkWStream* sk_stream = (SkWStream*)png_get_io_ptr(png_ptr);
    if (!sk_stream->write(data, len)) {
        png_error(png_ptr, "sk_write_fn Error!");
    }
}

static transform_scanline_proc choose_proc(SkColorType ct, SkAlphaType alphaType) {
    static const struct {
        SkColorType             fColorType;
        SkAlphaType             fAlphaType;
        transform_scanline_proc fProc;
    } gMap[] = {
        { kRGB_565_SkColorType,   kOpaque_SkAlphaType,   transform_scanline_565    },
        { kRGBA_8888_SkColorType, kOpaque_SkAlphaType,   transform_scanline_RGBX   },
        { kBGRA_8888_SkColorType, kOpaque_SkAlphaType,   transform_scanline_BGRX   },
        { kRGBA_8888_SkColorType, kPremul_SkAlphaType,   transform_scanline_rgbA   },
        { kBGRA_8888_SkColorType, kPremul_SkAlphaType,   transform_scanline_bgrA   },
        { kRGBA_8888_SkColorType, kUnpremul_SkAlphaType, transform_scanline_memcpy },
        { kBGRA_8888_SkColorType, kUnpremul_SkAlphaType, transform_scanline_BGRA   },
        { kARGB_4444_SkColorType, kOpaque_SkAlphaType,   transform_scanline_444    },
        { kARGB_4444_SkColorType, kPremul_SkAlphaType,   transform_scanline_4444   },
        { kIndex_8_SkColorType,   kOpaque_SkAlphaType,   transform_scanline_memcpy },
        { kIndex_8_SkColorType,   kPremul_SkAlphaType,   transform_scanline_memcpy },
        { kIndex_8_SkColorType,   kUnpremul_SkAlphaType, transform_scanline_memcpy },
        { kGray_8_SkColorType,    kOpaque_SkAlphaType,   transform_scanline_memcpy },
    };

    for (auto entry : gMap) {
        if (entry.fColorType == ct && entry.fAlphaType == alphaType) {
            return entry.fProc;
        }
    }
    sk_throw();
    return nullptr;
}

/*  Pack palette[] with the corresponding colors, and if the image has alpha, also
    pack trans[] and return the number of alphas[] entries written. If the image is
    opaque, the return value will always be 0.
*/
static inline int pack_palette(SkColorTable* ctable, png_color* SK_RESTRICT palette,
                               png_byte* SK_RESTRICT alphas, SkAlphaType alphaType) {
    const SkPMColor* SK_RESTRICT colors = ctable->readColors();
    const int count = ctable->count();
    int numWithAlpha = 0;
    if (kOpaque_SkAlphaType != alphaType) {
        auto getUnpremulColor = [alphaType](uint8_t color, uint8_t alpha) {
            if (kPremul_SkAlphaType == alphaType) {
                const SkUnPreMultiply::Scale* table = SkUnPreMultiply::GetScaleTable();
                const SkUnPreMultiply::Scale scale = table[alpha];
                return (uint8_t) SkUnPreMultiply::ApplyScale(scale, color);
            } else {
                return color;
            }
        };

        // PNG requires that all non-opaque colors come first in the palette.  Write these first.
        for (int i = 0; i < count; i++) {
            uint8_t alpha = SkGetPackedA32(colors[i]);
            if (0xFF != alpha) {
                alphas[numWithAlpha] = alpha;
                palette[numWithAlpha].red   = getUnpremulColor(SkGetPackedR32(colors[i]), alpha);
                palette[numWithAlpha].green = getUnpremulColor(SkGetPackedG32(colors[i]), alpha);
                palette[numWithAlpha].blue  = getUnpremulColor(SkGetPackedB32(colors[i]), alpha);
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

static bool do_encode(SkWStream*, const SkPixmap&, int, int, png_color_8&);

bool SkEncodeImageAsPNG(SkWStream* stream, const SkPixmap& pixmap) {
    if (!pixmap.addr() || pixmap.info().isEmpty()) {
        return false;
    }
    const SkColorType ct = pixmap.colorType();
    switch (ct) {
        case kIndex_8_SkColorType:
        case kGray_8_SkColorType:
        case kRGBA_8888_SkColorType:
        case kBGRA_8888_SkColorType:
        case kARGB_4444_SkColorType:
        case kRGB_565_SkColorType:
            break;
        default:
            return false;
    }

    const SkAlphaType alphaType = pixmap.alphaType();
    switch (alphaType) {
        case kUnpremul_SkAlphaType:
            if (kARGB_4444_SkColorType == ct) {
                return false;
            }

            break;
        case kOpaque_SkAlphaType:
        case kPremul_SkAlphaType:
            break;
        default:
            return false;
    }

    const bool isOpaque = (kOpaque_SkAlphaType == alphaType);
    const int bitDepth = 8;
    png_color_8 sig_bit;
    sk_bzero(&sig_bit, sizeof(png_color_8));

    int colorType;
    switch (ct) {
        case kIndex_8_SkColorType:
            sig_bit.red = 8;
            sig_bit.green = 8;
            sig_bit.blue = 8;
            sig_bit.alpha = 8;
            colorType = PNG_COLOR_TYPE_PALETTE;
            break;
        case kGray_8_SkColorType:
            sig_bit.gray = 8;
            colorType = PNG_COLOR_TYPE_GRAY;
            SkASSERT(isOpaque);
            break;
        case kRGBA_8888_SkColorType:
        case kBGRA_8888_SkColorType:
            sig_bit.red = 8;
            sig_bit.green = 8;
            sig_bit.blue = 8;
            sig_bit.alpha = 8;
            colorType = isOpaque ? PNG_COLOR_TYPE_RGB : PNG_COLOR_TYPE_RGB_ALPHA;
            break;
        case kARGB_4444_SkColorType:
            sig_bit.red = 4;
            sig_bit.green = 4;
            sig_bit.blue = 4;
            sig_bit.alpha = 4;
            colorType = isOpaque ? PNG_COLOR_TYPE_RGB : PNG_COLOR_TYPE_RGB_ALPHA;
            break;
        case kRGB_565_SkColorType:
            sig_bit.red = 5;
            sig_bit.green = 6;
            sig_bit.blue = 5;
            colorType = PNG_COLOR_TYPE_RGB;
            SkASSERT(isOpaque);
            break;
        default:
            return false;
    }
    if (kIndex_8_SkColorType == ct) {
        SkColorTable* ctable = pixmap.ctable();
        if (!ctable || ctable->count() == 0) {
            return false;
        }

        // Currently, we always use 8-bit indices for paletted pngs.
        // When ctable->count() <= 16, we could potentially use 1, 2,
        // or 4 bit indices.
    }
    return do_encode(stream, pixmap, colorType, bitDepth, sig_bit);
}

static bool do_encode(SkWStream* stream, const SkPixmap& pixmap,
                      int colorType, int bitDepth, png_color_8& sig_bit) {
    SkAlphaType alphaType = pixmap.alphaType();
    SkColorType ct = pixmap.colorType();

    png_structp png_ptr;
    png_infop info_ptr;

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, sk_error_fn, nullptr);
    if (nullptr == png_ptr) {
        return false;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (nullptr == info_ptr) {
        png_destroy_write_struct(&png_ptr,  nullptr);
        return false;
    }

    /* Set error handling.  REQUIRED if you aren't supplying your own
    * error handling functions in the png_create_write_struct() call.
    */
    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        return false;
    }

    png_set_write_fn(png_ptr, (void*)stream, sk_write_fn, nullptr);

    /* Set the image information here.  Width and height are up to 2^31,
    * bit_depth is one of 1, 2, 4, 8, or 16, but valid values also depend on
    * the color_type selected. color_type is one of PNG_COLOR_TYPE_GRAY,
    * PNG_COLOR_TYPE_GRAY_ALPHA, PNG_COLOR_TYPE_PALETTE, PNG_COLOR_TYPE_RGB,
    * or PNG_COLOR_TYPE_RGB_ALPHA.  interlace is either PNG_INTERLACE_NONE or
    * PNG_INTERLACE_ADAM7, and the compression_type and filter_type MUST
    * currently be PNG_COMPRESSION_TYPE_BASE and PNG_FILTER_TYPE_BASE. REQUIRED
    */

    png_set_IHDR(png_ptr, info_ptr, pixmap.width(), pixmap.height(),
                 bitDepth, colorType,
                 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE,
                 PNG_FILTER_TYPE_BASE);

    // set our colortable/trans arrays if needed
    png_color paletteColors[256];
    png_byte trans[256];
    if (kIndex_8_SkColorType == ct) {
        SkColorTable* colorTable = pixmap.ctable();
        SkASSERT(colorTable);
        int numTrans = pack_palette(colorTable, paletteColors, trans, alphaType);
        png_set_PLTE(png_ptr, info_ptr, paletteColors, colorTable->count());
        if (numTrans > 0) {
            png_set_tRNS(png_ptr, info_ptr, trans, numTrans, nullptr);
        }
    }

    png_set_sBIT(png_ptr, info_ptr, &sig_bit);
    png_write_info(png_ptr, info_ptr);

    const char* srcImage = (const char*)pixmap.addr();
    SkAutoSTMalloc<1024, char> rowStorage(pixmap.width() << 2);
    char* storage = rowStorage.get();
    transform_scanline_proc proc = choose_proc(ct, alphaType);

    for (int y = 0; y < pixmap.height(); y++) {
        png_bytep row_ptr = (png_bytep)storage;
        proc(storage, srcImage, pixmap.width(), SkColorTypeBytesPerPixel(ct));
        png_write_rows(png_ptr, &row_ptr, 1);
        srcImage += pixmap.rowBytes();
    }

    png_write_end(png_ptr, info_ptr);

    /* clean up after the write, and free any memory allocated */
    png_destroy_write_struct(&png_ptr, &info_ptr);
    return true;
}

#endif
