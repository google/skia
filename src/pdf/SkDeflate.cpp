/*
 * Copyright 2010 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDeflate.h"

#include "SkData.h"
#include "SkMakeUnique.h"
#include "SkMalloc.h"
#include "SkTo.h"
#include "SkTraceEvent.h"

#include "zlib.h"

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
    : fImpl(skstd::make_unique<SkDeflateWStream::Impl>()) {
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

// Hide all zlib impl details.
struct SkDeflateStream::Impl {
    static constexpr size_t kBufferSize = 0xffff;

    SkStream* fIn;
    uint8_t   fInBuffer[kBufferSize];
    size_t    fInBufferIndex;
    z_stream  fZStream;
};

SkDeflateStream::SkDeflateStream(SkStream* in)
    : fImpl(skstd::make_unique<SkDeflateStream::Impl>()) {
    fImpl->fIn = in;
    fImpl->fInBufferIndex = 0;

    fImpl->fZStream.next_in = nullptr;
    fImpl->fZStream.avail_in = 0;
    fImpl->fZStream.next_out = nullptr;
    fImpl->fZStream.avail_out = 0;
    fImpl->fZStream.zalloc = &skia_alloc_func;
    fImpl->fZStream.zfree = &skia_free_func;
    fImpl->fZStream.opaque = nullptr;

    // gzip hack
    SkAssertResult(inflateInit2(&fImpl->fZStream, 16 + MAX_WBITS) == Z_OK);
}

SkDeflateStream::~SkDeflateStream() {
    SkAssertResult(inflateEnd(&fImpl->fZStream) == Z_OK);
}

bool SkDeflateStream::isAtEnd() const {
    return fImpl->fIn ? fImpl->fIn->isAtEnd() : true;
}

size_t SkDeflateStream::read(void* buffer, size_t size) {
    if (!fImpl->fIn) {
        return 0;
    }

    fImpl->fZStream.next_out  = static_cast<uint8_t*>(buffer);
    fImpl->fZStream.avail_out = SkToUInt(size);

    const auto* buffer_end = fImpl->fZStream.next_out + size;
    while (fImpl->fZStream.next_out < buffer_end) {
        if (!fImpl->fZStream.avail_in) {
            fImpl->fZStream.next_in  = fImpl->fInBuffer;
            fImpl->fZStream.avail_in = SkToUInt(fImpl->fIn->read(fImpl->fInBuffer,
                                                                 Impl::kBufferSize));
            if (!fImpl->fZStream.avail_in) {
                break;
            }

        }

        const auto rc = inflate(&fImpl->fZStream, Z_NO_FLUSH);
        if (rc == Z_STREAM_END) {
            break;
        }

        if (rc != Z_OK) {
            SkDebugf("SkDeflateStream -- unexpected inflate() return code: %d\n", rc);
            fImpl->fIn = nullptr;
            return 0;
        }
    }

    return SkToSizeT(fImpl->fZStream.next_out - static_cast<uint8_t*>(buffer));
}
