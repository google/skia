
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkImageDecoder.h"
#include "SkImageEncoder.h"
#include "SkColor.h"
#include "SkColorPriv.h"
#include "SkDither.h"
#include "SkMath.h"
#include "SkScaledBitmapSampler.h"
#include "SkStream.h"
#include "SkTemplates.h"
#include "SkUtils.h"

extern "C" {
#include "png.h"
}

class SkPNGImageDecoder : public SkImageDecoder {
public:
    virtual Format getFormat() const {
        return kPNG_Format;
    }
    
protected:
    virtual bool onDecode(SkStream* stream, SkBitmap* bm, Mode);
};

#ifndef png_jmpbuf
#  define png_jmpbuf(png_ptr) ((png_ptr)->jmpbuf)
#endif

#define PNG_BYTES_TO_CHECK 4

/* Automatically clean up after throwing an exception */
struct PNGAutoClean {
    PNGAutoClean(png_structp p, png_infop i): png_ptr(p), info_ptr(i) {}
    ~PNGAutoClean() {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    }
private:
    png_structp png_ptr;
    png_infop info_ptr;
};

static void sk_read_fn(png_structp png_ptr, png_bytep data, png_size_t length) {
    SkStream* sk_stream = (SkStream*)png_get_io_ptr(png_ptr);
    size_t bytes = sk_stream->read(data, length);
    if (bytes != length) {
        png_error(png_ptr, "Read Error!");
    }
}

static int sk_read_user_chunk(png_structp png_ptr, png_unknown_chunkp chunk) {
    SkImageDecoder::Peeker* peeker =
                    (SkImageDecoder::Peeker*)png_get_user_chunk_ptr(png_ptr);
    // peek() returning true means continue decoding
    return peeker->peek((const char*)chunk->name, chunk->data, chunk->size) ?
            1 : -1;
}

static void sk_error_fn(png_structp png_ptr, png_const_charp msg) {
#if 0
    SkDebugf("------ png error %s\n", msg);
#endif
    longjmp(png_jmpbuf(png_ptr), 1);
}

static void skip_src_rows(png_structp png_ptr, uint8_t storage[], int count) {
    for (int i = 0; i < count; i++) {
        uint8_t* tmp = storage;
        png_read_rows(png_ptr, &tmp, NULL, 1);
    }
}

static bool pos_le(int value, int max) {
    return value > 0 && value <= max;
}

static bool substituteTranspColor(SkBitmap* bm, SkPMColor match) {
    SkASSERT(bm->config() == SkBitmap::kARGB_8888_Config);
    
    bool reallyHasAlpha = false;

    for (int y = bm->height() - 1; y >= 0; --y) {
        SkPMColor* p = bm->getAddr32(0, y);
        for (int x = bm->width() - 1; x >= 0; --x) {
            if (match == *p) {
                *p = 0;
                reallyHasAlpha = true;
            }
            p += 1;
        }
    }
    return reallyHasAlpha;
}

static bool canUpscalePaletteToConfig(SkBitmap::Config dstConfig,
                                      bool srcHasAlpha) {
    switch (dstConfig) {
        case SkBitmap::kARGB_8888_Config:
        case SkBitmap::kARGB_4444_Config:
            return true;
        case SkBitmap::kRGB_565_Config:
            // only return true if the src is opaque (since 565 is opaque)
            return !srcHasAlpha;
        default:
            return false;
    }
}

// call only if color_type is PALETTE. Returns true if the ctable has alpha
static bool hasTransparencyInPalette(png_structp png_ptr, png_infop info_ptr) {
    png_bytep trans;
    int num_trans;

    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
        png_get_tRNS(png_ptr, info_ptr, &trans, &num_trans, NULL);
        return num_trans > 0;
    }
    return false;
}

