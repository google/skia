/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkData.h"
#include "SkOSPath.h"
#include "SkStream.h"
#include "SkStreamBuffer.h"

#include "FakeStreams.h"
#include "Test.h"

static const char* gText = "Four score and seven years ago";

static void test_read_from_position(skiatest::Reporter* r, SkStreamBuffer* buffer, size_t position,
                                    size_t length) {
    SkAutoMalloc storage(length);
    REPORTER_ASSERT(r, buffer->readFromPosition(storage.get(), position, length));
    REPORTER_ASSERT(r, !memcmp(storage.get(), gText + position, length));
}

// Test buffering from the beginning, by different amounts.
static void test_buffer_from_beginning(skiatest::Reporter* r, SkStream* stream, size_t length,
                                       bool rewindable) {
    SkStreamBuffer buffer(stream);
    REPORTER_ASSERT(r, buffer.rewindable() == rewindable);

    // Buffer an arbitrary amount:
    size_t buffered = length / 2;
    REPORTER_ASSERT(r, buffer.buffer(buffered));
    REPORTER_ASSERT(r, !memcmp(buffer.get(), gText, buffered));
    if (rewindable) {
        REPORTER_ASSERT(r, buffer.getPosition() == 0);
    }

    // Buffering less is free:
    REPORTER_ASSERT(r, buffer.buffer(buffered / 2));
    if (rewindable) {
        // And should not change the position
        REPORTER_ASSERT(r, buffer.getPosition() == 0);
    }

    // Buffer more should succeed:
    REPORTER_ASSERT(r, buffer.buffer(length));
    REPORTER_ASSERT(r, !memcmp(buffer.get(), gText, length));
    if (rewindable) {
        REPORTER_ASSERT(r, buffer.getPosition() == 0);

        test_read_from_position(r, &buffer, 0, length);
        test_read_from_position(r, &buffer, length / 2, length / 4);
        test_read_from_position(r, &buffer, length / 3, length / 5);
    }
}

// Test flushing the stream as we read.
static void test_flushing(skiatest::Reporter* r, SkStream* stream, size_t length, bool rewindable) {
    SkStreamBuffer buffer(stream);
    const size_t step = 5;
    for (size_t position = 0; position + step <= length; position += step) {
        REPORTER_ASSERT(r, buffer.buffer(step));
        if (rewindable) {
            REPORTER_ASSERT(r, buffer.getPosition() == position);
        }

        REPORTER_ASSERT(r, !memcmp(buffer.get(), gText + position, step));
        buffer.flush();
    }

    REPORTER_ASSERT(r, !buffer.buffer(step));

    if (rewindable) {
        const size_t differentStep = 7;
        for (size_t position = 0; position + differentStep <= length; position += differentStep) {
            test_read_from_position(r, &buffer, position, differentStep);
        }
    }
}

DEF_TEST(StreamBuffer, r) {
    const size_t size = strlen(gText);
    sk_sp<SkData> data(SkData::MakeWithoutCopy(gText, size));

    SkString tmpDir = skiatest::GetTmpDir();
    const char* subdir = "streamBuffer.txt";
    SkString path;

    if (!tmpDir.isEmpty()) {
        path = SkOSPath::Join(tmpDir.c_str(), subdir);
        SkFILEWStream writer(path.c_str());
        writer.write(gText, size);
    }

    struct {
        std::function<SkStream*()> createStream;
        bool                       rewindable;
        bool                       skipIfNoTmpDir;
    } factories[] = {
        { [&data]() { return new SkMemoryStream(data); },       true,  false },
        { [&data]() { return new NotAssetMemStream(data); },    false, false },
        { [&path]() { return new SkFILEStream(path.c_str()); }, true,  true  },
    };

    for (auto f : factories) {
        if (tmpDir.isEmpty() && f.skipIfNoTmpDir) {
            continue;
        }
        test_buffer_from_beginning(r, f.createStream(), size, f.rewindable);
        test_flushing(r, f.createStream(), size, f.rewindable);
    }

    // Stream that will receive more data. Will be owned by the SkStreamBuffer.
    HaltingStream* stream = new HaltingStream(data, 6);
    SkStreamBuffer buffer(stream);
    REPORTER_ASSERT(r, buffer.rewindable());

    // Can only buffer less than what's available (6).
    REPORTER_ASSERT(r, !buffer.buffer(7));
    REPORTER_ASSERT(r, buffer.buffer(5));
    REPORTER_ASSERT(r, !memcmp(buffer.get(), gText, 5));
    REPORTER_ASSERT(r, buffer.getPosition() == 0);

    // Add some more data. We can buffer and read all of it.
    stream->addNewData(8);
    REPORTER_ASSERT(r, buffer.buffer(14));
    REPORTER_ASSERT(r, !memcmp(buffer.get(), gText, 14));
    REPORTER_ASSERT(r, buffer.getPosition() == 0);

    // Flush the buffer, which moves the position.
    buffer.flush();
    REPORTER_ASSERT(r, buffer.getPosition() == 14);

    // Add some data, and try to read more. Can only read what is
    // available.
    stream->addNewData(9);
    REPORTER_ASSERT(r, !buffer.buffer(13));
    stream->addNewData(4);
    REPORTER_ASSERT(r, buffer.buffer(13));

    // Do not call get on this data. We'll come back to this data after adding
    // more.
    buffer.flush();
    const size_t remaining = size - 27;
    REPORTER_ASSERT(r, remaining > 0);
    stream->addNewData(remaining);
    REPORTER_ASSERT(r, buffer.buffer(remaining));
    REPORTER_ASSERT(r, !memcmp(buffer.get(), gText + 27, remaining));

    // Now go back to the data we skipped.
    test_read_from_position(r, &buffer, 14, 13);
}
