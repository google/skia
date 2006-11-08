/* libs/graphics/images/SkImageDecoder_libpng.cpp
**
** Copyright 2006, Google Inc.
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/

#include "SkImageDecoder.h"
#include "SkColor.h"
#include "SkColorPriv.h"
#include "SkMath.h"
#include "SkStream.h"
#include "SkTemplates.h"

class SkPNGImageDecoder : public SkImageDecoder {
protected:
    virtual bool onDecode(SkStream* stream, SkBitmap* bm, SkBitmap::Config pref);
};

#define TEST_DITHER

extern "C" {
#include "png.h"
}

static void convert_from_32_to_16(SkBitmap* bm)
{
    SkBitmap    tmp;
    unsigned    width = bm->width();
    unsigned    height = bm->height();

    tmp.setConfig(SkBitmap::kRGB_565_Config, width, height, 0);
    tmp.allocPixels();

    for (unsigned y = 0; y < height; y++) {
        const uint32_t* src = bm->getAddr32(0, y);
        uint16_t*       dst = tmp.getAddr16(0, y);
        for (unsigned x = 0; x < width; x++) {
            *dst++ = SkPixel32ToPixel16(*src++);
        }
    }
    bm->swap(tmp);
}

#ifndef png_jmpbuf
#  define png_jmpbuf(png_ptr) ((png_ptr)->jmpbuf)
#endif

#define PNG_BYTES_TO_CHECK 4

/* Automatically clean up after throwing an exception */
struct PNGAutoClean
{
    PNGAutoClean(png_structp p, png_infop i): png_ptr(p), info_ptr(i) {}
    ~PNGAutoClean()
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
    }
private:
    png_structp png_ptr;
    png_infop info_ptr;
};

SkImageDecoder* SkImageDecoder_PNG_Factory(SkStream* stream)
{
    char buf[PNG_BYTES_TO_CHECK];
    if (stream->read(buf, PNG_BYTES_TO_CHECK) == PNG_BYTES_TO_CHECK &&
        !png_sig_cmp((png_bytep) buf, (png_size_t)0, PNG_BYTES_TO_CHECK))
    {
        stream->rewind();
        return SkNEW(SkPNGImageDecoder);
    }
    return NULL;
}

static void sk_read_fn(png_structp png_ptr, png_bytep data, png_size_t length) {
    SkStream* sk_stream = (SkStream*) png_ptr->io_ptr;
    size_t bytes = sk_stream->read(data, length);
    if (bytes != length)
        png_error(png_ptr, "Read Error!");
}

static int sk_read_user_chunk(png_structp png_ptr, png_unknown_chunkp chunk)
{
    SkImageDecoder::Peeker* peeker = (SkImageDecoder::Peeker*)png_get_user_chunk_ptr(png_ptr);
    // peek() returning true means continue decoding
    return peeker->peek((const char*)chunk->name, chunk->data, chunk->size) ? 1 : -1;
}

// return true if the scanline has at least one alpha != 0xFF
typedef bool (*ConvertProc)(int y, int width, const uint8_t rgba[], SkBitmap* bm);

static bool Convert_RGB_To32(int y, int width, const uint8_t rgba[], SkBitmap* bm)
{
    uint32_t* dst = bm->getAddr32(0, y);
    uint32_t* stop = dst + width;

    while (dst < stop) {
        *dst++ = SkPackARGB32(0xFF, rgba[0], rgba[1], rgba[2]);
        rgba += 4;
    }
    return false;
}

static bool Convert_ARGB_To32(int y, int width, const uint8_t rgba[], SkBitmap* bm)
{
    uint32_t* dst = bm->getAddr32(0, y);
    uint32_t* stop = dst + width;
    bool      reallyHasAlpha = false;

    while (dst < stop) {
        unsigned alpha = rgba[3];
        *dst++ = SkPreMultiplyARGB(alpha, rgba[0], rgba[1], rgba[2]);
        rgba += 4;
        reallyHasAlpha |= (alpha != 0xFF);
    }
    return reallyHasAlpha;
}

