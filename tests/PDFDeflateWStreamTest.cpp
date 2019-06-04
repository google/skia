/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#ifdef SK_SUPPORT_PDF

#include "include/private/SkTo.h"
#include "include/utils/SkRandom.h"
#include "src/pdf/SkDeflate.h"

#include "tools/Inflate.h"

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
        SkString error;
        std::unique_ptr<SkStreamAsset> decompressed(InflateStream(compressed.get(), &error));

        if (!decompressed) {
            ERRORF(r, "Decompression failed: '%s'.", error.c_str());
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

#endif
