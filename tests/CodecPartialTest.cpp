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
    std::unique_ptr<SkCodec> codec(SkCodec::NewFromData(std::move(data)));
    if (!codec) {
        return false;
    }

    const SkImageInfo info = standardize_info(codec.get());
    dst->allocPixels(info);
    return SkCodec::kSuccess == codec->getPixels(info, dst->getPixels(), dst->rowBytes());
}

static void compare_bitmaps(skiatest::Reporter* r, const SkBitmap& bm1, const SkBitmap& bm2) {
    const SkImageInfo& info = bm1.info();
    if (info != bm2.info()) {
        ERRORF(r, "Bitmaps have different image infos!");
        return;
    }
    const size_t rowBytes = info.minRowBytes();
    for (int i = 0; i < info.height(); i++) {
        REPORTER_ASSERT(r, !memcmp(bm1.getAddr(0, 0), bm2.getAddr(0, 0), rowBytes));
    }
}

/*
 *  Represents a stream without all of its data.
 */
class HaltingStream : public SkStream {
public:
    HaltingStream(sk_sp<SkData> data, size_t initialLimit)
        : fTotalSize(data->size())
        , fLimit(initialLimit)
        , fStream(std::move(data))
    {}

    void addNewData(size_t extra) {
        fLimit = SkTMin(fTotalSize, fLimit + extra);
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

    bool isAllDataReceived() const { return fLimit == fTotalSize; }

private:
    const size_t    fTotalSize;
    size_t          fLimit;
    SkMemoryStream  fStream;
};

static void test_partial(skiatest::Reporter* r, const char* name, size_t minBytes = 0) {
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

    // Now decode part of the file
    HaltingStream* stream = new HaltingStream(file, SkTMax(file->size() / 2, minBytes));

    // Note that we cheat and hold on to a pointer to stream, though it is owned by
    // partialCodec.
    std::unique_ptr<SkCodec> partialCodec(SkCodec::NewFromStream(stream));
    if (!partialCodec) {
        // Technically, this could be a small file where half the file is not
        // enough.
        ERRORF(r, "Failed to create codec for %s", name);
        return;
    }

    const SkImageInfo info = standardize_info(partialCodec.get());
    SkASSERT(info == truth.info());
    SkBitmap incremental;
    incremental.allocPixels(info);

    while (true) {
        const SkCodec::Result startResult = partialCodec->startIncrementalDecode(info,
                incremental.getPixels(), incremental.rowBytes());
        if (startResult == SkCodec::kSuccess) {
            break;
        }

        if (stream->isAllDataReceived()) {
            ERRORF(r, "Failed to start incremental decode\n");
            return;
        }

        // Append some data. The size is arbitrary, but deliberately different from
        // the buffer size used by SkPngCodec.
        stream->addNewData(1000);
    }

    while (true) {
        const SkCodec::Result result = partialCodec->incrementalDecode();

        if (result == SkCodec::kSuccess) {
            break;
        }

        REPORTER_ASSERT(r, result == SkCodec::kIncompleteInput);

        if (stream->isAllDataReceived()) {
            ERRORF(r, "Failed to completely decode %s", name);
            return;
        }

        // Append some data. The size is arbitrary, but deliberately different from
        // the buffer size used by SkPngCodec.
        stream->addNewData(1000);
    }

    // compare to original
    compare_bitmaps(r, truth, incremental);
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

    test_partial(r, "box.gif");
    test_partial(r, "randPixels.gif", 215);
    test_partial(r, "color_wheel.gif");
}

DEF_TEST(Codec_partialAnim, r) {
    auto path = "test640x479.gif";
    sk_sp<SkData> file = make_from_resource(path);
    if (!file) {
        return;
    }

    // This stream will be owned by fullCodec, but we hang on to the pointer
    // to determine frame offsets.
    SkStream* stream = new SkMemoryStream(file);
    std::unique_ptr<SkCodec> fullCodec(SkCodec::NewFromStream(stream));
    const auto info = standardize_info(fullCodec.get());

    // frameByteCounts stores the number of bytes to decode a particular frame.
    // - [0] is the number of bytes for the header
    // - frames[i] requires frameByteCounts[i+1] bytes to decode
    std::vector<size_t> frameByteCounts;
    std::vector<SkBitmap> frames;
    size_t lastOffset = 0;
    for (size_t i = 0; true; i++) {
        frameByteCounts.push_back(stream->getPosition() - lastOffset);
        lastOffset = stream->getPosition();

        SkBitmap frame;
        frame.allocPixels(info);

        SkCodec::Options opts;
        opts.fFrameIndex = i;
        const SkCodec::Result result = fullCodec->getPixels(info, frame.getPixels(),
                frame.rowBytes(), &opts, nullptr, nullptr);

        if (result == SkCodec::kIncompleteInput || result == SkCodec::kInvalidInput) {
            frameByteCounts.push_back(stream->getPosition() - lastOffset);

            // We need to distinguish between a partial frame and no more frames.
            // getFrameInfo lets us do this, since it tells the number of frames
            // not considering whether they are complete.
            // FIXME: Should we use a different Result?
            if (fullCodec->getFrameInfo().size() > i) {
                // This is a partial frame.
                frames.push_back(frame);
            }
            break;
        }

        if (result != SkCodec::kSuccess) {
            ERRORF(r, "Failed to decode frame %i from %s", i, path);
            return;
        }

        frames.push_back(frame);
    }

    // Now decode frames partially, then completely, and compare to the original.
    HaltingStream* haltingStream = new HaltingStream(file, frameByteCounts[0]);
    std::unique_ptr<SkCodec> partialCodec(SkCodec::NewFromStream(haltingStream));
    if (!partialCodec) {
        ERRORF(r, "Failed to create a partial codec from %s with %i bytes out of %i",
               path, frameByteCounts[0], file->size());
        return;
    }

    SkASSERT(frameByteCounts.size() > frames.size());
    for (size_t i = 0; i < frames.size(); i++) {
        const size_t fullFrameBytes = frameByteCounts[i + 1];
        const size_t firstHalf = fullFrameBytes / 2;
        const size_t secondHalf = fullFrameBytes - firstHalf;

        haltingStream->addNewData(firstHalf);

        SkBitmap frame;
        frame.allocPixels(info);

        SkCodec::Options opts;
        opts.fFrameIndex = i;
        SkCodec::Result result = partialCodec->startIncrementalDecode(info,
                frame.getPixels(), frame.rowBytes(), &opts);
        if (result != SkCodec::kSuccess) {
            ERRORF(r, "Failed to start incremental decode for %s on frame %i",
                   path, i);
            return;
        }

        result = partialCodec->incrementalDecode();
        REPORTER_ASSERT(r, SkCodec::kIncompleteInput == result);

        haltingStream->addNewData(secondHalf);
        result = partialCodec->incrementalDecode();
        REPORTER_ASSERT(r, SkCodec::kSuccess == result);

        // allocPixels locked the pixels for frame, but frames[i] was copied
        // from another bitmap, and did not retain the locked status.
        SkAutoLockPixels alp(frames[i]);
        compare_bitmaps(r, frames[i], frame);
    }
}

// Test that calling getPixels when an incremental decode has been
// started (but not finished) makes the next call to incrementalDecode
// require a call to startIncrementalDecode.
static void test_interleaved(skiatest::Reporter* r, const char* name) {
    sk_sp<SkData> file = make_from_resource(name);
    if (!file) {
        return;
    }
    const size_t halfSize = file->size() / 2;
    std::unique_ptr<SkCodec> partialCodec(SkCodec::NewFromStream(
            new HaltingStream(std::move(file), halfSize)));
    if (!partialCodec) {
        ERRORF(r, "Failed to create codec for %s", name);
        return;
    }

    const SkImageInfo info = standardize_info(partialCodec.get());
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
    test_interleaved(r, "box.gif");
}

// Modified version of the giflib logo, from
// http://giflib.sourceforge.net/whatsinagif/bits_and_bytes.html
// The global color map has been replaced with a local color map.
static unsigned char gNoGlobalColorMap[] = {
  // Header
  0x47, 0x49, 0x46, 0x38, 0x39, 0x61,

  // Logical screen descriptor
  0x0A, 0x00, 0x0A, 0x00, 0x11, 0x00, 0x00,

  // Image descriptor
  0x2C, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0x81,

  // Local color table
  0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00,

  // Image data
  0x02, 0x16, 0x8C, 0x2D, 0x99, 0x87, 0x2A, 0x1C, 0xDC, 0x33, 0xA0, 0x02, 0x75,
  0xEC, 0x95, 0xFA, 0xA8, 0xDE, 0x60, 0x8C, 0x04, 0x91, 0x4C, 0x01, 0x00,

  // Trailer
  0x3B,
};

// Test that a gif file truncated before its local color map behaves as expected.
DEF_TEST(Codec_GifPreMap, r) {
    sk_sp<SkData> data = SkData::MakeWithoutCopy(gNoGlobalColorMap, sizeof(gNoGlobalColorMap));
    std::unique_ptr<SkCodec> codec(SkCodec::NewFromData(data));
    if (!codec) {
        ERRORF(r, "failed to create codec");
        return;
    }

    SkBitmap truth;
    auto info = standardize_info(codec.get());
    truth.allocPixels(info);

    auto result = codec->getPixels(info, truth.getPixels(), truth.rowBytes());
    REPORTER_ASSERT(r, result == SkCodec::kSuccess);

    // Truncate to 23 bytes, just before the color map. This should fail to decode.
    codec.reset(SkCodec::NewFromData(SkData::MakeWithoutCopy(gNoGlobalColorMap, 23)));
    REPORTER_ASSERT(r, codec);
    if (codec) {
        SkBitmap bm;
        bm.allocPixels(info);
        result = codec->getPixels(info, bm.getPixels(), bm.rowBytes());
        REPORTER_ASSERT(r, result == SkCodec::kInvalidInput);
    }

    // Again, truncate to 23 bytes, this time for an incremental decode. We
    // cannot start an incremental decode until we have more data. If we did,
    // we would be using the wrong color table.
    HaltingStream* stream = new HaltingStream(data, 23);
    codec.reset(SkCodec::NewFromStream(stream));
    REPORTER_ASSERT(r, codec);
    if (codec) {
        SkBitmap bm;
        bm.allocPixels(info);
        result = codec->startIncrementalDecode(info, bm.getPixels(), bm.rowBytes());
        REPORTER_ASSERT(r, result == SkCodec::kIncompleteInput);

        stream->addNewData(data->size());
        result = codec->startIncrementalDecode(info, bm.getPixels(), bm.rowBytes());
        REPORTER_ASSERT(r, result == SkCodec::kSuccess);

        result = codec->incrementalDecode();
        REPORTER_ASSERT(r, result == SkCodec::kSuccess);
        compare_bitmaps(r, truth, bm);
    }
}
