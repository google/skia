/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImageEncoderPriv.h"
#include "SkColor.h"
#include "SkColorPriv.h"
#include "SkDither.h"
#include "SkMath.h"
#include "SkStream.h"
#include "SkTemplates.h"
#include "SkUtils.h"
#include "transform_scanline.h"

#include "png.h"

/* These were dropped in libpng >= 1.4 */
#ifndef png_infopp_NULL
#define png_infopp_NULL nullptr
#endif

#ifndef png_bytepp_NULL
#define png_bytepp_NULL nullptr
#endif

#ifndef int_p_NULL
#define int_p_NULL nullptr
#endif

#ifndef png_flush_ptr_NULL
#define png_flush_ptr_NULL nullptr
#endif

#define DEFAULT_FOR_SUPPRESS_PNG_IMAGE_DECODER_WARNINGS true
// Suppress most PNG warnings when calling image decode functions.
static const bool c_suppressPNGImageDecoderWarnings{
    DEFAULT_FOR_SUPPRESS_PNG_IMAGE_DECODER_WARNINGS};

///////////////////////////////////////////////////////////////////////////////

#include "SkColorPriv.h"
#include "SkUnPreMultiply.h"

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

// return the minimum legal bitdepth (by png standards) for this many colortable
// entries. SkBitmap always stores in 8bits per pixel, but for colorcount <= 16,
// we can use fewer bits per in png
static int computeBitDepth(int colorCount) {
#if 0
    int bits = SkNextLog2(colorCount);
    SkASSERT(bits >= 1 && bits <= 8);
    // now we need bits itself to be a power of 2 (e.g. 1, 2, 4, 8)
    return SkNextPow2(bits);
#else
    // for the moment, we don't know how to pack bitdepth < 8
    return 8;
#endif
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

class SkPNGImageEncoder : public SkImageEncoder {
protected:
    bool onEncode(SkWStream* stream, const SkBitmap& bm, int quality) override;
private:
    bool doEncode(SkWStream* stream, const SkBitmap& bm,
                  SkAlphaType alphaType, int colorType,
                  int bitDepth, SkColorType ct,
                  png_color_8& sig_bit);

    typedef SkImageEncoder INHERITED;
};

bool SkPNGImageEncoder::onEncode(SkWStream* stream,
                                 const SkBitmap& bitmap,
                                 int /*quality*/) {
    const SkColorType ct = bitmap.colorType();
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

    const SkAlphaType alphaType = bitmap.alphaType();
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
    int bitDepth = 8;   // default for color
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

    SkAutoLockPixels alp(bitmap);
    // readyToDraw checks for pixels (and colortable if that is required)
    if (!bitmap.readyToDraw()) {
        return false;
    }

    // we must do this after we have locked the pixels
    SkColorTable* ctable = bitmap.getColorTable();
    if (ctable) {
        if (ctable->count() == 0) {
            return false;
        }
        // check if we can store in fewer than 8 bits
        bitDepth = computeBitDepth(ctable->count());
    }

    return doEncode(stream, bitmap, alphaType, colorType, bitDepth, ct, sig_bit);
}

bool SkPNGImageEncoder::doEncode(SkWStream* stream, const SkBitmap& bitmap,
                  SkAlphaType alphaType, int colorType,
                  int bitDepth, SkColorType ct,
                  png_color_8& sig_bit) {

    png_structp png_ptr;
    png_infop info_ptr;

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, sk_error_fn,
                                      nullptr);
    if (nullptr == png_ptr) {
        return false;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (nullptr == info_ptr) {
        png_destroy_write_struct(&png_ptr,  png_infopp_NULL);
        return false;
    }

    /* Set error handling.  REQUIRED if you aren't supplying your own
    * error handling functions in the png_create_write_struct() call.
    */
    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        return false;
    }

    png_set_write_fn(png_ptr, (void*)stream, sk_write_fn, png_flush_ptr_NULL);

    /* Set the image information here.  Width and height are up to 2^31,
    * bit_depth is one of 1, 2, 4, 8, or 16, but valid values also depend on
    * the color_type selected. color_type is one of PNG_COLOR_TYPE_GRAY,
    * PNG_COLOR_TYPE_GRAY_ALPHA, PNG_COLOR_TYPE_PALETTE, PNG_COLOR_TYPE_RGB,
    * or PNG_COLOR_TYPE_RGB_ALPHA.  interlace is either PNG_INTERLACE_NONE or
    * PNG_INTERLACE_ADAM7, and the compression_type and filter_type MUST
    * currently be PNG_COMPRESSION_TYPE_BASE and PNG_FILTER_TYPE_BASE. REQUIRED
    */

    png_set_IHDR(png_ptr, info_ptr, bitmap.width(), bitmap.height(),
                 bitDepth, colorType,
                 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE,
                 PNG_FILTER_TYPE_BASE);

    // set our colortable/trans arrays if needed
    png_color paletteColors[256];
    png_byte trans[256];
    if (kIndex_8_SkColorType == ct) {
        SkColorTable* colorTable = bitmap.getColorTable();
        SkASSERT(colorTable);
        int numTrans = pack_palette(colorTable, paletteColors, trans, alphaType);
        png_set_PLTE(png_ptr, info_ptr, paletteColors, colorTable->count());
        if (numTrans > 0) {
            png_set_tRNS(png_ptr, info_ptr, trans, numTrans, nullptr);
        }
    }

    png_set_sBIT(png_ptr, info_ptr, &sig_bit);
    png_write_info(png_ptr, info_ptr);

    const char* srcImage = (const char*)bitmap.getPixels();
    SkAutoSTMalloc<1024, char> rowStorage(bitmap.width() << 2);
    char* storage = rowStorage.get();
    transform_scanline_proc proc = choose_proc(ct, alphaType);

    for (int y = 0; y < bitmap.height(); y++) {
        png_bytep row_ptr = (png_bytep)storage;
        proc(storage, srcImage, bitmap.width(), SkColorTypeBytesPerPixel(ct));
        png_write_rows(png_ptr, &row_ptr, 1);
        srcImage += bitmap.rowBytes();
    }

    png_write_end(png_ptr, info_ptr);

    /* clean up after the write, and free any memory allocated */
    png_destroy_write_struct(&png_ptr, &info_ptr);
    return true;
}

///////////////////////////////////////////////////////////////////////////////
DEFINE_ENCODER_CREATOR(PNGImageEncoder);
///////////////////////////////////////////////////////////////////////////////

SkImageEncoder* sk_libpng_efactory(SkImageEncoder::Type t) {
    return (SkEncodedImageFormat::kPNG == (SkEncodedImageFormat)t) ? new SkPNGImageEncoder : nullptr;
}

static SkImageEncoder_EncodeReg gEReg(sk_libpng_efactory);
