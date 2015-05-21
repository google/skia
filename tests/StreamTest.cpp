/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Resources.h"
#include "SkData.h"
#include "SkFrontBufferedStream.h"
#include "SkOSFile.h"
#include "SkRandom.h"
#include "SkStream.h"
#include "Test.h"

#ifndef SK_BUILD_FOR_WIN
#include <unistd.h>
#include <fcntl.h>
#endif

#define MAX_SIZE    (256 * 1024)

static void test_loop_stream(skiatest::Reporter* reporter, SkStream* stream,
                             const void* src, size_t len, int repeat) {
    SkAutoSMalloc<256> storage(len);
    void* tmp = storage.get();

    for (int i = 0; i < repeat; ++i) {
        size_t bytes = stream->read(tmp, len);
        REPORTER_ASSERT(reporter, bytes == len);
        REPORTER_ASSERT(reporter, !memcmp(tmp, src, len));
    }

    // expect EOF
    size_t bytes = stream->read(tmp, 1);
    REPORTER_ASSERT(reporter, 0 == bytes);
    // isAtEnd might not return true until after the first failing read.
    REPORTER_ASSERT(reporter, stream->isAtEnd());
}

static void test_filestreams(skiatest::Reporter* reporter, const char* tmpDir) {
    SkString path = SkOSPath::Join(tmpDir, "wstream_test");

    const char s[] = "abcdefghijklmnopqrstuvwxyz";

    {
        SkFILEWStream writer(path.c_str());
        if (!writer.isValid()) {
            ERRORF(reporter, "Failed to create tmp file %s\n", path.c_str());
            return;
        }

        for (int i = 0; i < 100; ++i) {
            writer.write(s, 26);
        }
    }

    {
        SkFILEStream stream(path.c_str());
        REPORTER_ASSERT(reporter, stream.isValid());
        test_loop_stream(reporter, &stream, s, 26, 100);

        SkAutoTDelete<SkStreamAsset> stream2(stream.duplicate());
        test_loop_stream(reporter, stream2.get(), s, 26, 100);
    }

    {
        FILE* file = ::fopen(path.c_str(), "rb");
        SkFILEStream stream(file, SkFILEStream::kCallerPasses_Ownership);
        REPORTER_ASSERT(reporter, stream.isValid());
        test_loop_stream(reporter, &stream, s, 26, 100);

        SkAutoTDelete<SkStreamAsset> stream2(stream.duplicate());
        test_loop_stream(reporter, stream2.get(), s, 26, 100);
    }
}

static void TestWStream(skiatest::Reporter* reporter) {
    SkDynamicMemoryWStream  ds;
    const char s[] = "abcdefghijklmnopqrstuvwxyz";
    int i;
    for (i = 0; i < 100; i++) {
        REPORTER_ASSERT(reporter, ds.write(s, 26));
    }
    REPORTER_ASSERT(reporter, ds.getOffset() == 100 * 26);

    char* dst = new char[100 * 26 + 1];
    dst[100*26] = '*';
    ds.copyTo(dst);
    REPORTER_ASSERT(reporter, dst[100*26] == '*');
    for (i = 0; i < 100; i++) {
        REPORTER_ASSERT(reporter, memcmp(&dst[i * 26], s, 26) == 0);
    }

    {
        SkAutoTDelete<SkStreamAsset> stream(ds.detachAsStream());
        REPORTER_ASSERT(reporter, 100 * 26 == stream->getLength());
        REPORTER_ASSERT(reporter, ds.getOffset() == 0);
        test_loop_stream(reporter, stream.get(), s, 26, 100);

        SkAutoTDelete<SkStreamAsset> stream2(stream->duplicate());
        test_loop_stream(reporter, stream2.get(), s, 26, 100);

        SkAutoTDelete<SkStreamAsset> stream3(stream->fork());
        REPORTER_ASSERT(reporter, stream3->isAtEnd());
        char tmp;
        size_t bytes = stream->read(&tmp, 1);
        REPORTER_ASSERT(reporter, 0 == bytes);
        stream3->rewind();
        test_loop_stream(reporter, stream3.get(), s, 26, 100);
    }

    for (i = 0; i < 100; i++) {
        REPORTER_ASSERT(reporter, ds.write(s, 26));
    }
    REPORTER_ASSERT(reporter, ds.getOffset() == 100 * 26);

    {
        SkAutoTUnref<SkData> data(ds.copyToData());
        REPORTER_ASSERT(reporter, 100 * 26 == data->size());
        REPORTER_ASSERT(reporter, memcmp(dst, data->data(), data->size()) == 0);
    }

    {
        // Test that this works after a copyToData.
        SkAutoTDelete<SkStreamAsset> stream(ds.detachAsStream());
        REPORTER_ASSERT(reporter, ds.getOffset() == 0);
        test_loop_stream(reporter, stream.get(), s, 26, 100);

        SkAutoTDelete<SkStreamAsset> stream2(stream->duplicate());
        test_loop_stream(reporter, stream2.get(), s, 26, 100);
    }
    delete[] dst;

    SkString tmpDir = skiatest::GetTmpDir();
    if (!tmpDir.isEmpty()) {
        test_filestreams(reporter, tmpDir.c_str());
    }
}

