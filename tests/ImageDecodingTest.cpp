/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkColorPriv.h"
#include "SkForceLinking.h"
#include "SkImageDecoder.h"
#include "SkOSFile.h"
#include "SkStream.h"
#include "SkString.h"
#include "Test.h"

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
            // Other components may differ if rounding is done differently,
            // but currently that is not the case. If an image fails here
            // in the future, we can change these to account for differences.
            REPORTER_ASSERT(reporter, 0 == dr);
            REPORTER_ASSERT(reporter, 0 == dg);
            REPORTER_ASSERT(reporter, 0 == db);
        }
    }
}

static void test_imageDecodingTests(skiatest::Reporter* reporter) {
    // This test cannot run if there is no resource path.
    SkString resourcePath = skiatest::Test::GetResourcePath();
    if (resourcePath.isEmpty()) {
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

#include "TestClassDef.h"
DEFINE_TESTCLASS("ImageDecoding", ImageDecodingTestClass,
                 test_imageDecodingTests)
