/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "TestClassDef.h"
#include "SkRandom.h"
#include "SkReader32.h"
#include "SkWriter32.h"

static void check_contents(skiatest::Reporter* reporter, const SkWriter32& writer,
                           const void* expected, size_t size) {
    SkAutoSMalloc<256> storage(size);
    REPORTER_ASSERT(reporter, writer.bytesWritten() == size);
    writer.flatten(storage.get());
    REPORTER_ASSERT(reporter, !memcmp(storage.get(), expected, size));
}


static void test_reserve(skiatest::Reporter* reporter) {
    // There used to be a bug where we'd assert your first reservation had to
    // fit in external storage if you used it.  This would crash in debug mode.
    uint8_t storage[4];
    SkWriter32 writer(0, storage, sizeof(storage));
    writer.reserve(40);
}

static void test_string_null(skiatest::Reporter* reporter) {
    uint8_t storage[8];
    SkWriter32 writer(0, storage, sizeof(storage));

    // Can we write NULL?
    writer.writeString(NULL);
    const int32_t expected[] = { 0x0, 0x0 };
    check_contents(reporter, writer, expected, sizeof(expected));
}

static void test_rewind(skiatest::Reporter* reporter) {
    SkSWriter32<32> writer(32);
    int32_t array[3] = { 1, 2, 4 };

    REPORTER_ASSERT(reporter, 0 == writer.bytesWritten());
    for (size_t i = 0; i < SK_ARRAY_COUNT(array); ++i) {
        writer.writeInt(array[i]);
    }
    check_contents(reporter, writer, array, sizeof(array));

    writer.rewindToOffset(2*sizeof(int32_t));
    REPORTER_ASSERT(reporter, sizeof(array) - 4 == writer.bytesWritten());
    writer.writeInt(3);
    REPORTER_ASSERT(reporter, sizeof(array) == writer.bytesWritten());
    array[2] = 3;
    check_contents(reporter, writer, array, sizeof(array));

    // test rewinding past allocated chunks. This used to crash because we
    // didn't truncate our link-list after freeing trailing blocks
    {
        SkWriter32 writer(64);
        for (int i = 0; i < 100; ++i) {
            writer.writeInt(i);
        }
        REPORTER_ASSERT(reporter, 100*4 == writer.bytesWritten());
        for (int j = 100*4; j >= 0; j -= 16) {
            writer.rewindToOffset(j);
        }
        REPORTER_ASSERT(reporter, writer.bytesWritten() < 16);
    }
}

static void test_ptr(skiatest::Reporter* reporter) {
    SkSWriter32<32> writer(32);

    void* p0 = reporter;
    void* p1 = &writer;

    // try writing ptrs where at least one of them may be at a non-multiple of
    // 8 boundary, to confirm this works on 64bit machines.

    writer.writePtr(p0);
    writer.write8(0x33);
    writer.writePtr(p1);
    writer.write8(0x66);

    size_t size = writer.bytesWritten();
    REPORTER_ASSERT(reporter, 2 * sizeof(void*) + 2 * sizeof(int32_t));

    char buffer[32];
    SkASSERT(sizeof(buffer) >= size);
    writer.flatten(buffer);

    SkReader32 reader(buffer, size);
    REPORTER_ASSERT(reporter, reader.readPtr() == p0);
    REPORTER_ASSERT(reporter, reader.readInt() == 0x33);
    REPORTER_ASSERT(reporter, reader.readPtr() == p1);
    REPORTER_ASSERT(reporter, reader.readInt() == 0x66);
}

