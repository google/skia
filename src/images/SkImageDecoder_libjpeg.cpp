/*
 * Copyright 2007, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "SkImageDecoder.h"
#include "SkImageEncoder.h"
#include "SkColorPriv.h"
#include "SkDither.h"
#include "SkScaledBitmapSampler.h"
#include "SkStream.h"
#include "SkTemplates.h"
#include "SkUtils.h"

#include <stdio.h>
extern "C" {
    #include "jpeglib.h"
    #include "jerror.h"
}

// this enables timing code to report milliseconds for an encode
//#define TIME_ENCODE
//#define TIME_DECODE

// this enables our rgb->yuv code, which is faster than libjpeg on ARM
// disable for the moment, as we have some glitches when width != multiple of 4
#define WE_CONVERT_TO_YUV

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class SkJPEGImageDecoder : public SkImageDecoder {
public:
    virtual Format getFormat() const {
        return kJPEG_Format;
    }

protected:
    virtual bool onDecode(SkStream* stream, SkBitmap* bm,
                          SkBitmap::Config pref, Mode);
};

//////////////////////////////////////////////////////////////////////////

#include "SkTime.h"

class AutoTimeMillis {
public:
    AutoTimeMillis(const char label[]) : fLabel(label) {
        if (!fLabel) {
            fLabel = "";
        }
        fNow = SkTime::GetMSecs();
    }
    ~AutoTimeMillis() {
        SkDebugf("---- Time (ms): %s %d\n", fLabel, SkTime::GetMSecs() - fNow);
    }
private:
    const char* fLabel;
    SkMSec      fNow;
};

/* our source struct for directing jpeg to our stream object
*/
struct sk_source_mgr : jpeg_source_mgr {
    sk_source_mgr(SkStream* stream, SkImageDecoder* decoder);

    SkStream*   fStream;
    const void* fMemoryBase;
    size_t      fMemoryBaseSize;
    SkImageDecoder* fDecoder;
    enum {
        kBufferSize = 1024
    };
    char    fBuffer[kBufferSize];
};

/* Automatically clean up after throwing an exception */
class JPEGAutoClean {
public:
    JPEGAutoClean(): cinfo_ptr(NULL) {}
    ~JPEGAutoClean() {
        if (cinfo_ptr) {
            jpeg_destroy_decompress(cinfo_ptr);
        }
    }
    void set(jpeg_decompress_struct* info) {
        cinfo_ptr = info;
    }
private:
    jpeg_decompress_struct* cinfo_ptr;
};

static void sk_init_source(j_decompress_ptr cinfo) {
    sk_source_mgr*  src = (sk_source_mgr*)cinfo->src;
    src->next_input_byte = (const JOCTET*)src->fBuffer;
    src->bytes_in_buffer = 0;
}

static boolean sk_fill_input_buffer(j_decompress_ptr cinfo) {
    sk_source_mgr* src = (sk_source_mgr*)cinfo->src;
    if (src->fDecoder != NULL && src->fDecoder->shouldCancelDecode()) {
        return FALSE;
    }
    size_t bytes = src->fStream->read(src->fBuffer, sk_source_mgr::kBufferSize);
    // note that JPEG is happy with less than the full read,
    // as long as the result is non-zero
    if (bytes == 0) {
        return FALSE;
    }

    src->next_input_byte = (const JOCTET*)src->fBuffer;
    src->bytes_in_buffer = bytes;
    return TRUE;
}

static void sk_skip_input_data(j_decompress_ptr cinfo, long num_bytes) {
    SkASSERT(num_bytes > 0);

    sk_source_mgr*  src = (sk_source_mgr*)cinfo->src;

    long bytesToSkip = num_bytes - src->bytes_in_buffer;

    // check if the skip amount exceeds the current buffer
    if (bytesToSkip > 0) {
        size_t bytes = src->fStream->skip(bytesToSkip);
        if (bytes != (size_t)bytesToSkip) {
//            SkDebugf("xxxxxxxxxxxxxx failure to skip request %d actual %d\n", bytesToSkip, bytes);
            cinfo->err->error_exit((j_common_ptr)cinfo);
        }
        src->next_input_byte = (const JOCTET*)src->fBuffer;
        src->bytes_in_buffer = 0;
    } else {
        src->next_input_byte += num_bytes;
        src->bytes_in_buffer -= num_bytes;
    }
}