bool SkPNGImageDecoder::onDecode(SkStream* sk_stream, SkBitmap* decodedBitmap,
                                 Mode mode) {
//    SkAutoTrace    apr("SkPNGImageDecoder::onDecode");

    /* Create and initialize the png_struct with the desired error handler
    * functions.  If you want to use the default stderr and longjump method,
    * you can supply NULL for the last three parameters.  We also supply the
    * the compiler header file version, so that we know if the application
    * was compiled with a compatible version of the library.  */
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
        NULL, sk_error_fn, NULL);
    //   png_voidp user_error_ptr, user_error_fn, user_warning_fn);
    if (png_ptr == NULL) {
        return false;
    }

    /* Allocate/initialize the memory for image information. */
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL) {
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        return false;
    }

    PNGAutoClean autoClean(png_ptr, info_ptr);

    /* Set error handling if you are using the setjmp/longjmp method (this is
    * the normal method of doing things with libpng).  REQUIRED unless you
    * set up your own error handlers in the png_create_read_struct() earlier.
    */
    if (setjmp(png_jmpbuf(png_ptr))) {
        return false;
    }

    /* If you are using replacement read functions, instead of calling
    * png_init_io() here you would call:
    */
    png_set_read_fn(png_ptr, (void *)sk_stream, sk_read_fn);
    /* where user_io_ptr is a structure you want available to the callbacks */
    /* If we have already read some of the signature */
