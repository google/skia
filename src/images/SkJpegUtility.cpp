
/*
 * Copyright 2010 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkJpegUtility.h"

// Uncomment to enable the code path used by the Android framework with their
// custom image decoders.
//#if defined(SK_BUILD_FOR_ANDROID) && defined(SK_DEBUG)
//  #define SK_BUILD_FOR_ANDROID_FRAMEWORK
//#endif

/////////////////////////////////////////////////////////////////////
static void sk_init_source(j_decompress_ptr cinfo) {
    skjpeg_source_mgr*  src = (skjpeg_source_mgr*)cinfo->src;
    src->next_input_byte = (const JOCTET*)src->fBuffer;
    src->bytes_in_buffer = 0;
#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    src->current_offset = 0;
#endif
    src->fStream->rewind();
}

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
static boolean sk_seek_input_data(j_decompress_ptr cinfo, long byte_offset) {
    skjpeg_source_mgr* src = (skjpeg_source_mgr*)cinfo->src;

    if (byte_offset > src->current_offset) {
        (void)src->fStream->skip(byte_offset - src->current_offset);
    } else {
        src->fStream->rewind();
        (void)src->fStream->skip(byte_offset);
    }

    src->current_offset = byte_offset;
    src->next_input_byte = (const JOCTET*)src->fBuffer;
    src->bytes_in_buffer = 0;
    return true;
}
#endif

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

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    src->current_offset += bytes;
#endif
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
#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
            src->current_offset += bytes;
#endif
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


///////////////////////////////////////////////////////////////////////////////

skjpeg_source_mgr::skjpeg_source_mgr(SkStream* stream, SkImageDecoder* decoder,
                                     bool ownStream) : fStream(stream) {
    fDecoder = decoder;
    fMemoryBase = NULL;
    fUnrefStream = ownStream;
    fMemoryBaseSize = 0;

    init_source = sk_init_source;
    fill_input_buffer = sk_fill_input_buffer;
    skip_input_data = sk_skip_input_data;
    resync_to_restart = sk_resync_to_restart;
    term_source = sk_term_source;
#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    seek_input_data = sk_seek_input_data;
#endif
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
