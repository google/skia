/*
 * Copyright 2007 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkImageEncoder.h"
#include "SkColorPriv.h"
#include "SkDither.h"
#include "SkStream.h"
#include "SkTemplates.h"
#include "SkTime.h"
#include "SkUtils.h"
#include "SkRTConf.h"
#include "SkRect.h"
#include "SkCanvas.h"


#include <stdio.h>
#include "SkJPEGWriteUtility.h"
extern "C" {
    #include "jpeglib.h"
    #include "jerror.h"
}

// These enable timing code that report milliseconds for an encoding
//#define TIME_ENCODE

// this enables our rgb->yuv code, which is faster than libjpeg on ARM
#define WE_CONVERT_TO_YUV

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
    switch (bm.colorType()) {
        case kN32_SkColorType:
            return Write_32_YUV;
        case kRGB_565_SkColorType:
            return Write_16_YUV;
        case kARGB_4444_SkColorType:
            return Write_4444_YUV;
        case kIndex_8_SkColorType:
            return Write_Index_YUV;
        default:
            return nullptr;
    }
}

class SkJPEGImageEncoder : public SkImageEncoder {
protected:
    virtual bool onEncode(SkWStream* stream, const SkBitmap& bm, int quality) {
#ifdef TIME_ENCODE
        SkAutoTime atm("JPEG Encode");
#endif

        SkAutoLockPixels alp(bm);
        if (nullptr == bm.getPixels()) {
            return false;
        }

        jpeg_compress_struct    cinfo;
        skjpeg_error_mgr        sk_err;
        skjpeg_destination_mgr  sk_wstream(stream);

        // allocate these before set call setjmp
        SkAutoTMalloc<uint8_t>  oneRow;

        cinfo.err = jpeg_std_error(&sk_err);
        sk_err.error_exit = skjpeg_error_exit;
        if (setjmp(sk_err.fJmpBuf)) {
            return false;
        }

        // Keep after setjmp or mark volatile.
        const WriteScanline writer = ChooseWriter(bm);
        if (nullptr == writer) {
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
        cinfo.optimize_coding = TRUE;
        jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);
#ifdef DCT_IFAST_SUPPORTED
        cinfo.dct_method = JDCT_IFAST;
#endif

        jpeg_start_compress(&cinfo, TRUE);

        const int       width = bm.width();
        uint8_t*        oneRowP = oneRow.reset(width * 3);

        const SkPMColor* colors = bm.getColorTable() ? bm.getColorTable()->readColors() : nullptr;
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
DEFINE_ENCODER_CREATOR(JPEGImageEncoder);
///////////////////////////////////////////////////////////////////////////////

static SkImageEncoder* sk_libjpeg_efactory(SkImageEncoder::Type t) {
    return (SkImageEncoder::kJPEG_Type == t) ? new SkJPEGImageEncoder : nullptr;
}

static SkImageEncoder_EncodeReg gEReg(sk_libjpeg_efactory);
