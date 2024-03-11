/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkData.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkTemplates.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkAutoMalloc.h"
#include "src/base/SkBuffer.h"
#include "src/base/SkRandom.h"
#include "src/core/SkOSFile.h"
#include "src/core/SkStreamPriv.h"
#include "src/utils/SkOSPath.h"
#include "tests/Test.h"
#include "tools/Resources.h"

#include <algorithm>
#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <functional>
#include <limits>
#include <memory>
#include <string>

using namespace skia_private;

#ifdef SK_ENABLE_ANDROID_UTILS
#include "client_utils/android/FrontBufferedStream.h"
#endif

#ifndef SK_BUILD_FOR_WIN
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

        std::unique_ptr<SkStreamAsset> stream2(stream.duplicate());
        test_loop_stream(reporter, stream2.get(), s, 26, 100);
    }

    {
        FILE* file = ::fopen(path.c_str(), "rb");
        SkFILEStream stream(file);
        REPORTER_ASSERT(reporter, stream.isValid());
        test_loop_stream(reporter, &stream, s, 26, 100);

        std::unique_ptr<SkStreamAsset> stream2(stream.duplicate());
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
    REPORTER_ASSERT(reporter, ds.bytesWritten() == 100 * 26);

    char* dst = new char[100 * 26 + 1];
    dst[100*26] = '*';
    ds.copyTo(dst);
    REPORTER_ASSERT(reporter, dst[100*26] == '*');
    for (i = 0; i < 100; i++) {
        REPORTER_ASSERT(reporter, memcmp(&dst[i * 26], s, 26) == 0);
    }

    {
        std::unique_ptr<SkStreamAsset> stream(ds.detachAsStream());
        REPORTER_ASSERT(reporter, 100 * 26 == stream->getLength());
        REPORTER_ASSERT(reporter, ds.bytesWritten() == 0);
        test_loop_stream(reporter, stream.get(), s, 26, 100);

        std::unique_ptr<SkStreamAsset> stream2(stream->duplicate());
        test_loop_stream(reporter, stream2.get(), s, 26, 100);

        std::unique_ptr<SkStreamAsset> stream3(stream->fork());
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
    REPORTER_ASSERT(reporter, ds.bytesWritten() == 100 * 26);

    {
        // Test that this works after a snapshot.
        std::unique_ptr<SkStreamAsset> stream(ds.detachAsStream());
        REPORTER_ASSERT(reporter, ds.bytesWritten() == 0);
        test_loop_stream(reporter, stream.get(), s, 26, 100);

        std::unique_ptr<SkStreamAsset> stream2(stream->duplicate());
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
    SkDynamicMemoryWStream wstream;

    for (i = 0; i < std::size(sizes); ++i) {
        bool success = wstream.writePackedUInt(sizes[i]);
        REPORTER_ASSERT(reporter, success);
    }

    std::unique_ptr<SkStreamAsset> rstream(wstream.detachAsStream());
    for (i = 0; i < std::size(sizes); ++i) {
        size_t n;
        if (!rstream->readPackedUInt(&n)) {
            ERRORF(reporter, "[%zu] sizes:%zx could not be read\n", i, sizes[i]);
        }
        if (sizes[i] != n) {
            ERRORF(reporter, "[%zu] sizes:%zx != n:%zx\n", i, sizes[i], n);
        }
    }
}

// Test that setting an SkMemoryStream to a nullptr data does not result in a crash when calling
// methods that access fData.
static void TestDereferencingData(SkMemoryStream* memStream) {
    memStream->read(nullptr, 0);
    memStream->getMemoryBase();
    (void)memStream->getData();
}

static void TestNullData() {
    SkMemoryStream memStream(nullptr);
    TestDereferencingData(&memStream);

    memStream.setData(nullptr);
    TestDereferencingData(&memStream);

}

DEF_TEST(Stream, reporter) {
    TestWStream(reporter);
    TestPackedUInt(reporter);
    TestNullData();
}

#ifndef SK_BUILD_FOR_IOS
/**
 *  Tests peeking and then reading the same amount. The two should provide the
 *  same results.
 *  Returns the amount successfully read minus the amount successfully peeked.
 */
static size_t compare_peek_to_read(skiatest::Reporter* reporter,
                                   SkStream* stream, size_t bytesToPeek) {
    // The rest of our tests won't be very interesting if bytesToPeek is zero.
    REPORTER_ASSERT(reporter, bytesToPeek > 0);
    SkAutoMalloc peekStorage(bytesToPeek);
    SkAutoMalloc readStorage(bytesToPeek);
    void* peekPtr = peekStorage.get();
    void* readPtr = peekStorage.get();

    const size_t bytesPeeked = stream->peek(peekPtr, bytesToPeek);
    const size_t bytesRead = stream->read(readPtr, bytesToPeek);

    // bytesRead should only be less than attempted if the stream is at the
    // end.
    REPORTER_ASSERT(reporter, bytesRead == bytesToPeek || stream->isAtEnd());

    // peek and read should behave the same, except peek returned to the
    // original position, so they read the same data.
    REPORTER_ASSERT(reporter, !memcmp(peekPtr, readPtr, bytesPeeked));

    // A stream should never be able to peek more than it can read.
    REPORTER_ASSERT(reporter, bytesRead >= bytesPeeked);

    return bytesRead - bytesPeeked;
}

static void test_fully_peekable_stream(skiatest::Reporter* r, SkStream* stream, size_t limit) {
    for (size_t i = 1; !stream->isAtEnd(); i++) {
        REPORTER_ASSERT(r, compare_peek_to_read(r, stream, i) == 0);
    }
}

#ifdef SK_ENABLE_ANDROID_UTILS
static void test_peeking_front_buffered_stream(skiatest::Reporter* r,
                                               const SkStream& original,
                                               size_t bufferSize) {
    std::unique_ptr<SkStream> dupe(original.duplicate());
    REPORTER_ASSERT(r, dupe != nullptr);
    auto bufferedStream = android::skia::FrontBufferedStream::Make(
            std::move(dupe), bufferSize);
    REPORTER_ASSERT(r, bufferedStream != nullptr);

    size_t peeked = 0;
    for (size_t i = 1; !bufferedStream->isAtEnd(); i++) {
        const size_t unpeekableBytes = compare_peek_to_read(r, bufferedStream.get(), i);
        if (unpeekableBytes > 0) {
            // This could not have returned a number greater than i.
            REPORTER_ASSERT(r, unpeekableBytes <= i);

            // We have reached the end of the buffer. Verify that it was at least
            // bufferSize.
            REPORTER_ASSERT(r, peeked + i - unpeekableBytes >= bufferSize);
            // No more peeking is supported.
            break;
        }
        peeked += i;
    }

    // Test that attempting to peek beyond the length of the buffer does not prevent rewinding.
    bufferedStream = android::skia::FrontBufferedStream::Make(original.duplicate(), bufferSize);
    REPORTER_ASSERT(r, bufferedStream != nullptr);

    const size_t bytesToPeek = bufferSize + 1;
    SkAutoMalloc peekStorage(bytesToPeek);
    SkAutoMalloc readStorage(bytesToPeek);

    for (size_t start = 0; start <= bufferSize; start++) {
        // Skip to the starting point
        REPORTER_ASSERT(r, bufferedStream->skip(start) == start);

        const size_t bytesPeeked = bufferedStream->peek(peekStorage.get(), bytesToPeek);
        if (0 == bytesPeeked) {
            // Peeking should only fail completely if we have read/skipped beyond the buffer.
            REPORTER_ASSERT(r, start >= bufferSize);
            break;
        }

        // Only read the amount that was successfully peeked.
        const size_t bytesRead = bufferedStream->read(readStorage.get(), bytesPeeked);
        REPORTER_ASSERT(r, bytesRead == bytesPeeked);
        REPORTER_ASSERT(r, !memcmp(peekStorage.get(), readStorage.get(), bytesPeeked));

        // This should be safe to rewind.
        REPORTER_ASSERT(r, bufferedStream->rewind());
    }
}
#endif

// This test uses file system operations that don't work out of the
// box on iOS. It's likely that we don't need them on iOS. Ignoring for now.
// TODO(stephana): Re-evaluate if we need this in the future.
DEF_TEST(StreamPeek, reporter) {
    // Test a memory stream.
    const char gAbcs[] = "abcdefghijklmnopqrstuvwxyz";
    SkMemoryStream memStream(gAbcs, strlen(gAbcs), false);
    test_fully_peekable_stream(reporter, &memStream, memStream.getLength());

    // Test an arbitrary file stream. file streams do not support peeking.
    auto tmpdir = skiatest::GetTmpDir();
    if (tmpdir.isEmpty()) {
        ERRORF(reporter, "no tmp dir!");
        return;
    }
    auto path = SkOSPath::Join(tmpdir.c_str(), "file");
    {
        SkFILEWStream wStream(path.c_str());
        constexpr char filename[] = "images/baby_tux.webp";
        auto data = GetResourceAsData(filename);
        if (!data || data->size() == 0) {
            ERRORF(reporter, "resource missing: %s\n", filename);
            return;
        }
        if (!wStream.isValid() || !wStream.write(data->data(), data->size())) {
            ERRORF(reporter, "error wrtiting to file %s", path.c_str());
            return;
        }
    }
    SkFILEStream fileStream(path.c_str());
    REPORTER_ASSERT(reporter, fileStream.isValid());
    if (!fileStream.isValid()) {
        return;
    }
    SkAutoMalloc storage(fileStream.getLength());
    for (size_t i = 1; i < fileStream.getLength(); i++) {
        REPORTER_ASSERT(reporter, fileStream.peek(storage.get(), i) == 0);
    }

#ifdef SK_ENABLE_ANDROID_UTILS
    // Now test some FrontBufferedStreams
    for (size_t i = 1; i < memStream.getLength(); i++) {
        test_peeking_front_buffered_stream(reporter, memStream, i);
    }
#endif
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
                SkToU32(std::min(sizeof(buffer), asset->getLength() - i));
        size_t size = rand.nextRangeU(1, maxSize);
        SkASSERT(size >= 1);
        SkASSERT(size <= sizeof(buffer));
        SkASSERT(size + i <= asset->getLength());
        if (asset->peek(buffer, size) < size) {
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
    size_t totalWritten = 0;
    for (int i = 0; i < 32; ++i) {
        // Randomize the length of the blocks.
        size_t size = rand.nextRangeU(1, sizeof(buffer));
        for (size_t j = 0; j < size; ++j) {
            buffer[j] = valueSource.nextU() & 0xFF;
        }
        dynamicMemoryWStream.write(buffer, size);
        totalWritten += size;
        REPORTER_ASSERT(rep, totalWritten == dynamicMemoryWStream.bytesWritten());
    }
    std::unique_ptr<SkStreamAsset> asset(dynamicMemoryWStream.detachAsStream());
    sk_sp<SkData> expected(SkData::MakeUninitialized(asset->getLength()));
    uint8_t* expectedPtr = static_cast<uint8_t*>(expected->writable_data());
    valueSource.setSeed(kSeed);  // reseed.
    // We want the exact same same "random" string of numbers to put
    // in expected. i.e.: don't rely on SkDynamicMemoryStream to work
    // correctly while we are testing SkDynamicMemoryStream.
    for (size_t i = 0; i < asset->getLength(); ++i) {
        expectedPtr[i] = valueSource.nextU() & 0xFF;
    }
    stream_peek_test(rep, asset.get(), expected.get());
}

DEF_TEST(StreamRemainingLengthIsBelow_MemoryStream, rep) {
    SkMemoryStream stream(100);
    REPORTER_ASSERT(rep, !StreamRemainingLengthIsBelow(&stream, 0));
    REPORTER_ASSERT(rep, !StreamRemainingLengthIsBelow(&stream, 90));
    REPORTER_ASSERT(rep, !StreamRemainingLengthIsBelow(&stream, 100));

    REPORTER_ASSERT(rep, StreamRemainingLengthIsBelow(&stream, 101));
    REPORTER_ASSERT(rep, StreamRemainingLengthIsBelow(&stream, ULONG_MAX));

    uint8_t buff[75];
    REPORTER_ASSERT(rep, stream.read(buff, 75) == 75);

    REPORTER_ASSERT(rep, !StreamRemainingLengthIsBelow(&stream, 0));
    REPORTER_ASSERT(rep, !StreamRemainingLengthIsBelow(&stream, 24));
    REPORTER_ASSERT(rep, !StreamRemainingLengthIsBelow(&stream, 25));

    REPORTER_ASSERT(rep, StreamRemainingLengthIsBelow(&stream, 26));
    REPORTER_ASSERT(rep, StreamRemainingLengthIsBelow(&stream, 100));
    REPORTER_ASSERT(rep, StreamRemainingLengthIsBelow(&stream, ULONG_MAX));
}

namespace {
class DumbStream : public SkStream {
public:
    DumbStream(const uint8_t* data, size_t n)
        : fData(data), fCount(n), fIdx(0) {}
    size_t read(void* buffer, size_t size) override {
        size_t copyCount = std::min(fCount - fIdx, size);
        if (copyCount) {
            memcpy(buffer, &fData[fIdx], copyCount);
            fIdx += copyCount;
        }
        return copyCount;
    }
    bool isAtEnd() const override {
        return fCount == fIdx;
    }
 private:
    const uint8_t* fData;
    size_t fCount, fIdx;
};
}  // namespace

static void stream_copy_test(skiatest::Reporter* reporter,
                             const void* srcData,
                             size_t N,
                             SkStream* stream) {
    SkDynamicMemoryWStream tgt;
    if (!SkStreamCopy(&tgt, stream)) {
        ERRORF(reporter, "SkStreamCopy failed");
        return;
    }
    sk_sp<SkData> data(tgt.detachAsData());
    if (data->size() != N) {
        ERRORF(reporter, "SkStreamCopy incorrect size");
        return;
    }
    if (0 != memcmp(data->data(), srcData, N)) {
        ERRORF(reporter, "SkStreamCopy bad copy");
    }
}

DEF_TEST(DynamicMemoryWStream_detachAsData, r) {
    const char az[] = "abcdefghijklmnopqrstuvwxyz";
    const unsigned N = 40000;
    SkDynamicMemoryWStream dmws;
    for (unsigned i = 0; i < N; ++i) {
        dmws.writeText(az);
    }
    REPORTER_ASSERT(r, dmws.bytesWritten() == N * strlen(az));
    auto data = dmws.detachAsData();
    REPORTER_ASSERT(r, data->size() == N * strlen(az));
    const uint8_t* ptr = data->bytes();
    for (unsigned i = 0; i < N; ++i) {
        if (0 != memcmp(ptr, az, strlen(az))) {
            ERRORF(r, "detachAsData() memcmp failed");
            return;
        }
        ptr += strlen(az);
    }
}

DEF_TEST(StreamCopy, reporter) {
    SkRandom random(123456);
    static const int N = 10000;
    AutoTMalloc<uint8_t> src((size_t)N);
    for (int j = 0; j < N; ++j) {
        src[j] = random.nextU() & 0xff;
    }
    // SkStreamCopy had two code paths; this test both.
    DumbStream dumbStream(src.get(), (size_t)N);
    stream_copy_test(reporter, src, N, &dumbStream);
    SkMemoryStream smartStream(src.get(), (size_t)N);
    stream_copy_test(reporter, src, N, &smartStream);
}

DEF_TEST(StreamEmptyStreamMemoryBase, r) {
    SkDynamicMemoryWStream tmp;
    std::unique_ptr<SkStreamAsset> asset(tmp.detachAsStream());
    REPORTER_ASSERT(r, nullptr == asset->getMemoryBase());
}

DEF_TEST(FILEStreamWithOffset, r) {
    if (GetResourcePath().isEmpty()) {
        return;
    }

    SkString filename = GetResourcePath("images/baby_tux.png");
    SkFILEStream stream1(filename.c_str());
    if (!stream1.isValid()) {
        ERRORF(r, "Could not create SkFILEStream from %s", filename.c_str());
        return;
    }
    REPORTER_ASSERT(r, stream1.hasLength());
    REPORTER_ASSERT(r, stream1.hasPosition());

    // Seek halfway through the file. The second SkFILEStream will be created
    // with the same filename and offset and therefore will treat that offset as
    // the beginning.
    const size_t size = stream1.getLength();
    const size_t middle = size / 2;
    if (!stream1.seek(middle)) {
        ERRORF(r, "Could not seek SkFILEStream to %zu out of %zu", middle, size);
        return;
    }
    REPORTER_ASSERT(r, stream1.getPosition() == middle);

    FILE* file = sk_fopen(filename.c_str(), kRead_SkFILE_Flag);
    if (!file) {
        ERRORF(r, "Could not open %s as a FILE", filename.c_str());
        return;
    }

    if (fseek(file, (long) middle, SEEK_SET) != 0) {
        ERRORF(r, "Could not fseek FILE to %zu out of %zu", middle, size);
        return;
    }
    SkFILEStream stream2(file);

    const size_t remaining = size - middle;
    AutoTMalloc<uint8_t> expected(remaining);
    REPORTER_ASSERT(r, stream1.read(expected.get(), remaining) == remaining);

    auto test_full_read = [&r, &expected, remaining](SkStream* stream) {
        AutoTMalloc<uint8_t> actual(remaining);
        REPORTER_ASSERT(r, stream->read(actual.get(), remaining) == remaining);
        REPORTER_ASSERT(r, !memcmp(expected.get(), actual.get(), remaining));

        REPORTER_ASSERT(r, stream->getPosition() == stream->getLength());
        REPORTER_ASSERT(r, stream->isAtEnd());
    };

    auto test_rewind = [&r, &expected, remaining](SkStream* stream) {
        // Rewind goes back to original offset.
        REPORTER_ASSERT(r, stream->rewind());
        REPORTER_ASSERT(r, stream->getPosition() == 0);
        AutoTMalloc<uint8_t> actual(remaining);
        REPORTER_ASSERT(r, stream->read(actual.get(), remaining) == remaining);
        REPORTER_ASSERT(r, !memcmp(expected.get(), actual.get(), remaining));
    };

    auto test_move = [&r, &expected, size, remaining](SkStream* stream) {
        // Cannot move to before the original offset.
        REPORTER_ASSERT(r, stream->move(- (long) size));
        REPORTER_ASSERT(r, stream->getPosition() == 0);

        REPORTER_ASSERT(r, stream->move(std::numeric_limits<long>::min()));
        REPORTER_ASSERT(r, stream->getPosition() == 0);

        AutoTMalloc<uint8_t> actual(remaining);
        REPORTER_ASSERT(r, stream->read(actual.get(), remaining) == remaining);
        REPORTER_ASSERT(r, !memcmp(expected.get(), actual.get(), remaining));

        REPORTER_ASSERT(r, stream->isAtEnd());
        REPORTER_ASSERT(r, stream->getPosition() == remaining);

        // Cannot move beyond the end.
        REPORTER_ASSERT(r, stream->move(1));
        REPORTER_ASSERT(r, stream->isAtEnd());
        REPORTER_ASSERT(r, stream->getPosition() == remaining);
    };

    auto test_seek = [&r, &expected, middle, remaining](SkStream* stream) {
        // Seek to an arbitrary position.
        const size_t arbitrary = middle / 2;
        REPORTER_ASSERT(r, stream->seek(arbitrary));
        REPORTER_ASSERT(r, stream->getPosition() == arbitrary);
        const size_t miniRemaining = remaining - arbitrary;
        AutoTMalloc<uint8_t> actual(miniRemaining);
        REPORTER_ASSERT(r, stream->read(actual.get(), miniRemaining) == miniRemaining);
        REPORTER_ASSERT(r, !memcmp(expected.get() + arbitrary, actual.get(), miniRemaining));
    };

    auto test_seek_beginning = [&r, &expected, remaining](SkStream* stream) {
        // Seek to the beginning.
        REPORTER_ASSERT(r, stream->seek(0));
        REPORTER_ASSERT(r, stream->getPosition() == 0);
        AutoTMalloc<uint8_t> actual(remaining);
        REPORTER_ASSERT(r, stream->read(actual.get(), remaining) == remaining);
        REPORTER_ASSERT(r, !memcmp(expected.get(), actual.get(), remaining));
    };

    auto test_seek_end = [&r, remaining](SkStream* stream) {
        // Cannot seek past the end.
        REPORTER_ASSERT(r, stream->isAtEnd());

        REPORTER_ASSERT(r, stream->seek(remaining + 1));
        REPORTER_ASSERT(r, stream->isAtEnd());
        REPORTER_ASSERT(r, stream->getPosition() == remaining);

        const size_t middle = remaining / 2;
        REPORTER_ASSERT(r, stream->seek(middle));
        REPORTER_ASSERT(r, !stream->isAtEnd());
        REPORTER_ASSERT(r, stream->getPosition() == middle);

        REPORTER_ASSERT(r, stream->seek(remaining * 2));
        REPORTER_ASSERT(r, stream->isAtEnd());
        REPORTER_ASSERT(r, stream->getPosition() == remaining);

        REPORTER_ASSERT(r, stream->seek(std::numeric_limits<long>::max()));
        REPORTER_ASSERT(r, stream->isAtEnd());
        REPORTER_ASSERT(r, stream->getPosition() == remaining);
    };


    std::function<void (SkStream* stream, bool recurse)> test_all;
    test_all = [&](SkStream* stream, bool recurse) {
        REPORTER_ASSERT(r, stream->getLength() == remaining);
        REPORTER_ASSERT(r, stream->getPosition() == 0);

        test_full_read(stream);
        test_rewind(stream);
        test_move(stream);
        test_seek(stream);
        test_seek_beginning(stream);
        test_seek_end(stream);

        if (recurse) {
            // Duplicate shares the original offset.
            auto duplicate = stream->duplicate();
            if (!duplicate) {
                ERRORF(r, "Failed to duplicate the stream!");
            } else {
                test_all(duplicate.get(), false);
            }

            // Fork shares the original offset, too.
            auto fork = stream->fork();
            if (!fork) {
                ERRORF(r, "Failed to fork the stream!");
            } else {
                REPORTER_ASSERT(r, fork->isAtEnd());
                REPORTER_ASSERT(r, fork->getLength() == remaining);
                REPORTER_ASSERT(r, fork->rewind());

                test_all(fork.get(), false);
            }
        }
    };

    test_all(&stream2, true);
}

DEF_TEST(RBuffer, reporter) {
    int32_t value = 0;
    SkRBuffer buffer(&value, 4);
    REPORTER_ASSERT(reporter, buffer.isValid());

    int32_t tmp;
    REPORTER_ASSERT(reporter, buffer.read(&tmp, 4));
    REPORTER_ASSERT(reporter, buffer.isValid());

    REPORTER_ASSERT(reporter, !buffer.read(&tmp, 4));
    REPORTER_ASSERT(reporter, !buffer.isValid());
}
