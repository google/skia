/*
 * Copyright 2010 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkJpegUtility_DEFINED
#define SkJpegUtility_DEFINED

#include "include/core/SkTypes.h"

#include <cstdint>

extern "C" {
    // We need to include stdio.h before jpeg because jpeg does not include it, but uses FILE
    // See https://github.com/libjpeg-turbo/libjpeg-turbo/issues/17
    #include <stdio.h> // IWYU pragma: keep
    #include "jpeglib.h"  // NO_G3_REWRITE
}

class SkWStream;

void skjpeg_error_exit(j_common_ptr cinfo);

/////////////////////////////////////////////////////////////////////////////
/* Our destination struct for directing decompressed pixels to our stream
 * object.
 */
struct SK_SPI skjpeg_destination_mgr : jpeg_destination_mgr {
    skjpeg_destination_mgr(SkWStream* stream);

    SkWStream* const fStream;

    enum {
        kBufferSize = 1024
    };
    uint8_t fBuffer[kBufferSize];
};

#endif
