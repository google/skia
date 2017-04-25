/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkJpegUtility.h"

#include "SkCodecPriv.h"

/*
 * Initialize the source manager
 */
static void sk_init_source(j_decompress_ptr dinfo) {
    skjpeg_source_mgr* src = (skjpeg_source_mgr*) dinfo->src;
    src->next_input_byte = (const JOCTET*) src->fBuffer.get();
    src->bytes_in_buffer = 0;
}

/*
 * Fill the input buffer from the stream
 */
static boolean sk_fill_input_buffer(j_decompress_ptr dinfo) {
    skjpeg_source_mgr* src = (skjpeg_source_mgr*) dinfo->src;
    size_t bytes = src->fStream->read(src->fBuffer.get(), src->fBufferSize);

    // libjpeg is still happy with a less than full read, as long as the result is non-zero
    if (bytes == 0) {
        return false;
    }

    src->next_input_byte = (const JOCTET*) src->fBuffer.get();
    src->bytes_in_buffer = bytes;
    return true;
}

/*
 * Skip a certain number of bytes in the stream
 */
static void sk_skip_input_data(j_decompress_ptr dinfo, long numBytes) {
    skjpeg_source_mgr* src = (skjpeg_source_mgr*) dinfo->src;
    size_t bytes = (size_t) numBytes;

    if (bytes > src->bytes_in_buffer) {
        size_t bytesToSkip = bytes - src->bytes_in_buffer;
        if (bytesToSkip != src->fStream->skip(bytesToSkip)) {
            SkCodecPrintf("Failure to skip.\n");
            dinfo->err->error_exit((j_common_ptr) dinfo);
            return;
        }

        src->next_input_byte = (const JOCTET*) src->fBuffer.get();
        src->bytes_in_buffer = 0;
    } else {
        src->next_input_byte += numBytes;
        src->bytes_in_buffer -= numBytes;
    }
}

/*
 * We do not need to do anything to terminate our stream
 */
static void sk_term_source(j_decompress_ptr dinfo)
{
    // The current implementation of SkJpegCodec does not call
    // jpeg_finish_decompress(), so this function is never called.
    // If we want to modify this function to do something, we also
    // need to modify SkJpegCodec to call jpeg_finish_decompress().
}

/*
 * Constructor for the source manager that we provide to libjpeg
 * We provide skia implementations of all of the stream processing functions required by libjpeg
 */
skjpeg_source_mgr::skjpeg_source_mgr(SkStream* stream)
    : fStream(stream)
{
    /*  make the buffer size long enough to for libjpeg-turbo huffman
        decoding to by-pass costly end of bitstream check.
        Currently set minimum as 1KB and maximum as 512KB.
        stream without a length will get 1KB by default as well.
    */
#define SKJPEG_SRC_MAX_SIZE (512*1024)
    fBufferSize = stream->getLength();
    fBufferSize = ((fBufferSize>>10)+1)<<10;
    if (fBufferSize > SKJPEG_SRC_MAX_SIZE) {
        fBufferSize = SKJPEG_SRC_MAX_SIZE;
    }
    fBuffer.reset(new char[fBufferSize]);
    if (fBuffer.get() == nullptr) {
        fBufferSize = 0;
        sk_throw();
    }

    init_source = sk_init_source;
    fill_input_buffer = sk_fill_input_buffer;
    skip_input_data = sk_skip_input_data;
    resync_to_restart = jpeg_resync_to_restart;
    term_source = sk_term_source;
}

/*
 * Call longjmp to continue execution on an error
 */
void skjpeg_err_exit(j_common_ptr dinfo) {
    // Simply return to Skia client code
    // JpegDecoderMgr will take care of freeing memory
    skjpeg_error_mgr* error = (skjpeg_error_mgr*) dinfo->err;
    (*error->output_message) (dinfo);
    longjmp(error->fJmpBuf, 1);
}
