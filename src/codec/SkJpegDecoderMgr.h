/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkJpegDecoderMgr_DEFINED
#define SkJpegDecoderMgr_DEFINED

#include "SkCodec.h"
#include "SkCodecPriv.h"
#include <stdio.h>
#include "SkJpegUtility.h"

extern "C" {
    #include "jpeglib.h"
}

class JpegDecoderMgr : SkNoncopyable {
public:

    /*
     * Print a useful error message and return false
     */
    bool returnFalse(const char caller[]);

    /*
     * Print a useful error message and return a decode failure
     */
    SkCodec::Result returnFailure(const char caller[], SkCodec::Result result);

    /*
     * Create the decode manager
     * Does not take ownership of stream
     */
    JpegDecoderMgr(SkStream* stream);

    /*
     * Initialize decompress struct
     * Initialize the source manager
     */
    void  init();

    /*
     * Recommend a color type based on the encoded format
     */
    SkColorType getColorType();

    /*
     * Free memory used by the decode manager
     */
    ~JpegDecoderMgr();

    /*
     * Get the jump buffer in order to set an error return point
     */
    jmp_buf& getJmpBuf();

    /*
     * Get function for the decompress info struct
     */
    jpeg_decompress_struct* dinfo();

private:

    jpeg_decompress_struct fDInfo;
    skjpeg_source_mgr      fSrcMgr;
    skjpeg_error_mgr       fErrorMgr;
    bool                   fInit;
};

#endif
