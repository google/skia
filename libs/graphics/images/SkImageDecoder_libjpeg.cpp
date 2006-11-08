/* libs/graphics/images/SkImageDecoder_libjpeg.cpp
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
#include "SkColorPriv.h"
#include "SkStream.h"
#include "SkTemplates.h"

#include <stdio.h>
extern "C" {
    #include "jpeglib.h"
}

class SkJPEGImageDecoder : public SkImageDecoder {
protected:
    virtual bool onDecode(SkStream* stream, SkBitmap* bm, SkBitmap::Config pref);
};

SkImageDecoder* SkImageDecoder_JPEG_Factory(SkStream* stream) { 
    // !!! unimplemented; rely on PNG test first for now
    return SkNEW(SkJPEGImageDecoder);
}

//////////////////////////////////////////////////////////////////////////

/* our source struct for directing jpeg to our stream object
*/
struct sk_source_mgr : jpeg_source_mgr {
    sk_source_mgr(SkStream* stream);

    SkStream*   fStream;

    enum {
        kBufferSize = 1024
    };
    char    fBuffer[kBufferSize];
};

/* Automatically clean up after throwing an exception */
struct JPEGAutoClean
{
    JPEGAutoClean(jpeg_decompress_struct* info): cinfo_ptr(info) {}
    ~JPEGAutoClean()
    {
        jpeg_destroy_decompress(cinfo_ptr);
    }
private:
    jpeg_decompress_struct* cinfo_ptr;
};

static void sk_init_source(j_decompress_ptr cinfo)
{
    sk_source_mgr*  src = (sk_source_mgr*)cinfo->src;

    src->next_input_byte = (const JOCTET*)src->fBuffer;
    src->bytes_in_buffer = 0;
}

static boolean sk_fill_input_buffer(j_decompress_ptr cinfo)
{
    sk_source_mgr*  src = (sk_source_mgr*)cinfo->src;
    size_t          bytes = src->fStream->read(src->fBuffer, sk_source_mgr::kBufferSize);
    // note that JPEG is happy with less than the full read, as long as the result is non-zero
    if (bytes == 0)
        cinfo->err->error_exit((j_common_ptr)cinfo);

    src->next_input_byte = (const JOCTET*)src->fBuffer;
    src->bytes_in_buffer = bytes;
    return TRUE;
}

static void sk_skip_input_data(j_decompress_ptr cinfo, long num_bytes)
{
    SkASSERT(num_bytes > 0);

    sk_source_mgr*  src = (sk_source_mgr*)cinfo->src;

    long skip = num_bytes - src->bytes_in_buffer;

    if (skip > 0)
    {
        size_t bytes = src->fStream->read(nil, skip);
        if (bytes != (size_t)skip)
            cinfo->err->error_exit((j_common_ptr)cinfo);

        src->next_input_byte = (const JOCTET*)src->fBuffer;
        src->bytes_in_buffer = 0;
    }
    else
    {
        src->next_input_byte += num_bytes;
        src->bytes_in_buffer -= num_bytes;
    }
}

static boolean sk_resync_to_restart(j_decompress_ptr cinfo, int desired)
{
    sk_source_mgr*  src = (sk_source_mgr*)cinfo->src;

    // what is the desired param for???

    if (!src->fStream->rewind())
        cinfo->err->error_exit((j_common_ptr)cinfo);

    return TRUE;
}

static void sk_term_source(j_decompress_ptr /*cinfo*/)
{
    // nothing to do
}

sk_source_mgr::sk_source_mgr(SkStream* stream)
    : fStream(stream)
{
    init_source = sk_init_source;
    fill_input_buffer = sk_fill_input_buffer;
    skip_input_data = sk_skip_input_data;
    resync_to_restart = sk_resync_to_restart;
    term_source = sk_term_source;
}

#include <setjmp.h>

struct sk_error_mgr : jpeg_error_mgr {
    jmp_buf fJmpBuf;
};

