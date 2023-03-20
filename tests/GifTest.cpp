/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/codec/SkAndroidCodec.h"
#include "include/codec/SkCodec.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorType.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/core/SkUnPreMultiply.h"
#include "tests/CodecPriv.h"
#include "tests/Test.h"
#include "tools/Resources.h"

#include <cstring>
#include <initializer_list>
#include <memory>
#include <utility>
#include <vector>

static unsigned char gGIFData[] = {
  0x47, 0x49, 0x46, 0x38, 0x37, 0x61, 0x03, 0x00, 0x03, 0x00, 0xe3, 0x08,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0x00,
  0xff, 0x80, 0x80, 0x80, 0x00, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
  0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0x2c, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x03, 0x00, 0x00, 0x04,
  0x07, 0x50, 0x1c, 0x43, 0x40, 0x41, 0x23, 0x44, 0x00, 0x3b
};

static unsigned char gGIFDataNoColormap[] = {
  // Header
  0x47, 0x49, 0x46, 0x38, 0x39, 0x61,
  // Screen descriptor
  0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
  // Graphics control extension
  0x21, 0xf9, 0x04, 0x01, 0x0a, 0x00, 0x01, 0x00,
  // Image descriptor
  0x2c, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00,
  // Image data
  0x02, 0x02, 0x4c, 0x01, 0x00,
  // Trailer
  0x3b
};

static unsigned char gInterlacedGIF[] = {
  0x47, 0x49, 0x46, 0x38, 0x37, 0x61, 0x09, 0x00, 0x09, 0x00, 0xe3, 0x08, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0x00, 0xff, 0x80,
  0x80, 0x80, 0x00, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x2c, 0x00, 0x00, 0x00,
  0x00, 0x09, 0x00, 0x09, 0x00, 0x40, 0x04, 0x1b, 0x50, 0x1c, 0x23, 0xe9, 0x44,
  0x23, 0x60, 0x9d, 0x09, 0x28, 0x1e, 0xf8, 0x6d, 0x64, 0x56, 0x9d, 0x53, 0xa8,
  0x7e, 0xa8, 0x65, 0x94, 0x5c, 0xb0, 0x8a, 0x45, 0x04, 0x00, 0x3b
};

