/*
 * Copyright 2010 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkJpegUtility_DEFINED
#define SkJpegUtility_DEFINED

#include "include/core/SkStream.h"
#include "src/codec/SkJpegPriv.h"

extern "C" {
    #include "jpeglib.h"
    #include "jerror.h"
}

#include <setjmp.h>

void SK_API skjpeg_error_exit(j_common_ptr cinfo);

/////////////////////////////////////////////////////////////////////////////
/* Our destination struct for directing decompressed pixels to our stream
 * object.
 */
struct SK_API skjpeg_destination_mgr : jpeg_destination_mgr {
    skjpeg_destination_mgr(SkWStream* stream);

    SkWStream*  fStream;

    enum {
        kBufferSize = 1024
    };
    uint8_t fBuffer[kBufferSize];
};

#endif
