/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/utils/SkRandom.h"
#include "src/core/SkAutoMalloc.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriter32.h"
#include "tests/Test.h"

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
    SkWriter32 writer(storage, sizeof(storage));
    writer.reserve(40);
}

static void test_string_null(skiatest::Reporter* reporter) {
    uint8_t storage[8];
    SkWriter32 writer(storage, sizeof(storage));

    // Can we write nullptr?
    writer.writeString(nullptr);
    const int32_t expected[] = { 0x0, 0x0 };
    check_contents(reporter, writer, expected, sizeof(expected));
}

static void test_rewind(skiatest::Reporter* reporter) {
    SkSWriter32<32> swriter;
    int32_t array[3] = { 1, 2, 4 };

    REPORTER_ASSERT(reporter, 0 == swriter.bytesWritten());
    for (size_t i = 0; i < SK_ARRAY_COUNT(array); ++i) {
        swriter.writeInt(array[i]);
    }
    check_contents(reporter, swriter, array, sizeof(array));

    swriter.rewindToOffset(2*sizeof(int32_t));
    REPORTER_ASSERT(reporter, sizeof(array) - 4 == swriter.bytesWritten());
    swriter.writeInt(3);
    REPORTER_ASSERT(reporter, sizeof(array) == swriter.bytesWritten());
    array[2] = 3;
    check_contents(reporter, swriter, array, sizeof(array));

    // test rewinding past allocated chunks. This used to crash because we
    // didn't truncate our link-list after freeing trailing blocks
    SkWriter32 writer;
    for (int i = 0; i < 100; ++i) {
        writer.writeInt(i);
    }
    REPORTER_ASSERT(reporter, 100*4 == writer.bytesWritten());
    for (int j = 100*4; j >= 0; j -= 16) {
        writer.rewindToOffset(j);
    }
    REPORTER_ASSERT(reporter, writer.bytesWritten() < 16);
}

