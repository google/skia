/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/codec/SkCodec.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkData.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "include/core/SkTypes.h"
#include "src/core/SkMakeUnique.h"
#include "tests/CodecPriv.h"
#include "tests/FakeStreams.h"
#include "tests/Test.h"
#include "tools/Resources.h"

#include <cstring>
#include <memory>
#include <utility>
#include <vector>

static SkImageInfo standardize_info(SkCodec* codec) {
    SkImageInfo defaultInfo = codec->getInfo();
    // Note: This drops the SkColorSpace, allowing the equality check between two
    // different codecs created from the same file to have the same SkImageInfo.
    return SkImageInfo::MakeN32Premul(defaultInfo.width(), defaultInfo.height());
}

static bool create_truth(sk_sp<SkData> data, SkBitmap* dst) {
    std::unique_ptr<SkCodec> codec(SkCodec::MakeFromData(std::move(data)));
    if (!codec) {
        return false;
    }

    const SkImageInfo info = standardize_info(codec.get());
    dst->allocPixels(info);
    return SkCodec::kSuccess == codec->getPixels(info, dst->getPixels(), dst->rowBytes());
}

static bool compare_bitmaps(skiatest::Reporter* r, const SkBitmap& bm1, const SkBitmap& bm2) {
    const SkImageInfo& info = bm1.info();
    if (info != bm2.info()) {
        ERRORF(r, "Bitmaps have different image infos!");
        return false;
    }
    const size_t rowBytes = info.minRowBytes();
    for (int i = 0; i < info.height(); i++) {
        if (memcmp(bm1.getAddr(0, i), bm2.getAddr(0, i), rowBytes)) {
            ERRORF(r, "Bitmaps have different pixels, starting on line %i!", i);
            return false;
        }
    }

    return true;
}

static void test_partial(skiatest::Reporter* r, const char* name, const sk_sp<SkData>& file,
                         size_t minBytes, size_t increment) {
    SkBitmap truth;
    if (!create_truth(file, &truth)) {
        ERRORF(r, "Failed to decode %s\n", name);
        return;
    }

    // Now decode part of the file
    HaltingStream* stream = new HaltingStream(file, minBytes);

    // Note that we cheat and hold on to a pointer to stream, though it is owned by
    // partialCodec.
    auto partialCodec = SkCodec::MakeFromStream(std::unique_ptr<SkStream>(stream));
    if (!partialCodec) {
        ERRORF(r, "Failed to create codec for %s with %zu bytes", name, minBytes);
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

        stream->addNewData(increment);
    }

    while (true) {
        // This imitates how Chromium calls getFrameCount before resuming a decode.
        // Without this line, the test passes. With it, it fails when skia_use_wuffs
        // is true.
        partialCodec->getFrameCount();
        const SkCodec::Result result = partialCodec->incrementalDecode();

        if (result == SkCodec::kSuccess) {
            break;
        }

        REPORTER_ASSERT(r, result == SkCodec::kIncompleteInput);

        if (stream->isAllDataReceived()) {
            ERRORF(r, "Failed to completely decode %s", name);
            return;
        }

        stream->addNewData(increment);
    }

    // compare to original
    compare_bitmaps(r, truth, incremental);
}

static void test_partial(skiatest::Reporter* r, const char* name, size_t minBytes = 0) {
    sk_sp<SkData> file = GetResourceAsData(name);
    if (!file) {
        SkDebugf("missing resource %s\n", name);
        return;
    }

    // This size is arbitrary, but deliberately different from the buffer size used by SkPngCodec.
    constexpr size_t kIncrement = 1000;
    test_partial(r, name, file, SkTMax(file->size() / 2, minBytes), kIncrement);
}

DEF_TEST(Codec_partial, r) {
#if 0
    // FIXME (scroggo): SkPngCodec needs to use SkStreamBuffer in order to
    // support incremental decoding.
    test_partial(r, "images/plane.png");
    test_partial(r, "images/plane_interlaced.png");
    test_partial(r, "images/yellow_rose.png");
    test_partial(r, "images/index8.png");
    test_partial(r, "images/color_wheel.png");
    test_partial(r, "images/mandrill_256.png");
    test_partial(r, "images/mandrill_32.png");
    test_partial(r, "images/arrow.png");
    test_partial(r, "images/randPixels.png");
    test_partial(r, "images/baby_tux.png");
#endif
    test_partial(r, "images/box.gif");
    test_partial(r, "images/randPixels.gif", 215);
    test_partial(r, "images/color_wheel.gif");
}