//  png_set_sig_bytes(png_ptr, 0 /* sig_read */ );

    // hookup our peeker so we can see any user-chunks the caller may be interested in
    png_set_keep_unknown_chunks(png_ptr, PNG_HANDLE_CHUNK_ALWAYS, (png_byte*)"", 0);
    if (this->getPeeker()) {
        png_set_read_user_chunk_fn(png_ptr, (png_voidp)this->getPeeker(), sk_read_user_chunk);
    }

    /* The call to png_read_info() gives us all of the information from the
    * PNG file before the first IDAT (image data chunk). */
    png_read_info(png_ptr, info_ptr);
    png_uint_32 origWidth, origHeight;
    int bit_depth, color_type, interlace_type;
    png_get_IHDR(png_ptr, info_ptr, &origWidth, &origHeight, &bit_depth, &color_type,
        &interlace_type, NULL, NULL);

    /* tell libpng to strip 16 bit/color files down to 8 bits/color */
    if (bit_depth == 16) {
        png_set_strip_16(png_ptr);
    }
    /* Extract multiple pixels with bit depths of 1, 2, and 4 from a single
     * byte into separate bytes (useful for paletted and grayscale images). */
    if (bit_depth < 8) {
        png_set_packing(png_ptr);
    }
    /* Expand grayscale images to the full 8 bits from 1, 2, or 4 bits/pixel */
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) {
        png_set_expand_gray_1_2_4_to_8(png_ptr);
    }
    
    /* Make a grayscale image into RGB. */
    if (color_type == PNG_COLOR_TYPE_GRAY ||
        color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
        png_set_gray_to_rgb(png_ptr);
    }
        
    SkBitmap::Config    config;
    bool                hasAlpha = false;
    bool                doDither = this->getDitherImage();
    SkPMColor           theTranspColor = 0; // 0 tells us not to try to match
    
    // check for sBIT chunk data, in case we should disable dithering because
    // our data is not truely 8bits per component
    if (doDither) {
        png_color_8p sig_bit = NULL;
        bool has_sbit = PNG_INFO_sBIT == png_get_sBIT(png_ptr, info_ptr,
                                                      &sig_bit);
#if 0
        if (has_sbit) {
            SkDebugf("----- sBIT %d %d %d %d\n", sig_bit->red, sig_bit->green,
                     sig_bit->blue, sig_bit->alpha);
        }
#endif
        // 0 seems to indicate no information available
        if (has_sbit && pos_le(sig_bit->red, SK_R16_BITS) &&
                pos_le(sig_bit->green, SK_G16_BITS) &&
                pos_le(sig_bit->blue, SK_B16_BITS)) {
            doDither = false;
        }
    }
    
    if (color_type == PNG_COLOR_TYPE_PALETTE) {
        bool paletteHasAlpha = hasTransparencyInPalette(png_ptr, info_ptr);
        config = this->getPrefConfig(kIndex_SrcDepth, paletteHasAlpha);
        // now see if we can upscale to their requested config
        if (!canUpscalePaletteToConfig(config, paletteHasAlpha)) {
            config = SkBitmap::kIndex8_Config;
        }
    } else {
        png_color_16p   transpColor = NULL;
        int             numTransp = 0;
        
        png_get_tRNS(png_ptr, info_ptr, NULL, &numTransp, &transpColor);
        
        bool valid = png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS);
        
        if (valid && numTransp == 1 && transpColor != NULL) {
            /*  Compute our transparent color, which we'll match against later.
                We don't really handle 16bit components properly here, since we
                do our compare *after* the values have been knocked down to 8bit
                which means we will find more matches than we should. The real
                fix seems to be to see the actual 16bit components, do the
                compare, and then knock it down to 8bits ourselves.
            */
            if (color_type & PNG_COLOR_MASK_COLOR) {
                if (16 == bit_depth) {
                    theTranspColor = SkPackARGB32(0xFF, transpColor->red >> 8,
                              transpColor->green >> 8, transpColor->blue >> 8);
                } else {
                    theTranspColor = SkPackARGB32(0xFF, transpColor->red,
                                      transpColor->green, transpColor->blue);
                }
            } else {    // gray
                if (16 == bit_depth) {
                    theTranspColor = SkPackARGB32(0xFF, transpColor->gray >> 8,
                              transpColor->gray >> 8, transpColor->gray >> 8);
                } else {
                    theTranspColor = SkPackARGB32(0xFF, transpColor->gray,
                                          transpColor->gray, transpColor->gray);
                }
            }
        }

        if (valid ||
                PNG_COLOR_TYPE_RGB_ALPHA == color_type ||
                PNG_COLOR_TYPE_GRAY_ALPHA == color_type) {
            hasAlpha = true;
        }
        config = this->getPrefConfig(k32Bit_SrcDepth, hasAlpha);
        // now match the request against our capabilities
        if (hasAlpha) {
            if (config != SkBitmap::kARGB_4444_Config) {
                config = SkBitmap::kARGB_8888_Config;
            }
        } else {
            if (config != SkBitmap::kRGB_565_Config &&
                config != SkBitmap::kARGB_4444_Config) {
                config = SkBitmap::kARGB_8888_Config;
            }
        }
    }

    // sanity check for size
    {
        Sk64 size;
        size.setMul(origWidth, origHeight);
        if (size.isNeg() || !size.is32()) {
            return false;
        }
        // now check that if we are 4-bytes per pixel, we also don't overflow
        if (size.get32() > (0x7FFFFFFF >> 2)) {
            return false;
        }
    }

    if (!this->chooseFromOneChoice(config, origWidth, origHeight)) {
        return false;
    }
    
    const int sampleSize = this->getSampleSize();
    SkScaledBitmapSampler sampler(origWidth, origHeight, sampleSize);

    decodedBitmap->setConfig(config, sampler.scaledWidth(),
                             sampler.scaledHeight(), 0);
    if (SkImageDecoder::kDecodeBounds_Mode == mode) {
        return true;
    }
    
    // from here down we are concerned with colortables and pixels

    // we track if we actually see a non-opaque pixels, since sometimes a PNG sets its colortype
    // to |= PNG_COLOR_MASK_ALPHA, but all of its pixels are in fact opaque. We care, since we
    // draw lots faster if we can flag the bitmap has being opaque
    bool reallyHasAlpha = false;
    SkColorTable* colorTable = NULL;

    if (color_type == PNG_COLOR_TYPE_PALETTE) {
        int num_palette;
        png_colorp palette;
        png_bytep trans;
        int num_trans;

        png_get_PLTE(png_ptr, info_ptr, &palette, &num_palette);
        
        /*  BUGGY IMAGE WORKAROUND
            
            We hit some images (e.g. fruit_.png) who contain bytes that are == colortable_count
            which is a problem since we use the byte as an index. To work around this we grow
            the colortable by 1 (if its < 256) and duplicate the last color into that slot.
        */
        int colorCount = num_palette + (num_palette < 256);

        colorTable = SkNEW_ARGS(SkColorTable, (colorCount));

        SkPMColor* colorPtr = colorTable->lockColors();
        if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
            png_get_tRNS(png_ptr, info_ptr, &trans, &num_trans, NULL);
            hasAlpha = (num_trans > 0);
        } else {
            num_trans = 0;
            colorTable->setFlags(colorTable->getFlags() | SkColorTable::kColorsAreOpaque_Flag);
        }        
        // check for bad images that might make us crash
        if (num_trans > num_palette) {
            num_trans = num_palette;
        }

        int index = 0;
        int transLessThanFF = 0;

        for (; index < num_trans; index++) {
            transLessThanFF |= (int)*trans - 0xFF;
            *colorPtr++ = SkPreMultiplyARGB(*trans++, palette->red, palette->green, palette->blue);
            palette++;
        }
        reallyHasAlpha |= (transLessThanFF < 0);

        for (; index < num_palette; index++) {
            *colorPtr++ = SkPackARGB32(0xFF, palette->red, palette->green, palette->blue);
            palette++;
        }

        // see BUGGY IMAGE WORKAROUND comment above
        if (num_palette < 256) {
            *colorPtr = colorPtr[-1];
        }
        colorTable->unlockColors(true);
    }
    
    SkAutoUnref aur(colorTable);

    if (!this->allocPixelRef(decodedBitmap,
                             SkBitmap::kIndex8_Config == config ?
                                colorTable : NULL)) {
        return false;
    }
    
    SkAutoLockPixels alp(*decodedBitmap);

    /* swap the RGBA or GA data to ARGB or AG (or BGRA to ABGR) */
