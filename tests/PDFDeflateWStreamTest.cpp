/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"

#ifdef SK_SUPPORT_PDF

#include "SkDeflate.h"
#include "SkRandom.h"
#include "SkTo.h"

namespace {

#include "zlib.h"

// Different zlib implementations use different T.
// We've seen size_t and unsigned.
template <typename T> void* skia_alloc_func(void*, T items, T size) {
    return sk_calloc_throw(SkToSizeT(items) * SkToSizeT(size));
}

void skia_free_func(void*, void* address) { sk_free(address); }

/**
 *  Use the un-deflate compression algorithm to decompress the data in src,
 *  returning the result.  Returns nullptr if an error occurs.
 */
std::unique_ptr<SkStreamAsset> stream_inflate(skiatest::Reporter* reporter, SkStream* src) {
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
        ERRORF(reporter, "Zlib: inflateInit failed");
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
        if (rc != Z_OK)
            break;
        if (flateData.avail_in == 0) {
            if (input != nullptr)
                break;
            size_t read = src->read(&inputBuffer, kBufferSize);
            if (read == 0)
                break;
            flateData.next_in = inputBuffer;
            flateData.avail_in = SkToUInt(read);
        }
        rc = inflate(&flateData, Z_NO_FLUSH);
    }
    while (rc == Z_OK) {
        rc = inflate(&flateData, Z_FINISH);
        if (flateData.avail_out < kBufferSize) {
            if (!dst->write(outputBuffer, kBufferSize - flateData.avail_out)) {
                ERRORF(reporter, "write failed");
                return nullptr;
            }
            flateData.next_out = outputBuffer;
            flateData.avail_out = kBufferSize;
        }
    }

    inflateEnd(&flateData);
    if (rc != Z_STREAM_END) {
        ERRORF(reporter, "Zlib: inflateEnd failed");
        return nullptr;
    }
    return decompressedDynamicMemoryWStream.detachAsStream();
}
}  // namespace

DEF_TEST(SkPDF_DeflateWStream, r) {
    SkRandom random(123456);
    for (int i = 0; i < 50; ++i) {
        uint32_t size = random.nextULessThan(10000);
        SkAutoTMalloc<uint8_t> buffer(size);
        for (uint32_t j = 0; j < size; ++j) {
            buffer[j] = random.nextU() & 0xff;
        }

        SkDynamicMemoryWStream dynamicMemoryWStream;
        {
            SkDeflateWStream deflateWStream(&dynamicMemoryWStream);
            uint32_t j = 0;
            while (j < size) {
                uint32_t writeSize =
                        SkTMin(size - j, random.nextRangeU(1, 400));
                if (!deflateWStream.write(&buffer[j], writeSize)) {
                    ERRORF(r, "something went wrong.");
                    return;
                }
                j += writeSize;
            }
            REPORTER_ASSERT(r, deflateWStream.bytesWritten() == size);
        }
        std::unique_ptr<SkStreamAsset> compressed(dynamicMemoryWStream.detachAsStream());
        std::unique_ptr<SkStreamAsset> decompressed(stream_inflate(r, compressed.get()));

        if (!decompressed) {
            ERRORF(r, "Decompression failed.");
            return;
        }
        if (decompressed->getLength() != size) {
            ERRORF(r, "Decompression failed to get right size [%d]."
                   " %u != %u", i,  (unsigned)(decompressed->getLength()),
                   (unsigned)size);
            SkString s = SkStringPrintf("/tmp/deftst_compressed_%d", i);
            SkFILEWStream o(s.c_str());
            o.writeStream(compressed.get(), compressed->getLength());
            compressed->rewind();

            s = SkStringPrintf("/tmp/deftst_input_%d", i);
            SkFILEWStream o2(s.c_str());
            o2.write(&buffer[0], size);

            continue;
        }
        uint32_t minLength = SkTMin(size,
                                    (uint32_t)(decompressed->getLength()));
        for (uint32_t i = 0; i < minLength; ++i) {
            uint8_t c;
            SkDEBUGCODE(size_t rb =)decompressed->read(&c, sizeof(uint8_t));
            SkASSERT(sizeof(uint8_t) == rb);
            if (buffer[i] != c) {
                ERRORF(r, "Decompression failed at byte %u.", (unsigned)i);
                break;
            }
        }
    }
    SkDeflateWStream emptyDeflateWStream(nullptr);
    REPORTER_ASSERT(r, !emptyDeflateWStream.writeText("FOO"));
}

DEF_TEST(SkPDF_DeflateStream, r) {
    static constexpr size_t kBufferSize = 524288;
    SkAutoTMalloc<uint8_t> buffer(kBufferSize),
                           out_buffer(kBufferSize);

    SkRandom random(123456);
    for (auto* p = reinterpret_cast<uint32_t*>(buffer.get());
         p < reinterpret_cast<uint32_t*>(buffer.get() + kBufferSize); ++p) {
        *p = random.nextU();
    }

    for (size_t size = 0; size <= kBufferSize; size += 4096) {
        SkDynamicMemoryWStream compressed_out;
        SkDeflateWStream compress_stream(&compressed_out);
        REPORTER_ASSERT(r, compress_stream.write(buffer, size));
        compress_stream.finalize();

        auto compressed_data = compressed_out.detachAsData();

        for (size_t chunk = 1; chunk <= kBufferSize; chunk *= 2) {
            SkMemoryStream compressed_in(compressed_data);
            SkDeflateStream decompress_stream(&compressed_in);
            memset(out_buffer, 0, size);

            size_t bytes = 0;
            while (bytes < size) {
                const auto to_read = SkTMin(chunk, size - bytes),
                              read = decompress_stream.read(out_buffer + bytes, to_read);
                REPORTER_ASSERT(r, read == to_read);
                if (!read) break;

                bytes += read;
            }

            REPORTER_ASSERT(r, decompress_stream.read(out_buffer, chunk) == 0);
            REPORTER_ASSERT(r, memcmp(buffer, out_buffer, size) == 0);
        }
    }
}

#endif
