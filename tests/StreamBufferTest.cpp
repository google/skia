/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkData.h"
#include "include/core/SkStream.h"
#include "src/codec/SkStreamBuffer.h"
#include "src/core/SkMakeUnique.h"
#include "src/utils/SkOSPath.h"

#include "tests/FakeStreams.h"
#include "tests/Test.h"

static const char* gText = "Four score and seven years ago";

static void test_get_data_at_position(skiatest::Reporter* r, SkStreamBuffer* buffer, size_t position,
                                    size_t length) {
    sk_sp<SkData> data = buffer->getDataAtPosition(position, length);
    REPORTER_ASSERT(r, data);
    if (data) {
        REPORTER_ASSERT(r, !memcmp(data->data(), gText + position, length));
    }
}

// Test buffering from the beginning, by different amounts.
static void test_buffer_from_beginning(skiatest::Reporter* r, std::unique_ptr<SkStream> stream,
                                       size_t length) {
    if (!stream) {
        return;
    }
    SkStreamBuffer buffer(std::move(stream));

    // Buffer an arbitrary amount:
    size_t buffered = length / 2;
    REPORTER_ASSERT(r, buffer.buffer(buffered));
    REPORTER_ASSERT(r, !memcmp(buffer.get(), gText, buffered));

    // Buffering less is free:
    REPORTER_ASSERT(r, buffer.buffer(buffered / 2));

    // Buffer more should succeed:
    REPORTER_ASSERT(r, buffer.buffer(length));
    REPORTER_ASSERT(r, !memcmp(buffer.get(), gText, length));
}

// Test flushing the stream as we read.
static void test_flushing(skiatest::Reporter* r, std::unique_ptr<SkStream> stream, size_t length,
                          bool getDataAtPosition) {
    if (!stream) {
        return;
    }
    SkStreamBuffer buffer(std::move(stream));
    const size_t step = 5;
    for (size_t position = 0; position + step <= length; position += step) {
        REPORTER_ASSERT(r, buffer.buffer(step));
        REPORTER_ASSERT(r, buffer.markPosition() == position);

        if (!getDataAtPosition) {
            REPORTER_ASSERT(r, !memcmp(buffer.get(), gText + position, step));
        }
        buffer.flush();
    }

    REPORTER_ASSERT(r, !buffer.buffer(step));

    if (getDataAtPosition) {
        for (size_t position = 0; position + step <= length; position += step) {
            test_get_data_at_position(r, &buffer, position, step);
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
        if (!writer.isValid()) {
            ERRORF(r, "unable to write to '%s'\n", path.c_str());
            return;
        }
        writer.write(gText, size);
    }

    struct {
        std::function<std::unique_ptr<SkStream>()>  createStream;
        bool                                        skipIfNoTmpDir;
    } factories[] = {
        { [&data]() { return skstd::make_unique<SkMemoryStream>(data); },       false  },
        { [&data]() { return skstd::make_unique<NotAssetMemStream>(data); },    false  },
        { [&path]() { return path.isEmpty()
                             ? nullptr
                             : skstd::make_unique<SkFILEStream>(path.c_str()); }, true },
    };

    for (auto f : factories) {
        if (tmpDir.isEmpty() && f.skipIfNoTmpDir) {
            continue;
        }
        test_buffer_from_beginning(r, f.createStream(), size);
        test_flushing(r, f.createStream(), size, false);
        test_flushing(r, f.createStream(), size, true);
    }

    // Stream that will receive more data. Will be owned by the SkStreamBuffer.
    auto halting = skstd::make_unique<HaltingStream>(data, 6);
    HaltingStream* peekHalting = halting.get();
    SkStreamBuffer buffer(std::move(halting));

    // Can only buffer less than what's available (6).
    REPORTER_ASSERT(r, !buffer.buffer(7));
    REPORTER_ASSERT(r, buffer.buffer(5));
    REPORTER_ASSERT(r, !memcmp(buffer.get(), gText, 5));

    // Add some more data. We can buffer and read all of it.
    peekHalting->addNewData(8);
    REPORTER_ASSERT(r, buffer.buffer(14));
    REPORTER_ASSERT(r, !memcmp(buffer.get(), gText, 14));

    // Flush the buffer, which moves the position.
    buffer.flush();

    // Add some data, and try to read more. Can only read what is
    // available.
    peekHalting->addNewData(9);
    REPORTER_ASSERT(r, !buffer.buffer(13));
    peekHalting->addNewData(4);
    REPORTER_ASSERT(r, buffer.buffer(13));

    // Do not call get on this data. We'll come back to this data after adding
    // more.
    buffer.flush();
    const size_t remaining = size - 27;
    REPORTER_ASSERT(r, remaining > 0);
    peekHalting->addNewData(remaining);
    REPORTER_ASSERT(r, buffer.buffer(remaining));
    REPORTER_ASSERT(r, !memcmp(buffer.get(), gText + 27, remaining));

    // Now go back to the data we skipped.
    test_get_data_at_position(r, &buffer, 14, 13);
}