//  if (color_type == PNG_COLOR_TYPE_RGB_ALPHA)
//      ; // png_set_swap_alpha(png_ptr);

    /* swap bytes of 16 bit files to least significant byte first */
    //   png_set_swap(png_ptr);

    /* Add filler (or alpha) byte (before/after each RGB triplet) */
    if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_GRAY) {
        png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
    }

    /* Turn on interlace handling.  REQUIRED if you are not using
    * png_read_image().  To see how to handle interlacing passes,
    * see the png_read_row() method below:
    */
    const int number_passes = interlace_type != PNG_INTERLACE_NONE ? 
                        png_set_interlace_handling(png_ptr) : 1;

    /* Optional call to gamma correct and add the background to the palette
    * and update info structure.  REQUIRED if you are expecting libpng to
    * update the palette for you (ie you selected such a transform above).
    */
    png_read_update_info(png_ptr, info_ptr);

    if (SkBitmap::kIndex8_Config == config && 1 == sampleSize) {
        for (int i = 0; i < number_passes; i++) {
            for (png_uint_32 y = 0; y < origHeight; y++) {
                uint8_t* bmRow = decodedBitmap->getAddr8(0, y);
                png_read_rows(png_ptr, &bmRow, NULL, 1);
            }
        }
    } else {
        SkScaledBitmapSampler::SrcConfig sc;
        int srcBytesPerPixel = 4;
        
        if (colorTable != NULL) {
            sc = SkScaledBitmapSampler::kIndex;
            srcBytesPerPixel = 1;
        } else if (hasAlpha) {
            sc = SkScaledBitmapSampler::kRGBA;
        } else {
            sc = SkScaledBitmapSampler::kRGBX;
        }

        /*  We have to pass the colortable explicitly, since we may have one
            even if our decodedBitmap doesn't, due to the request that we
            upscale png's palette to a direct model
         */
        SkAutoLockColors ctLock(colorTable);
        if (!sampler.begin(decodedBitmap, sc, doDither, ctLock.colors())) {
            return false;
        }
        const int height = decodedBitmap->height();

        if (number_passes > 1) {
            SkAutoMalloc storage(origWidth * origHeight * srcBytesPerPixel);
            uint8_t* base = (uint8_t*)storage.get();
            size_t rb = origWidth * srcBytesPerPixel;

            for (int i = 0; i < number_passes; i++) {
                uint8_t* row = base;
                for (png_uint_32 y = 0; y < origHeight; y++) {
                    uint8_t* bmRow = row;
                    png_read_rows(png_ptr, &bmRow, NULL, 1);
                    row += rb;
                }
            }
            // now sample it
            base += sampler.srcY0() * rb;
            for (int y = 0; y < height; y++) {
                reallyHasAlpha |= sampler.next(base);
                base += sampler.srcDY() * rb;
            }
        } else {
            SkAutoMalloc storage(origWidth * srcBytesPerPixel);
            uint8_t* srcRow = (uint8_t*)storage.get();
            skip_src_rows(png_ptr, srcRow, sampler.srcY0());

            for (int y = 0; y < height; y++) {
                uint8_t* tmp = srcRow;
                png_read_rows(png_ptr, &tmp, NULL, 1);
                reallyHasAlpha |= sampler.next(srcRow);
                if (y < height - 1) {
                    skip_src_rows(png_ptr, srcRow, sampler.srcDY() - 1);
                }
            }

            // skip the rest of the rows (if any)
            png_uint_32 read = (height - 1) * sampler.srcDY() +
                               sampler.srcY0() + 1;
            SkASSERT(read <= origHeight);
            skip_src_rows(png_ptr, srcRow, origHeight - read);
        }
    }

    /* read rest of file, and get additional chunks in info_ptr - REQUIRED */
    png_read_end(png_ptr, info_ptr);

    if (0 != theTranspColor) {
        reallyHasAlpha |= substituteTranspColor(decodedBitmap, theTranspColor);
    }
    decodedBitmap->setIsOpaque(!reallyHasAlpha);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

