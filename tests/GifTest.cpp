/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "CodecPriv.h"
#include "Resources.h"
#include "SkAndroidCodec.h"
#include "SkBitmap.h"
#include "SkData.h"
#include "SkImage.h"
#include "SkStream.h"
#include "SkTypes.h"
#include "Test.h"

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

    // Since there is no color map, we do not even need to parse the image data
    // to know that we should draw transparent. Truncate the file before the
    // data. This should still succeed.
    test_gif_data_no_colormap(reporter, static_cast<void *>(gGIFDataNoColormap), 31);

    // Likewise, incremental decoding should succeed here.
    {
        sk_sp<SkData> data = SkData::MakeWithoutCopy(gGIFDataNoColormap, 31);
        std::unique_ptr<SkCodec> codec(SkCodec::NewFromData(data));
        REPORTER_ASSERT(reporter, codec);
        if (codec) {
            auto info = codec->getInfo().makeColorType(kN32_SkColorType);
            SkBitmap bm;
            bm.allocPixels(info);
            REPORTER_ASSERT(reporter, SkCodec::kSuccess == codec->startIncrementalDecode(
                    info, bm.getPixels(), bm.rowBytes()));
            REPORTER_ASSERT(reporter, SkCodec::kSuccess == codec->incrementalDecode());
            REPORTER_ASSERT(reporter, bm.width() == 1);
            REPORTER_ASSERT(reporter, bm.height() == 1);
            REPORTER_ASSERT(reporter, !(bm.empty()));
            if (!(bm.empty())) {
                REPORTER_ASSERT(reporter, bm.getColor(0, 0) == 0x00000000);
            }
        }
    }

    // test short Gif.  80 is missing a few bytes.
    test_gif_data_short(reporter, static_cast<void *>(gGIFData), 80);
    // "libgif warning [DGifGetLine]"

    test_interlaced_gif_data(reporter, static_cast<void *>(gInterlacedGIF),
                             100);  // 100 is missing a few bytes
    // "libgif warning [interlace DGifGetLine]"
}

// Regression test for decoding a gif image with sampleSize of 4, which was
// previously crashing.
DEF_TEST(Gif_Sampled, r) {
    std::unique_ptr<SkFILEStream> stream(
            new SkFILEStream(GetResourcePath("test640x479.gif").c_str()));
    REPORTER_ASSERT(r, stream->isValid());
    if (!stream->isValid()) {
        return;
    }

    std::unique_ptr<SkAndroidCodec> codec(SkAndroidCodec::NewFromStream(stream.release()));
    REPORTER_ASSERT(r, codec);
    if (!codec) {
        return;
    }

    // Construct a color table for the decode if necessary
    sk_sp<SkColorTable> colorTable(nullptr);
    SkPMColor* colorPtr = nullptr;
    int* colorCountPtr = nullptr;
    int maxColors = 256;
    if (kIndex_8_SkColorType == codec->getInfo().colorType()) {
        SkPMColor colors[256];
        colorTable.reset(new SkColorTable(colors, maxColors));
        colorPtr = const_cast<SkPMColor*>(colorTable->readColors());
        colorCountPtr = &maxColors;
    }

    SkAndroidCodec::AndroidOptions options;
    options.fSampleSize = 4;
    options.fColorPtr = colorPtr;
    options.fColorCount = colorCountPtr;

    SkBitmap bm;
    bm.allocPixels(codec->getInfo(), colorTable);
    const SkCodec::Result result = codec->getAndroidPixels(codec->getInfo(), bm.getPixels(),
            bm.rowBytes(), &options);
    REPORTER_ASSERT(r, result == SkCodec::kSuccess);
}

// If a GIF file is truncated before the header for the first image is defined,
// we should not create an SkCodec.
DEF_TEST(Codec_GifTruncated, r) {
    SkString path = GetResourcePath("test640x479.gif");
    sk_sp<SkData> data(SkData::MakeFromFileName(path.c_str()));

    // This is right before the header for the first image.
    data = SkData::MakeSubset(data.get(), 0, 446);
    std::unique_ptr<SkCodec> codec(SkCodec::NewFromData(data));
    REPORTER_ASSERT(r, !codec);
}

// There was a bug where SkAndroidCodec::computeOutputColorType returned kIndex_8 for
// GIFs that did not support kIndex_8. Verify that for such an image, the method computes
// something that it can actually decode to.
DEF_TEST(Codec_GifIndex8, r) {
    std::unique_ptr<SkStream> stream(GetResourceAsStream("randPixelsOffset.gif"));
    if (!stream) {
        return;
    }

    std::unique_ptr<SkAndroidCodec> codec(SkAndroidCodec::NewFromStream(stream.release()));
    REPORTER_ASSERT(r, codec);
    if (!codec) {
        return;
    }

    REPORTER_ASSERT(r, codec->getInfo().colorType() == kN32_SkColorType);
    const SkColorType outputColorType = codec->computeOutputColorType(kN32_SkColorType);
    REPORTER_ASSERT(r, outputColorType == kN32_SkColorType);

    SkAndroidCodec::AndroidOptions options;
    sk_sp<SkColorTable> colorTable(nullptr);
    int maxColors = 256;
    if (kIndex_8_SkColorType == outputColorType) {
        SkPMColor colors[256];
        colorTable.reset(new SkColorTable(colors, maxColors));
        options.fColorPtr = const_cast<SkPMColor*>(colorTable->readColors());
        options.fColorCount = &maxColors;
    }

    auto info = codec->getInfo().makeColorType(outputColorType);
    SkBitmap bm;
    bm.setInfo(info);
    bm.allocPixels(colorTable.get());

    REPORTER_ASSERT(r, SkCodec::kSuccess == codec->getAndroidPixels(info, bm.getPixels(),
            bm.rowBytes(), &options));
}