static void sk_error_exit(j_common_ptr cinfo)
{
    sk_error_mgr* error = (sk_error_mgr*)cinfo->err;

    /* Always display the message */
    (*error->output_message) (cinfo);

    /* Let the memory manager delete any temp files before we die */
    jpeg_destroy(cinfo);

    longjmp(error->fJmpBuf, -1);
}

typedef void (*ConvertTo16Proc)(int y, int width, const uint8_t rgbTriple[], uint16_t dst[]);

#define DITHER_TO_16

static void ConvertTo16_Gray(int y, int width, const uint8_t rgbTriple[], uint16_t dst[])
{
#ifndef DITHER_TO_16
    uint16_t* stop = dst + width;

    while (dst < stop) {
        *dst++ = SkPack888ToRGB16(rgbTriple[0], rgbTriple[0], rgbTriple[0]);
        rgbTriple += 1;
    }
#else   // dither
    SkASSERT(width > 0);

    // do the first pixel (if we're out of phase)
    if (SkShouldDitherXY(0, y)) {
        *dst++ = SkDitherPack888ToRGB16(rgbTriple[0], rgbTriple[0], rgbTriple[0]);
        rgbTriple += 1;
        width -= 1;
    }
    
    int dcount = width >> 1;
    for (int i = 0; i < dcount; i++) {
        *dst++ = SkPack888ToRGB16(rgbTriple[0], rgbTriple[0], rgbTriple[0]);
        *dst++ = SkDitherPack888ToRGB16(rgbTriple[1], rgbTriple[1], rgbTriple[1]);
        rgbTriple += 2;
    }
    
    // now do the last pixel (if any)
    if (width & 1) {
        *dst++ = SkPack888ToRGB16(rgbTriple[0], rgbTriple[0], rgbTriple[0]);
    }
#endif
}

static void ConvertTo16_Color(int y, int width, const uint8_t rgbTriple[], uint16_t dst[])
{
#ifndef DITHER_TO_16
    uint16_t* stop = dst + width;

    while (dst < stop) {
        *dst++ = SkPack888ToRGB16(rgbTriple[0], rgbTriple[1], rgbTriple[2]);
        rgbTriple += 3;
    }
#else   // dither
    SkASSERT(width > 0);

    // do the first pixel (if we're out of phase)
    if (SkShouldDitherXY(0, y)) {
        *dst++ = SkDitherPack888ToRGB16(rgbTriple[0], rgbTriple[1], rgbTriple[2]);
        rgbTriple += 3;
        width -= 1;
    }
    
    int dcount = width >> 1;
    for (int i = 0; i < dcount; i++) {
        *dst++ = SkPack888ToRGB16(rgbTriple[0], rgbTriple[1], rgbTriple[2]);
        *dst++ = SkDitherPack888ToRGB16(rgbTriple[3], rgbTriple[4], rgbTriple[5]);
        rgbTriple += 6;
    }
    
    // now do the last pixel (if any)
    if (width & 1) {
        *dst = SkPack888ToRGB16(rgbTriple[0], rgbTriple[1], rgbTriple[2]);
    }
#endif
}