#include "SkColorPriv.h"
#include "SkUnPreMultiply.h"

static void sk_write_fn(png_structp png_ptr, png_bytep data, png_size_t len) {
    SkWStream* sk_stream = (SkWStream*)png_get_io_ptr(png_ptr);
    if (!sk_stream->write(data, len)) {
        png_error(png_ptr, "sk_write_fn Error!");
    }
}

typedef void (*transform_scanline_proc)(const char* SK_RESTRICT src,
                                        int width, char* SK_RESTRICT dst);

static void transform_scanline_565(const char* SK_RESTRICT src, int width,
                                   char* SK_RESTRICT dst) {
    const uint16_t* SK_RESTRICT srcP = (const uint16_t*)src;    
    for (int i = 0; i < width; i++) {
        unsigned c = *srcP++;
        *dst++ = SkPacked16ToR32(c);
        *dst++ = SkPacked16ToG32(c);
        *dst++ = SkPacked16ToB32(c);
    }
}

static void transform_scanline_888(const char* SK_RESTRICT src, int width,
                                   char* SK_RESTRICT dst) {
    const SkPMColor* SK_RESTRICT srcP = (const SkPMColor*)src;    
    for (int i = 0; i < width; i++) {
        SkPMColor c = *srcP++;
        *dst++ = SkGetPackedR32(c);
        *dst++ = SkGetPackedG32(c);
        *dst++ = SkGetPackedB32(c);
    }
}

static void transform_scanline_444(const char* SK_RESTRICT src, int width,
                                   char* SK_RESTRICT dst) {
    const SkPMColor16* SK_RESTRICT srcP = (const SkPMColor16*)src;    
    for (int i = 0; i < width; i++) {
        SkPMColor16 c = *srcP++;
        *dst++ = SkPacked4444ToR32(c);
        *dst++ = SkPacked4444ToG32(c);
        *dst++ = SkPacked4444ToB32(c);
    }
}

