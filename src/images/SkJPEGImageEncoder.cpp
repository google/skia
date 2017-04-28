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
#include "SkImageInfoPriv.h"
#include "SkJpegEncoder.h"
#include "SkJPEGWriteUtility.h"
#include "SkStream.h"
#include "SkTemplates.h"

#include <stdio.h>

extern "C" {
    #include "jpeglib.h"
    #include "jerror.h"
}

// This warning triggers false postives way too often in here.
#if defined(__GNUC__) && !defined(__clang__)
    #pragma GCC diagnostic ignored "-Wclobbered"
#endif

/**
 *  Returns true if |info| is supported by the jpeg encoder and false otherwise.
 *  |jpegColorType| will be set to the proper libjpeg-turbo type for input to the library.
 *  |numComponents| will be set to the number of components in the |jpegColorType|.
 *  |proc|          will be set if we need to pre-convert the input before passing to
 *                  libjpeg-turbo.  Otherwise will be set to nullptr.
 */
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

class SkJpegEncoderMgr : SkNoncopyable {
public:

    /*
     * Create the decode manager
     * Does not take ownership of stream
     */
    static std::unique_ptr<SkJpegEncoderMgr> Make(SkWStream* stream) {
        return std::unique_ptr<SkJpegEncoderMgr>(new SkJpegEncoderMgr(stream));
    }

    /*
     * Initialize compress struct
     * Set the source manager
     */
    void init() {
        jpeg_create_compress(&fCInfo);
        fCInfo.dest = &fDstMgr;
        fInit = true;
    }

    jpeg_compress_struct* cinfo() { return &fCInfo; }

    jmp_buf& jmpBuf() { return fErrMgr.fJmpBuf; }

    ~SkJpegEncoderMgr() {
        if (fInit) {
            jpeg_destroy_compress(&fCInfo);
        }
    }

private:

    SkJpegEncoderMgr(SkWStream* stream)
        : fDstMgr(stream)
        , fInit(false)
    {
        fCInfo.err = jpeg_std_error(&fErrMgr);
        fErrMgr.error_exit = skjpeg_error_exit;
    }

    jpeg_compress_struct   fCInfo;
    skjpeg_error_mgr       fErrMgr;
    skjpeg_destination_mgr fDstMgr;
    bool                   fInit;
};

class SkJpegEncoder_Base : SkJpegEncoder {
private:
    SkJpegEncoder_Base(std::unique_ptr<SkJpegEncoderMgr> encoderMgr, const SkPixmap& src,
                       transform_scanline_proc proc);

    bool onEncodeRows(int numRows);

    std::unique_ptr<SkJpegEncoderMgr> fEncoderMgr;
    SkPixmap                          fSrc;
    transform_scanline_proc           fProc;
    int                               fCurrRow;
    SkAutoTMalloc<uint8_t>            fStorage;

    friend class SkJpegEncoder;
};

std::unique_ptr<SkJpegEncoder> SkJpegEncoder::Make(SkWStream* dst, const SkPixmap& src,
                                                   const Options& options) {
    if (!SkImageInfoIsValidAllowNumericalCS(src.info()) || !src.addr() ||
            src.rowBytes() < src.info().minRowBytes()) {
        return nullptr;
    }

    std::unique_ptr<SkJpegEncoderMgr> encoderMgr = SkJpegEncoderMgr::Make(dst);
    encoderMgr->init();
    if (setjmp(encoderMgr->jmpBuf())) {
        return nullptr;
    }

    J_COLOR_SPACE jpegColorType;
    int numComponents;
    transform_scanline_proc proc;
    if (!set_encode_config(&jpegColorType, &numComponents, &proc, src.info())) {
        return nullptr;
    }

    encoderMgr->cinfo()->image_width = src.width();
    encoderMgr->cinfo()->image_height = src.height();
    encoderMgr->cinfo()->in_color_space = jpegColorType;
    encoderMgr->cinfo()->input_components = numComponents;
    jpeg_set_defaults(encoderMgr->cinfo());

    // Tells libjpeg-turbo to compute optimal Huffman coding tables
    // for the image.  This improves compression at the cost of
    // slower encode performance.
    encoderMgr->cinfo()->optimize_coding = TRUE;

    jpeg_set_quality(encoderMgr->cinfo(), options.fQuality, TRUE);

    jpeg_start_compress(encoderMgr->cinfo(), TRUE);

    if (src.colorSpace()) {
        sk_sp<SkData> icc = icc_from_color_space(*src.colorSpace());
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

            jpeg_write_marker(encoderMgr->cinfo(), kICCMarker, markerData->bytes(),
                              markerData->size());
        }
    }

    return std::unique_ptr<SkJpegEncoder>(new SkJpegEncoder_Base(std::move(encoderMgr), src, proc));
}


SkJpegEncoder_Base::SkJpegEncoder_Base(std::unique_ptr<SkJpegEncoderMgr> encoderMgr,
                                       const SkPixmap& src, transform_scanline_proc proc)
    : fEncoderMgr(std::move(encoderMgr))
    , fSrc(src)
    , fProc(proc)
    , fCurrRow(0)
    , fStorage(proc ? fEncoderMgr->cinfo()->input_components*src.width() : 0)
{}

bool SkJpegEncoder::encodeRows(int numRows) {
    return ((SkJpegEncoder_Base*) this)->onEncodeRows(numRows);
}

bool SkJpegEncoder_Base::onEncodeRows(int numRows) {
    if (numRows <= 0 || fCurrRow == fSrc.height()) {
        return false;
    }

    SkASSERT(fCurrRow < fSrc.height());
    if (fCurrRow + numRows > fSrc.height()) {
        numRows = fSrc.height() - fCurrRow;
    }

    if (setjmp(fEncoderMgr->jmpBuf())) {
        SkASSERT(false);
        return false;
    }

    const void* srcRow = fSrc.addr(0, fCurrRow);
    const SkPMColor* colors = fSrc.ctable() ? fSrc.ctable()->readColors() : nullptr;
    for (int i = 0; i < numRows; i++) {
        JSAMPLE* jpegSrcRow = (JSAMPLE*) srcRow;
        if (fProc) {
            fProc((char*)fStorage.get(), (const char*)srcRow, fSrc.width(),
                  fEncoderMgr->cinfo()->input_components, colors);
            jpegSrcRow = fStorage.get();
        }

        jpeg_write_scanlines(fEncoderMgr->cinfo(), &jpegSrcRow, 1);
        srcRow = SkTAddOffset<const void>(srcRow, fSrc.rowBytes());
    }

    fCurrRow += numRows;
    if (fCurrRow == fSrc.height()) {
        jpeg_finish_compress(fEncoderMgr->cinfo());
    }

    return true;
}

bool SkJpegEncoder::Encode(SkWStream* dst, const SkPixmap& src, const Options& options) {
    auto encoder = SkJpegEncoder::Make(dst, src, options);
    return encoder && encoder->encodeRows(src.height());
}

bool SkEncodeImageAsJPEG(SkWStream* dst, const SkPixmap& src, int quality) {
    SkJpegEncoder::Options opts;
    opts.fQuality = quality;
    return SkJpegEncoder::Encode(dst, src, opts);
}

#endif