bool SkJPEGImageDecoder::onDecode(SkStream* stream, SkBitmap* bm, SkBitmap::Config prefConfig)
{
    jpeg_decompress_struct  cinfo;
    sk_error_mgr            sk_err;
    sk_source_mgr           sk_stream(stream);

    cinfo.err = jpeg_std_error(&sk_err);
    sk_err.error_exit = sk_error_exit;

    if (setjmp(sk_err.fJmpBuf))
        return false;

    jpeg_create_decompress(&cinfo);
    JPEGAutoClean autoClean(&cinfo);

    //jpeg_stdio_src(&cinfo, file);
    cinfo.src = &sk_stream;

    jpeg_read_header(&cinfo, true);

    // now we know the dimensions. can call jpeg_destroy() if we wish

    // check for supported formats
    bool isRGB; // as opposed to gray8
    if (3 == cinfo.num_components && JCS_RGB == cinfo.out_color_space)
        isRGB = true;
    else if (1 == cinfo.num_components && JCS_GRAYSCALE == cinfo.out_color_space)
        isRGB = false;  // could use Index8 config if we want...
    else {
        SkDEBUGF(("SkJPEGImageDecoder: unsupported jpeg colorspace %d with %d components\n",
                    cinfo.jpeg_color_space, cinfo.num_components));
        return false;
    }
    
    SkBitmap::Config config = prefConfig;
    // if no user preference, see what the device recommends
    if (config == SkBitmap::kNo_Config)
        config = SkImageDecoder::GetDeviceConfig();

    // only one of these two makes sense for jpegs
    if (config != SkBitmap::kARGB_8888_Config && config != SkBitmap::kRGB_565_Config)
        config = SkBitmap::kARGB_8888_Config;

    // <reed> should we allow the Chooser (if present) to pick a config for us???
    if (!this->chooseFromOneChoice(config, cinfo.image_width, cinfo.image_height))
        return false;

    bm->setConfig(config, cinfo.image_width, cinfo.image_height, 0);
    bm->allocPixels();
    bm->setIsOpaque(true);

    jpeg_start_decompress(&cinfo);

    SkASSERT(cinfo.output_width == (unsigned)bm->width());
    SkASSERT(cinfo.output_height == (unsigned)bm->height());

    int width = bm->width();

    if (config == SkBitmap::kARGB_8888_Config)
    {
        for (unsigned y = 0; y < cinfo.output_height; y++)
        {
            uint32_t* bmRow = bm->getAddr32(0, y);
            JSAMPLE* rowptr = (JSAMPLE*)bmRow;
            int      row_count = jpeg_read_scanlines(&cinfo, &rowptr, 1);
            SkASSERT(row_count == 1);

            // since we want to reuse bmRow for the RGB triples and the final ARGB quads
            // we need to expand backwards, so we don't overwrite our src

            uint32_t* dst = bmRow + width;

            if (isRGB) {
                const uint8_t*  src = rowptr + width * 3;
                while (dst > bmRow)
                {
                    src -= 3;
                    *--dst = SkPackARGB32(0xFF, src[0], src[1], src[2]);
                }
            }
            else {  // grayscale
                const uint8_t*  src = rowptr + width;
                while (dst > bmRow)
                {
                    src -= 1;
                    *--dst = SkPackARGB32(0xFF, src[0], src[0], src[0]);
                }
            }
        }
    }
    else    // convert to 16bit
    {
        SkAutoMalloc    srcStorage(width * 3);
        uint8_t*        srcRow = (uint8_t*)srcStorage.get();
        ConvertTo16Proc convert_row = isRGB ? ConvertTo16_Color : ConvertTo16_Gray;

        for (unsigned y = 0; y < cinfo.output_height; y++)
        {
            int row_count = jpeg_read_scanlines(&cinfo, &srcRow, 1);
            SkASSERT(row_count == 1);
            convert_row(y, width, srcRow, bm->getAddr16(0, y));
        }
    }

    jpeg_finish_decompress(&cinfo);

    return true;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifdef SK_SUPPORT_IMAGE_ENCODE

#include "SkColorPriv.h"

static void convert_to_rgb24(uint8_t dst[], const SkBitmap& bm, int y)
{
    int width = bm.width();

    switch (bm.getConfig()) {
    case SkBitmap::kARGB_8888_Config:
        {
            const uint32_t* src = bm.getAddr32(0, y);
            while (--width >= 0)
            {
                uint32_t c = *src++;
                dst[0] = SkGetPackedR32(c);
                dst[1] = SkGetPackedG32(c);
                dst[2] = SkGetPackedB32(c);
                dst += 3;
            }
        }
        break;
    case SkBitmap::kRGB_565_Config: // hmmm, I should encode 16bit jpegs, not expand to 24 bit (doh!)
        {
            const uint16_t* src = bm.getAddr16(0, y);
            while (--width >= 0)
            {
                uint16_t c = *src++;
                dst[0] = SkPacked16ToR32(c);
                dst[1] = SkPacked16ToG32(c);
                dst[2] = SkPacked16ToB32(c);
                dst += 3;
            }
        }
        break;
    default:
        SkASSERT(!"unknown bitmap format for jpeg encode");
    }
}

struct sk_destination_mgr : jpeg_destination_mgr
{
    sk_destination_mgr(SkWStream* stream);

    SkWStream*  fStream;

    enum {
        kBufferSize = 1024
    };
    uint8_t fBuffer[kBufferSize];
};

static void sk_init_destination(j_compress_ptr cinfo)
{
    sk_destination_mgr* dest = (sk_destination_mgr*)cinfo->dest;

    dest->next_output_byte = dest->fBuffer;
    dest->free_in_buffer = sk_destination_mgr::kBufferSize;
}

static boolean sk_empty_output_buffer(j_compress_ptr cinfo)
{
    sk_destination_mgr* dest = (sk_destination_mgr*)cinfo->dest;

//  if (!dest->fStream->write(dest->fBuffer, sk_destination_mgr::kBufferSize - dest->free_in_buffer))
    if (!dest->fStream->write(dest->fBuffer, sk_destination_mgr::kBufferSize))
        sk_throw();
    //  ERREXIT(cinfo, JERR_FILE_WRITE);

    dest->next_output_byte = dest->fBuffer;
    dest->free_in_buffer = sk_destination_mgr::kBufferSize;
    return TRUE;
}

static void sk_term_destination (j_compress_ptr cinfo)
{
    sk_destination_mgr* dest = (sk_destination_mgr*)cinfo->dest;

    size_t size = sk_destination_mgr::kBufferSize - dest->free_in_buffer;
    if (size > 0)
    {
        if (!dest->fStream->write(dest->fBuffer, size))
            sk_throw();
    }
    dest->fStream->flush();
}

sk_destination_mgr::sk_destination_mgr(SkWStream* stream)
    : fStream(stream)
{
    this->init_destination = sk_init_destination;
    this->empty_output_buffer = sk_empty_output_buffer;
    this->term_destination = sk_term_destination;
}


class SkJPEGImageEncoder : public SkImageEncoder {
protected:
    virtual bool onEncode(SkWStream* stream, const SkBitmap& bm, int quality)
    {
        jpeg_compress_struct    cinfo;
        sk_error_mgr            sk_err;
        sk_destination_mgr      sk_wstream(stream);

        cinfo.err = jpeg_std_error(&sk_err);
        sk_err.error_exit = sk_error_exit;
        if (setjmp(sk_err.fJmpBuf))
            return false;

        jpeg_create_compress(&cinfo);

        cinfo.dest = &sk_wstream;
        cinfo.image_width = bm.width();
        cinfo.image_height = bm.height();
        cinfo.input_components = 3;
        cinfo.in_color_space = JCS_RGB;

        jpeg_set_defaults(&cinfo);
        jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);
        jpeg_start_compress(&cinfo, TRUE);

        SkAutoMalloc    oneRow(bm.width() * 3);
        uint8_t*        oneRowP = (uint8_t*)oneRow.get();

        while (cinfo.next_scanline < cinfo.image_height)
        {
            JSAMPROW row_pointer[1];    /* pointer to JSAMPLE row[s] */

            convert_to_rgb24(oneRowP, bm, cinfo.next_scanline);
            row_pointer[0] = oneRowP;
            (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
        }

        jpeg_finish_compress(&cinfo);
        jpeg_destroy_compress(&cinfo);
        return true;
    }
};

SkImageEncoder* sk_create_jpeg_encoder();
SkImageEncoder* sk_create_jpeg_encoder()
{
    return SkNEW(SkJPEGImageEncoder);
}

#endif /* SK_SUPPORT_IMAGE_ENCODE */

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG

void SkImageDecoder::UnitTest()
{
    SkBitmap    bm;

    (void)SkImageDecoder::DecodeFile("logo.jpg", &bm);
}

#endif