static void transform_scanline_8888(const char* SK_RESTRICT src, int width,
                                    char* SK_RESTRICT dst) {
    const SkPMColor* SK_RESTRICT srcP = (const SkPMColor*)src;
    const SkUnPreMultiply::Scale* SK_RESTRICT table = 
                                              SkUnPreMultiply::GetScaleTable();

    for (int i = 0; i < width; i++) {
        SkPMColor c = *srcP++;
        unsigned a = SkGetPackedA32(c);
        unsigned r = SkGetPackedR32(c);
        unsigned g = SkGetPackedG32(c);
        unsigned b = SkGetPackedB32(c);

        if (0 != a && 255 != a) {
            SkUnPreMultiply::Scale scale = table[a];
            r = SkUnPreMultiply::ApplyScale(scale, r);
            g = SkUnPreMultiply::ApplyScale(scale, g);
            b = SkUnPreMultiply::ApplyScale(scale, b);
        }
        *dst++ = r;
        *dst++ = g;
        *dst++ = b;
        *dst++ = a;
    }
}

static void transform_scanline_4444(const char* SK_RESTRICT src, int width,
                                    char* SK_RESTRICT dst) {
    const SkPMColor16* SK_RESTRICT srcP = (const SkPMColor16*)src;
    const SkUnPreMultiply::Scale* SK_RESTRICT table = 
                                              SkUnPreMultiply::GetScaleTable();

    for (int i = 0; i < width; i++) {
        SkPMColor16 c = *srcP++;
        unsigned a = SkPacked4444ToA32(c);
        unsigned r = SkPacked4444ToR32(c);
        unsigned g = SkPacked4444ToG32(c);
        unsigned b = SkPacked4444ToB32(c);

        if (0 != a && 255 != a) {
            SkUnPreMultiply::Scale scale = table[a];
            r = SkUnPreMultiply::ApplyScale(scale, r);
            g = SkUnPreMultiply::ApplyScale(scale, g);
            b = SkUnPreMultiply::ApplyScale(scale, b);
        }
        *dst++ = r;
        *dst++ = g;
        *dst++ = b;
        *dst++ = a;
    }
}

static void transform_scanline_index8(const char* SK_RESTRICT src, int width,
                                      char* SK_RESTRICT dst) {
    memcpy(dst, src, width);
}

static transform_scanline_proc choose_proc(SkBitmap::Config config,
                                           bool hasAlpha) {
    // we don't care about search on alpha if we're kIndex8, since only the
    // colortable packing cares about that distinction, not the pixels
    if (SkBitmap::kIndex8_Config == config) {
        hasAlpha = false;   // we store false in the table entries for kIndex8
    }
    
    static const struct {
        SkBitmap::Config        fConfig;
        bool                    fHasAlpha;
        transform_scanline_proc fProc;
    } gMap[] = {
        { SkBitmap::kRGB_565_Config,    false,  transform_scanline_565 },
        { SkBitmap::kARGB_8888_Config,  false,  transform_scanline_888 },
        { SkBitmap::kARGB_8888_Config,  true,   transform_scanline_8888 },
        { SkBitmap::kARGB_4444_Config,  false,  transform_scanline_444 },
        { SkBitmap::kARGB_4444_Config,  true,   transform_scanline_4444 },
        { SkBitmap::kIndex8_Config,     false,   transform_scanline_index8 },
    };

    for (int i = SK_ARRAY_COUNT(gMap) - 1; i >= 0; --i) {
        if (gMap[i].fConfig == config && gMap[i].fHasAlpha == hasAlpha) {
            return gMap[i].fProc;
        }
    }
    sk_throw();
    return NULL;
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
    SkAutoLockColors alc(ctable);
    const SkPMColor* SK_RESTRICT colors = alc.colors();
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
    virtual bool onEncode(SkWStream* stream, const SkBitmap& bm, int quality);
private:
    bool doEncode(SkWStream* stream, const SkBitmap& bm,
                  const bool& hasAlpha, int colorType,
                  int bitDepth, SkBitmap::Config config,
                  png_color_8& sig_bit);
};

