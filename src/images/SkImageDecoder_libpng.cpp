/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImageEncoder.h"
#include "SkColor.h"
#include "SkColorPriv.h"
#include "SkDither.h"
#include "SkMath.h"
#include "SkRTConf.h"
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
SK_CONF_DECLARE(bool, c_suppressPNGImageDecoderWarnings,
                "images.png.suppressDecoderWarnings",
                DEFAULT_FOR_SUPPRESS_PNG_IMAGE_DECODER_WARNINGS,
                "Suppress most PNG warnings when calling image decode "
                "functions.");

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

static transform_scanline_proc choose_proc(SkColorType ct, bool hasAlpha) {
    // we don't care about search on alpha if we're kIndex8, since only the
    // colortable packing cares about that distinction, not the pixels
    if (kIndex_8_SkColorType == ct) {
        hasAlpha = false;   // we store false in the table entries for kIndex8
    }

    static const struct {
        SkColorType             fColorType;
        bool                    fHasAlpha;
        transform_scanline_proc fProc;
    } gMap[] = {
        { kRGB_565_SkColorType,     false,  transform_scanline_565 },
        { kN32_SkColorType,         false,  transform_scanline_888 },
        { kN32_SkColorType,         true,   transform_scanline_8888 },
        { kARGB_4444_SkColorType,   false,  transform_scanline_444 },
        { kARGB_4444_SkColorType,   true,   transform_scanline_4444 },
        { kIndex_8_SkColorType,     false,  transform_scanline_memcpy },
    };

    for (int i = SK_ARRAY_COUNT(gMap) - 1; i >= 0; --i) {
        if (gMap[i].fColorType == ct && gMap[i].fHasAlpha == hasAlpha) {
            return gMap[i].fProc;
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

/*  Pack palette[] with the corresponding colors, and if hasAlpha is true, also
    pack trans[] and return the number of trans[] entries written. If hasAlpha
    is false, the return value will always be 0.

    Note: this routine takes care of unpremultiplying the RGB values when we
    have alpha in the colortable, since png doesn't support premul colors
*/
static inline int pack_palette(SkColorTable* ctable,
                               png_color* SK_RESTRICT palette,
                               png_byte* SK_RESTRICT trans, bool hasAlpha) {
    const SkPMColor* SK_RESTRICT colors = ctable ? ctable->readColors() : nullptr;
    const int ctCount = ctable->count();
    int i, num_trans = 0;

    if (hasAlpha) {
        /*  first see if we have some number of fully opaque at the end of the
            ctable. PNG allows num_trans < num_palette, but all of the trans
            entries must come first in the palette. If I was smarter, I'd
            reorder the indices and ctable so that all non-opaque colors came
            first in the palette. But, since that would slow down the encode,
            I'm leaving the indices and ctable order as is, and just looking
            at the tail of the ctable for opaqueness.
        */
        num_trans = ctCount;
        for (i = ctCount - 1; i >= 0; --i) {
            if (SkGetPackedA32(colors[i]) != 0xFF) {
                break;
            }
            num_trans -= 1;
        }

        const SkUnPreMultiply::Scale* SK_RESTRICT table =
                                            SkUnPreMultiply::GetScaleTable();

        for (i = 0; i < num_trans; i++) {
            const SkPMColor c = *colors++;
            const unsigned a = SkGetPackedA32(c);
            const SkUnPreMultiply::Scale s = table[a];
            trans[i] = a;
            palette[i].red = SkUnPreMultiply::ApplyScale(s, SkGetPackedR32(c));
            palette[i].green = SkUnPreMultiply::ApplyScale(s,SkGetPackedG32(c));
            palette[i].blue = SkUnPreMultiply::ApplyScale(s, SkGetPackedB32(c));
        }
        // now fall out of this if-block to use common code for the trailing
        // opaque entries
    }

    // these (remaining) entries are opaque
    for (i = num_trans; i < ctCount; i++) {
        SkPMColor c = *colors++;
        palette[i].red = SkGetPackedR32(c);
        palette[i].green = SkGetPackedG32(c);
        palette[i].blue = SkGetPackedB32(c);
    }
    return num_trans;
}

class SkPNGImageEncoder : public SkImageEncoder {
protected:
    bool onEncode(SkWStream* stream, const SkBitmap& bm, int quality) override;
private:
    bool doEncode(SkWStream* stream, const SkBitmap& bm,
                  const bool& hasAlpha, int colorType,
                  int bitDepth, SkColorType ct,
                  png_color_8& sig_bit);

    typedef SkImageEncoder INHERITED;
};

bool SkPNGImageEncoder::onEncode(SkWStream* stream,
                                 const SkBitmap& originalBitmap,
                                 int /*quality*/) {
    SkBitmap copy;
    const SkBitmap* bitmap = &originalBitmap;
    switch (originalBitmap.colorType()) {
        case kIndex_8_SkColorType:
        case kN32_SkColorType:
        case kARGB_4444_SkColorType:
        case kRGB_565_SkColorType:
            break;
        default:
            // TODO(scroggo): support 8888-but-not-N32 natively.
            // TODO(scroggo): support kGray_8 directly.
            // TODO(scroggo): support Alpha_8 as Grayscale(black)+Alpha
            if (originalBitmap.copyTo(&copy, kN32_SkColorType)) {
                bitmap = &copy;
            }
    }
    SkColorType ct = bitmap->colorType();

    const bool hasAlpha = !bitmap->isOpaque();
    int colorType = PNG_COLOR_MASK_COLOR;
    int bitDepth = 8;   // default for color
    png_color_8 sig_bit;

    switch (ct) {
        case kIndex_8_SkColorType:
            colorType |= PNG_COLOR_MASK_PALETTE;
            // fall through to the ARGB_8888 case
        case kN32_SkColorType:
            sig_bit.red = 8;
            sig_bit.green = 8;
            sig_bit.blue = 8;
            sig_bit.alpha = 8;
            break;
        case kARGB_4444_SkColorType:
            sig_bit.red = 4;
            sig_bit.green = 4;
            sig_bit.blue = 4;
            sig_bit.alpha = 4;
            break;
        case kRGB_565_SkColorType:
            sig_bit.red = 5;
            sig_bit.green = 6;
            sig_bit.blue = 5;
            sig_bit.alpha = 0;
            break;
        default:
            return false;
    }

    if (hasAlpha) {
        // don't specify alpha if we're a palette, even if our ctable has alpha
        if (!(colorType & PNG_COLOR_MASK_PALETTE)) {
            colorType |= PNG_COLOR_MASK_ALPHA;
        }
    } else {
        sig_bit.alpha = 0;
    }

    SkAutoLockPixels alp(*bitmap);
    // readyToDraw checks for pixels (and colortable if that is required)
    if (!bitmap->readyToDraw()) {
        return false;
    }

    // we must do this after we have locked the pixels
    SkColorTable* ctable = bitmap->getColorTable();
    if (ctable) {
        if (ctable->count() == 0) {
            return false;
        }
        // check if we can store in fewer than 8 bits
        bitDepth = computeBitDepth(ctable->count());
    }

    return doEncode(stream, *bitmap, hasAlpha, colorType, bitDepth, ct, sig_bit);
}

bool SkPNGImageEncoder::doEncode(SkWStream* stream, const SkBitmap& bitmap,
                  const bool& hasAlpha, int colorType,
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
        SkColorTable* ct = bitmap.getColorTable();
        int numTrans = pack_palette(ct, paletteColors, trans, hasAlpha);
        png_set_PLTE(png_ptr, info_ptr, paletteColors, ct->count());
        if (numTrans > 0) {
            png_set_tRNS(png_ptr, info_ptr, trans, numTrans, nullptr);
        }
    }
#ifdef PNG_sBIT_SUPPORTED
    png_set_sBIT(png_ptr, info_ptr, &sig_bit);
#endif
    png_write_info(png_ptr, info_ptr);

    const char* srcImage = (const char*)bitmap.getPixels();
    SkAutoSTMalloc<1024, char> rowStorage(bitmap.width() << 2);
    char* storage = rowStorage.get();
    transform_scanline_proc proc = choose_proc(ct, hasAlpha);

    for (int y = 0; y < bitmap.height(); y++) {
        png_bytep row_ptr = (png_bytep)storage;
        proc(srcImage, bitmap.width(), storage);
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
    return (SkImageEncoder::kPNG_Type == t) ? new SkPNGImageEncoder : nullptr;
}

static SkImageEncoder_EncodeReg gEReg(sk_libpng_efactory);