static bool Convert_RGB_To16(int y, int width, const uint8_t rgba[], SkBitmap* bm)
{
    SkASSERT(width > 0);
    
    uint16_t* dst = bm->getAddr16(0, y);

    // do the first pixel (if we're out of phase)
    if (SkShouldDitherXY(0, y)) {
        *dst++ = SkDitherPack888ToRGB16(rgba[0], rgba[1], rgba[2]);
        rgba += 4;
        width -= 1;
    }
    
    int dcount = width >> 1;
    for (int i = 0; i < dcount; i++) {
        *dst++ = SkPack888ToRGB16(rgba[0], rgba[1], rgba[2]);
        *dst++ = SkDitherPack888ToRGB16(rgba[4], rgba[5], rgba[6]);
        rgba += 8;
    }
    
    // now do the last pixel (if any)
    if (width & 1) {
        *dst++ = SkPack888ToRGB16(rgba[0], rgba[1], rgba[2]);
    }
    return false;
}

static ConvertProc ChooseProc(SkBitmap::Config config, bool srcHasAlpha)
{
    if (SkBitmap::kARGB_8888_Config == config)
        return srcHasAlpha ? Convert_ARGB_To32 : Convert_RGB_To32;
    else if (SkBitmap::kRGB_565_Config == config) {
        SkASSERT(srcHasAlpha == false);
        return Convert_RGB_To16;
    }
    return NULL;
}