bool SkPNGImageEncoder::onEncode(SkWStream* stream, const SkBitmap& bitmap,
                                 int /*quality*/) {
    SkBitmap::Config config = bitmap.getConfig();

    const bool hasAlpha = !bitmap.isOpaque();
    int colorType = PNG_COLOR_MASK_COLOR;
    int bitDepth = 8;   // default for color
    png_color_8 sig_bit;

    switch (config) {
        case SkBitmap::kIndex8_Config:
            colorType |= PNG_COLOR_MASK_PALETTE;
            // fall through to the ARGB_8888 case
        case SkBitmap::kARGB_8888_Config:
            sig_bit.red = 8;
            sig_bit.green = 8;
            sig_bit.blue = 8;
            sig_bit.alpha = 8;
            break;
        case SkBitmap::kARGB_4444_Config:
            sig_bit.red = 4;
            sig_bit.green = 4;
            sig_bit.blue = 4;
            sig_bit.alpha = 4;
            break;
        case SkBitmap::kRGB_565_Config:
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
    
    SkAutoLockPixels alp(bitmap);
    // readyToDraw checks for pixels (and colortable if that is required)
    if (!bitmap.readyToDraw()) {
        return false;
    }

    // we must do this after we have locked the pixels
    SkColorTable* ctable = bitmap.getColorTable();
    if (NULL != ctable) {
        if (ctable->count() == 0) {
            return false;
        }
        // check if we can store in fewer than 8 bits
        bitDepth = computeBitDepth(ctable->count());
    }

    return doEncode(stream, bitmap, hasAlpha, colorType,
                    bitDepth, config, sig_bit);
}

bool SkPNGImageEncoder::doEncode(SkWStream* stream, const SkBitmap& bitmap,
                  const bool& hasAlpha, int colorType,
                  int bitDepth, SkBitmap::Config config,
                  png_color_8& sig_bit) {

    png_structp png_ptr;
    png_infop info_ptr;

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, sk_error_fn,
                                      NULL);
    if (NULL == png_ptr) {
        return false;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (NULL == info_ptr) {
        png_destroy_write_struct(&png_ptr,  NULL);
        return false;
    }

    /* Set error handling.  REQUIRED if you aren't supplying your own
    * error handling functions in the png_create_write_struct() call.
    */
    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        return false;
    }

    png_set_write_fn(png_ptr, (void*)stream, sk_write_fn, NULL);

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
    if (SkBitmap::kIndex8_Config == config) {
        SkColorTable* ct = bitmap.getColorTable();
        int numTrans = pack_palette(ct, paletteColors, trans, hasAlpha);
        png_set_PLTE(png_ptr, info_ptr, paletteColors, ct->count());
        if (numTrans > 0) {
            png_set_tRNS(png_ptr, info_ptr, trans, numTrans, NULL);
        }
    }

    png_set_sBIT(png_ptr, info_ptr, &sig_bit);
    png_write_info(png_ptr, info_ptr);

    const char* srcImage = (const char*)bitmap.getPixels();
    SkAutoSMalloc<1024> rowStorage(bitmap.width() << 2);
    char* storage = (char*)rowStorage.get();
    transform_scanline_proc proc = choose_proc(config, hasAlpha);

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

#include "SkTRegistry.h"

#ifdef SK_ENABLE_LIBPNG
    SkImageDecoder* sk_libpng_dfactory(SkStream*);
    SkImageEncoder* sk_libpng_efactory(SkImageEncoder::Type);
#endif

SkImageDecoder* sk_libpng_dfactory(SkStream* stream) {
    char buf[PNG_BYTES_TO_CHECK];
    if (stream->read(buf, PNG_BYTES_TO_CHECK) == PNG_BYTES_TO_CHECK &&
        !png_sig_cmp((png_bytep) buf, (png_size_t)0, PNG_BYTES_TO_CHECK)) {
        return SkNEW(SkPNGImageDecoder);
    }
    return NULL;
}

SkImageEncoder* sk_libpng_efactory(SkImageEncoder::Type t) {
    return (SkImageEncoder::kPNG_Type == t) ? SkNEW(SkPNGImageEncoder) : NULL;
}

static SkTRegistry<SkImageEncoder*, SkImageEncoder::Type> gEReg(sk_libpng_efactory);
static SkTRegistry<SkImageDecoder*, SkStream*> gDReg(sk_libpng_dfactory);
