/*
 * Copyright 2010 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/pdf/SkDeflate.h"

#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkMalloc.h"
#include "include/private/base/SkTo.h"
#include "src/core/SkTraceEvent.h"

#include <algorithm>
#include <cstdint>
#include <cstring>

#include "zlib.h"  // NO_G3_REWRITE

namespace {

// Different zlib implementations use different T.
// We've seen size_t and unsigned.
template <typename T> void* skia_alloc_func(void*, T items, T size) {
    return sk_calloc_throw(SkToSizeT(items) * SkToSizeT(size));
}

void skia_free_func(void*, void* address) { sk_free(address); }

}  // namespace

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
    zStream->avail_in = SkToInt(inBufferSize);
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

SkDeflateWStream::SkDeflateWStream(SkWStream* out,
                                   int compressionLevel,
                                   bool gzip)
    : fImpl(std::make_unique<SkDeflateWStream::Impl>()) {

    // There has existed at some point at least one zlib implementation which thought it was being
    // clever by randomizing the compression level. This is actually not entirely incorrect, except
    // for the no-compression level which should always be deterministically pass-through.
    // Users should instead consider the zero compression level broken and handle it themselves.
    SkASSERT(compressionLevel != 0);

    fImpl->fOut = out;
    fImpl->fInBufferIndex = 0;
    if (!fImpl->fOut) {
        return;
    }
    fImpl->fZStream.next_in = nullptr;
    fImpl->fZStream.zalloc = &skia_alloc_func;
    fImpl->fZStream.zfree = &skia_free_func;
    fImpl->fZStream.opaque = nullptr;
    SkASSERT(compressionLevel <= 9 && compressionLevel >= -1);
    SkDEBUGCODE(int r =) deflateInit2(&fImpl->fZStream, compressionLevel,
                                      Z_DEFLATED, gzip ? 0x1F : 0x0F,
                                      8, Z_DEFAULT_STRATEGY);
    SkASSERT(Z_OK == r);
}

SkDeflateWStream::~SkDeflateWStream() { this->finalize(); }

void SkDeflateWStream::finalize() {
    TRACE_EVENT0("skia", TRACE_FUNC);
    if (!fImpl->fOut) {
        return;
    }
    do_deflate(Z_FINISH, &fImpl->fZStream, fImpl->fOut, fImpl->fInBuffer,
               fImpl->fInBufferIndex);
    (void)deflateEnd(&fImpl->fZStream);
    fImpl->fOut = nullptr;
}

bool SkDeflateWStream::write(const void* void_buffer, size_t len) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    if (!fImpl->fOut) {
        return false;
    }
    const char* buffer = (const char*)void_buffer;
    while (len > 0) {
        size_t tocopy =
                std::min(len, sizeof(fImpl->fInBuffer) - fImpl->fInBufferIndex);
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
