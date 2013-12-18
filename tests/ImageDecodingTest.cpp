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
#include "SkDecodingImageGenerator.h"
#include "SkDiscardableMemoryPool.h"
#include "SkForceLinking.h"
#include "SkGradientShader.h"
#include "SkImageDecoder.h"
#include "SkImageEncoder.h"
#include "SkImageGenerator.h"
#include "SkImagePriv.h"
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
            // SkDebugf("about to decode \"%s\"\n", filename.c_str());
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
        // SkDebugf("encoding to %i\n", i);
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

#endif  // SK_DEBUG

DEF_TEST(ImageDecoding, reporter) {
    test_unpremul(reporter);
#ifdef SK_DEBUG
    test_stream_life();
    test_row_proc_choice();
#endif
}

////////////////////////////////////////////////////////////////////////////////

// example of how Android will do this inside their BitmapFactory
static SkPixelRef* install_pixel_ref(SkBitmap* bitmap,
                                     SkStreamRewindable* stream,
                                     int sampleSize, bool ditherImage) {
    SkASSERT(bitmap != NULL);
    SkASSERT(stream != NULL);
    SkASSERT(stream->rewind());
    SkASSERT(stream->unique());
    SkColorType colorType;
    if (!SkBitmapConfigToColorType(bitmap->config(), &colorType)) {
        return NULL;
    }
    SkDecodingImageGenerator::Options opts(sampleSize, ditherImage, colorType);
    SkAutoTDelete<SkImageGenerator> gen(
        SkDecodingImageGenerator::Create(stream, opts));
    SkImageInfo info;
    if ((NULL == gen.get()) || !gen->getInfo(&info)) {
        return NULL;
    }
    SkDiscardableMemory::Factory* factory = NULL;
    if (info.getSafeSize(info.minRowBytes()) < (32 * 1024)) {
        // only use ashmem for large images, since mmaps come at a price
        factory = SkGetGlobalDiscardableMemoryPool();
    }
    if (SkInstallDiscardablePixelRef(gen.detach(), bitmap, factory)) {
        return bitmap->pixelRef();
    }
    return NULL;
}
/**
 *  A test for the SkDecodingImageGenerator::Create and
 *  SkInstallDiscardablePixelRef functions.
 */
DEF_TEST(ImprovedBitmapFactory, reporter) {
    SkString resourcePath = skiatest::Test::GetResourcePath();
    SkString directory = SkOSPath::SkPathJoin(resourcePath.c_str(), "encoding");
    SkString path = SkOSPath::SkPathJoin(directory.c_str(), "randPixels.png");
    SkAutoTUnref<SkStreamRewindable> stream(
        SkStream::NewFromFile(path.c_str()));
    if (sk_exists(path.c_str())) {
        SkBitmap bm;
        SkAssertResult(bm.setConfig(SkBitmap::kARGB_8888_Config, 1, 1));
        REPORTER_ASSERT(reporter,
            NULL != install_pixel_ref(&bm, stream.detach(), 1, true));
        SkAutoLockPixels alp(bm);
        REPORTER_ASSERT(reporter, NULL != bm.getPixels());
    }
}


////////////////////////////////////////////////////////////////////////////////

#if defined(SK_BUILD_FOR_ANDROID) || defined(SK_BUILD_FOR_UNIX)
static inline bool check_rounding(int value, int dividend, int divisor) {
    // returns true if (dividend/divisor) rounds up OR down to value
    return (((divisor * value) > (dividend - divisor))
            && ((divisor * value) < (dividend + divisor)));
}
#endif  // SK_BUILD_FOR_ANDROID || SK_BUILD_FOR_UNIX
namespace {
// expected output for 8x8 bitmap
const int kExpectedWidth = 8;
const int kExpectedHeight = 8;
const SkColor kExpectedPixels[] = {
    0xffbba570, 0xff395f5d, 0xffe25c39, 0xff197666,
    0xff3cba27, 0xffdefcb0, 0xffc13874, 0xfffa0093,
    0xffbda60e, 0xffc01db6, 0xff2bd688, 0xff9362d4,
    0xffc641b2, 0xffa5cede, 0xff606eba, 0xff8f4bf3,
    0xff3bf742, 0xff8f02a8, 0xff5509df, 0xffc7027e,
    0xff24aa8a, 0xff886c96, 0xff625481, 0xff403689,
    0xffc52152, 0xff78ccd6, 0xffdcb4ab, 0xff09d27d,
    0xffca00f3, 0xff605d47, 0xff446fb2, 0xff576e46,
    0xff273df9, 0xffb41a83, 0xfff812c3, 0xffccab67,
    0xff034218, 0xff7db9a7, 0xff821048, 0xfffe4ab4,
    0xff6fac98, 0xff941d27, 0xff5fe411, 0xfffbb283,
    0xffd86e99, 0xff169162, 0xff71128c, 0xff39cab4,
    0xffa7fe63, 0xff4c956b, 0xffbc22e0, 0xffb272e4,
    0xff129f4a, 0xffe34513, 0xff3d3742, 0xffbd190a,
    0xffb07222, 0xff2e23f8, 0xfff089d9, 0xffb35738,
    0xffa86022, 0xff3340fe, 0xff95fe71, 0xff6a71df
};
SK_COMPILE_ASSERT((kExpectedWidth * kExpectedHeight)
                  == SK_ARRAY_COUNT(kExpectedPixels), array_size_mismatch);
}  // namespace

/**
 * Given either a SkStream or a SkData, try to decode the encoded
 * image using the specified options and report errors.
 */