DEF_TEST(Codec_partialWuffs, r) {
    const char* path = "images/alphabetAnim.gif";
    auto file = GetResourceAsData(path);
    if (!file) {
        ERRORF(r, "missing %s", path);
    } else {
        // This is the end of the first frame. SkCodec will treat this as a
        // single frame gif.
        file = SkData::MakeSubset(file.get(), 0, 153);
        // Start with 100 to get a partial decode, then add the rest of the
        // first frame to decode a full image.
        test_partial(r, path, file, 100, 53);
    }
}

// Verify that when decoding an animated gif byte by byte we report the correct
// fRequiredFrame as soon as getFrameInfo reports the frame.
DEF_TEST(Codec_requiredFrame, r) {
    auto path = "images/colorTables.gif";
    sk_sp<SkData> file = GetResourceAsData(path);
    if (!file) {
        return;
    }

    std::unique_ptr<SkCodec> codec(SkCodec::MakeFromData(file));
    if (!codec) {
        ERRORF(r, "Failed to create codec from %s", path);
        return;
    }

    auto frameInfo = codec->getFrameInfo();
    if (frameInfo.size() <= 1) {
        ERRORF(r, "Test is uninteresting with 0 or 1 frames");
        return;
    }

    HaltingStream* stream(nullptr);
    std::unique_ptr<SkCodec> partialCodec(nullptr);
    for (size_t i = 0; !partialCodec; i++) {
        if (file->size() == i) {
            ERRORF(r, "Should have created a partial codec for %s", path);
            return;
        }
        stream = new HaltingStream(file, i);
        partialCodec = SkCodec::MakeFromStream(std::unique_ptr<SkStream>(stream));
    }

    std::vector<SkCodec::FrameInfo> partialInfo;
    size_t frameToCompare = 0;
    while (true) {
        partialInfo = partialCodec->getFrameInfo();
        for (; frameToCompare < partialInfo.size(); frameToCompare++) {
            REPORTER_ASSERT(r, partialInfo[frameToCompare].fRequiredFrame
                                == frameInfo[frameToCompare].fRequiredFrame);
        }

        if (frameToCompare == frameInfo.size()) {
            break;
        }

        if (stream->getLength() == file->size()) {
            ERRORF(r, "Should have found all frames for %s", path);
            return;
        }
        stream->addNewData(1);
    }
}