static void TestPackedUInt(skiatest::Reporter* reporter) {
    // we know that packeduint tries to write 1, 2 or 4 bytes for the length,
    // so we test values around each of those transitions (and a few others)
    const size_t sizes[] = {
        0, 1, 2, 0xFC, 0xFD, 0xFE, 0xFF, 0x100, 0x101, 32767, 32768, 32769,
        0xFFFD, 0xFFFE, 0xFFFF, 0x10000, 0x10001,
        0xFFFFFD, 0xFFFFFE, 0xFFFFFF, 0x1000000, 0x1000001,
        0x7FFFFFFE, 0x7FFFFFFF, 0x80000000, 0x80000001, 0xFFFFFFFE, 0xFFFFFFFF
    };


    size_t i;
    char buffer[sizeof(sizes) * 4];

    SkMemoryWStream wstream(buffer, sizeof(buffer));
    for (i = 0; i < SK_ARRAY_COUNT(sizes); ++i) {
        bool success = wstream.writePackedUInt(sizes[i]);
        REPORTER_ASSERT(reporter, success);
    }
    wstream.flush();

    SkMemoryStream rstream(buffer, sizeof(buffer));
    for (i = 0; i < SK_ARRAY_COUNT(sizes); ++i) {
        size_t n = rstream.readPackedUInt();
        if (sizes[i] != n) {
            SkDebugf("-- %d: sizes:%x n:%x\n", i, sizes[i], n);
        }
        REPORTER_ASSERT(reporter, sizes[i] == n);
    }
}

// Test that setting an SkMemoryStream to a NULL data does not result in a crash when calling
// methods that access fData.
static void TestDereferencingData(SkMemoryStream* memStream) {
    memStream->read(NULL, 0);
    memStream->getMemoryBase();
    SkAutoDataUnref data(memStream->copyToData());
}

static void TestNullData() {
    SkData* nullData = NULL;
    SkMemoryStream memStream(nullData);
    TestDereferencingData(&memStream);

    memStream.setData(nullData);
    TestDereferencingData(&memStream);

}

DEF_TEST(Stream, reporter) {
    TestWStream(reporter);
    TestPackedUInt(reporter);
    TestNullData();
}

/**
 *  Tests peeking and then reading the same amount. The two should provide the
 *  same results.
 *  Returns whether the stream could peek.
 */
static bool compare_peek_to_read(skiatest::Reporter* reporter,
                                 SkStream* stream, size_t bytesToPeek) {
    // The rest of our tests won't be very interesting if bytesToPeek is zero.
    REPORTER_ASSERT(reporter, bytesToPeek > 0);
    SkAutoMalloc peekStorage(bytesToPeek);
    SkAutoMalloc readStorage(bytesToPeek);
    void* peekPtr = peekStorage.get();
    void* readPtr = peekStorage.get();

    if (!stream->peek(peekPtr, bytesToPeek)) {
        return false;
    }
    const size_t bytesRead = stream->read(readPtr, bytesToPeek);

    // bytesRead should only be less than attempted if the stream is at the
    // end.
    REPORTER_ASSERT(reporter, bytesRead == bytesToPeek || stream->isAtEnd());

    // peek and read should behave the same, except peek returned to the
    // original position, so they read the same data.
    REPORTER_ASSERT(reporter, !memcmp(peekPtr, readPtr, bytesRead));

    return true;
}

static void test_peeking_stream(skiatest::Reporter* r, SkStream* stream, size_t limit) {
    size_t peeked = 0;
    for (size_t i = 1; !stream->isAtEnd(); i++) {
        const bool couldPeek = compare_peek_to_read(r, stream, i);
        if (!couldPeek) {
            REPORTER_ASSERT(r, peeked + i > limit);
            // No more peeking is supported.
            break;
        }
        peeked += i;
    }
}

