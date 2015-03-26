/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkFrontBufferedStream.h"
#include "SkImageDecoder.h"
#include "SkRefCnt.h"
#include "SkStream.h"
#include "SkTypes.h"
#include "Test.h"

static void test_read(skiatest::Reporter* reporter, SkStream* bufferedStream,
                      const void* expectations, size_t bytesToRead) {
    // output for reading bufferedStream.
    SkAutoMalloc storage(bytesToRead);

    const size_t bytesRead = bufferedStream->read(storage.get(), bytesToRead);
    REPORTER_ASSERT(reporter, bytesRead == bytesToRead || bufferedStream->isAtEnd());
    REPORTER_ASSERT(reporter, memcmp(storage.get(), expectations, bytesRead) == 0);
}

static void test_rewind(skiatest::Reporter* reporter,
                        SkStream* bufferedStream, bool shouldSucceed) {
    const bool success = bufferedStream->rewind();
    REPORTER_ASSERT(reporter, success == shouldSucceed);
}

// Test that hasLength() returns the correct value, based on the stream
// being wrapped. A length can only be known if the wrapped stream has a
// length and it has a position (so its initial position can be taken into
// account when computing the length).
static void test_hasLength(skiatest::Reporter* reporter,
                           const SkStream& bufferedStream,
                           const SkStream& streamBeingBuffered) {
    if (streamBeingBuffered.hasLength() && streamBeingBuffered.hasPosition()) {
        REPORTER_ASSERT(reporter, bufferedStream.hasLength());
    } else {
        REPORTER_ASSERT(reporter, !bufferedStream.hasLength());
    }
}

// All tests will buffer this string, and compare output to the original.
// The string is long to ensure that all of our lengths being tested are
// smaller than the string length.
const char gAbcs[] = "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwx";