static boolean sk_resync_to_restart(j_decompress_ptr cinfo, int desired) {
    sk_source_mgr*  src = (sk_source_mgr*)cinfo->src;

    // what is the desired param for???

    if (!src->fStream->rewind()) {
        SkDebugf("xxxxxxxxxxxxxx failure to rewind\n");
        cinfo->err->error_exit((j_common_ptr)cinfo);
        return FALSE;
    }
    src->next_input_byte = (const JOCTET*)src->fBuffer;
    src->bytes_in_buffer = 0;
    return TRUE;
}

static void sk_term_source(j_decompress_ptr /*cinfo*/) {}

///////////////////////////////////////////////////////////////////////////////

static void skmem_init_source(j_decompress_ptr cinfo) {
    sk_source_mgr*  src = (sk_source_mgr*)cinfo->src;
    src->next_input_byte = (const JOCTET*)src->fMemoryBase;
    src->bytes_in_buffer = src->fMemoryBaseSize;
}

static boolean skmem_fill_input_buffer(j_decompress_ptr cinfo) {
    SkDebugf("xxxxxxxxxxxxxx skmem_fill_input_buffer called\n");
    return FALSE;
}

static void skmem_skip_input_data(j_decompress_ptr cinfo, long num_bytes) {
    sk_source_mgr*  src = (sk_source_mgr*)cinfo->src;
//    SkDebugf("xxxxxxxxxxxxxx skmem_skip_input_data called %d\n", num_bytes);
    src->next_input_byte = (const JOCTET*)((const char*)src->next_input_byte + num_bytes);
    src->bytes_in_buffer -= num_bytes;
}

static boolean skmem_resync_to_restart(j_decompress_ptr cinfo, int desired) {
    SkDebugf("xxxxxxxxxxxxxx skmem_resync_to_restart called\n");
    return TRUE;
}

static void skmem_term_source(j_decompress_ptr /*cinfo*/) {}

///////////////////////////////////////////////////////////////////////////////

sk_source_mgr::sk_source_mgr(SkStream* stream, SkImageDecoder* decoder) : fStream(stream) {
    fDecoder = decoder;
    const void* baseAddr = stream->getMemoryBase();
    if (baseAddr && false) {
        fMemoryBase = baseAddr;
        fMemoryBaseSize = stream->getLength();
        
        init_source = skmem_init_source;
        fill_input_buffer = skmem_fill_input_buffer;
        skip_input_data = skmem_skip_input_data;
        resync_to_restart = skmem_resync_to_restart;
        term_source = skmem_term_source;
    } else {
        fMemoryBase = NULL;
        fMemoryBaseSize = 0;

        init_source = sk_init_source;
        fill_input_buffer = sk_fill_input_buffer;
        skip_input_data = sk_skip_input_data;
        resync_to_restart = sk_resync_to_restart;
        term_source = sk_term_source;
    }
//    SkDebugf("**************** use memorybase %p %d\n", fMemoryBase, fMemoryBaseSize);
}

#include <setjmp.h>

struct sk_error_mgr : jpeg_error_mgr {
    jmp_buf fJmpBuf;
};

static void sk_error_exit(j_common_ptr cinfo) {
    sk_error_mgr* error = (sk_error_mgr*)cinfo->err;

    (*error->output_message) (cinfo);

    /* Let the memory manager delete any temp files before we die */
    jpeg_destroy(cinfo);

    longjmp(error->fJmpBuf, -1);
}

///////////////////////////////////////////////////////////////////////////////

static bool skip_src_rows(jpeg_decompress_struct* cinfo, void* buffer,
                          int count) {
    for (int i = 0; i < count; i++) {
        JSAMPLE* rowptr = (JSAMPLE*)buffer;
        int row_count = jpeg_read_scanlines(cinfo, &rowptr, 1);
        if (row_count != 1) {
            return false;
        }
    }
    return true;
}