static void test1(skiatest::Reporter* reporter, SkWriter32* writer) {
    const uint32_t data[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    for (size_t i = 0; i < SK_ARRAY_COUNT(data); ++i) {
        REPORTER_ASSERT(reporter, i*4 == writer->bytesWritten());
        writer->write32(data[i]);
        uint32_t* addr = writer->peek32(i * 4);
        REPORTER_ASSERT(reporter, data[i] == *addr);
    }

    char buffer[sizeof(data)];
    REPORTER_ASSERT(reporter, sizeof(buffer) == writer->bytesWritten());
    writer->flatten(buffer);
    REPORTER_ASSERT(reporter, !memcmp(data, buffer, sizeof(buffer)));
}

static void test2(skiatest::Reporter* reporter, SkWriter32* writer) {
    static const char gStr[] = "abcdefghimjklmnopqrstuvwxyz";
    size_t i;

    size_t len = 0;
    for (i = 0; i <= 26; ++i) {
        len += SkWriter32::WriteStringSize(gStr, i);
        writer->writeString(gStr, i);
    }
    REPORTER_ASSERT(reporter, writer->bytesWritten() == len);

    SkAutoMalloc storage(len);
    writer->flatten(storage.get());

    SkReader32 reader;
    reader.setMemory(storage.get(), len);
    for (i = 0; i <= 26; ++i) {
        REPORTER_ASSERT(reporter, !reader.eof());
        const char* str = reader.readString(&len);
        REPORTER_ASSERT(reporter, i == len);
        REPORTER_ASSERT(reporter, strlen(str) == len);
        REPORTER_ASSERT(reporter, !memcmp(str, gStr, len));
        // Ensure that the align4 of the string is padded with zeroes.
        size_t alignedSize = SkAlign4(len + 1);
        for (size_t j = len; j < alignedSize; j++) {
            REPORTER_ASSERT(reporter, 0 == str[j]);
        }
    }
    REPORTER_ASSERT(reporter, reader.eof());
}

static void testWritePad(skiatest::Reporter* reporter, SkWriter32* writer) {
    // Create some random data to write.
    const size_t dataSize = 10<<2;
    SkASSERT(SkIsAlign4(dataSize));

    SkAutoMalloc originalData(dataSize);
    {
        SkRandom rand(0);
        uint32_t* ptr = static_cast<uint32_t*>(originalData.get());
        uint32_t* stop = ptr + (dataSize>>2);
        while (ptr < stop) {
            *ptr++ = rand.nextU();
        }

        // Write  the random data to the writer at different lengths for
        // different alignments.
        for (size_t len = 0; len < dataSize; len++) {
            writer->writePad(originalData.get(), len);
        }
    }

    uint32_t totalBytes = writer->bytesWritten();

    SkAutoMalloc readStorage(totalBytes);
    writer->flatten(readStorage.get());

    SkReader32 reader;
    reader.setMemory(readStorage.get(), totalBytes);

    for (size_t len = 0; len < dataSize; len++) {
        const char* readPtr = static_cast<const char*>(reader.skip(len));
        // Ensure that the data read is the same as what was written.
        REPORTER_ASSERT(reporter, memcmp(readPtr, originalData.get(), len) == 0);
        // Ensure that the rest is padded with zeroes.
        const char* stop = readPtr + SkAlign4(len);
        readPtr += len;
        while (readPtr < stop) {
            REPORTER_ASSERT(reporter, *readPtr++ == 0);
        }
    }
}

DEF_TEST(Writer32, reporter) {
    // dynamic allocator
    {
        SkWriter32 writer(256 * 4);
        test1(reporter, &writer);

        writer.reset();
        test2(reporter, &writer);

        writer.reset();
        testWritePad(reporter, &writer);
    }

    // storage-block
    {
        SkWriter32 writer(0);
        uint32_t storage[256];
        writer.reset(storage, sizeof(storage));
        // These three writes are small enough to fit in storage.
        test1(reporter, &writer);
        REPORTER_ASSERT(reporter, writer.wroteOnlyToStorage());

        writer.reset(storage, sizeof(storage));
        test2(reporter, &writer);
        REPORTER_ASSERT(reporter, writer.wroteOnlyToStorage());

        writer.reset(storage, sizeof(storage));
        testWritePad(reporter, &writer);
        REPORTER_ASSERT(reporter, writer.wroteOnlyToStorage());

        // Try overflowing the storage-block.
        uint32_t smallStorage[8];
        writer.reset(smallStorage, sizeof(smallStorage));
        test2(reporter, &writer);
        REPORTER_ASSERT(reporter, !writer.wroteOnlyToStorage());
    }

    // small storage
    {
        SkSWriter32<8 * sizeof(intptr_t)> writer(100);
        test1(reporter, &writer);
        writer.reset(); // should just rewind our storage
        test2(reporter, &writer);

        writer.reset();
        testWritePad(reporter, &writer);
    }

    // large storage
    {
        SkSWriter32<1024 * sizeof(intptr_t)> writer(100);
        test1(reporter, &writer);
        writer.reset(); // should just rewind our storage
        test2(reporter, &writer);

        writer.reset();
        testWritePad(reporter, &writer);
    }

    test_reserve(reporter);
    test_string_null(reporter);
    test_ptr(reporter);
    test_rewind(reporter);
}