static void test_gif_data_no_colormap(skiatest::Reporter* r,
                                      void* data,
                                      size_t size) {
    SkBitmap bm;
    bool imageDecodeSuccess = decode_memory(data, size, &bm);
    REPORTER_ASSERT(r, imageDecodeSuccess);
    REPORTER_ASSERT(r, bm.width() == 1);
    REPORTER_ASSERT(r, bm.height() == 1);
    REPORTER_ASSERT(r, !(bm.empty()));
    if (!(bm.empty())) {
        REPORTER_ASSERT(r, bm.getColor(0, 0) == 0x00000000);
    }
}
static void test_gif_data(skiatest::Reporter* r, void* data, size_t size) {
    SkBitmap bm;
    bool imageDecodeSuccess = decode_memory(data, size, &bm);
    REPORTER_ASSERT(r, imageDecodeSuccess);
    REPORTER_ASSERT(r, bm.width() == 3);
    REPORTER_ASSERT(r, bm.height() == 3);
    REPORTER_ASSERT(r, !(bm.empty()));
    if (!(bm.empty())) {
        REPORTER_ASSERT(r, bm.getColor(0, 0) == 0xffff0000);
        REPORTER_ASSERT(r, bm.getColor(1, 0) == 0xffffff00);
        REPORTER_ASSERT(r, bm.getColor(2, 0) == 0xff00ffff);
        REPORTER_ASSERT(r, bm.getColor(0, 1) == 0xff808080);
        REPORTER_ASSERT(r, bm.getColor(1, 1) == 0xff000000);
        REPORTER_ASSERT(r, bm.getColor(2, 1) == 0xff00ff00);
        REPORTER_ASSERT(r, bm.getColor(0, 2) == 0xffffffff);
        REPORTER_ASSERT(r, bm.getColor(1, 2) == 0xffff00ff);
        REPORTER_ASSERT(r, bm.getColor(2, 2) == 0xff0000ff);
    }
}
static void test_gif_data_dims(skiatest::Reporter* r, void* data, size_t size, int width,
        int height) {
    SkBitmap bm;
    bool imageDecodeSuccess = decode_memory(data, size, &bm);
    REPORTER_ASSERT(r, imageDecodeSuccess);
    REPORTER_ASSERT(r, bm.width() == width);
    REPORTER_ASSERT(r, bm.height() == height);
    REPORTER_ASSERT(r, !(bm.empty()));
}
static void test_interlaced_gif_data(skiatest::Reporter* r,
                                     void* data,
                                     size_t size) {
    SkBitmap bm;
    bool imageDecodeSuccess = decode_memory(data, size, &bm);
    REPORTER_ASSERT(r, imageDecodeSuccess);
    REPORTER_ASSERT(r, bm.width() == 9);
    REPORTER_ASSERT(r, bm.height() == 9);
    REPORTER_ASSERT(r, !(bm.empty()));
    if (!(bm.empty())) {
        REPORTER_ASSERT(r, bm.getColor(0, 0) == 0xffff0000);
        REPORTER_ASSERT(r, bm.getColor(1, 0) == 0xffffff00);
        REPORTER_ASSERT(r, bm.getColor(2, 0) == 0xff00ffff);

        REPORTER_ASSERT(r, bm.getColor(0, 2) == 0xffffffff);
        REPORTER_ASSERT(r, bm.getColor(1, 2) == 0xffff00ff);
        REPORTER_ASSERT(r, bm.getColor(2, 2) == 0xff0000ff);

        REPORTER_ASSERT(r, bm.getColor(0, 4) == 0xff808080);
        REPORTER_ASSERT(r, bm.getColor(1, 4) == 0xff000000);
        REPORTER_ASSERT(r, bm.getColor(2, 4) == 0xff00ff00);

        REPORTER_ASSERT(r, bm.getColor(0, 6) == 0xffff0000);
        REPORTER_ASSERT(r, bm.getColor(1, 6) == 0xffffff00);
        REPORTER_ASSERT(r, bm.getColor(2, 6) == 0xff00ffff);

        REPORTER_ASSERT(r, bm.getColor(0, 8) == 0xffffffff);
        REPORTER_ASSERT(r, bm.getColor(1, 8) == 0xffff00ff);
        REPORTER_ASSERT(r, bm.getColor(2, 8) == 0xff0000ff);
    }
}

static void test_gif_data_short(skiatest::Reporter* r,
                                void* data,
                                size_t size) {
    SkBitmap bm;
    bool imageDecodeSuccess = decode_memory(data, size, &bm);
    REPORTER_ASSERT(r, imageDecodeSuccess);
    REPORTER_ASSERT(r, bm.width() == 3);
    REPORTER_ASSERT(r, bm.height() == 3);
    REPORTER_ASSERT(r, !(bm.empty()));
    if (!(bm.empty())) {
        REPORTER_ASSERT(r, bm.getColor(0, 0) == 0xffff0000);
        REPORTER_ASSERT(r, bm.getColor(1, 0) == 0xffffff00);
        REPORTER_ASSERT(r, bm.getColor(2, 0) == 0xff00ffff);
        REPORTER_ASSERT(r, bm.getColor(0, 1) == 0xff808080);
        REPORTER_ASSERT(r, bm.getColor(1, 1) == 0xff000000);
        REPORTER_ASSERT(r, bm.getColor(2, 1) == 0xff00ff00);
    }
}

