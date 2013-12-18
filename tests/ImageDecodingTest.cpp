/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "TestClassDef.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkColor.h"
#include "SkColorPriv.h"
#include "SkData.h"
#include "SkForceLinking.h"
#include "SkGradientShader.h"
#include "SkImageDecoder.h"
#include "SkImageEncoder.h"
#include "SkOSFile.h"
#include "SkPoint.h"
#include "SkShader.h"
#include "SkStream.h"
#include "SkString.h"

__SK_FORCE_IMAGE_DECODER_LINKING;

/**
 *  Interprets c as an unpremultiplied color, and returns the
 *  premultiplied equivalent.
 */
static SkPMColor premultiply_unpmcolor(SkPMColor c) {
    U8CPU a = SkGetPackedA32(c);
    U8CPU r = SkGetPackedR32(c);
    U8CPU g = SkGetPackedG32(c);
    U8CPU b = SkGetPackedB32(c);
    return SkPreMultiplyARGB(a, r, g, b);
}

/**
 *  Return true if this stream format should be skipped, due
 *  to do being an opaque format or not a valid format.
 */
static bool skip_image_format(SkImageDecoder::Format format) {
    switch (format) {
        case SkImageDecoder::kPNG_Format:
        case SkImageDecoder::kWEBP_Format:
            return false;
        // Skip unknown since it will not be decoded anyway.
        case SkImageDecoder::kUnknown_Format:
        // Technically ICO and BMP supports alpha channels, but our image
        // decoders do not, so skip them as well.
        case SkImageDecoder::kICO_Format:
        case SkImageDecoder::kBMP_Format:
        // The rest of these are opaque.
        case SkImageDecoder::kWBMP_Format:
        case SkImageDecoder::kGIF_Format:
        case SkImageDecoder::kJPEG_Format:
            return true;
    }
    SkASSERT(false);
    return true;
}

/**
 *  Test decoding an image in premultiplied mode and unpremultiplied mode and compare
 *  them.
 */
static void compare_unpremul(skiatest::Reporter* reporter, const SkString& filename) {
    // Decode a resource:
    SkBitmap bm8888;
    SkBitmap bm8888Unpremul;

    SkFILEStream stream(filename.c_str());

    SkImageDecoder::Format format = SkImageDecoder::GetStreamFormat(&stream);
    if (skip_image_format(format)) {
        return;
    }

    SkAutoTDelete<SkImageDecoder> decoder(SkImageDecoder::Factory(&stream));
    if (NULL == decoder.get()) {
        SkDebugf("couldn't decode %s\n", filename.c_str());
        return;
    }

    bool success = decoder->decode(&stream, &bm8888, SkBitmap::kARGB_8888_Config,
                                   SkImageDecoder::kDecodePixels_Mode);
    if (!success) {
        return;
    }

    success = stream.rewind();
    REPORTER_ASSERT(reporter, success);
    if (!success) {
        return;
    }

    decoder->setRequireUnpremultipliedColors(true);
    success = decoder->decode(&stream, &bm8888Unpremul, SkBitmap::kARGB_8888_Config,
                              SkImageDecoder::kDecodePixels_Mode);
    if (!success) {
        return;
    }

    bool dimensionsMatch = bm8888.width() == bm8888Unpremul.width()
                           && bm8888.height() == bm8888Unpremul.height();
    REPORTER_ASSERT(reporter, dimensionsMatch);
    if (!dimensionsMatch) {
        return;
    }

    // Only do the comparison if the two bitmaps are both 8888.
    if (bm8888.config() != SkBitmap::kARGB_8888_Config
        || bm8888Unpremul.config() != SkBitmap::kARGB_8888_Config) {
        return;
    }

    // Now compare the two bitmaps.
    for (int i = 0; i < bm8888.width(); ++i) {
        for (int j = 0; j < bm8888.height(); ++j) {
            // "c0" is the color of the premultiplied bitmap at (i, j).
            const SkPMColor c0 = *bm8888.getAddr32(i, j);
            // "c1" is the result of premultiplying the color of the unpremultiplied
            // bitmap at (i, j).
            const SkPMColor c1 = premultiply_unpmcolor(*bm8888Unpremul.getAddr32(i, j));
            // Compute the difference for each component.
            int da = SkAbs32(SkGetPackedA32(c0) - SkGetPackedA32(c1));
            int dr = SkAbs32(SkGetPackedR32(c0) - SkGetPackedR32(c1));
            int dg = SkAbs32(SkGetPackedG32(c0) - SkGetPackedG32(c1));
            int db = SkAbs32(SkGetPackedB32(c0) - SkGetPackedB32(c1));

            // Alpha component must be exactly the same.
            REPORTER_ASSERT(reporter, 0 == da);

            // Color components may not match exactly due to rounding error.
            REPORTER_ASSERT(reporter, dr <= 1);
            REPORTER_ASSERT(reporter, dg <= 1);
            REPORTER_ASSERT(reporter, db <= 1);
        }
    }
}