bool SkPNGImageDecoder::onDecode(SkStream* sk_stream, SkBitmap* bm, SkBitmap::Config prefConfig)
{
    /* Create and initialize the png_struct with the desired error handler
    * functions.  If you want to use the default stderr and longjump method,
    * you can supply NULL for the last three parameters.  We also supply the
    * the compiler header file version, so that we know if the application
    * was compiled with a compatible version of the library.  */
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
        NULL, NULL, NULL);
    //   png_voidp user_error_ptr, user_error_fn, user_warning_fn);
    if (png_ptr == NULL)
        return false; // error
    /* Allocate/initialize the memory for image information. */
    png_infop info_ptr = png_create_info_struct(png_ptr);

    if (info_ptr == NULL)
    {
        png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
        return false; // error
    }
    PNGAutoClean autoClean(png_ptr, info_ptr);
    /* Set error handling if you are using the setjmp/longjmp method (this is
    * the normal method of doing things with libpng).  REQUIRED unless you
    * set up your own error handlers in the png_create_read_struct() earlier.
    */
    if (setjmp(png_jmpbuf(png_ptr)))
    {
        /* If we get here, we had a problem reading the file */
        return false; // error
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
    png_uint_32 width, height;
    int bit_depth, color_type, interlace_type;
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
        &interlace_type, int_p_NULL, int_p_NULL);

    /* tell libpng to strip 16 bit/color files down to 8 bits/color */
    if (bit_depth == 16)
        png_set_strip_16(png_ptr);

    /* Extract multiple pixels with bit depths of 1, 2, and 4 from a single
    * byte into separate bytes (useful for paletted and grayscale images). */
    if (bit_depth < 8)
        png_set_packing(png_ptr);

    /* Expand grayscale images to the full 8 bits from 1, 2, or 4 bits/pixel */
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
        png_set_gray_1_2_4_to_8(png_ptr);

    /* Make a grayscale image into RGB. */
    if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
          png_set_gray_to_rgb(png_ptr);

    // record if the png claims to have alpha in one or more pixels (or palette entries)
    bool    hasAlpha = false;
    // we track if we actually see a non-opaque pixels, since sometimes a PNG sets its colortype
    // to |= PNG_COLOR_MASK_ALPHA, but all of its pixels are in fact opaque. We care, since we
    // draw lots faster if we can flag the bitmap has being opaque
    bool    reallyHasAlpha = false;

    if (color_type == PNG_COLOR_TYPE_PALETTE)
    {
        int num_palette;
        png_colorp palette;
        png_bytep trans;
        int num_trans;

        SkColorTable* colorTable = SkNEW(SkColorTable);
        SkAutoTDelete<SkColorTable> autoDel(colorTable);

        png_get_PLTE(png_ptr, info_ptr, &palette, &num_palette);

        /*  BUGGY IMAGE WORKAROUND

            We hit some images (e.g. fruit_.png) who contain bytes that are == colortable_count
            which is a problem since we use the byte as an index. To work around this we grow
            the colortable by 1 (if its < 256) and duplicate the last color into that slot.
        */
        colorTable->setColors(num_palette + (num_palette < 256));

        SkPMColor* colorPtr = colorTable->lockColors();
        if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
        {
            png_get_tRNS(png_ptr, info_ptr, &trans, &num_trans, NULL);
            hasAlpha = (num_trans > 0);
        }
        else
        {
            num_trans = 0;
            colorTable->setFlags(colorTable->getFlags() | SkColorTable::kColorsAreOpaque_Flag);
        }        
        if (num_trans > num_palette)    // check for bad images that might make us crash
            num_trans = num_palette;

        int index = 0;
        int transLessThanFF = 0;

        for (; index < num_trans; index++)
        {
            transLessThanFF |= (int)*trans - 0xFF;
            *colorPtr++ = SkPreMultiplyARGB(*trans++, palette->red, palette->green, palette->blue);
            palette++;
        }
        reallyHasAlpha |= (transLessThanFF < 0);

        for (; index < num_palette; index++)
        {
            *colorPtr++ = SkPackARGB32(0xFF, palette->red, palette->green, palette->blue);
            palette++;
        }

        // see BUGGY IMAGE WORKAROUND comment above
        if (num_palette < 256)
            *colorPtr = colorPtr[-1];

        colorTable->unlockColors(true);
        bm->setColorTable(colorTable)->unref();
        (void)autoDel.detach();
    }

    SkBitmap::Config    config;

    if (color_type == PNG_COLOR_TYPE_PALETTE)
        config = SkBitmap::kIndex8_Config;
    else
    {
        png_color_16p   transColor;
        
        if (png_get_tRNS(png_ptr, info_ptr, NULL, NULL, &transColor))
        {
            SkDEBUGF(("********************* png_get_tRNS [%d %d %d]\n", transColor->red, transColor->green, transColor->blue));
        }

        if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS) ||  color_type == PNG_COLOR_TYPE_RGB_ALPHA)
        {
            hasAlpha = true;
            config = SkBitmap::kARGB_8888_Config;
        }
        else    // we get to choose the config
        {
            config = prefConfig;
            if (config == SkBitmap::kNo_Config)
                config = SkImageDecoder::GetDeviceConfig();
            // only 565 or 8888 are "natural" if we have the choice
            if (config != SkBitmap::kRGB_565_Config)
                config = SkBitmap::kARGB_8888_Config;
        }
    }

    if (!this->chooseFromOneChoice(config, width, height))
        return false;

    bm->setConfig(config, width, height, 0);
    bm->allocPixels();

    /* swap the RGBA or GA data to ARGB or AG (or BGRA to ABGR) */
