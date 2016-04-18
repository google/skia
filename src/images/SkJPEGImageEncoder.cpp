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

typedef void (*WriteScanline)(uint8_t* SK_RESTRICT dst,
                              const void* SK_RESTRICT src, int width,
                              const SkPMColor* SK_RESTRICT ctable);

static void Write_32_RGB(uint8_t* SK_RESTRICT dst,
                         const void* SK_RESTRICT srcRow, int width,
                         const SkPMColor*) {
    const uint32_t* SK_RESTRICT src = (const uint32_t*)srcRow;
    while (--width >= 0) {
        uint32_t c = *src++;
        dst[0] = SkGetPackedR32(c);
        dst[1] = SkGetPackedG32(c);
        dst[2] = SkGetPackedB32(c);
        dst += 3;
    }
}

static void Write_4444_RGB(uint8_t* SK_RESTRICT dst,
                           const void* SK_RESTRICT srcRow, int width,
                           const SkPMColor*) {
    const SkPMColor16* SK_RESTRICT src = (const SkPMColor16*)srcRow;
    while (--width >= 0) {
        SkPMColor16 c = *src++;
        dst[0] = SkPacked4444ToR32(c);
        dst[1] = SkPacked4444ToG32(c);
        dst[2] = SkPacked4444ToB32(c);
        dst += 3;
    }
}

static void Write_16_RGB(uint8_t* SK_RESTRICT dst,
                         const void* SK_RESTRICT srcRow, int width,
                         const SkPMColor*) {
    const uint16_t* SK_RESTRICT src = (const uint16_t*)srcRow;
    while (--width >= 0) {
        uint16_t c = *src++;
        dst[0] = SkPacked16ToR32(c);
        dst[1] = SkPacked16ToG32(c);
        dst[2] = SkPacked16ToB32(c);
        dst += 3;
    }
}

static void Write_Index_RGB(uint8_t* SK_RESTRICT dst,
                            const void* SK_RESTRICT srcRow, int width,
                            const SkPMColor* SK_RESTRICT ctable) {
    const uint8_t* SK_RESTRICT src = (const uint8_t*)srcRow;
    while (--width >= 0) {
        uint32_t c = ctable[*src++];
        dst[0] = SkGetPackedR32(c);
        dst[1] = SkGetPackedG32(c);
        dst[2] = SkGetPackedB32(c);
        dst += 3;
    }
}

static WriteScanline ChooseWriter(const SkBitmap& bm) {
    switch (bm.colorType()) {
        case kN32_SkColorType:
            return Write_32_RGB;
        case kRGB_565_SkColorType:
            return Write_16_RGB;
        case kARGB_4444_SkColorType:
            return Write_4444_RGB;
        case kIndex_8_SkColorType:
            return Write_Index_RGB;
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
        
        // FIXME: Can we take advantage of other in_color_spaces in libjpeg-turbo?
        cinfo.in_color_space = JCS_RGB;

        // The gamma value is ignored by libjpeg-turbo.
        cinfo.input_gamma = 1;

        jpeg_set_defaults(&cinfo);
        
        // Tells libjpeg-turbo to compute optimal Huffman coding tables
        // for the image.  This improves compression at the cost of
        // slower encode performance.
        cinfo.optimize_coding = TRUE;
        jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);

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
