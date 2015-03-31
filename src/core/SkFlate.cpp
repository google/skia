
/*
 * Copyright 2010 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkData.h"
#include "SkFlate.h"
#include "SkStream.h"

namespace {

#ifdef ZLIB_INCLUDE
    #include ZLIB_INCLUDE
#else
    #include "zlib.h"
#endif

// static
const size_t kBufferSize = 1024;

// Different zlib implementations use different T.
// We've seen size_t and unsigned.
template <typename T> void* skia_alloc_func(void*, T items, T size) {
    return sk_calloc_throw(SkToSizeT(items) * SkToSizeT(size));
}

static void skia_free_func(void*, void* address) { sk_free(address); }

bool doFlate(bool compress, SkStream* src, SkWStream* dst) {
    uint8_t inputBuffer[kBufferSize];
    uint8_t outputBuffer[kBufferSize];
    z_stream flateData;
    flateData.zalloc = &skia_alloc_func;
    flateData.zfree = &skia_free_func;
    flateData.opaque = NULL;
    flateData.next_in = NULL;
    flateData.avail_in = 0;
    flateData.next_out = outputBuffer;
    flateData.avail_out = kBufferSize;
    int rc;
    if (compress)
        rc = deflateInit(&flateData, Z_DEFAULT_COMPRESSION);
    else
        rc = inflateInit(&flateData);
    if (rc != Z_OK)
        return false;

    uint8_t* input = (uint8_t*)src->getMemoryBase();
    size_t inputLength = src->getLength();
    if (input == NULL || inputLength == 0) {
        input = NULL;
        flateData.next_in = inputBuffer;
        flateData.avail_in = 0;
    } else {
        flateData.next_in = input;
        flateData.avail_in = SkToUInt(inputLength);
    }

    rc = Z_OK;
    while (true) {
        if (flateData.avail_out < kBufferSize) {
            if (!dst->write(outputBuffer, kBufferSize - flateData.avail_out)) {
                rc = Z_BUF_ERROR;
                break;
            }
            flateData.next_out = outputBuffer;
            flateData.avail_out = kBufferSize;
        }
        if (rc != Z_OK)
            break;
        if (flateData.avail_in == 0) {
            if (input != NULL)
                break;
            size_t read = src->read(&inputBuffer, kBufferSize);
            if (read == 0)
                break;
            flateData.next_in = inputBuffer;
            flateData.avail_in = SkToUInt(read);
        }
        if (compress)
            rc = deflate(&flateData, Z_NO_FLUSH);
        else
            rc = inflate(&flateData, Z_NO_FLUSH);
    }
    while (rc == Z_OK) {
        if (compress)
            rc = deflate(&flateData, Z_FINISH);
        else
            rc = inflate(&flateData, Z_FINISH);
        if (flateData.avail_out < kBufferSize) {
            if (!dst->write(outputBuffer, kBufferSize - flateData.avail_out))
                return false;
            flateData.next_out = outputBuffer;
            flateData.avail_out = kBufferSize;
        }
    }

    if (compress)
        deflateEnd(&flateData);
    else
        inflateEnd(&flateData);
    if (rc == Z_STREAM_END)
        return true;
    return false;
}

}

// static
bool SkFlate::Deflate(SkStream* src, SkWStream* dst) {
    return doFlate(true, src, dst);
}

bool SkFlate::Deflate(const void* ptr, size_t len, SkWStream* dst) {
    SkMemoryStream stream(ptr, len);
    return doFlate(true, &stream, dst);
}

bool SkFlate::Deflate(const SkData* data, SkWStream* dst) {
    if (data) {
        SkMemoryStream stream(data->data(), data->size());
        return doFlate(true, &stream, dst);
    }
    return false;
}

// static
bool SkFlate::Inflate(SkStream* src, SkWStream* dst) {
    return doFlate(false, src, dst);
}


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

SkDeflateWStream::SkDeflateWStream(SkWStream* out)
    : fImpl(SkNEW(SkDeflateWStream::Impl)) {
    fImpl->fOut = out;
    fImpl->fInBufferIndex = 0;
    if (!fImpl->fOut) {
        return;
    }
    fImpl->fZStream.zalloc = &skia_alloc_func;
    fImpl->fZStream.zfree = &skia_free_func;
    fImpl->fZStream.opaque = NULL;
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
