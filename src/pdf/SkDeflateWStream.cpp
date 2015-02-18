/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDeflateWStream.h"
// https://skia.org/dev/contrib/style#no-define-before-sktypes

#ifndef SK_NO_FLATE

#include "zlib.h"

#define SKDEFLATEWSTREAM_INPUT_BUFFER_SIZE 4096
#define SKDEFLATEWSTREAM_OUTPUT_BUFFER_SIZE 4224  // 4096 + 128, usually big
                                                  // enough to always do a
                                                  // single loop.

// called by both write() and finalize()
static void do_deflate(int flush,
                       z_stream* zStream,
                       SkWStream* out,
                       unsigned char* inBuffer,
                       size_t inBufferSize) {
    zStream->next_in = inBuffer;
    zStream->avail_in = inBufferSize;
    unsigned char outBuffer[SKDEFLATEWSTREAM_OUTPUT_BUFFER_SIZE];
    SkDEBUGCODE(int returnValue;)
    do {
        zStream->next_out = outBuffer;
        zStream->avail_out = sizeof(outBuffer);
        SkDEBUGCODE(returnValue =) deflate(zStream, flush);
        SkASSERT(!zStream->msg);

        out->write(outBuffer, sizeof(outBuffer) - zStream->avail_out);
    } while (zStream->avail_in || !zStream->avail_out);
    SkASSERT(flush == Z_FINISH
                 ? returnValue == Z_STREAM_END
                 : returnValue == Z_OK);
}

// Hide all zlib impl details.
struct SkDeflateWStream::Impl {
    SkWStream* fOut;
    unsigned char fInBuffer[SKDEFLATEWSTREAM_INPUT_BUFFER_SIZE];
    size_t fInBufferIndex;
    z_stream fZStream;
};

SkDeflateWStream::SkDeflateWStream(SkWStream* out)
    : fImpl(SkNEW(SkDeflateWStream::Impl)) {
    fImpl->fOut = out;
    fImpl->fInBufferIndex = 0;
    if (!fImpl->fOut) {
        return;
    }
    fImpl->fZStream.zalloc = Z_NULL;
    fImpl->fZStream.zfree = Z_NULL;
    fImpl->fZStream.opaque = Z_NULL;
    fImpl->fZStream.data_type = Z_BINARY;
    SkDEBUGCODE(int r =) deflateInit(&fImpl->fZStream, Z_DEFAULT_COMPRESSION);
    SkASSERT(Z_OK == r);
}

SkDeflateWStream::~SkDeflateWStream() { this->finalize(); }

void SkDeflateWStream::finalize() {
    if (!fImpl->fOut) {
        return;
    }
    do_deflate(Z_FINISH, &fImpl->fZStream, fImpl->fOut, fImpl->fInBuffer,
               fImpl->fInBufferIndex);
    (void)deflateEnd(&fImpl->fZStream);
    fImpl->fOut = NULL;
}

bool SkDeflateWStream::write(const void* void_buffer, size_t len) {
    if (!fImpl->fOut) {
        return false;
    }
    const char* buffer = (const char*)void_buffer;
    while (len > 0) {
        size_t tocopy =
                SkTMin(len, sizeof(fImpl->fInBuffer) - fImpl->fInBufferIndex);
        memcpy(fImpl->fInBuffer + fImpl->fInBufferIndex, buffer, tocopy);
        len -= tocopy;
        buffer += tocopy;
        fImpl->fInBufferIndex += tocopy;
        SkASSERT(fImpl->fInBufferIndex <= sizeof(fImpl->fInBuffer));

        // if the buffer isn't filled, don't call into zlib yet.
        if (sizeof(fImpl->fInBuffer) == fImpl->fInBufferIndex) {
            do_deflate(Z_NO_FLUSH, &fImpl->fZStream, fImpl->fOut,
                       fImpl->fInBuffer, fImpl->fInBufferIndex);
            fImpl->fInBufferIndex = 0;
        }
    }
    return true;
}

size_t SkDeflateWStream::bytesWritten() const {
    return fImpl->fZStream.total_in + fImpl->fInBufferIndex;
}

#endif  // SK_NO_ZLIB
