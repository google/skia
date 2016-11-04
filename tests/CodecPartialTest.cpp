/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkCodec.h"
#include "SkData.h"
#include "SkImageInfo.h"
#include "SkRWBuffer.h"
#include "SkString.h"

#include "Resources.h"
#include "Test.h"

static sk_sp<SkData> make_from_resource(const char* name) {
    SkString fullPath = GetResourcePath(name);
    return SkData::MakeFromFileName(fullPath.c_str());
}

static SkImageInfo standardize_info(SkCodec* codec) {
    SkImageInfo defaultInfo = codec->getInfo();
    // Note: This drops the SkColorSpace, allowing the equality check between two
    // different codecs created from the same file to have the same SkImageInfo.
    return SkImageInfo::MakeN32Premul(defaultInfo.width(), defaultInfo.height());
}

static bool create_truth(sk_sp<SkData> data, SkBitmap* dst) {
    SkAutoTDelete<SkCodec> codec(SkCodec::NewFromData(std::move(data)));
    if (!codec) {
        return false;
    }

    const SkImageInfo info = standardize_info(codec);
    dst->allocPixels(info);
    return SkCodec::kSuccess == codec->getPixels(info, dst->getPixels(), dst->rowBytes());
}

/*
 *  Represents a stream without all of its data.
 */
class HaltingStream : public SkStream {
public:
    HaltingStream(sk_sp<SkData> data)
        : fTotalSize(data->size())
        , fLimit(fTotalSize / 2)
        , fStream(std::move(data))
    {}

    void addNewData() {
        // Arbitrary size, but deliberately different from
        // the buffer size used by SkPngCodec.
        fLimit = SkTMin(fTotalSize, fLimit + 1000);
    }

    size_t read(void* buffer, size_t size) override {
        if (fStream.getPosition() + size > fLimit) {
            size = fLimit - fStream.getPosition();
        }

        return fStream.read(buffer, size);
    }

    bool isAtEnd() const override {
        return fStream.isAtEnd();
    }

    bool hasPosition() const override { return true; }
    size_t getPosition() const override { return fStream.getPosition(); }
    bool rewind() override { return fStream.rewind(); }
    bool move(long offset) override { return fStream.move(offset); }

private:
    const size_t    fTotalSize;
    size_t          fLimit;
    SkMemoryStream  fStream;
};

static void test_partial(skiatest::Reporter* r, const char* name) {
    sk_sp<SkData> file = make_from_resource(name);
    if (!file) {
        SkDebugf("missing resource %s\n", name);
        return;
    }

    SkBitmap truth;
    if (!create_truth(file, &truth)) {
        ERRORF(r, "Failed to decode %s\n", name);
        return;
    }

    const size_t fileSize = file->size();

    // Now decode part of the file
    HaltingStream* stream = new HaltingStream(file);

    // Note that we cheat and hold on to a pointer to stream, though it is owned by
    // partialCodec.
    SkAutoTDelete<SkCodec> partialCodec(SkCodec::NewFromStream(stream));
    if (!partialCodec) {
        // Technically, this could be a small file where half the file is not
        // enough.
        ERRORF(r, "Failed to create codec for %s", name);
        return;
    }

    const SkImageInfo info = standardize_info(partialCodec);
    SkASSERT(info == truth.info());
    SkBitmap incremental;
    incremental.allocPixels(info);

    const SkCodec::Result startResult = partialCodec->startIncrementalDecode(info,
            incremental.getPixels(), incremental.rowBytes());
    if (startResult != SkCodec::kSuccess) {
        ERRORF(r, "Failed to start incremental decode\n");
        return;
    }

    while (true) {
        const SkCodec::Result result = partialCodec->incrementalDecode();

        if (stream->getPosition() == fileSize) {
            REPORTER_ASSERT(r, result == SkCodec::kSuccess);
            break;
        }

        SkASSERT(stream->getPosition() < fileSize);

        REPORTER_ASSERT(r, result == SkCodec::kIncompleteInput);

        // Append an arbitrary amount of data.
        stream->addNewData();
    }

    // compare to original
    for (int i = 0; i < info.height(); i++) {
        REPORTER_ASSERT(r, !memcmp(truth.getAddr(0, 0), incremental.getAddr(0, 0),
                                   info.minRowBytes()));
    }
}

DEF_TEST(Codec_partial, r) {
    test_partial(r, "plane.png");
    test_partial(r, "plane_interlaced.png");
    test_partial(r, "yellow_rose.png");
    test_partial(r, "index8.png");
    test_partial(r, "color_wheel.png");
    test_partial(r, "mandrill_256.png");
    test_partial(r, "mandrill_32.png");
    test_partial(r, "arrow.png");
    test_partial(r, "randPixels.png");
    test_partial(r, "baby_tux.png");
}

// Test that calling getPixels when an incremental decode has been
// started (but not finished) makes the next call to incrementalDecode
// require a call to startIncrementalDecode.
static void test_interleaved(skiatest::Reporter* r, const char* name) {
    sk_sp<SkData> file = make_from_resource(name);
    SkAutoTDelete<SkCodec> partialCodec(SkCodec::NewFromStream(
            new HaltingStream(std::move(file))));
    if (!partialCodec) {
        ERRORF(r, "Failed to create codec for %s", name);
        return;
    }

    const SkImageInfo info = standardize_info(partialCodec);
    SkBitmap incremental;
    incremental.allocPixels(info);

    const SkCodec::Result startResult = partialCodec->startIncrementalDecode(info,
            incremental.getPixels(), incremental.rowBytes());
    if (startResult != SkCodec::kSuccess) {
        ERRORF(r, "Failed to start incremental decode\n");
        return;
    }

    SkCodec::Result result = partialCodec->incrementalDecode();
    REPORTER_ASSERT(r, result == SkCodec::kIncompleteInput);

    SkBitmap full;
    full.allocPixels(info);
    result = partialCodec->getPixels(info, full.getPixels(), full.rowBytes());
    REPORTER_ASSERT(r, result == SkCodec::kIncompleteInput);

    // Now incremental decode will fail
    result = partialCodec->incrementalDecode();
    REPORTER_ASSERT(r, result == SkCodec::kInvalidParameters);
}

DEF_TEST(Codec_rewind, r) {
    test_interleaved(r, "plane.png");
    test_interleaved(r, "plane_interlaced.png");
}