// This guy exists just to aid in debugging, as it allows debuggers to just
// set a break-point in one place to see all error exists.
static bool return_false(const jpeg_decompress_struct& cinfo,
                         const SkBitmap& bm, const char msg[]) {
#if 0
    SkDebugf("libjpeg error %d <%s> from %s [%d %d]", cinfo.err->msg_code,
             cinfo.err->jpeg_message_table[cinfo.err->msg_code], msg,
             bm.width(), bm.height());
#endif
    return false;   // must always return false
}

bool SkJPEGImageDecoder::onDecode(SkStream* stream, SkBitmap* bm,
                                  SkBitmap::Config prefConfig, Mode mode) {
#ifdef TIME_DECODE
    AutoTimeMillis atm("JPEG Decode");
#endif

    SkAutoMalloc  srcStorage;
    JPEGAutoClean autoClean;

    jpeg_decompress_struct  cinfo;
    sk_error_mgr            sk_err;
    sk_source_mgr           sk_stream(stream, this);

    cinfo.err = jpeg_std_error(&sk_err);
    sk_err.error_exit = sk_error_exit;

    // All objects need to be instantiated before this setjmp call so that
    // they will be cleaned up properly if an error occurs.
    if (setjmp(sk_err.fJmpBuf)) {
        return return_false(cinfo, *bm, "setjmp");
    }

    jpeg_create_decompress(&cinfo);
    autoClean.set(&cinfo);

    //jpeg_stdio_src(&cinfo, file);
    cinfo.src = &sk_stream;

    int status = jpeg_read_header(&cinfo, true);
    if (status != JPEG_HEADER_OK) {
        return return_false(cinfo, *bm, "read_header");
    }

    /*  Try to fulfill the requested sampleSize. Since jpeg can do it (when it
        can) much faster that we, just use their num/denom api to approximate
        the size.
    */
    int sampleSize = this->getSampleSize();

    cinfo.dct_method = JDCT_IFAST;
    cinfo.scale_num = 1;
    cinfo.scale_denom = sampleSize;

    /* this gives about 30% performance improvement. In theory it may
       reduce the visual quality, in practice I'm not seeing a difference
     */
    cinfo.do_fancy_upsampling = 0;

    /* this gives another few percents */
    cinfo.do_block_smoothing = 0;

    /* default format is RGB */
    cinfo.out_color_space = JCS_RGB;

    SkBitmap::Config config = prefConfig;
    // if no user preference, see what the device recommends
    if (config == SkBitmap::kNo_Config)
        config = SkImageDecoder::GetDeviceConfig();

    // only these make sense for jpegs
    if (config != SkBitmap::kARGB_8888_Config &&
        config != SkBitmap::kARGB_4444_Config &&
        config != SkBitmap::kRGB_565_Config) {
        config = SkBitmap::kARGB_8888_Config;
    }

#ifdef ANDROID_RGB
    cinfo.dither_mode = JDITHER_NONE;
    if (config == SkBitmap::kARGB_8888_Config) {
        cinfo.out_color_space = JCS_RGBA_8888;
    } else if (config == SkBitmap::kRGB_565_Config) {
        if (sampleSize == 1) {
            // SkScaledBitmapSampler can't handle RGB_565 yet,
            // so don't even try.
            cinfo.out_color_space = JCS_RGB_565;
            if (this->getDitherImage()) {
                cinfo.dither_mode = JDITHER_ORDERED;
            }
        }
    }
#endif

    if (sampleSize == 1 && mode == SkImageDecoder::kDecodeBounds_Mode) {
        bm->setConfig(config, cinfo.image_width, cinfo.image_height);
        bm->setIsOpaque(true);
        return true;
    }

    /*  image_width and image_height are the original dimensions, available
        after jpeg_read_header(). To see the scaled dimensions, we have to call
        jpeg_start_decompress(), and then read output_width and output_height.
    */
    if (!jpeg_start_decompress(&cinfo)) {
        return return_false(cinfo, *bm, "start_decompress");
    }

    /*  If we need to better match the request, we might examine the image and
        output dimensions, and determine if the downsampling jpeg provided is
        not sufficient. If so, we can recompute a modified sampleSize value to
        make up the difference.

        To skip this additional scaling, just set sampleSize = 1; below.
    */
    sampleSize = sampleSize * cinfo.output_width / cinfo.image_width;


    // should we allow the Chooser (if present) to pick a config for us???
    if (!this->chooseFromOneChoice(config, cinfo.output_width,
                                   cinfo.output_height)) {
        return return_false(cinfo, *bm, "chooseFromOneChoice");
    }

#ifdef ANDROID_RGB
    /* short-circuit the SkScaledBitmapSampler when possible, as this gives
       a significant performance boost.
    */
    if (sampleSize == 1 &&
        ((config == SkBitmap::kARGB_8888_Config && 
                cinfo.out_color_space == JCS_RGBA_8888) ||
        (config == SkBitmap::kRGB_565_Config && 
                cinfo.out_color_space == JCS_RGB_565)))
    {
        bm->setConfig(config, cinfo.output_width, cinfo.output_height);
        bm->setIsOpaque(true);
        if (SkImageDecoder::kDecodeBounds_Mode == mode) {
            return true;
        }
        if (!this->allocPixelRef(bm, NULL)) {
            return return_false(cinfo, *bm, "allocPixelRef");
        }
        SkAutoLockPixels alp(*bm);
        JSAMPLE* rowptr = (JSAMPLE*)bm->getPixels();
        INT32 const bpr =  bm->rowBytes();
        
        while (cinfo.output_scanline < cinfo.output_height) {
            int row_count = jpeg_read_scanlines(&cinfo, &rowptr, 1);
            // if row_count == 0, then we didn't get a scanline, so abort.
            // if we supported partial images, we might return true in this case
            if (0 == row_count) {
                return return_false(cinfo, *bm, "read_scanlines");
            }
            if (this->shouldCancelDecode()) {
                return return_false(cinfo, *bm, "shouldCancelDecode");
            }
            rowptr += bpr;
        }
        jpeg_finish_decompress(&cinfo);
        return true;
    }
#endif
    
    // check for supported formats
    SkScaledBitmapSampler::SrcConfig sc;
    if (3 == cinfo.out_color_components && JCS_RGB == cinfo.out_color_space) {
        sc = SkScaledBitmapSampler::kRGB;
#ifdef ANDROID_RGB
    } else if (JCS_RGBA_8888 == cinfo.out_color_space) {
        sc = SkScaledBitmapSampler::kRGBX;
    //} else if (JCS_RGB_565 == cinfo.out_color_space) {
    //    sc = SkScaledBitmapSampler::kRGB_565;
#endif
    } else if (1 == cinfo.out_color_components &&
               JCS_GRAYSCALE == cinfo.out_color_space) {
        sc = SkScaledBitmapSampler::kGray;
    } else {
        return return_false(cinfo, *bm, "jpeg colorspace");
    }

    SkScaledBitmapSampler sampler(cinfo.output_width, cinfo.output_height,
                                  sampleSize);

    bm->setConfig(config, sampler.scaledWidth(), sampler.scaledHeight());
    // jpegs are always opauqe (i.e. have no per-pixel alpha)
    bm->setIsOpaque(true);

    if (SkImageDecoder::kDecodeBounds_Mode == mode) {
        return true;
    }
    if (!this->allocPixelRef(bm, NULL)) {
        return return_false(cinfo, *bm, "allocPixelRef");
    }

    SkAutoLockPixels alp(*bm);                          
    if (!sampler.begin(bm, sc, this->getDitherImage())) {
        return return_false(cinfo, *bm, "sampler.begin");
    }

    uint8_t* srcRow = (uint8_t*)srcStorage.alloc(cinfo.output_width * 4);

    //  Possibly skip initial rows [sampler.srcY0]
    if (!skip_src_rows(&cinfo, srcRow, sampler.srcY0())) {
        return return_false(cinfo, *bm, "skip rows");
    }

    // now loop through scanlines until y == bm->height() - 1
    for (int y = 0;; y++) {
        JSAMPLE* rowptr = (JSAMPLE*)srcRow;
        int row_count = jpeg_read_scanlines(&cinfo, &rowptr, 1);
        if (0 == row_count) {
            return return_false(cinfo, *bm, "read_scanlines");
        }
        if (this->shouldCancelDecode()) {
            return return_false(cinfo, *bm, "shouldCancelDecode");
        }
        
        sampler.next(srcRow);
        if (bm->height() - 1 == y) {
            // we're done
            break;
        }

        if (!skip_src_rows(&cinfo, srcRow, sampler.srcDY() - 1)) {
            return return_false(cinfo, *bm, "skip rows");
        }
    }

    // we formally skip the rest, so we don't get a complaint from libjpeg
    if (!skip_src_rows(&cinfo, srcRow,
                       cinfo.output_height - cinfo.output_scanline)) {
        return return_false(cinfo, *bm, "skip rows");
    }
    jpeg_finish_decompress(&cinfo);

//    SkDebugf("------------------- bm2 size %d [%d %d] %d\n", bm->getSize(), bm->width(), bm->height(), bm->config());
    return true;
}

