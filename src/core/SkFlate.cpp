
/*
 * Copyright 2010 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkData.h"
#include "SkFlate.h"
#include "SkStream.h"

#ifndef SK_ZLIB_INCLUDE
bool SkFlate::HaveFlate() { return false; }
bool SkFlate::Deflate(SkStream*, SkWStream*) { return false; }
bool SkFlate::Deflate(const void*, size_t, SkWStream*) { return false; }
bool SkFlate::Deflate(const SkData*, SkWStream*) { return false; }
bool SkFlate::Inflate(SkStream*, SkWStream*) { return false; }
#else

// static
bool SkFlate::HaveFlate() {
    return true;
}

namespace {

#include SK_ZLIB_INCLUDE

// static
const size_t kBufferSize = 1024;

bool doFlate(bool compress, SkStream* src, SkWStream* dst) {
    uint8_t inputBuffer[kBufferSize];
    uint8_t outputBuffer[kBufferSize];
    z_stream flateData;
    flateData.zalloc = NULL;
    flateData.zfree = NULL;
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
        flateData.avail_in = inputLength;
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
            flateData.avail_in = read;
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

#endif