static void test1(skiatest::Reporter* reporter, SkWriter32* writer) {
    const uint32_t data[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    for (size_t i = 0; i < SK_ARRAY_COUNT(data); ++i) {
        REPORTER_ASSERT(reporter, i*4 == writer->bytesWritten());
        writer->write32(data[i]);
        REPORTER_ASSERT(reporter, data[i] == writer->readTAt<uint32_t>(i * 4));
    }

    char buffer[sizeof(data)];
    REPORTER_ASSERT(reporter, sizeof(buffer) == writer->bytesWritten());
    writer->flatten(buffer);
    REPORTER_ASSERT(reporter, !memcmp(data, buffer, sizeof(buffer)));
}

static void testWritePad(skiatest::Reporter* reporter, SkWriter32* writer) {
    // Create some random data to write.
    const size_t dataSize = 10;

    SkAutoTMalloc<uint32_t> originalData(dataSize);
    {
        SkRandom rand(0);
        for (size_t i = 0; i < dataSize; i++) {
            originalData[(int) i] = rand.nextU();
        }

        // Write  the random data to the writer at different lengths for
        // different alignments.
        for (size_t len = 0; len < dataSize; len++) {
            writer->writePad(originalData.get(), len);
        }
    }

    size_t totalBytes = writer->bytesWritten();

    SkAutoMalloc readStorage(totalBytes);
    writer->flatten(readStorage.get());

    SkReadBuffer reader(readStorage.get(), totalBytes);

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

static void testOverwriteT(skiatest::Reporter* reporter, SkWriter32* writer) {
    const size_t padding = 64;

    const uint32_t uint1 = 0x12345678;
    const uint32_t uint2 = 0x98765432;
    const SkScalar scalar1 = 1234.5678f;
    const SkScalar scalar2 = 9876.5432f;
    const SkRect rect1 = SkRect::MakeXYWH(1, 2, 3, 4);
    const SkRect rect2 = SkRect::MakeXYWH(5, 6, 7, 8);

    for (size_t i = 0; i < (padding / 4); ++i) {
        writer->write32(0);
    }

    writer->write32(uint1);
    writer->writeRect(rect1);
    writer->writeScalar(scalar1);

    for (size_t i = 0; i < (padding / 4); ++i) {
        writer->write32(0);
    }

    REPORTER_ASSERT(reporter, writer->readTAt<uint32_t>(padding) == uint1);
    REPORTER_ASSERT(reporter, writer->readTAt<SkRect>(padding + sizeof(uint32_t)) == rect1);
    REPORTER_ASSERT(reporter, writer->readTAt<SkScalar>(
                        padding + sizeof(uint32_t) + sizeof(SkRect)) == scalar1);

    writer->overwriteTAt(padding, uint2);
    writer->overwriteTAt(padding + sizeof(uint32_t), rect2);
    writer->overwriteTAt(padding + sizeof(uint32_t) + sizeof(SkRect), scalar2);

    REPORTER_ASSERT(reporter, writer->readTAt<uint32_t>(padding) == uint2);
    REPORTER_ASSERT(reporter, writer->readTAt<SkRect>(padding + sizeof(uint32_t)) == rect2);
    REPORTER_ASSERT(reporter, writer->readTAt<SkScalar>(
                        padding + sizeof(uint32_t) + sizeof(SkRect)) == scalar2);
}

DEF_TEST(Writer32_dynamic, reporter) {
    SkWriter32 writer;
    test1(reporter, &writer);

    writer.reset();
    testWritePad(reporter, &writer);

    writer.reset();
    testOverwriteT(reporter, &writer);
}

DEF_TEST(Writer32_small, reporter) {
    SkSWriter32<8 * sizeof(intptr_t)> writer;
    test1(reporter, &writer);

    writer.reset(); // should just rewind our storage
    testWritePad(reporter, &writer);

    writer.reset();
    testOverwriteT(reporter, &writer);
}

DEF_TEST(Writer32_large, reporter) {
    SkSWriter32<1024 * sizeof(intptr_t)> writer;
    test1(reporter, &writer);

    writer.reset(); // should just rewind our storage
    testWritePad(reporter, &writer);

    writer.reset();
    testOverwriteT(reporter, &writer);
}

DEF_TEST(Writer32_misc, reporter) {
    test_reserve(reporter);
    test_string_null(reporter);
    test_rewind(reporter);
}

DEF_TEST(Writer32_data, reporter) {
    const char* str = "0123456789";
    sk_sp<SkData> data0(SkData::MakeWithCString(str));
    sk_sp<SkData> data1(SkData::MakeEmpty());

    const size_t sizes[] = {
        SkWriter32::WriteDataSize(nullptr),
        SkWriter32::WriteDataSize(data0.get()),
        SkWriter32::WriteDataSize(data1.get()),
    };

    SkSWriter32<1000> writer;
    size_t sizeWritten = 0;

    writer.writeData(nullptr);
    sizeWritten += sizes[0];
    REPORTER_ASSERT(reporter, sizeWritten == writer.bytesWritten());

    writer.writeData(data0.get());
    sizeWritten += sizes[1];
    REPORTER_ASSERT(reporter, sizeWritten == writer.bytesWritten());

    writer.writeData(data1.get());
    sizeWritten += sizes[2];
    REPORTER_ASSERT(reporter, sizeWritten == writer.bytesWritten());

    auto result(writer.snapshotAsData());

    SkReadBuffer reader(result->data(), result->size());
    auto d0(reader.readByteArrayAsData()),
         d1(reader.readByteArrayAsData()),
         d2(reader.readByteArrayAsData());

    REPORTER_ASSERT(reporter, 0 == d0->size());
    REPORTER_ASSERT(reporter, strlen(str)+1 == d1->size());
    REPORTER_ASSERT(reporter, !memcmp(str, d1->data(), strlen(str)+1));
    REPORTER_ASSERT(reporter, 0 == d2->size());

    REPORTER_ASSERT(reporter, reader.offset() == sizeWritten);
    REPORTER_ASSERT(reporter, reader.eof());
}