static void test_options(skiatest::Reporter* reporter,
                         const SkDecodingImageGenerator::Options& opts,
                         SkStreamRewindable* encodedStream,
                         SkData* encodedData,
                         bool useData,
                         const SkString& path) {
    SkBitmap bm;
    bool success = false;
    if (useData) {
        if (NULL == encodedData) {
            return;
        }
        success = SkInstallDiscardablePixelRef(
            SkDecodingImageGenerator::Create(encodedData, opts), &bm, NULL);
    } else {
        if (NULL == encodedStream) {
            return;
        }
        success = SkInstallDiscardablePixelRef(
            SkDecodingImageGenerator::Create(encodedStream->duplicate(), opts),
            &bm, NULL);
    }

    REPORTER_ASSERT(reporter, success
                    || (kARGB_4444_SkColorType == opts.fRequestedColorType));
    if (!success) {
        return;
    }
    #if defined(SK_BUILD_FOR_ANDROID) || defined(SK_BUILD_FOR_UNIX)
    // Android is the only system that use Skia's image decoders in
    // production.  For now, we'll only verify that samplesize works
    // on systems where it already is known to work.
    REPORTER_ASSERT(reporter, check_rounding(bm.height(), kExpectedHeight,
                                             opts.fSampleSize));
    REPORTER_ASSERT(reporter, check_rounding(bm.width(), kExpectedWidth,
                                             opts.fSampleSize));
    #endif  // SK_BUILD_FOR_ANDROID || SK_BUILD_FOR_UNIX
    SkAutoLockPixels alp(bm);
    REPORTER_ASSERT(reporter, bm.getPixels() != NULL);

    SkBitmap::Config requestedConfig
        = SkColorTypeToBitmapConfig(opts.fRequestedColorType);
    REPORTER_ASSERT(reporter,
                    (!opts.fUseRequestedColorType)
                    || (bm.config() == requestedConfig));

    // Condition under which we should check the decoding results:
    if ((SkBitmap::kARGB_8888_Config == bm.config())
        && (NULL != bm.getPixels())
        && (!path.endsWith(".jpg"))  // lossy
        && (!path.endsWith(".webp"))  // decoder error
        && (opts.fSampleSize == 1)) {  // scaled
        bool pixelError = false;
        const SkColor* correctPixels = kExpectedPixels;
        SkASSERT(bm.height() == kExpectedHeight);
        SkASSERT(bm.width() == kExpectedWidth);
        for (int y = 0; y < bm.height(); ++y) {
            for (int x = 0; x < bm.width(); ++x) {
                pixelError |= (*correctPixels != bm.getColor(x, y));
                ++correctPixels;
            }
        }
        REPORTER_ASSERT(reporter, !pixelError);
    }
}

/**
 *  SkDecodingImageGenerator has an Options struct which lets the
 *  client of the generator set sample size, dithering, and bitmap
 *  config.  This test loops through many possible options and tries
 *  them on a set of 5 small encoded images (each in a different
 *  format).  We test both SkData and SkStreamRewindable decoding.
 */
DEF_TEST(ImageDecoderOptions, reporter) {
    const char* files[]  = {
        "randPixels.bmp",
        "randPixels.jpg",
        "randPixels.png",
        "randPixels.webp",
        "randPixels.gif"
    };

    SkString resourceDir = skiatest::Test::GetResourcePath();
    SkString directory = SkOSPath::SkPathJoin(resourceDir.c_str(), "encoding");
    if (!sk_exists(directory.c_str())) {
        return;
    }

    int scaleList[] = {1, 2, 3, 4};
    bool ditherList[] = {true, false};
    SkColorType colorList[] = {
        kAlpha_8_SkColorType,
        kRGB_565_SkColorType,
        kARGB_4444_SkColorType,  // Most decoders will fail on 4444.
        kPMColor_SkColorType
        // Note that indexed color is left out of the list.  Lazy
        // decoding doesn't do indexed color.
    };
    const bool useDataList[] = {true, false};

    for (size_t fidx = 0; fidx < SK_ARRAY_COUNT(files); ++fidx) {
        SkString path = SkOSPath::SkPathJoin(directory.c_str(), files[fidx]);
        if (!sk_exists(path.c_str())) {
            continue;
        }

        SkAutoDataUnref encodedData(SkData::NewFromFileName(path.c_str()));
        REPORTER_ASSERT(reporter, encodedData.get() != NULL);
        SkAutoTUnref<SkStreamRewindable> encodedStream(
            SkStream::NewFromFile(path.c_str()));
        REPORTER_ASSERT(reporter, encodedStream.get() != NULL);

        for (size_t i = 0; i < SK_ARRAY_COUNT(scaleList); ++i) {
            for (size_t j = 0; j < SK_ARRAY_COUNT(ditherList); ++j) {
                for (size_t m = 0; m < SK_ARRAY_COUNT(useDataList); ++m) {
                    for (size_t k = 0; k < SK_ARRAY_COUNT(colorList); ++k) {
                        SkDecodingImageGenerator::Options opts(scaleList[i],
                                                               ditherList[j],
                                                               colorList[k]);
                        test_options(reporter, opts, encodedStream, encodedData,
                                     useDataList[m], path);

                    }
                    SkDecodingImageGenerator::Options options(scaleList[i],
                                                              ditherList[j]);
                    test_options(reporter, options, encodedStream, encodedData,
                                 useDataList[m], path);
                }
            }
        }
    }
}