// Tests reading the stream across boundaries of what has been buffered so far and what
// the total buffer size is.
static void test_incremental_buffering(skiatest::Reporter* reporter, size_t bufferSize) {
    // NOTE: For this and other tests in this file, we cheat and continue to refer to the
    // wrapped stream, but that's okay because we know the wrapping stream has not been
    // deleted yet (and we only call const methods in it).
    SkMemoryStream* memStream = SkNEW_ARGS(SkMemoryStream, (gAbcs, strlen(gAbcs), false));

    SkAutoTDelete<SkStream> bufferedStream(SkFrontBufferedStream::Create(memStream, bufferSize));
    test_hasLength(reporter, *bufferedStream.get(), *memStream);

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
    SkMemoryStream* memStream = SkNEW_ARGS(SkMemoryStream, (gAbcs, strlen(gAbcs), false));
    SkAutoTDelete<SkStream> bufferedStream(SkFrontBufferedStream::Create(memStream, bufferSize));
    test_hasLength(reporter, *bufferedStream.get(), *memStream);

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
    SkMemoryStream* memStream = SkNEW_ARGS(SkMemoryStream, (gAbcs, strlen(gAbcs), false));
    SkAutoTDelete<SkStream> bufferedStream(SkFrontBufferedStream::Create(memStream, bufferSize));
    test_hasLength(reporter, *bufferedStream.get(), *memStream);

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

    size_t read(void* dst, size_t requested) override {
        size_t bytesRead = this->INHERITED::read(dst, requested);
        if (bytesRead < requested) {
            fIsAtEnd = true;
        }
        return bytesRead;
    }

    bool isAtEnd() const override {
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
    AndroidLikeMemoryStream* memStream = SkNEW_ARGS(AndroidLikeMemoryStream, ((void*)gAbcs, bufferSize, false));

    // Create a buffer that matches the length of the stream.
    SkAutoTDelete<SkStream> bufferedStream(SkFrontBufferedStream::Create(memStream, bufferSize));
    test_hasLength(reporter, *bufferedStream.get(), *memStream);

    // Attempt to read one more than the bufferSize
    test_read(reporter, bufferedStream.get(), gAbcs, bufferSize + 1);
    test_rewind(reporter, bufferedStream.get(), true);

    // Ensure that the initial read did not invalidate the buffer.
    test_read(reporter, bufferedStream, gAbcs, bufferSize);
}

// Dummy stream that optionally has a length and/or position. Tests that FrontBufferedStream's
// length depends on the stream it's buffering having a length and position.
class LengthOptionalStream : public SkStream {
public:
    LengthOptionalStream(bool hasLength, bool hasPosition)
        : fHasLength(hasLength)
        , fHasPosition(hasPosition)
    {}

    bool hasLength() const override {
        return fHasLength;
    }

    bool hasPosition() const override {
        return fHasPosition;
    }

    size_t read(void*, size_t) override {
        return 0;
    }

    bool isAtEnd() const override {
        return true;
    }

private:
    const bool fHasLength;
    const bool fHasPosition;
};

// Test all possible combinations of the wrapped stream having a length and a position.
static void test_length_combos(skiatest::Reporter* reporter, size_t bufferSize) {
    for (int hasLen = 0; hasLen <= 1; hasLen++) {
        for (int hasPos = 0; hasPos <= 1; hasPos++) {
            LengthOptionalStream* stream = SkNEW_ARGS(LengthOptionalStream, (SkToBool(hasLen), SkToBool(hasPos)));
            SkAutoTDelete<SkStream> buffered(SkFrontBufferedStream::Create(stream, bufferSize));
            test_hasLength(reporter, *buffered.get(), *stream);
        }
    }
}

// Test using a stream with an initial offset.
static void test_initial_offset(skiatest::Reporter* reporter, size_t bufferSize) {
    SkMemoryStream* memStream = SkNEW_ARGS(SkMemoryStream, (gAbcs, strlen(gAbcs), false));

    // Skip a few characters into the memStream, so that bufferedStream represents an offset into
    // the stream it wraps.
    const size_t arbitraryOffset = 17;
    memStream->skip(arbitraryOffset);
    SkAutoTDelete<SkStream> bufferedStream(SkFrontBufferedStream::Create(memStream, bufferSize));

    // Since SkMemoryStream has a length and a position, bufferedStream must also.
    REPORTER_ASSERT(reporter, bufferedStream->hasLength());

    const size_t amountToRead = 10;
    const size_t bufferedLength = bufferedStream->getLength();
    size_t currentPosition = bufferedStream->getPosition();
    REPORTER_ASSERT(reporter, 0 == currentPosition);

    // Read the stream in chunks. After each read, the position must match currentPosition,
    // which sums the amount attempted to read, unless the end of the stream has been reached.
    // Importantly, the end should not have been reached until currentPosition == bufferedLength.
    while (currentPosition < bufferedLength) {
        REPORTER_ASSERT(reporter, !bufferedStream->isAtEnd());
        test_read(reporter, bufferedStream, gAbcs + arbitraryOffset + currentPosition,
                  amountToRead);
        currentPosition = SkTMin(currentPosition + amountToRead, bufferedLength);
        REPORTER_ASSERT(reporter, bufferedStream->getPosition() == currentPosition);
    }
    REPORTER_ASSERT(reporter, bufferedStream->isAtEnd());
    REPORTER_ASSERT(reporter, bufferedLength == currentPosition);
}

static void test_buffers(skiatest::Reporter* reporter, size_t bufferSize) {
    test_incremental_buffering(reporter, bufferSize);
    test_perfectly_sized_buffer(reporter, bufferSize);
    test_skipping(reporter, bufferSize);
    test_read_beyond_buffer(reporter, bufferSize);
    test_length_combos(reporter, bufferSize);
    test_initial_offset(reporter, bufferSize);
}

DEF_TEST(FrontBufferedStream, reporter) {
    // Test 6 and 64, which are used by Android, as well as another arbitrary length.
    test_buffers(reporter, 6);
    test_buffers(reporter, 15);
    test_buffers(reporter, 64);
}

// Test that a FrontBufferedStream does not allow reading after the end of a stream.
// This class is a dummy SkStream which reports that it is at the end on the first
// read (simulating a failure). Then it tracks whether someone calls read() again.
class FailingStream : public SkStream {
public:
    FailingStream()
    : fAtEnd(false)
    , fReadAfterEnd(false)
    {}
    size_t read(void* buffer, size_t size) override {
        if (fAtEnd) {
            fReadAfterEnd = true;
        } else {
            fAtEnd = true;
        }
        return 0;
    }

    bool isAtEnd() const override {
        return fAtEnd;
    }

    bool readAfterEnd() const {
        return fReadAfterEnd;
    }
private:
    bool fAtEnd;
    bool fReadAfterEnd;
};

DEF_TEST(ShortFrontBufferedStream, reporter) {
    FailingStream* failingStream = SkNEW(FailingStream);
    SkAutoTDelete<SkStreamRewindable> stream(SkFrontBufferedStream::Create(failingStream, 64));
    SkBitmap bm;
    // The return value of DecodeStream is not important. We are just using DecodeStream because
    // it simulates a bug. DecodeStream will read the stream, then rewind, then attempt to read
    // again. FrontBufferedStream::read should not continue to read its underlying stream beyond
    // its end.
    SkImageDecoder::DecodeStream(stream, &bm);
    REPORTER_ASSERT(reporter, !failingStream->readAfterEnd());
}