//  if (color_type == PNG_COLOR_TYPE_RGB_ALPHA)
//      ; // png_set_swap_alpha(png_ptr);

    /* swap bytes of 16 bit files to least significant byte first */
    //   png_set_swap(png_ptr);

    /* Add filler (or alpha) byte (before/after each RGB triplet) */
    if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_GRAY)
        png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);

    /* Turn on interlace handling.  REQUIRED if you are not using
    * png_read_image().  To see how to handle interlacing passes,
    * see the png_read_row() method below:
    */
    int number_passes = interlace_type != PNG_INTERLACE_NONE ? 
        png_set_interlace_handling(png_ptr) : 1;

    /* Optional call to gamma correct and add the background to the palette
    * and update info structure.  REQUIRED if you are expecting libpng to
    * update the palette for you (ie you selected such a transform above).
    */
    png_read_update_info(png_ptr, info_ptr);

    uint8_t* tmpStorage = NULL;
    uint8_t* storage = NULL;

    if (SkBitmap::kRGB_565_Config == config)
    {
        SkASSERT(!hasAlpha);
        tmpStorage = (uint8_t*)sk_malloc_throw(width * 4);
        storage = tmpStorage;
    }
    else if (SkBitmap::kARGB_8888_Config == config)
        storage = (uint8_t*)bm->getAddr32(0, height - 1);

    ConvertProc convert_proc = ChooseProc(config, hasAlpha);

    /* The other way to read images - deal with interlacing: */
    for (int pass = 0; pass < number_passes; pass++)
    {
        if (SkBitmap::kIndex8_Config == config) {
            for (png_uint_32 y = 0; y < height; y++)
            {
                uint8_t* bmRow = bm->getAddr8(0, y);
                png_read_rows(png_ptr, &bmRow, png_bytepp_NULL, 1);
            }
        }
        else {
            SkASSERT(convert_proc != NULL);
            for (png_uint_32 y = 0; y < height; y++)
            {
                uint8_t* tmp = storage;
                png_read_rows(png_ptr, &tmp, png_bytepp_NULL, 1);
                reallyHasAlpha |= convert_proc(y, width, storage, bm);
            }
        }
    }
    
    if (hasAlpha && !reallyHasAlpha) {
        SkDEBUGF(("Image doesn't really have alpha [%d %d]\n", width, height));
        if (config == SkBitmap::kARGB_8888_Config && prefConfig != SkBitmap::kARGB_8888_Config)
            convert_from_32_to_16(bm);
    }
    bm->setIsOpaque(!reallyHasAlpha);

    sk_free(tmpStorage);

    /* read rest of file, and get additional chunks in info_ptr - REQUIRED */
    png_read_end(png_ptr, info_ptr);

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef SK_SUPPORT_IMAGE_ENCODE

#include "SkColorPriv.h"

static void sk_error_fn(png_structp png_ptr, png_const_charp msg)
{
    SkDEBUGF(("SkPNGImageEncoder::onEncode error: %s\n", msg));
}

static void sk_write_fn(png_structp png_ptr, png_bytep data, png_size_t length)
{
    SkWStream* sk_stream = (SkWStream*)png_ptr->io_ptr;
    if (!sk_stream->write(data, length))
        png_error(png_ptr, "sk_write_fn Error!");
}

typedef void (*transform_scanline_proc)(const char src[], int width, char dst[]);

static void transform_scanline_565(const char src[], int width, char dst[])
{
    const uint16_t* srcP = (const uint16_t*)src;    
    for (int i = 0; i < width; i++)
    {
        unsigned c = *srcP++;
        *dst++ = SkPacked16ToR32(c);
        *dst++ = SkPacked16ToG32(c);
        *dst++ = SkPacked16ToB32(c);
    }
}

static void transform_scanline_888(const char src[], int width, char dst[])
{
    const SkPMColor* srcP = (const SkPMColor*)src;    
    for (int i = 0; i < width; i++)
    {
        SkPMColor c = *srcP++;
        *dst++ = SkGetPackedR32(c);
        *dst++ = SkGetPackedG32(c);
        *dst++ = SkGetPackedB32(c);
    }
}

static char unpremul(unsigned v, unsigned alpha)
{
    if (alpha == 0)
        return 0;

    if (alpha == 255)
        return v;

    return v * 255 / alpha;
}

static void transform_scanline_8888(const char src[], int width, char dst[])
{
    const SkPMColor* srcP = (const SkPMColor*)src;    
    for (int i = 0; i < width; i++)
    {
        SkPMColor c = *srcP++;
        unsigned alpha = SkGetPackedA32(c);
        
        *dst++ = unpremul(SkGetPackedR32(c), alpha);
        *dst++ = unpremul(SkGetPackedG32(c), alpha);
        *dst++ = unpremul(SkGetPackedB32(c), alpha);
        *dst++ = (char)alpha;
    }
}

static transform_scanline_proc choose_proc(SkBitmap::Config config, bool hasAlpha)
{
    static const struct {
        SkBitmap::Config        fConfig;
        bool                    fHasAlpha;
        transform_scanline_proc fProc;
    } gMap[] = {
        { SkBitmap::kRGB_565_Config,    false,  transform_scanline_565 },
        { SkBitmap::kARGB_8888_Config,  false,  transform_scanline_888 },
        { SkBitmap::kARGB_8888_Config,  true,   transform_scanline_8888 }
    };

    for (int i = SK_ARRAY_COUNT(gMap) - 1; i >= 0; --i)
        if (gMap[i].fConfig == config && gMap[i].fHasAlpha == hasAlpha)
            return gMap[i].fProc;

    sk_throw();
    return nil;
}

