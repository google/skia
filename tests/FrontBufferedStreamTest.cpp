/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "TestClassDef.h"
#include "SkFrontBufferedStream.h"
#include "SkRefCnt.h"
#include "SkStream.h"
#include "SkTypes.h"

static void test_read(skiatest::Reporter* reporter, SkStream* bufferedStream,
                      const void* expectations, size_t bytesToRead) {
    // output for reading bufferedStream.
    SkAutoMalloc storage(bytesToRead);

    size_t bytesRead = bufferedStream->read(storage.get(), bytesToRead);
    REPORTER_ASSERT(reporter, bytesRead == bytesToRead || bufferedStream->isAtEnd());
    REPORTER_ASSERT(reporter, memcmp(storage.get(), expectations, bytesRead) == 0);
}

static void test_rewind(skiatest::Reporter* reporter,
                        SkStream* bufferedStream, bool shouldSucceed) {
    const bool success = bufferedStream->rewind();
    REPORTER_ASSERT(reporter, success == shouldSucceed);
}

// All tests will buffer this string, and compare output to the original.
// The string is long to ensure that all of our lengths being tested are
// smaller than the string length.
const char gAbcs[] = "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwx";

// Tests reading the stream across boundaries of what has been buffered so far and what
// the total buffer size is.
static void test_incremental_buffering(skiatest::Reporter* reporter, size_t bufferSize) {
    SkMemoryStream memStream(gAbcs, strlen(gAbcs), false);

    SkAutoTUnref<SkStream> bufferedStream(SkFrontBufferedStream::Create(&memStream, bufferSize));

    // First, test reading less than the max buffer size.
    test_read(reporter, bufferedStream, gAbcs, bufferSize / 2);

    // Now test rewinding back to the beginning and reading less than what was
    // already buffered.
    test_rewind(reporter, bufferedStream, true);
    test_read(reporter, bufferedStream, gAbcs, bufferSize / 4);

    // Now test reading part of what was buffered, and buffering new data.
    test_read(reporter, bufferedStream, gAbcs + bufferedStream->getPosition(), bufferSize / 2);

    // Now test reading what was buffered, buffering new data, and
    // reading directly from the stream.
    test_rewind(reporter, bufferedStream, true);
    test_read(reporter, bufferedStream, gAbcs, bufferSize << 1);

    // We have reached the end of the buffer, so rewinding will fail.
    // This test assumes that the stream is larger than the buffer; otherwise the
    // result of rewind should be true.
    test_rewind(reporter, bufferedStream, false);
}

static void test_perfectly_sized_buffer(skiatest::Reporter* reporter, size_t bufferSize) {
    SkMemoryStream memStream(gAbcs, strlen(gAbcs), false);
    SkAutoTUnref<SkStream> bufferedStream(SkFrontBufferedStream::Create(&memStream, bufferSize));

    // Read exactly the amount that fits in the buffer.
    test_read(reporter, bufferedStream, gAbcs, bufferSize);

    // Rewinding should succeed.
    test_rewind(reporter, bufferedStream, true);

    // Once again reading buffered info should succeed
    test_read(reporter, bufferedStream, gAbcs, bufferSize);

    // Read past the size of the buffer. At this point, we cannot return.
    test_read(reporter, bufferedStream, gAbcs + bufferedStream->getPosition(), 1);
    test_rewind(reporter, bufferedStream, false);
}

static void test_skipping(skiatest::Reporter* reporter, size_t bufferSize) {
    SkMemoryStream memStream(gAbcs, strlen(gAbcs), false);
    SkAutoTUnref<SkStream> bufferedStream(SkFrontBufferedStream::Create(&memStream, bufferSize));

    // Skip half the buffer.
    bufferedStream->skip(bufferSize / 2);

    // Rewind, then read part of the buffer, which should have been read.
    test_rewind(reporter, bufferedStream, true);
    test_read(reporter, bufferedStream, gAbcs, bufferSize / 4);

    // Now skip beyond the buffered piece, but still within the total buffer.
    bufferedStream->skip(bufferSize / 2);

    // Test that reading will still work.
    test_read(reporter, bufferedStream, gAbcs + bufferedStream->getPosition(), bufferSize / 4);

    test_rewind(reporter, bufferedStream, true);
    test_read(reporter, bufferedStream, gAbcs, bufferSize);
}

// A custom class whose isAtEnd behaves the way Android's stream does - since it is an adaptor to a
// Java InputStream, it does not know that it is at the end until it has attempted to read beyond
// the end and failed. Used by test_read_beyond_buffer.
class AndroidLikeMemoryStream : public SkMemoryStream {
public:
    AndroidLikeMemoryStream(void* data, size_t size, bool ownMemory)
        : INHERITED(data, size, ownMemory)
        , fIsAtEnd(false) {}

    size_t read(void* dst, size_t requested) SK_OVERRIDE {
        size_t bytesRead = this->INHERITED::read(dst, requested);
        if (bytesRead < requested) {
            fIsAtEnd = true;
        }
        return bytesRead;
    }

    bool isAtEnd() const SK_OVERRIDE {
        return fIsAtEnd;
    }

private:
    bool fIsAtEnd;
    typedef SkMemoryStream INHERITED;
};

// This test ensures that buffering the exact length of the stream and attempting to read beyond it
// does not invalidate the buffer.
static void test_read_beyond_buffer(skiatest::Reporter* reporter, size_t bufferSize) {
    // Use a stream that behaves like Android's stream.
    AndroidLikeMemoryStream memStream((void*)gAbcs, bufferSize, false);

    // Create a buffer that matches the length of the stream.
    SkAutoTUnref<SkStream> bufferedStream(SkFrontBufferedStream::Create(&memStream, bufferSize));

    // Attempt to read one more than the bufferSize
    test_read(reporter, bufferedStream.get(), gAbcs, bufferSize + 1);
    test_rewind(reporter, bufferedStream.get(), true);

    // Ensure that the initial read did not invalidate the buffer.
    test_read(reporter, bufferedStream, gAbcs, bufferSize);
}

static void test_buffers(skiatest::Reporter* reporter, size_t bufferSize) {
    test_incremental_buffering(reporter, bufferSize);
    test_perfectly_sized_buffer(reporter, bufferSize);
    test_skipping(reporter, bufferSize);
    test_read_beyond_buffer(reporter, bufferSize);
}

DEF_TEST(FrontBufferedStream, reporter) {
    // Test 6 and 64, which are used by Android, as well as another arbitrary length.
    test_buffers(reporter, 6);
    test_buffers(reporter, 15);
    test_buffers(reporter, 64);
}