static void test_unpremul(skiatest::Reporter* reporter) {
    // This test cannot run if there is no resource path.
    SkString resourcePath = skiatest::Test::GetResourcePath();
    if (resourcePath.isEmpty()) {
        SkDebugf("Could not run unpremul test because resourcePath not specified.");
        return;
    }
    SkOSFile::Iter iter(resourcePath.c_str());
    SkString basename;
    if (iter.next(&basename)) {
        do {
            SkString filename = SkOSPath::SkPathJoin(resourcePath.c_str(), basename.c_str());
            //SkDebugf("about to decode \"%s\"\n", filename.c_str());
            compare_unpremul(reporter, filename);
        } while (iter.next(&basename));
    } else {
        SkDebugf("Failed to find any files :(\n");
    }
}

#ifdef SK_DEBUG
// Create a stream containing a bitmap encoded to Type type.
static SkMemoryStream* create_image_stream(SkImageEncoder::Type type) {
    SkBitmap bm;
    const int size = 50;
    bm.setConfig(SkBitmap::kARGB_8888_Config, size, size);
    bm.allocPixels();
    SkCanvas canvas(bm);
    SkPoint points[2] = {
        { SkIntToScalar(0), SkIntToScalar(0) },
        { SkIntToScalar(size), SkIntToScalar(size) }
    };
    SkColor colors[2] = { SK_ColorWHITE, SK_ColorBLUE };
    SkShader* shader = SkGradientShader::CreateLinear(points, colors, NULL,
                                                      SK_ARRAY_COUNT(colors),
                                                      SkShader::kClamp_TileMode);
    SkPaint paint;
    paint.setShader(shader)->unref();
    canvas.drawPaint(paint);
    // Now encode it to a stream.
    SkAutoTUnref<SkData> data(SkImageEncoder::EncodeData(bm, type, 100));
    if (NULL == data.get()) {
        return NULL;
    }
    return SkNEW_ARGS(SkMemoryStream, (data.get()));
}

// For every format that supports tile based decoding, ensure that
// calling decodeSubset will not fail if the caller has unreffed the
// stream provided in buildTileIndex.
// Only runs in debug mode since we are testing for a crash.
static void test_stream_life() {
    const SkImageEncoder::Type gTypes[] = {
#ifdef SK_BUILD_FOR_ANDROID
        SkImageEncoder::kJPEG_Type,
        SkImageEncoder::kPNG_Type,
#endif
        SkImageEncoder::kWEBP_Type,
    };
    for (size_t i = 0; i < SK_ARRAY_COUNT(gTypes); ++i) {
        //SkDebugf("encoding to %i\n", i);
        SkAutoTUnref<SkMemoryStream> stream(create_image_stream(gTypes[i]));
        if (NULL == stream.get()) {
            SkDebugf("no stream\n");
            continue;
        }
        SkAutoTDelete<SkImageDecoder> decoder(SkImageDecoder::Factory(stream));
        if (NULL == decoder.get()) {
            SkDebugf("no decoder\n");
            continue;
        }
        int width, height;
        if (!decoder->buildTileIndex(stream.get(), &width, &height)) {
            SkDebugf("could not build a tile index\n");
            continue;
        }
        // Now unref the stream to make sure it survives
        stream.reset(NULL);
        SkBitmap bm;
        decoder->decodeSubset(&bm, SkIRect::MakeWH(width, height),
                              SkBitmap::kARGB_8888_Config);
    }
}

// Test inside SkScaledBitmapSampler.cpp
extern void test_row_proc_choice();

#endif // SK_DEBUG

DEF_TEST(ImageDecoding, reporter) {
    test_unpremul(reporter);
#ifdef SK_DEBUG
    test_stream_life();
    test_row_proc_choice();
#endif
}