/**
  This test will test the ability of the SkCodec to deal with
  GIF files which have been mangled somehow.  We want to display as
  much of the GIF as possible.
*/
DEF_TEST(Gif, reporter) {
    // test perfectly good images.
    test_gif_data(reporter, static_cast<void *>(gGIFData), sizeof(gGIFData));
    test_interlaced_gif_data(reporter, static_cast<void *>(gInterlacedGIF),
                          sizeof(gInterlacedGIF));

    unsigned char badData[sizeof(gGIFData)];

    memcpy(badData, gGIFData, sizeof(gGIFData));
    badData[6] = 0x01;  // image too wide
    test_gif_data(reporter, static_cast<void *>(badData), sizeof(gGIFData));
    // "libgif warning [image too wide, expanding output to size]"

    memcpy(badData, gGIFData, sizeof(gGIFData));
    badData[8] = 0x01;  // image too tall
    test_gif_data(reporter, static_cast<void *>(badData), sizeof(gGIFData));
    // "libgif warning [image too tall,  expanding output to size]"

    memcpy(badData, gGIFData, sizeof(gGIFData));
    badData[62] = 0x01;  // image shifted right
    test_gif_data_dims(reporter, static_cast<void *>(badData), sizeof(gGIFData), 4, 3);

    memcpy(badData, gGIFData, sizeof(gGIFData));
    badData[64] = 0x01;  // image shifted down
    test_gif_data_dims(reporter, static_cast<void *>(badData), sizeof(gGIFData), 3, 4);

    memcpy(badData, gGIFData, sizeof(gGIFData));
    badData[62] = 0xff;  // image shifted right
    badData[63] = 0xff;
    test_gif_data_dims(reporter, static_cast<void *>(badData), sizeof(gGIFData), 3 + 0xFFFF, 3);

    memcpy(badData, gGIFData, sizeof(gGIFData));
    badData[64] = 0xff;  // image shifted down
    badData[65] = 0xff;
    test_gif_data_dims(reporter, static_cast<void *>(badData), sizeof(gGIFData), 3, 3 + 0xFFFF);

    test_gif_data_no_colormap(reporter, static_cast<void *>(gGIFDataNoColormap),
                              sizeof(gGIFDataNoColormap));

    // test short Gif.  80 is missing a few bytes.
    test_gif_data_short(reporter, static_cast<void *>(gGIFData), 80);
    // "libgif warning [DGifGetLine]"

    test_interlaced_gif_data(reporter, static_cast<void *>(gInterlacedGIF),
                             100);  // 100 is missing a few bytes
    // "libgif warning [interlace DGifGetLine]"
}

DEF_TEST(Codec_GifInterlacedTruncated, r) {
    // Check that gInterlacedGIF is exactly 102 bytes long, and that the final
    // 30 bytes, in the half-open range [72, 102), consists of 0x1b (indicating
    // a block of 27 bytes), then those 27 bytes, then 0x00 (end of the blocks)
    // then 0x3b (end of the GIF).
    if ((sizeof(gInterlacedGIF) != 102) ||
        (gInterlacedGIF[72] != 0x1b) ||
        (gInterlacedGIF[100] != 0x00) ||
        (gInterlacedGIF[101] != 0x3b)) {
        ERRORF(r, "Invalid gInterlacedGIF data");
        return;
    }

    // We want to test the GIF codec's output on some (but not all) of the
    // LZW-compressed data. As is, there is only one block of LZW-compressed
    // data, 27 bytes long. Wuffs can output partial results from a partial
    // block, but some other GIF implementations output intermediate rows only
    // on block boundaries, so truncating to a prefix of gInterlacedGIF isn't
    // enough. We also have to modify the block size down from 0x1b so that the
    // edited version still contains a complete block. In this case, it's a
    // block of 10 bytes.
    unsigned char data[83];
    memcpy(data, gInterlacedGIF, sizeof(data));
    data[72] = sizeof(data) - 73;

    // Just like test_interlaced_gif_data, check that we get a 9x9 image.
    SkBitmap bm;
    bool imageDecodeSuccess = decode_memory(data, sizeof(data), &bm);
    REPORTER_ASSERT(r, imageDecodeSuccess);
    REPORTER_ASSERT(r, bm.width() == 9);
    REPORTER_ASSERT(r, bm.height() == 9);

    // For an interlaced, non-transparent image, we thicken or replicate the
    // rows of earlier interlace passes so that, when e.g. decoding a GIF
    // sourced from a slow network connection, we show a richer intermediate
    // image while waiting for the complete image. This replication is
    // sometimes described as a "Haeberli inspired technique".
    //
    // For a 9 pixel high image, interlacing shuffles the row order to be: 0,
    // 8, 4, 2, 6, 1, 3, 5, 7. Even though truncating to 10 bytes of
    // LZW-compressed data only explicitly contains completed rows 0 and 8, we
    // still expect row 7 to be set, due to replication, and therefore not
    // transparent black (zero).
    REPORTER_ASSERT(r, bm.getColor(0, 7) != 0);
}

