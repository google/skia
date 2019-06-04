// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "tools/Inflate.h"

#include "include/private/SkMalloc.h"
#include "include/private/SkTo.h"

#include "zlib.h"

namespace {

// Different zlib implementations use different T.
// We've seen size_t and unsigned.
template <typename T> void* skia_alloc_func(void*, T items, T size) {
    return sk_calloc_throw(SkToSizeT(items) * SkToSizeT(size));
}

void skia_free_func(void*, void* address) { sk_free(address); }
}  // namespace

std::unique_ptr<SkStreamAsset> InflateStream(SkStream* src, SkString* error) {
    SkDynamicMemoryWStream decompressedDynamicMemoryWStream;
    SkWStream* dst = &decompressedDynamicMemoryWStream;

    static const size_t kBufferSize = 1024;
    uint8_t inputBuffer[kBufferSize];
    uint8_t outputBuffer[kBufferSize];
    z_stream flateData;
    flateData.zalloc = &skia_alloc_func;
    flateData.zfree = &skia_free_func;
    flateData.opaque = nullptr;
    flateData.next_in = nullptr;
    flateData.avail_in = 0;
    flateData.next_out = outputBuffer;
    flateData.avail_out = kBufferSize;
    int rc;
    rc = inflateInit(&flateData);
    if (rc != Z_OK) {
        if (error) {
            *error = "Zlib: inflateInit failed";
        }
        return nullptr;
    }
    uint8_t* input = (uint8_t*)src->getMemoryBase();
    size_t inputLength = src->getLength();
    if (input == nullptr || inputLength == 0) {
        input = nullptr;
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
        if (rc != Z_OK) break;
        if (flateData.avail_in == 0) {
            if (input != nullptr) break;
            size_t read = src->read(&inputBuffer, kBufferSize);
            if (read == 0) break;
            flateData.next_in = inputBuffer;
            flateData.avail_in = SkToUInt(read);
        }
        rc = inflate(&flateData, Z_NO_FLUSH);
    }
    while (rc == Z_OK) {
        rc = inflate(&flateData, Z_FINISH);
        if (flateData.avail_out < kBufferSize) {
            if (!dst->write(outputBuffer, kBufferSize - flateData.avail_out)) {
                if (error) {
                    *error = "write failed";
                }
                return nullptr;
            }
            flateData.next_out = outputBuffer;
            flateData.avail_out = kBufferSize;
        }
    }

    (void)inflateEnd(&flateData);
    if (rc != Z_STREAM_END) {
        if (error) {
            *error = SkStringPrintf("Zlib: inflateEnd failed %d", rc);
        }
        return nullptr;
    }

    if (error) {
        error->reset();
    }
    return decompressedDynamicMemoryWStream.detachAsStream();
}