///////////////////////////////////////////////////////////////////////////////

#include "SkColorPriv.h"

// taken from jcolor.c in libjpeg
#if 0   // 16bit - precise but slow
    #define CYR     19595   // 0.299
    #define CYG     38470   // 0.587
    #define CYB      7471   // 0.114

    #define CUR    -11059   // -0.16874
    #define CUG    -21709   // -0.33126
    #define CUB     32768   // 0.5

    #define CVR     32768   // 0.5
    #define CVG    -27439   // -0.41869
    #define CVB     -5329   // -0.08131

    #define CSHIFT  16
#else      // 8bit - fast, slightly less precise
    #define CYR     77    // 0.299
    #define CYG     150    // 0.587
    #define CYB      29    // 0.114

    #define CUR     -43    // -0.16874
    #define CUG    -85    // -0.33126
    #define CUB     128    // 0.5

    #define CVR      128   // 0.5
    #define CVG     -107   // -0.41869
    #define CVB      -21   // -0.08131

    #define CSHIFT  8
#endif

static void rgb2yuv_32(uint8_t dst[], SkPMColor c) {
    int r = SkGetPackedR32(c);
    int g = SkGetPackedG32(c);
    int b = SkGetPackedB32(c);

    int  y = ( CYR*r + CYG*g + CYB*b ) >> CSHIFT;
    int  u = ( CUR*r + CUG*g + CUB*b ) >> CSHIFT;
    int  v = ( CVR*r + CVG*g + CVB*b ) >> CSHIFT;

    dst[0] = SkToU8(y);
    dst[1] = SkToU8(u + 128);
    dst[2] = SkToU8(v + 128);
}