// Regression test for decoding a gif image with sampleSize of 4, which was
// previously crashing.
DEF_TEST(Gif_Sampled, r) {
    auto data = GetResourceAsData("images/test640x479.gif");
    REPORTER_ASSERT(r, data);
    if (!data) {
        return;
    }
    std::unique_ptr<SkStreamAsset> stream(new SkMemoryStream(std::move(data)));
    std::unique_ptr<SkAndroidCodec> codec(SkAndroidCodec::MakeFromStream(std::move(stream)));
    REPORTER_ASSERT(r, codec);
    if (!codec) {
        return;
    }

    SkAndroidCodec::AndroidOptions options;
    options.fSampleSize = 4;

    SkBitmap bm;
    bm.allocPixels(codec->getInfo());
    const SkCodec::Result result = codec->getAndroidPixels(codec->getInfo(), bm.getPixels(),
            bm.rowBytes(), &options);
    REPORTER_ASSERT(r, result == SkCodec::kSuccess);
}

// If a GIF file is truncated before the header for the first image is defined,
// we should not create an SkCodec.
DEF_TEST(Codec_GifTruncated, r) {
    sk_sp<SkData> data(GetResourceAsData("images/test640x479.gif"));
    if (!data) {
        return;
    }

    // This is right before the header for the first image.
    data = SkData::MakeSubset(data.get(), 0, 446);
    std::unique_ptr<SkCodec> codec(SkCodec::MakeFromData(data));
    REPORTER_ASSERT(r, !codec);
}

/*
For the Codec_GifTruncated2 test, immediately below,
resources/images/box.gif's first 23 bytes are:

00000000: 4749 4638 3961 c800 3700 203f 002c 0000  GIF89a..7. ?.,..
00000010: 0000 c800 3700 85                        ....7..

The breakdown:

@000  6 bytes magic "GIF89a"
@006  7 bytes Logical Screen Descriptor: 0xC8 0x00 ... 0x00
   - width     =   200
   - height    =    55
   - flags     =  0x20
   - background color index, pixel aspect ratio bytes ignored
@00D 10 bytes Image Descriptor header: 0x2C 0x00 ... 0x85
   - origin_x  =     0
   - origin_y  =     0
   - width     =   200
   - height    =    55
   - flags     =  0x85, local color table, 64 RGB entries

In particular, 23 bytes is after the header, but before the color table.
*/

DEF_TEST(Codec_GifTruncated2, r) {
    // Truncate box.gif at 21, 22 and 23 bytes.
    //
    // See also Codec_GifTruncated3 in this file, below.
    //
    // See also Codec_trunc in CodecAnimTest.cpp for this magic 23.
    //
    // See also Codec_GifPreMap in CodecPartialTest.cpp for this magic 23.
    for (int i = 21; i < 24; i++) {
        sk_sp<SkData> data(GetResourceAsData("images/box.gif"));
        if (!data) {
            return;
        }

        data = SkData::MakeSubset(data.get(), 0, i);
        std::unique_ptr<SkCodec> codec(SkCodec::MakeFromData(data));

        if (i <= 21) {
            if (codec) {
                ERRORF(r, "Invalid data gave non-nullptr codec");
            }
            return;
        }

        if (!codec) {
            ERRORF(r, "Failed to create codec with partial data (truncated at %d)", i);
            return;
        }

        // The input is truncated in the Image Descriptor, before the local
        // color table, and before (21) or after (22, 23) the first frame's
        // XYWH (left / top / width / height) can be decoded. A detailed
        // breakdown of those 23 bytes is in a comment above this function.
        //
        // At 21 bytes, the MakeFromStream factory method returns a nullptr
        // SkCodec*, because creating a SkCodec requires knowing the image width
        // and height (as its constructor takes an SkEncodedInfo argument), and
        // specifically for GIF, decoding the image width and height requires
        // decoding the first frame's XYWH, as per
        // https://raw.githubusercontent.com/google/wuffs/master/test/data/artificial/gif-frame-out-of-bounds.gif.make-artificial.txt
        //
        // At 22 or 23 bytes, the first frame is complete enough that we can
        // fill in all of a SkCodec::FrameInfo's fields (other than
        // fFullyReceived). Specifically, we can fill in fRequiredFrame and
        // fAlphaType, even though we haven't yet decoded the frame's RGB
        // palette entries, as we do know the frame rectangle and that every
        // palette entry is fully opaque, due to the lack of a Graphic Control
        // Extension before the Image Descriptor.
        REPORTER_ASSERT(r, codec->getFrameCount() == 1);
    }
}

