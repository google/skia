/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkJpegUtility.h"

#include "SkCodecPriv.h"

/*
 * Initialize the buffered source manager
 */
static void sk_init_buffered_source(j_decompress_ptr dinfo) {
    skjpeg_buffered_source_mgr* src = (skjpeg_buffered_source_mgr*) dinfo->src;
    src->next_input_byte = (const JOCTET*) src->fBuffer.get();
    src->bytes_in_buffer = 0;
}

/*
 * Fill the input buffer from the stream
 */
static boolean sk_fill_buffered_input_buffer(j_decompress_ptr dinfo) {
    skjpeg_buffered_source_mgr* src = (skjpeg_buffered_source_mgr*) dinfo->src;
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
static void sk_skip_buffered_input_data(j_decompress_ptr dinfo, long numBytes) {
    skjpeg_buffered_source_mgr* src = (skjpeg_buffered_source_mgr*) dinfo->src;
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
skjpeg_buffered_source_mgr::skjpeg_buffered_source_mgr(SkStream* stream)
    : fStream(stream)
{
    /*  make the buffer size long enough to for libjpeg-turbo huffman
        decoding to by-pass costly end of bitstream check.
        Currently set maximum as 512KB.
        stream without a length will get 4KB by default as well.
    */
    fBufferSize = stream->getLength();
    if(fBufferSize == 0) {
        fBufferSize = kDefaultJpegBufferSize;
    } else if (fBufferSize > kMaxJpegBufferSize) {
        fBufferSize = kMaxJpegBufferSize;
    }

    fBuffer.reset(new char[fBufferSize]);

    init_source = sk_init_buffered_source;
    fill_input_buffer = sk_fill_buffered_input_buffer;
    skip_input_data = sk_skip_buffered_input_data;
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

/* memory backed source manager */

/*
 * Initialize the mem backed source manager
 */
static void sk_init_mem_source(j_decompress_ptr dinfo) {
}

static void sk_skip_mem_input_data (j_decompress_ptr cinfo, long num_bytes)
{
    struct skjpeg_mem_source_mgr *src = (struct skjpeg_mem_source_mgr*)cinfo->src;

    /* Just a dumb implementation for now.  Could use fseek() except
     * it doesn't work on pipes.  Not clear that being smart is worth
     * any trouble anyway --- large skips are infrequent.
     */
    if (num_bytes > 0) {
        while (num_bytes > (long) src->bytes_in_buffer) {
            num_bytes -= (long) src->bytes_in_buffer;
            (void) (*src->fill_input_buffer) (cinfo);
            /* note we assume that fill_input_buffer will never return FALSE,
             * so suspension need not be handled.
             */
        }
        src->next_input_byte += (size_t) num_bytes;
        src->bytes_in_buffer -= (size_t) num_bytes;
    }
}

static boolean sk_fill_mem_input_buffer (j_decompress_ptr cinfo)
{
    static const JOCTET mybuffer[4] = {
        (JOCTET) 0xFF, (JOCTET) JPEG_EOI, 0, 0
    };

    /* The whole JPEG data is expected to reside in the supplied memory
     * buffer, so any request for more data beyond the given buffer size
     * is treated as an error.
     */
    /* Insert a fake EOI marker */

    cinfo->src->next_input_byte = mybuffer;
    cinfo->src->bytes_in_buffer = 2;

    return true;
}

skjpeg_mem_source_mgr::skjpeg_mem_source_mgr(SkStream *stream)
{
    init_source = sk_init_mem_source;
    fill_input_buffer = sk_fill_mem_input_buffer;
    skip_input_data = sk_skip_mem_input_data;
    resync_to_restart = jpeg_resync_to_restart;
    term_source = sk_term_source;
    bytes_in_buffer = (size_t) stream->getLength();
    next_input_byte = (const JOCTET *)stream->getMemoryBase();
    //SkDebugf("skjpeg_mem_source_mgr %d bytes %p \n", bytes_in_buffer, next_input_byte);
}