static void rgb2yuv_4444(uint8_t dst[], U16CPU c) {
    int r = SkGetPackedR4444(c);
    int g = SkGetPackedG4444(c);
    int b = SkGetPackedB4444(c);

    int  y = ( CYR*r + CYG*g + CYB*b ) >> (CSHIFT - 4);
    int  u = ( CUR*r + CUG*g + CUB*b ) >> (CSHIFT - 4);
    int  v = ( CVR*r + CVG*g + CVB*b ) >> (CSHIFT - 4);

    dst[0] = SkToU8(y);
    dst[1] = SkToU8(u + 128);
    dst[2] = SkToU8(v + 128);
}

static void rgb2yuv_16(uint8_t dst[], U16CPU c) {
    int r = SkGetPackedR16(c);
    int g = SkGetPackedG16(c);
    int b = SkGetPackedB16(c);

    int  y = ( 2*CYR*r + CYG*g + 2*CYB*b ) >> (CSHIFT - 2);
    int  u = ( 2*CUR*r + CUG*g + 2*CUB*b ) >> (CSHIFT - 2);
    int  v = ( 2*CVR*r + CVG*g + 2*CVB*b ) >> (CSHIFT - 2);

    dst[0] = SkToU8(y);
    dst[1] = SkToU8(u + 128);
    dst[2] = SkToU8(v + 128);
}

///////////////////////////////////////////////////////////////////////////////

typedef void (*WriteScanline)(uint8_t* SK_RESTRICT dst,
                              const void* SK_RESTRICT src, int width,
                              const SkPMColor* SK_RESTRICT ctable);