// This tests that, after truncating the input, the pixels are still
// zero-initialized. If you comment out the SkSampler::Fill call in
// SkWuffsCodec::onStartIncrementalDecode, the test could still pass (in a
// standard configuration) but should fail with the MSAN memory sanitizer.
DEF_TEST(Codec_GifTruncated3, r) {
    sk_sp<SkData> data(GetResourceAsData("images/box.gif"));
    if (!data) {
        return;
    }

    data = SkData::MakeSubset(data.get(), 0, 23);
    sk_sp<SkImage> image(SkImage::MakeFromEncoded(data));

    if (!image) {
        ERRORF(r, "Missing image");
        return;
    }

    REPORTER_ASSERT(r, image->width() == 200);
    REPORTER_ASSERT(r, image->height() == 55);

    SkBitmap bm;
    if (!bm.tryAllocPixels(SkImageInfo::MakeN32Premul(200, 55))) {
        ERRORF(r, "Failed to allocate pixels");
        return;
    }

    bm.eraseColor(SK_ColorTRANSPARENT);
    REPORTER_ASSERT(r, image->readPixels(nullptr,
                                         bm.info(),
                                         bm.getPixels(),
                                         200 * 4,
                                         0, 0));

    for (int i = 0; i < image->width();  ++i)
    for (int j = 0; j < image->height(); ++j) {
        SkColor actual = SkUnPreMultiply::PMColorToColor(*bm.getAddr32(i, j));
        if (actual != SK_ColorTRANSPARENT) {
            ERRORF(r, "did not initialize pixels! %i, %i is %x", i, j, actual);
        }
    }
}

DEF_TEST(Codec_gif_out_of_palette, r) {
    if (GetResourcePath().isEmpty()) {
        return;
    }

    const char* path = "images/out-of-palette.gif";
    auto data = GetResourceAsData(path);
    if (!data) {
        ERRORF(r, "failed to find %s", path);
        return;
    }

    auto codec = SkCodec::MakeFromData(std::move(data));
    if (!codec) {
        ERRORF(r, "Could not create codec from %s", path);
        return;
    }

    SkBitmap bm;
    bm.allocPixels(codec->getInfo());
    auto result = codec->getPixels(bm.pixmap());
    REPORTER_ASSERT(r, result == SkCodec::kSuccess, "Failed to decode %s with error %s",
                    path, SkCodec::ResultToString(result));

    struct {
        int     x;
        int     y;
        SkColor expected;
    } pixels[] = {
        { 0, 0, SK_ColorBLACK },
        { 1, 0, SK_ColorWHITE },
        { 0, 1, SK_ColorTRANSPARENT },
        { 1, 1, SK_ColorTRANSPARENT },
    };
    for (auto& pixel : pixels) {
        auto actual = bm.getColor(pixel.x, pixel.y);
        REPORTER_ASSERT(r, actual == pixel.expected,
                        "pixel (%i,%i) mismatch! expected: %x actual: %x",
                        pixel.x, pixel.y, pixel.expected, actual);
    }
}

