/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * This file can parse information from a JPEG using libjpeg (which must be compiled in).
 */

#include "src/pdf/SkJpegInfo.h"

#include "include/codec/SkEncodedOrigin.h"
#include "include/core/SkSize.h"
#include "include/core/SkStream.h"
#include "include/core/SkTypes.h"
#include "include/private/SkEncodedInfo.h"
#include "include/private/base/SkTo.h"
#include "src/codec/SkJpegCodec.h"
#include "src/codec/SkJpegConstants.h"
#include "src/codec/SkJpegDecoderMgr.h"
#include "src/codec/SkJpegPriv.h"

#include <cstddef>
#include <csetjmp>

extern "C" {
    #include "jpeglib.h"  // NO_G3_REWRITE
}

bool SkGetJpegInfo(const void* data, size_t len,
                   SkISize* size,
                   SkEncodedInfo::Color* colorType,
                   SkEncodedOrigin* orientation) {
    if (!SkJpegCodec::IsJpeg(data, len)) {
        return false;
    }

    SkMemoryStream stream(data, len);
    JpegDecoderMgr decoderMgr(&stream);
    // libjpeg errors will be caught and reported here
    skjpeg_error_mgr::AutoPushJmpBuf jmp(decoderMgr.errorMgr());
    if (setjmp(jmp)) {
        return false;
    }
    decoderMgr.init();
    jpeg_decompress_struct* dinfo = decoderMgr.dinfo();
    jpeg_save_markers(dinfo, kExifMarker, 0xFFFF);
    jpeg_save_markers(dinfo, kICCMarker, 0xFFFF);
    jpeg_save_markers(dinfo, kMpfMarker, 0xFFFF);
    jpeg_save_markers(dinfo, kGainmapMarker, 0xFFFF);
    if (JPEG_HEADER_OK != jpeg_read_header(dinfo, true)) {
        return false;
    }
    SkEncodedInfo::Color encodedColorType;
    if (!decoderMgr.getEncodedColor(&encodedColorType)) {
        return false;  // Unable to interpret the color channels as colors.
    }
    if (colorType) {
        *colorType = encodedColorType;
    }
    if (orientation) {
        *orientation = SkJpegPriv::get_exif_orientation(dinfo);
    }
    if (size) {
        *size = {SkToS32(dinfo->image_width), SkToS32(dinfo->image_height)};
    }
    return true;
}