static void Write_32_YUV(uint8_t* SK_RESTRICT dst,
                         const void* SK_RESTRICT srcRow, int width,
                         const SkPMColor*) {
    const uint32_t* SK_RESTRICT src = (const uint32_t*)srcRow;
    while (--width >= 0) {
#ifdef WE_CONVERT_TO_YUV
        rgb2yuv_32(dst, *src++);
#else
        uint32_t c = *src++;
        dst[0] = SkGetPackedR32(c);
        dst[1] = SkGetPackedG32(c);
        dst[2] = SkGetPackedB32(c);
#endif
        dst += 3;
    }
}

static void Write_4444_YUV(uint8_t* SK_RESTRICT dst,
                           const void* SK_RESTRICT srcRow, int width,
                           const SkPMColor*) {
    const SkPMColor16* SK_RESTRICT src = (const SkPMColor16*)srcRow;
    while (--width >= 0) {
#ifdef WE_CONVERT_TO_YUV
        rgb2yuv_4444(dst, *src++);
#else
        SkPMColor16 c = *src++;
        dst[0] = SkPacked4444ToR32(c);
        dst[1] = SkPacked4444ToG32(c);
        dst[2] = SkPacked4444ToB32(c);
#endif
        dst += 3;
    }
}

static void Write_16_YUV(uint8_t* SK_RESTRICT dst,
                         const void* SK_RESTRICT srcRow, int width,
                         const SkPMColor*) {
    const uint16_t* SK_RESTRICT src = (const uint16_t*)srcRow;
    while (--width >= 0) {
#ifdef WE_CONVERT_TO_YUV
        rgb2yuv_16(dst, *src++);
#else
        uint16_t c = *src++;
        dst[0] = SkPacked16ToR32(c);
        dst[1] = SkPacked16ToG32(c);
        dst[2] = SkPacked16ToB32(c);
#endif
        dst += 3;
    }
}

static void Write_Index_YUV(uint8_t* SK_RESTRICT dst,
                            const void* SK_RESTRICT srcRow, int width,
                            const SkPMColor* SK_RESTRICT ctable) {
    const uint8_t* SK_RESTRICT src = (const uint8_t*)srcRow;
    while (--width >= 0) {
#ifdef WE_CONVERT_TO_YUV
        rgb2yuv_32(dst, ctable[*src++]);
#else
        uint32_t c = ctable[*src++];
        dst[0] = SkGetPackedR32(c);
        dst[1] = SkGetPackedG32(c);
        dst[2] = SkGetPackedB32(c);
#endif
        dst += 3;
    }
}

static WriteScanline ChooseWriter(const SkBitmap& bm) {
    switch (bm.config()) {
        case SkBitmap::kARGB_8888_Config:
            return Write_32_YUV;
        case SkBitmap::kRGB_565_Config:
            return Write_16_YUV;
        case SkBitmap::kARGB_4444_Config:
            return Write_4444_YUV;
        case SkBitmap::kIndex8_Config:
            return Write_Index_YUV;
        default:
            return NULL;
    }
}

struct sk_destination_mgr : jpeg_destination_mgr {
    sk_destination_mgr(SkWStream* stream);

    SkWStream*  fStream;

    enum {
        kBufferSize = 1024
    };
    uint8_t fBuffer[kBufferSize];
};

static void sk_init_destination(j_compress_ptr cinfo) {
    sk_destination_mgr* dest = (sk_destination_mgr*)cinfo->dest;

    dest->next_output_byte = dest->fBuffer;
    dest->free_in_buffer = sk_destination_mgr::kBufferSize;
}

static boolean sk_empty_output_buffer(j_compress_ptr cinfo) {
    sk_destination_mgr* dest = (sk_destination_mgr*)cinfo->dest;

//  if (!dest->fStream->write(dest->fBuffer, sk_destination_mgr::kBufferSize - dest->free_in_buffer))
    if (!dest->fStream->write(dest->fBuffer, sk_destination_mgr::kBufferSize)) {
        ERREXIT(cinfo, JERR_FILE_WRITE);
        return false;
    }

    dest->next_output_byte = dest->fBuffer;
    dest->free_in_buffer = sk_destination_mgr::kBufferSize;
    return TRUE;
}