// This tests decoding the GIF image created by this script:
// https://raw.githubusercontent.com/google/wuffs/6c2fb9a2fd9e3334ee7dabc1ad60bfc89158084f/test/data/artificial/gif-transparent-index.gif.make-artificial.txt
//
// It is a 4x2 animated image with 2 frames. The first frame is full of various
// red pixels. The second frame overlays a 3x1 rectangle at (1, 1): light blue,
// transparent, dark blue.
DEF_TEST(Codec_AnimatedTransparentGif, r) {
    const char* path = "images/gif-transparent-index.gif";
    auto data = GetResourceAsData(path);
    if (!data) {
        ERRORF(r, "failed to find %s", path);
        return;
    }

    auto codec = SkCodec::MakeFromData(std::move(data));
    if (!codec) {
        ERRORF(r, "Could not create codec from %s", path);
        return;
    }

    SkImageInfo info = codec->getInfo();
    if ((info.width() != 4) || (info.height() != 2) || (codec->getFrameInfo().size() != 2)) {
        ERRORF(r, "Unexpected image info");
        return;
    }

    for (bool use565 : { false, true }) {
        SkBitmap bm;
        bm.allocPixels(use565 ? info.makeColorType(kRGB_565_SkColorType) : info);

        for (int i = 0; i < 2; i++) {
            SkCodec::Options options;
            options.fFrameIndex = i;
            options.fPriorFrame = (i > 0) ? (i - 1) : SkCodec::kNoFrame;
            auto result = codec->getPixels(bm.pixmap(), &options);
            REPORTER_ASSERT(r, result == SkCodec::kSuccess, "Failed to decode frame %i", i);

            // Per above: the first frame is full of various red pixels.
            SkColor expectedPixels[2][4] = {
                { 0xFF800000, 0xFF900000, 0xFFA00000, 0xFFB00000 },
                { 0xFFC00000, 0xFFD00000, 0xFFE00000, 0xFFF00000 },
            };
            if (use565) {
                // For kRGB_565_SkColorType, copy the red channel's high 3 bits
                // to its low 3 bits.
                expectedPixels[0][0] = 0xFF840000;
                expectedPixels[0][1] = 0xFF940000;
                expectedPixels[0][2] = 0xFFA50000;
                expectedPixels[0][3] = 0xFFB50000;
                expectedPixels[1][0] = 0xFFC60000;
                expectedPixels[1][1] = 0xFFD60000;
                expectedPixels[1][2] = 0xFFE70000;
                expectedPixels[1][3] = 0xFFF70000;
            }
            if (i > 0) {
                // Per above: the second frame overlays a 3x1 rectangle at (1,
                // 1): light blue, transparent, dark blue.
                //
                // Again, for kRGB_565_SkColorType, copy the blue channel's
                // high 3 bits to its low 3 bits.
                expectedPixels[1][1] = use565 ? 0xFF0000FF : 0xFF0000FF;
                expectedPixels[1][3] = use565 ? 0xFF000052 : 0xFF000055;
            }

            for (int y = 0; y < 2; y++) {
                for (int x = 0; x < 4; x++) {
                    auto expected = expectedPixels[y][x];
                    auto actual = bm.getColor(x, y);
                    REPORTER_ASSERT(r, actual == expected,
                                    "use565 %i, frame %i, pixel (%i,%i) "
                                    "mismatch! expected: %x actual: %x",
                                    (int)use565, i, x, y, expected, actual);
                }
            }
        }
    }
}

// This test verifies that a GIF frame outside the image dimensions is handled
// as desired:
// - The image reports a size of 0 x 0, but the first frame is 100 x 90. The
// image (or "canvas") is expanded to fit the first frame. The first frame is red.
// - The second frame is a green 75 x 75 rectangle, reporting its x-offset and
// y-offset to be 105, placing it off screen. The decoder interprets this as no
// change from the first frame.
DEF_TEST(Codec_xOffsetTooBig, r) {
    const char* path = "images/xOffsetTooBig.gif";
    auto data = GetResourceAsData(path);
    if (!data) {
        ERRORF(r, "failed to find %s", path);
        return;
    }

    auto codec = SkCodec::MakeFromData(std::move(data));
    if (!codec) {
        ERRORF(r, "Could not create codec from %s", path);
        return;
    }

    REPORTER_ASSERT(r, codec->getFrameCount() == 2);

    auto info = codec->getInfo();
    REPORTER_ASSERT(r, info.width() == 100 && info.height() == 90);

    SkBitmap bm;
    bm.allocPixels(info);
    for (int i = 0; i < 2; i++) {
        SkCodec::FrameInfo frameInfo;
        REPORTER_ASSERT(r, codec->getFrameInfo(i, &frameInfo));

        SkIRect expectedRect = i == 0 ? SkIRect{0, 0, 100, 90} : SkIRect{100, 90, 100, 90};
        REPORTER_ASSERT(r, expectedRect == frameInfo.fFrameRect);

        SkCodec::Options options;
        options.fFrameIndex = i;
        REPORTER_ASSERT(r, SkCodec::kSuccess == codec->getPixels(bm.pixmap(), &options));

        REPORTER_ASSERT(r, bm.getColor(0, 0) == SK_ColorRED);
    }
}