DEF_TEST(Codec_partialAnim, r) {
    auto path = "images/test640x479.gif";
    sk_sp<SkData> file = GetResourceAsData(path);
    if (!file) {
        return;
    }

    // This stream will be owned by fullCodec, but we hang on to the pointer
    // to determine frame offsets.
    std::unique_ptr<SkCodec> fullCodec(SkCodec::MakeFromStream(skstd::make_unique<SkMemoryStream>(file)));
    const auto info = standardize_info(fullCodec.get());

    // frameByteCounts stores the number of bytes to decode a particular frame.
    // - [0] is the number of bytes for the header
    // - frames[i] requires frameByteCounts[i+1] bytes to decode
    const std::vector<size_t> frameByteCounts = { 455, 69350, 1344, 1346, 1327 };
    std::vector<SkBitmap> frames;
    for (size_t i = 0; true; i++) {
        SkBitmap frame;
        frame.allocPixels(info);

        SkCodec::Options opts;
        opts.fFrameIndex = i;
        const SkCodec::Result result = fullCodec->getPixels(info, frame.getPixels(),
                frame.rowBytes(), &opts);

        if (result == SkCodec::kIncompleteInput || result == SkCodec::kInvalidInput) {
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
    std::unique_ptr<SkCodec> partialCodec(SkCodec::MakeFromStream(
                                                      std::unique_ptr<SkStream>(haltingStream)));
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
        auto frameInfo = partialCodec->getFrameInfo();
        REPORTER_ASSERT(r, frameInfo.size() == i + 1);
        REPORTER_ASSERT(r, !frameInfo[i].fFullyReceived);

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

        frameInfo = partialCodec->getFrameInfo();
        REPORTER_ASSERT(r, frameInfo.size() == i + 1);
        REPORTER_ASSERT(r, frameInfo[i].fFullyReceived);
        if (!compare_bitmaps(r, frames[i], frame)) {
            ERRORF(r, "\tfailure was on frame %i", i);
            SkString name = SkStringPrintf("expected_%i", i);
            write_bm(name.c_str(), frames[i]);

            name = SkStringPrintf("actual_%i", i);
            write_bm(name.c_str(), frame);
        }
    }
}

// Test that calling getPixels when an incremental decode has been
// started (but not finished) makes the next call to incrementalDecode
// require a call to startIncrementalDecode.
static void test_interleaved(skiatest::Reporter* r, const char* name) {
    sk_sp<SkData> file = GetResourceAsData(name);
    if (!file) {
        return;
    }
    const size_t halfSize = file->size() / 2;
    std::unique_ptr<SkCodec> partialCodec(SkCodec::MakeFromStream(
                                  skstd::make_unique<HaltingStream>(std::move(file), halfSize)));
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
    test_interleaved(r, "images/plane.png");
    test_interleaved(r, "images/plane_interlaced.png");
    test_interleaved(r, "images/box.gif");
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
    std::unique_ptr<SkCodec> codec(SkCodec::MakeFromData(data));
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
    //
    // See also Codec_GifTruncated2 in GifTest.cpp for this magic 23.
    codec = SkCodec::MakeFromData(SkData::MakeWithoutCopy(gNoGlobalColorMap, 23));
    REPORTER_ASSERT(r, codec);
    if (codec) {
        SkBitmap bm;
        bm.allocPixels(info);
        result = codec->getPixels(info, bm.getPixels(), bm.rowBytes());

        // See the comments in Codec_GifTruncated2.
#ifdef SK_HAS_WUFFS_LIBRARY
        REPORTER_ASSERT(r, result == SkCodec::kIncompleteInput);
#else
        REPORTER_ASSERT(r, result == SkCodec::kInvalidInput);
#endif
    }

    // Again, truncate to 23 bytes, this time for an incremental decode. We
    // cannot start an incremental decode until we have more data. If we did,
    // we would be using the wrong color table.
    HaltingStream* stream = new HaltingStream(data, 23);
    codec = SkCodec::MakeFromStream(std::unique_ptr<SkStream>(stream));
    REPORTER_ASSERT(r, codec);
    if (codec) {
        SkBitmap bm;
        bm.allocPixels(info);
        result = codec->startIncrementalDecode(info, bm.getPixels(), bm.rowBytes());

        // See the comments in Codec_GifTruncated2.
#ifdef SK_HAS_WUFFS_LIBRARY
        REPORTER_ASSERT(r, result == SkCodec::kSuccess);

        // Note that this is incrementalDecode, not startIncrementalDecode.
        result = codec->incrementalDecode();
        REPORTER_ASSERT(r, result == SkCodec::kIncompleteInput);

        stream->addNewData(data->size());
#else
        REPORTER_ASSERT(r, result == SkCodec::kIncompleteInput);

        // Note that this is startIncrementalDecode, not incrementalDecode.
        stream->addNewData(data->size());
        result = codec->startIncrementalDecode(info, bm.getPixels(), bm.rowBytes());
        REPORTER_ASSERT(r, result == SkCodec::kSuccess);
#endif

        result = codec->incrementalDecode();
        REPORTER_ASSERT(r, result == SkCodec::kSuccess);
        compare_bitmaps(r, truth, bm);
    }
}

DEF_TEST(Codec_emptyIDAT, r) {
    const char* name = "images/baby_tux.png";
    sk_sp<SkData> file = GetResourceAsData(name);
    if (!file) {
        return;
    }

    // Truncate to the beginning of the IDAT, immediately after the IDAT tag.
    file = SkData::MakeSubset(file.get(), 0, 80);

    std::unique_ptr<SkCodec> codec(SkCodec::MakeFromData(std::move(file)));
    if (!codec) {
        ERRORF(r, "Failed to create a codec for %s", name);
        return;
    }

    SkBitmap bm;
    const auto info = standardize_info(codec.get());
    bm.allocPixels(info);

    const auto result = codec->getPixels(info, bm.getPixels(), bm.rowBytes());
    REPORTER_ASSERT(r, SkCodec::kIncompleteInput == result);
}

DEF_TEST(Codec_incomplete, r) {
    for (const char* name : { "images/baby_tux.png",
                              "images/baby_tux.webp",
                              "images/CMYK.jpg",
                              "images/color_wheel.gif",
                              "images/google_chrome.ico",
                              "images/rle.bmp",
                              "images/mandrill.wbmp",
                              }) {
        sk_sp<SkData> file = GetResourceAsData(name);
        if (!file) {
            continue;
        }

        for (size_t len = 14; len <= file->size(); len += 5) {
            SkCodec::Result result;
            std::unique_ptr<SkCodec> codec(SkCodec::MakeFromStream(
                                   skstd::make_unique<SkMemoryStream>(file->data(), len), &result));
            if (codec) {
                if (result != SkCodec::kSuccess) {
                    ERRORF(r, "Created an SkCodec for %s with %lu bytes, but "
                              "reported an error %i", name, len, result);
                }
                break;
            }

            if (SkCodec::kIncompleteInput != result) {
                ERRORF(r, "Reported error %i for %s with %lu bytes",
                       result, name, len);
                break;
            }
        }
    }
}