class SkPNGImageEncoder : public SkImageEncoder {
protected:
    virtual bool onEncode(SkWStream* stream, const SkBitmap& bm, int quality);
};

bool SkPNGImageEncoder::onEncode(SkWStream* stream, const SkBitmap& bitmap, int /*quality*/)
{
    SkBitmap::Config config = bitmap.getConfig();
    
    if (config != SkBitmap::kARGB_8888_Config &&
        config != SkBitmap::kRGB_565_Config)
    {
        SkDEBUGF(("SkPNGImageEncoder::onEncode can't encode %d config\n", config));
        return false;
    }

    png_structp png_ptr;
    png_infop info_ptr;

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, sk_error_fn, NULL);
    if (png_ptr == NULL)
        return false;

    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL)
    {
        png_destroy_write_struct(&png_ptr,  png_infopp_NULL);
        return false;
    }

    /* Set error handling.  REQUIRED if you aren't supplying your own
    * error handling functions in the png_create_write_struct() call.
    */
    if (setjmp(png_jmpbuf(png_ptr)))
    {
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
    
    bool hasAlpha = (config == SkBitmap::kARGB_8888_Config) && !bitmap.isOpaque();

    png_set_IHDR(png_ptr, info_ptr, bitmap.width(), bitmap.height(), 8,
                 hasAlpha ? PNG_COLOR_TYPE_RGB_ALPHA : PNG_COLOR_TYPE_RGB,
                 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

#if 0
    /* set the palette if there is one.  REQUIRED for indexed-color images */
    palette = (png_colorp)png_malloc(png_ptr, PNG_MAX_PALETTE_LENGTH
             * png_sizeof (png_color));
    /* ... set palette colors ... */
    png_set_PLTE(png_ptr, info_ptr, palette, PNG_MAX_PALETTE_LENGTH);
    /* You must not free palette here, because png_set_PLTE only makes a link to
      the palette that you malloced.  Wait until you are about to destroy
      the png structure. */
#endif

    {
        png_color_8 sig_bit;
        if (config == SkBitmap::kRGB_565_Config)
        {
            sig_bit.red = 5;
            sig_bit.green = 6;
            sig_bit.blue = 5;
        }
        else
        {
            sig_bit.red = 8;
            sig_bit.green = 8;
            sig_bit.blue = 8;
            if (hasAlpha)
                sig_bit.alpha = 8;
        }
        png_set_sBIT(png_ptr, info_ptr, &sig_bit);
    }

    png_write_info(png_ptr, info_ptr);

    const char*             srcImage = (const char*)bitmap.getPixels();
    char*                   storage = (char*)sk_malloc_throw(bitmap.width() << 2);
    transform_scanline_proc proc = choose_proc(config, hasAlpha);

    for (int y = 0; y < bitmap.height(); y++)
    {
        png_bytep row_ptr = (png_bytep)storage;
        proc(srcImage, bitmap.width(), storage);
        png_write_rows(png_ptr, &row_ptr, 1);
        srcImage += bitmap.rowBytes();
    }
    sk_free(storage);

    png_write_end(png_ptr, info_ptr);

#if 0
    /* If you png_malloced a palette, free it here (don't free info_ptr->palette,
      as recommended in versions 1.0.5m and earlier of this example; if
      libpng mallocs info_ptr->palette, libpng will free it).  If you
      allocated it with malloc() instead of png_malloc(), use free() instead
      of png_free(). */
    png_free(png_ptr, palette);
#endif

    /* clean up after the write, and free any memory allocated */
    png_destroy_write_struct(&png_ptr, &info_ptr);
    return true;
}

SkImageEncoder* sk_create_png_encoder();
SkImageEncoder* sk_create_png_encoder()
{
    return SkNEW(SkPNGImageEncoder);
}

#endif /* SK_SUPPORT_IMAGE_ENCODE */


