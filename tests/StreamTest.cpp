/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "TestClassDef.h"
#include "SkRandom.h"
#include "SkOSFile.h"
#include "SkStream.h"
#include "SkData.h"

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
    SkString path = SkOSPath::SkPathJoin(tmpDir, "wstream_test");

    const char s[] = "abcdefghijklmnopqrstuvwxyz";

    {
        SkFILEWStream writer(path.c_str());
        if (!writer.isValid()) {
            SkString msg;
            msg.printf("Failed to create tmp file %s\n", path.c_str());
            reporter->reportFailed(msg);
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

        SkAutoTUnref<SkStreamAsset> stream2(stream.duplicate());
        test_loop_stream(reporter, stream2.get(), s, 26, 100);
    }

    {
        FILE* file = ::fopen(path.c_str(), "rb");
        SkFILEStream stream(file, SkFILEStream::kCallerPasses_Ownership);
        REPORTER_ASSERT(reporter, stream.isValid());
        test_loop_stream(reporter, &stream, s, 26, 100);

        SkAutoTUnref<SkStreamAsset> stream2(stream.duplicate());
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
        SkAutoTUnref<SkStreamAsset> stream(ds.detachAsStream());
        REPORTER_ASSERT(reporter, 100 * 26 == stream->getLength());
        REPORTER_ASSERT(reporter, ds.getOffset() == 0);
        test_loop_stream(reporter, stream.get(), s, 26, 100);

        SkAutoTUnref<SkStreamAsset> stream2(stream->duplicate());
        test_loop_stream(reporter, stream2.get(), s, 26, 100);

        SkAutoTUnref<SkStreamAsset> stream3(stream->fork());
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
        SkAutoTUnref<SkStreamAsset> stream(ds.detachAsStream());
        REPORTER_ASSERT(reporter, ds.getOffset() == 0);
        test_loop_stream(reporter, stream.get(), s, 26, 100);

        SkAutoTUnref<SkStreamAsset> stream2(stream->duplicate());
        test_loop_stream(reporter, stream2.get(), s, 26, 100);
    }
    delete[] dst;

    SkString tmpDir = skiatest::Test::GetTmpDir();
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