static void test_peeking_front_buffered_stream(skiatest::Reporter* r,
                                               const SkStream& original,
                                               size_t bufferSize) {
    SkStream* dupe = original.duplicate();
    REPORTER_ASSERT(r, dupe != NULL);
    SkAutoTDelete<SkStream> bufferedStream(SkFrontBufferedStream::Create(dupe, bufferSize));
    REPORTER_ASSERT(r, bufferedStream != NULL);
    test_peeking_stream(r, bufferedStream, bufferSize);
}

// This test uses file system operations that don't work out of the 
// box on iOS. It's likely that we don't need them on iOS. Ignoring for now. 
// TODO(stephana): Re-evaluate if we need this in the future. 
#ifndef SK_BUILD_FOR_IOS
DEF_TEST(StreamPeek, reporter) {
    // Test a memory stream.
    const char gAbcs[] = "abcdefghijklmnopqrstuvwxyz";
    SkMemoryStream memStream(gAbcs, strlen(gAbcs), false);
    test_peeking_stream(reporter, &memStream, memStream.getLength());

    // Test an arbitrary file stream. file streams do not support peeking.
    SkFILEStream fileStream(GetResourcePath("baby_tux.webp").c_str());
    REPORTER_ASSERT(reporter, fileStream.isValid());
    if (!fileStream.isValid()) {
        return;
    }
    SkAutoMalloc storage(fileStream.getLength());
    for (size_t i = 1; i < fileStream.getLength(); i++) {
        REPORTER_ASSERT(reporter, !fileStream.peek(storage.get(), i));
    }

    // Now test some FrontBufferedStreams
    for (size_t i = 1; i < memStream.getLength(); i++) {
        test_peeking_front_buffered_stream(reporter, memStream, i);
    }
}
#endif

// Asserts that asset == expected and is peekable.
static void stream_peek_test(skiatest::Reporter* rep,
                             SkStreamAsset* asset,
                             const SkData* expected) {
    if (asset->getLength() != expected->size()) {
        ERRORF(rep, "Unexpected length.");
        return;
    }
    SkRandom rand;
    uint8_t buffer[4096];
    const uint8_t* expect = expected->bytes();
    for (size_t i = 0; i < asset->getLength(); ++i) {
        uint32_t maxSize =
                SkToU32(SkTMin(sizeof(buffer), asset->getLength() - i));
        size_t size = rand.nextRangeU(1, maxSize);
        SkASSERT(size >= 1);
        SkASSERT(size <= sizeof(buffer));
        SkASSERT(size + i <= asset->getLength());
        if (!asset->peek(buffer, size)) {
            ERRORF(rep, "Peek Failed!");
            return;
        }
        if (0 != memcmp(buffer, &expect[i], size)) {
            ERRORF(rep, "Peek returned wrong bytes!");
            return;
        }
        uint8_t value;
        REPORTER_ASSERT(rep, 1 == asset->read(&value, 1));
        if (value != expect[i]) {
            ERRORF(rep, "Read Failed!");
            return;
        }
    }
}

DEF_TEST(StreamPeek_BlockMemoryStream, rep) {
    const static int kSeed = 1234;
    SkRandom valueSource(kSeed);
    SkRandom rand(kSeed << 1);
    uint8_t buffer[4096];
    SkDynamicMemoryWStream dynamicMemoryWStream;
    for (int i = 0; i < 32; ++i) {
        // Randomize the length of the blocks.
        size_t size = rand.nextRangeU(1, sizeof(buffer));
        for (size_t j = 0; j < size; ++j) {
            buffer[j] = valueSource.nextU() & 0xFF;
        }
        dynamicMemoryWStream.write(buffer, size);
    }
    SkAutoTDelete<SkStreamAsset> asset(dynamicMemoryWStream.detachAsStream());
    SkAutoTUnref<SkData> expected(SkData::NewUninitialized(asset->getLength()));
    uint8_t* expectedPtr = static_cast<uint8_t*>(expected->writable_data());
    valueSource.setSeed(kSeed);  // reseed.
    // We want the exact same same "random" string of numbers to put
    // in expected. i.e.: don't rely on SkDynamicMemoryStream to work
    // correctly while we are testing SkDynamicMemoryStream.
    for (size_t i = 0; i < asset->getLength(); ++i) {
        expectedPtr[i] = valueSource.nextU() & 0xFF;
    }
    stream_peek_test(rep, asset, expected);
}
