/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"

// This tests out GIF decoder (SkImageDecoder_libgif.cpp)
// It is not used on these platforms:
#if (!defined(SK_BUILD_FOR_WIN32)) &&           \
    (!defined(SK_BUILD_FOR_IOS)) &&             \
    (!defined(SK_BUILD_FOR_MAC))

#include "Resources.h"
#include "SkBitmap.h"
#include "SkData.h"
#include "SkForceLinking.h"
#include "SkImage.h"
#include "SkImageDecoder.h"
#include "SkStream.h"
#include "Test.h"

__SK_FORCE_IMAGE_DECODER_LINKING;

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
  0x47, 0x49, 0x46, 0x38, 0x39, 0x61, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
  0x21, 0xf9, 0x04, 0x01, 0x0a, 0x00, 0x01, 0x00, 0x2c, 0x00, 0x00, 0x00, 0x00,
  0x01, 0x00, 0x01, 0x00, 0x00, 0x02, 0x02, 0x4c, 0x01, 0x00, 0x3b
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
    bool imageDecodeSuccess = SkImageDecoder::DecodeMemory(
        data, size, &bm);
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
    bool imageDecodeSuccess = SkImageDecoder::DecodeMemory(
        data, size, &bm);
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
static void test_interlaced_gif_data(skiatest::Reporter* r,
                                     void* data,
                                     size_t size) {
    SkBitmap bm;
    bool imageDecodeSuccess = SkImageDecoder::DecodeMemory(
        data, size, &bm);
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
    bool imageDecodeSuccess = SkImageDecoder::DecodeMemory(
        data, size, &bm);
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
  This test will test the ability of the SkImageDecoder to deal with
  GIF files which have been mangled somehow.  We want to display as
  much of the GIF as possible.
*/
DEF_TEST(Gif, reporter) {
    // test perfectly good images.
    test_gif_data(reporter, static_cast<void *>(gGIFData), sizeof(gGIFData));
    test_interlaced_gif_data(reporter, static_cast<void *>(gInterlacedGIF),
                          sizeof(gInterlacedGIF));

    unsigned char badData[sizeof(gGIFData)];

    /* If you set the environment variable
       skia_images_gif_suppressDecoderWarnings to 'false', you will
       see warnings on stderr.  This is a feature.  */

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
    test_gif_data(reporter, static_cast<void *>(badData), sizeof(gGIFData));
    // "libgif warning [shifting image left to fit]"

    memcpy(badData, gGIFData, sizeof(gGIFData));
    badData[64] = 0x01;  // image shifted down
    test_gif_data(reporter, static_cast<void *>(badData), sizeof(gGIFData));
    // "libgif warning [shifting image up to fit]"

    memcpy(badData, gGIFData, sizeof(gGIFData));
    badData[62] = 0xff;  // image shifted left
    badData[63] = 0xff;  // 2's complement -1 short
    test_gif_data(reporter, static_cast<void *>(badData), sizeof(gGIFData));
    // "libgif warning [shifting image left to fit]"

    memcpy(badData, gGIFData, sizeof(gGIFData));
    badData[64] = 0xff;  // image shifted up
    badData[65] = 0xff;  // 2's complement -1 short
    test_gif_data(reporter, static_cast<void *>(badData), sizeof(gGIFData));
    // "libgif warning [shifting image up to fit]"

    test_gif_data_no_colormap(reporter, static_cast<void *>(gGIFDataNoColormap),
                              sizeof(gGIFDataNoColormap));
    // "libgif warning [missing colormap]"

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
    SkFILEStream fileStream(GetResourcePath("test640x479.gif").c_str());
    REPORTER_ASSERT(r, fileStream.isValid());
    if (!fileStream.isValid()) {
        return;
    }

    SkAutoTDelete<SkImageDecoder> decoder(SkImageDecoder::Factory(&fileStream));
    REPORTER_ASSERT(r, decoder);
    if (!decoder) {
        return;
    }
    decoder->setSampleSize(4);
    SkBitmap bm;
    const SkImageDecoder::Result result = decoder->decode(&fileStream, &bm,
            SkImageDecoder::kDecodePixels_Mode);
    REPORTER_ASSERT(r, result == SkImageDecoder::kSuccess);
}

#endif  // !(SK_BUILD_FOR_WIN32||SK_BUILD_FOR_IOS||SK_BUILD_FOR_MAC)
