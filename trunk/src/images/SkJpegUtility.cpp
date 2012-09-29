
/*
 * Copyright 2010 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkJpegUtility.h"

/////////////////////////////////////////////////////////////////////
static void sk_init_source(j_decompress_ptr cinfo) {
    skjpeg_source_mgr*  src = (skjpeg_source_mgr*)cinfo->src;
    src->next_input_byte = (const JOCTET*)src->fBuffer;
    src->bytes_in_buffer = 0;
    src->fStream->rewind();
}

static boolean sk_fill_input_buffer(j_decompress_ptr cinfo) {
    skjpeg_source_mgr* src = (skjpeg_source_mgr*)cinfo->src;
    if (src->fDecoder != NULL && src->fDecoder->shouldCancelDecode()) {
        return FALSE;
    }
    size_t bytes = src->fStream->read(src->fBuffer, skjpeg_source_mgr::kBufferSize);
    // note that JPEG is happy with less than the full read,
    // as long as the result is non-zero
    if (bytes == 0) {
        return FALSE;
    }

    src->next_input_byte = (const JOCTET*)src->fBuffer;
    src->bytes_in_buffer = bytes;
    return TRUE;
}

static void sk_skip_input_data(j_decompress_ptr cinfo, long num_bytes) {
    skjpeg_source_mgr*  src = (skjpeg_source_mgr*)cinfo->src;

    if (num_bytes > (long)src->bytes_in_buffer) {
        long bytesToSkip = num_bytes - src->bytes_in_buffer;
        while (bytesToSkip > 0) {
            long bytes = (long)src->fStream->skip(bytesToSkip);
            if (bytes <= 0 || bytes > bytesToSkip) {
//              SkDebugf("xxxxxxxxxxxxxx failure to skip request %d returned %d\n", bytesToSkip, bytes);
                cinfo->err->error_exit((j_common_ptr)cinfo);
                return;
            }
            bytesToSkip -= bytes;
        }
        src->next_input_byte = (const JOCTET*)src->fBuffer;
        src->bytes_in_buffer = 0;
    } else {
        src->next_input_byte += num_bytes;
        src->bytes_in_buffer -= num_bytes;
    }
}

static boolean sk_resync_to_restart(j_decompress_ptr cinfo, int desired) {
    skjpeg_source_mgr*  src = (skjpeg_source_mgr*)cinfo->src;

    // what is the desired param for???

    if (!src->fStream->rewind()) {
        SkDebugf("xxxxxxxxxxxxxx failure to rewind\n");
        cinfo->err->error_exit((j_common_ptr)cinfo);
        return FALSE;
    }
    src->next_input_byte = (const JOCTET*)src->fBuffer;
    src->bytes_in_buffer = 0;
    return TRUE;
}

static void sk_term_source(j_decompress_ptr /*cinfo*/) {}


#if 0 // UNUSED
static void skmem_init_source(j_decompress_ptr cinfo) {
    skjpeg_source_mgr*  src = (skjpeg_source_mgr*)cinfo->src;
    src->next_input_byte = (const JOCTET*)src->fMemoryBase;
    src->bytes_in_buffer = src->fMemoryBaseSize;
}

static boolean skmem_fill_input_buffer(j_decompress_ptr cinfo) {
    SkDebugf("xxxxxxxxxxxxxx skmem_fill_input_buffer called\n");
    return FALSE;
}

static void skmem_skip_input_data(j_decompress_ptr cinfo, long num_bytes) {
    skjpeg_source_mgr*  src = (skjpeg_source_mgr*)cinfo->src;
//    SkDebugf("xxxxxxxxxxxxxx skmem_skip_input_data called %d\n", num_bytes);
    src->next_input_byte = (const JOCTET*)((const char*)src->next_input_byte + num_bytes);
    src->bytes_in_buffer -= num_bytes;
}

static boolean skmem_resync_to_restart(j_decompress_ptr cinfo, int desired) {
    SkDebugf("xxxxxxxxxxxxxx skmem_resync_to_restart called\n");
    return TRUE;
}

static void skmem_term_source(j_decompress_ptr /*cinfo*/) {}
#endif


///////////////////////////////////////////////////////////////////////////////

skjpeg_source_mgr::skjpeg_source_mgr(SkStream* stream, SkImageDecoder* decoder,
                                     bool ownStream) : fStream(stream) {
    fDecoder = decoder;
    // const void* baseAddr = stream->getMemoryBase();
    fMemoryBase = NULL;
    fUnrefStream = ownStream;
    fMemoryBaseSize = 0;

    init_source = sk_init_source;
    fill_input_buffer = sk_fill_input_buffer;
    skip_input_data = sk_skip_input_data;
    resync_to_restart = sk_resync_to_restart;
    term_source = sk_term_source;
//    SkDebugf("**************** use memorybase %p %d\n", fMemoryBase, fMemoryBaseSize);
}

skjpeg_source_mgr::~skjpeg_source_mgr() {
    if (fMemoryBase) {
        sk_free(fMemoryBase);
    }
    if (fUnrefStream) {
        fStream->unref();
    }
}

///////////////////////////////////////////////////////////////////////////////

static void sk_init_destination(j_compress_ptr cinfo) {
    skjpeg_destination_mgr* dest = (skjpeg_destination_mgr*)cinfo->dest;

    dest->next_output_byte = dest->fBuffer;
    dest->free_in_buffer = skjpeg_destination_mgr::kBufferSize;
}

static boolean sk_empty_output_buffer(j_compress_ptr cinfo) {
    skjpeg_destination_mgr* dest = (skjpeg_destination_mgr*)cinfo->dest;

//  if (!dest->fStream->write(dest->fBuffer, skjpeg_destination_mgr::kBufferSize - dest->free_in_buffer))
    if (!dest->fStream->write(dest->fBuffer,
            skjpeg_destination_mgr::kBufferSize)) {
        ERREXIT(cinfo, JERR_FILE_WRITE);
        return false;
    }

    dest->next_output_byte = dest->fBuffer;
    dest->free_in_buffer = skjpeg_destination_mgr::kBufferSize;
    return TRUE;
}

static void sk_term_destination (j_compress_ptr cinfo) {
    skjpeg_destination_mgr* dest = (skjpeg_destination_mgr*)cinfo->dest;

    size_t size = skjpeg_destination_mgr::kBufferSize - dest->free_in_buffer;
    if (size > 0) {
        if (!dest->fStream->write(dest->fBuffer, size)) {
            ERREXIT(cinfo, JERR_FILE_WRITE);
            return;
        }
    }
    dest->fStream->flush();
}

skjpeg_destination_mgr::skjpeg_destination_mgr(SkWStream* stream)
        : fStream(stream) {
    this->init_destination = sk_init_destination;
    this->empty_output_buffer = sk_empty_output_buffer;
    this->term_destination = sk_term_destination;
}

void skjpeg_error_exit(j_common_ptr cinfo) {
    skjpeg_error_mgr* error = (skjpeg_error_mgr*)cinfo->err;

    (*error->output_message) (cinfo);

    /* Let the memory manager delete any temp files before we die */
    jpeg_destroy(cinfo);

    longjmp(error->fJmpBuf, -1);
}
