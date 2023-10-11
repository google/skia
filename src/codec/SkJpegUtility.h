/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkJpegUtility_codec_DEFINED
#define SkJpegUtility_codec_DEFINED

#include <cstdint>

extern "C" {
    // We need to include stdio.h before jpeg because jpeg does not include it, but uses FILE
    // See https://github.com/libjpeg-turbo/libjpeg-turbo/issues/17
    #include <stdio.h> // IWYU pragma: keep
    #include "jpeglib.h"  // NO_G3_REWRITE
}

class SkStream;

/*
 * Error handling function
 */
void skjpeg_err_exit(j_common_ptr cinfo);

/*
 * Source handling struct for that allows libjpeg to use our stream object
 */
struct skjpeg_source_mgr : jpeg_source_mgr {
    skjpeg_source_mgr(SkStream* stream);

    SkStream* fStream; // unowned
    enum {
        // TODO (msarett): Experiment with different buffer sizes.
        // This size was chosen because it matches SkImageDecoder.
        kBufferSize = 1024
    };
    uint8_t fBuffer[kBufferSize];
};

#endif
