/*
 * Copyright 2007 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImageEncoderPriv.h"

#ifdef SK_HAS_JPEG_LIBRARY

#include "SkColorPriv.h"
#include "SkImageEncoderFns.h"
#include "SkJPEGWriteUtility.h"
#include "SkStream.h"
#include "SkTemplates.h"

#include <stdio.h>

extern "C" {
    #include "jpeglib.h"
    #include "jerror.h"
}

/**
 *  Returns true if |info| is supported by the jpeg encoder and false otherwise.
 *  |jpegColorType| will be set to the proper libjpeg-turbo type for input to the library.
 *  |numComponents| will be set to the number of components in the |jpegColorType|.
 *  |proc|          will be set if we need to pre-convert the input before passing to
 *                  libjpeg-turbo.  Otherwise will be set to nullptr.
 */
// TODO (skbug.com/1501):
// Should we fail on non-opaque encodes?
// Or should we change alpha behavior (ex: unpremultiply when the input is premul)?
// Or is ignoring the alpha type and alpha channel ok here?
static bool set_encode_config(J_COLOR_SPACE* jpegColorType, int* numComponents,
                              transform_scanline_proc* proc, const SkImageInfo& info) {
    *proc = nullptr;
    switch (info.colorType()) {
        case kRGBA_8888_SkColorType:
            *jpegColorType = JCS_EXT_RGBA;
            *numComponents = 4;
            return true;
        case kBGRA_8888_SkColorType:
            *jpegColorType = JCS_EXT_BGRA;
            *numComponents = 4;
            return true;
        case kRGB_565_SkColorType:
            *proc = transform_scanline_565;
            *jpegColorType = JCS_RGB;
            *numComponents = 3;
            return true;
        case kARGB_4444_SkColorType:
            *proc = transform_scanline_444;
            *jpegColorType = JCS_RGB;
            *numComponents = 3;
            return true;
        case kIndex_8_SkColorType:
            *proc = transform_scanline_index8_opaque;
            *jpegColorType = JCS_RGB;
            *numComponents = 3;
            return true;
        case kGray_8_SkColorType:
            SkASSERT(info.isOpaque());
            *jpegColorType = JCS_GRAYSCALE;
            *numComponents = 1;
            return true;
        case kRGBA_F16_SkColorType:
            if (!info.colorSpace() || !info.colorSpace()->gammaIsLinear()) {
                return false;
            }

            *proc = transform_scanline_F16_to_8888;
            *jpegColorType = JCS_EXT_RGBA;
            *numComponents = 4;
            return true;
        default:
            return false;
    }


}

bool SkEncodeImageAsJPEG(SkWStream* stream, const SkPixmap& pixmap, const SkEncodeOptions& opts) {
    if (SkTransferFunctionBehavior::kRespect == opts.fUnpremulBehavior) {
        // Respecting the transfer function requries a color space.  It's not actually critical
        // in the jpeg case (since jpegs are opaque), but Skia color correct behavior generally
        // requires pixels to be tagged with color spaces.
        if (!pixmap.colorSpace() || (!pixmap.colorSpace()->gammaCloseToSRGB() &&
                                     !pixmap.colorSpace()->gammaIsLinear())) {
            return false;
        }
    }

    return SkEncodeImageAsJPEG(stream, pixmap, 100);
}

bool SkEncodeImageAsJPEG(SkWStream* stream, const SkPixmap& pixmap, int quality) {
    if (!pixmap.addr()) {
        return false;
    }
    jpeg_compress_struct    cinfo;
    skjpeg_error_mgr        sk_err;
    skjpeg_destination_mgr  sk_wstream(stream);

    // Declare before calling setjmp.
    SkAutoTMalloc<uint8_t>  storage;

    cinfo.err = jpeg_std_error(&sk_err);
    sk_err.error_exit = skjpeg_error_exit;
    if (setjmp(sk_err.fJmpBuf)) {
        return false;
    }

    J_COLOR_SPACE jpegColorSpace;
    int numComponents;
    transform_scanline_proc proc;
    if (!set_encode_config(&jpegColorSpace, &numComponents, &proc, pixmap.info())) {
        return false;
    }

    jpeg_create_compress(&cinfo);
    cinfo.dest = &sk_wstream;
    cinfo.image_width = pixmap.width();
    cinfo.image_height = pixmap.height();
    cinfo.input_components = numComponents;
    cinfo.in_color_space = jpegColorSpace;

    jpeg_set_defaults(&cinfo);

    // Tells libjpeg-turbo to compute optimal Huffman coding tables
    // for the image.  This improves compression at the cost of
    // slower encode performance.
    cinfo.optimize_coding = TRUE;
    jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);

    jpeg_start_compress(&cinfo, TRUE);

    if (pixmap.colorSpace()) {
        sk_sp<SkData> icc = icc_from_color_space(*pixmap.colorSpace());
        if (icc) {
            // Create a contiguous block of memory with the icc signature followed by the profile.
            sk_sp<SkData> markerData =
                    SkData::MakeUninitialized(kICCMarkerHeaderSize + icc->size());
            uint8_t* ptr = (uint8_t*) markerData->writable_data();
            memcpy(ptr, kICCSig, sizeof(kICCSig));
            ptr += sizeof(kICCSig);
            *ptr++ = 1; // This is the first marker.
            *ptr++ = 1; // Out of one total markers.
            memcpy(ptr, icc->data(), icc->size());

            jpeg_write_marker(&cinfo, kICCMarker, markerData->bytes(), markerData->size());
        }
    }

    if (proc) {
        storage.reset(numComponents * pixmap.width());
    }

    const void* srcRow = pixmap.addr();
    const SkPMColor* colors = pixmap.ctable() ? pixmap.ctable()->readColors() : nullptr;
    while (cinfo.next_scanline < cinfo.image_height) {
        JSAMPLE* jpegSrcRow = (JSAMPLE*) srcRow;
        if (proc) {
            proc((char*)storage.get(), (const char*)srcRow, pixmap.width(), numComponents, colors);
            jpegSrcRow = storage.get();
        }

        (void) jpeg_write_scanlines(&cinfo, &jpegSrcRow, 1);
        srcRow = SkTAddOffset<const void>(srcRow, pixmap.rowBytes());
    }

    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);

    return true;
}
#endif