static void sk_term_destination (j_compress_ptr cinfo) {
    sk_destination_mgr* dest = (sk_destination_mgr*)cinfo->dest;

    size_t size = sk_destination_mgr::kBufferSize - dest->free_in_buffer;
    if (size > 0) {
        if (!dest->fStream->write(dest->fBuffer, size)) {
            ERREXIT(cinfo, JERR_FILE_WRITE);
            return;
        }
    }
    dest->fStream->flush();
}

sk_destination_mgr::sk_destination_mgr(SkWStream* stream)
        : fStream(stream) {
    this->init_destination = sk_init_destination;
    this->empty_output_buffer = sk_empty_output_buffer;
    this->term_destination = sk_term_destination;
}

class SkJPEGImageEncoder : public SkImageEncoder {
protected:
    virtual bool onEncode(SkWStream* stream, const SkBitmap& bm, int quality) {
#ifdef TIME_ENCODE
        AutoTimeMillis atm("JPEG Encode");
#endif

        const WriteScanline writer = ChooseWriter(bm);
        if (NULL == writer) {
            return false;
        }

        SkAutoLockPixels alp(bm);
        if (NULL == bm.getPixels()) {
            return false;
        }

        jpeg_compress_struct    cinfo;
        sk_error_mgr            sk_err;
        sk_destination_mgr      sk_wstream(stream);

        // allocate these before set call setjmp
        SkAutoMalloc    oneRow;
        SkAutoLockColors ctLocker;

        cinfo.err = jpeg_std_error(&sk_err);
        sk_err.error_exit = sk_error_exit;
        if (setjmp(sk_err.fJmpBuf)) {
            return false;
        }
        jpeg_create_compress(&cinfo);

        cinfo.dest = &sk_wstream;
        cinfo.image_width = bm.width();
        cinfo.image_height = bm.height();
        cinfo.input_components = 3;
#ifdef WE_CONVERT_TO_YUV
        cinfo.in_color_space = JCS_YCbCr;
#else
        cinfo.in_color_space = JCS_RGB;
#endif
        cinfo.input_gamma = 1;

        jpeg_set_defaults(&cinfo);
        jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);
        cinfo.dct_method = JDCT_IFAST;

        jpeg_start_compress(&cinfo, TRUE);

        const int       width = bm.width();
        uint8_t*        oneRowP = (uint8_t*)oneRow.alloc(width * 3);

        const SkPMColor* colors = ctLocker.lockColors(bm);
        const void*      srcRow = bm.getPixels();

        while (cinfo.next_scanline < cinfo.image_height) {
            JSAMPROW row_pointer[1];    /* pointer to JSAMPLE row[s] */

            writer(oneRowP, srcRow, width, colors);
            row_pointer[0] = oneRowP;
            (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
            srcRow = (const void*)((const char*)srcRow + bm.rowBytes());
        }

        jpeg_finish_compress(&cinfo);
        jpeg_destroy_compress(&cinfo);

        return true;
    }
};

///////////////////////////////////////////////////////////////////////////////

#include "SkTRegistry.h"

static SkImageDecoder* DFactory(SkStream* stream) {
    static const char gHeader[] = { 0xFF, 0xD8, 0xFF };
    static const size_t HEADER_SIZE = sizeof(gHeader);

    char buffer[HEADER_SIZE];
    size_t len = stream->read(buffer, HEADER_SIZE);

    if (len != HEADER_SIZE) {
        return NULL;   // can't read enough
    }
    if (memcmp(buffer, gHeader, HEADER_SIZE)) {
        return NULL;
    }
    return SkNEW(SkJPEGImageDecoder);
}

static SkImageEncoder* EFactory(SkImageEncoder::Type t) {
    return (SkImageEncoder::kJPEG_Type == t) ? SkNEW(SkJPEGImageEncoder) : NULL;
}

static SkTRegistry<SkImageDecoder*, SkStream*> gDReg(DFactory);
static SkTRegistry<SkImageEncoder*, SkImageEncoder::Type> gEReg(EFactory);

