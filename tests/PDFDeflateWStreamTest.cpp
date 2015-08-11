/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkFlate.h"
#include "SkRandom.h"
#include "Test.h"

DEF_TEST(SkDeflateWStream, r) {
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
        }
        SkAutoTDelete<SkStreamAsset> compressed(
                dynamicMemoryWStream.detachAsStream());

        SkDynamicMemoryWStream decompressedDynamicMemoryWStream;
        SkAssertResult(SkFlate::Inflate(compressed,
                                        &decompressedDynamicMemoryWStream));

        SkAutoTDelete<SkStreamAsset> decompressed(
                decompressedDynamicMemoryWStream.detachAsStream());

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
}
