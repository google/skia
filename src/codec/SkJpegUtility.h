/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkJpegUtility_codec_DEFINED
#define SkJpegUtility_codec_DEFINED

#include "SkJpegPriv.h"
#include "SkStream.h"

#include <setjmp.h>
// stdio is needed for jpeglib
#include <stdio.h>

extern "C" {
    #include "jpeglib.h"
    #include "jerror.h"
}


/*
 * Error handling function
 */
void skjpeg_err_exit(j_common_ptr cinfo);

/*
 * Source handling struct for that allows libjpeg to use our stream object
 */
struct skjpeg_source_mgr : jpeg_source_mgr {
    skjpeg_source_mgr(SkStream* stream);
    static constexpr size_t kDefaultJpegBufferSize = 4*1024;
    static constexpr size_t kMaxJpegBufferSize = 512*1024;
    SkStream* fStream; // unowned
    size_t fBufferSize = 0;
    std::unique_ptr<char[]> fBuffer;
};

#endif
