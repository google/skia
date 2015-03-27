/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Resources.h"
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
#include "SkImageGeneratorPriv.h"
#include "SkImagePriv.h"
#include "SkOSFile.h"
#include "SkPoint.h"
#include "SkShader.h"
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
        // KTX and ASTC are texture formats so it's not particularly clear how to 
        // decode the alpha from them.
        case SkImageDecoder::kKTX_Format:
        case SkImageDecoder::kASTC_Format:
        // The rest of these are opaque.
        case SkImageDecoder::kPKM_Format:
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

    bool success = decoder->decode(&stream, &bm8888, kN32_SkColorType,
                                   SkImageDecoder::kDecodePixels_Mode) != SkImageDecoder::kFailure;
    if (!success) {
        return;
    }

    success = stream.rewind();
    REPORTER_ASSERT(reporter, success);
    if (!success) {
        return;
    }

    decoder->setRequireUnpremultipliedColors(true);
    success = decoder->decode(&stream, &bm8888Unpremul, kN32_SkColorType,
                              SkImageDecoder::kDecodePixels_Mode) != SkImageDecoder::kFailure;
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
    if (bm8888.colorType() != kN32_SkColorType || bm8888Unpremul.colorType() != kN32_SkColorType) {
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
    SkString resourcePath = GetResourcePath();
    if (resourcePath.isEmpty()) {
        SkDebugf("Could not run unpremul test because resourcePath not specified.");
        return;
    }
    SkOSFile::Iter iter(resourcePath.c_str());
    SkString basename;
    if (iter.next(&basename)) {
        do {
            SkString filename = SkOSPath::Join(resourcePath.c_str(), basename.c_str());
            // SkDebugf("about to decode \"%s\"\n", filename.c_str());
            compare_unpremul(reporter, filename);
        } while (iter.next(&basename));
    } else {
        SkDebugf("Failed to find any files :(\n");
    }
}

#if defined(SK_BUILD_FOR_ANDROID) || defined(SK_BUILD_FOR_UNIX)
// Test that the alpha type is what we expect.
static void test_alphaType(skiatest::Reporter* reporter, const SkString& filename,
                           bool requireUnpremul) {
    SkBitmap bm;
    SkFILEStream stream(filename.c_str());

    SkAutoTDelete<SkImageDecoder> decoder(SkImageDecoder::Factory(&stream));
    if (NULL == decoder.get()) {
        return;
    }

    decoder->setRequireUnpremultipliedColors(requireUnpremul);

    // Decode just the bounds. This should always succeed.
    bool success = decoder->decode(&stream, &bm, kN32_SkColorType,
                                   SkImageDecoder::kDecodeBounds_Mode);
    REPORTER_ASSERT(reporter, success);
    if (!success) {
        return;
    }

    // Keep track of the alpha type for testing later. If the full decode
    // succeeds, the alpha type should be the same, unless the full decode
    // determined that the alpha type should actually be opaque, which may
    // not be known when only decoding the bounds.
    const SkAlphaType boundsAlphaType = bm.alphaType();

    // rewind should always succeed on SkFILEStream.
    success = stream.rewind();
    REPORTER_ASSERT(reporter, success);
    if (!success) {
        return;
    }

    success = decoder->decode(&stream, &bm, kN32_SkColorType, SkImageDecoder::kDecodePixels_Mode);

    if (!success) {
        // When the decoder is set to require unpremul, if it does not support
        // unpremul it will fail. This is the only reason the decode should
        // fail (since we know the files we are using to test can be decoded).
        REPORTER_ASSERT(reporter, requireUnpremul);
        return;
    }

    // The bounds decode should return with either the requested
    // premul/unpremul or opaque, if that value could be determined when only
    // decoding the bounds.
    if (requireUnpremul) {
        REPORTER_ASSERT(reporter, kUnpremul_SkAlphaType == boundsAlphaType
                                  || kOpaque_SkAlphaType == boundsAlphaType
                                  || filename.endsWith(".ico"));
        // TODO(halcanary): Find out why color_wheel.ico fails this test.
    } else {
        REPORTER_ASSERT(reporter, kPremul_SkAlphaType == boundsAlphaType
                                  || kOpaque_SkAlphaType == boundsAlphaType);
    }

    // When decoding the full image, the alpha type should match the one
    // returned by the bounds decode, unless the full decode determined that
    // the alpha type is actually opaque.
    REPORTER_ASSERT(reporter, bm.alphaType() == boundsAlphaType
                              || bm.alphaType() == kOpaque_SkAlphaType);
}

DEF_TEST(ImageDecoding_alphaType, reporter) {
    SkString resourcePath = GetResourcePath();
    if (resourcePath.isEmpty()) {
        SkDebugf("Could not run alphaType test because resourcePath not specified.");
        return;
    }

    SkOSFile::Iter iter(resourcePath.c_str());
    SkString basename;
    if (iter.next(&basename)) {
        do {
            SkString filename = SkOSPath::Join(resourcePath.c_str(), basename.c_str());
            for (int truth = 0; truth <= 1; ++truth) {
                test_alphaType(reporter, filename, SkToBool(truth));
            }
        } while (iter.next(&basename));
    } else {
        SkDebugf("Failed to find any files :(\n");
    }

}

// Using known images, test that decoding into unpremul and premul behave as expected.
DEF_TEST(ImageDecoding_unpremul, reporter) {
    SkString resourcePath = GetResourcePath();
    if (resourcePath.isEmpty()) {
        SkDebugf("Could not run unpremul test because resourcePath not specified.");
        return;
    }
    const char* root = "half-transparent-white-pixel";
    const char* suffixes[] = { ".png", ".webp" };

    for (size_t i = 0; i < SK_ARRAY_COUNT(suffixes); ++i) {
        SkString basename = SkStringPrintf("%s%s", root, suffixes[i]);
        SkString fullName = SkOSPath::Join(resourcePath.c_str(), basename.c_str());

        SkBitmap bm;
        SkFILEStream stream(fullName.c_str());

        if (!stream.isValid()) {
            SkDebugf("file %s missing from resource directoy %s\n",
                     basename.c_str(), resourcePath.c_str());
            continue;
        }

        // This should never fail since we know the images we're decoding.
        SkAutoTDelete<SkImageDecoder> decoder(SkImageDecoder::Factory(&stream));
        REPORTER_ASSERT(reporter, decoder.get());
        if (NULL == decoder.get()) {
            continue;
        }

        // Test unpremultiplied. We know what color this should result in.
        decoder->setRequireUnpremultipliedColors(true);
        bool success = decoder->decode(&stream, &bm, kN32_SkColorType,
                                       SkImageDecoder::kDecodePixels_Mode);
        REPORTER_ASSERT(reporter, success);
        if (!success) {
            continue;
        }

        REPORTER_ASSERT(reporter, bm.width() == 1 && bm.height() == 1);
        {
            SkAutoLockPixels alp(bm);
            REPORTER_ASSERT(reporter, bm.getAddr32(0, 0)[0] == 0x7fffffff);
        }

        success = stream.rewind();
        REPORTER_ASSERT(reporter, success);
        if (!success) {
            continue;
        }

        // Test premultiplied. Once again, we know which color this should
        // result in.
        decoder->setRequireUnpremultipliedColors(false);
        success = decoder->decode(&stream, &bm, kN32_SkColorType,
                                  SkImageDecoder::kDecodePixels_Mode);
        REPORTER_ASSERT(reporter, success);
        if (!success) {
            continue;
        }

        REPORTER_ASSERT(reporter, bm.width() == 1 && bm.height() == 1);
        {
            SkAutoLockPixels alp(bm);
            REPORTER_ASSERT(reporter, bm.getAddr32(0, 0)[0] == 0x7f7f7f7f);
        }
    }
}
#endif // SK_BUILD_FOR_UNIX/ANDROID skbug.com/2388

#ifdef SK_DEBUG
// Test inside SkScaledBitmapSampler.cpp
extern void test_row_proc_choice();
#endif  // SK_DEBUG

DEF_TEST(ImageDecoding, reporter) {
    test_unpremul(reporter);
#ifdef SK_DEBUG
    test_row_proc_choice();
#endif
}

// expected output for 8x8 bitmap
static const int kExpectedWidth = 8;
static const int kExpectedHeight = 8;
static const SkColor kExpectedPixels[] = {
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

DEF_TEST(WebP, reporter) {
    const unsigned char encodedWebP[] = {
        0x52, 0x49, 0x46, 0x46, 0x2c, 0x01, 0x00, 0x00, 0x57, 0x45, 0x42, 0x50,
        0x56, 0x50, 0x38, 0x4c, 0x20, 0x01, 0x00, 0x00, 0x2f, 0x07, 0xc0, 0x01,
        0x00, 0xff, 0x01, 0x45, 0x03, 0x00, 0xe2, 0xd5, 0xae, 0x60, 0x2b, 0xad,
        0xd9, 0x68, 0x76, 0xb6, 0x8d, 0x6a, 0x1d, 0xc0, 0xe6, 0x19, 0xd6, 0x16,
        0xb7, 0xb4, 0xef, 0xcf, 0xc3, 0x15, 0x6c, 0xb3, 0xbd, 0x77, 0x0d, 0x85,
        0x6d, 0x1b, 0xa9, 0xb1, 0x2b, 0xdc, 0x3d, 0x83, 0xdb, 0x00, 0x00, 0xc8,
        0x26, 0xe5, 0x01, 0x99, 0x8a, 0xd5, 0xdd, 0xfc, 0x82, 0xcd, 0xcd, 0x9a,
        0x8c, 0x13, 0xcc, 0x1b, 0xba, 0xf5, 0x05, 0xdb, 0xee, 0x6a, 0xdb, 0x38,
        0x60, 0xfe, 0x43, 0x2c, 0xd4, 0x6a, 0x99, 0x4d, 0xc6, 0xc0, 0xd3, 0x28,
        0x1b, 0xc1, 0xb1, 0x17, 0x4e, 0x43, 0x0e, 0x3d, 0x27, 0xe9, 0xe4, 0x84,
        0x4f, 0x24, 0x62, 0x69, 0x85, 0x43, 0x8d, 0xc2, 0x04, 0x00, 0x07, 0x59,
        0x60, 0xfd, 0x8b, 0x4d, 0x60, 0x32, 0x72, 0xcf, 0x88, 0x0c, 0x2f, 0x2f,
        0xad, 0x62, 0xbd, 0x27, 0x09, 0x16, 0x70, 0x78, 0x6c, 0xd9, 0x82, 0xef,
        0x1a, 0xa2, 0xcc, 0xf0, 0xf1, 0x6f, 0xd8, 0x78, 0x2e, 0x39, 0xa1, 0xcf,
        0x14, 0x4b, 0x89, 0xb4, 0x1b, 0x48, 0x15, 0x7c, 0x48, 0x6f, 0x8c, 0x20,
        0xb7, 0x00, 0xcf, 0xfc, 0xdb, 0xd0, 0xe9, 0xe7, 0x42, 0x09, 0xa4, 0x03,
        0x40, 0xac, 0xda, 0x40, 0x01, 0x00, 0x5f, 0xa1, 0x3d, 0x64, 0xe1, 0xf4,
        0x03, 0x45, 0x29, 0xe0, 0xe2, 0x4a, 0xc3, 0xa2, 0xe8, 0xe0, 0x25, 0x12,
        0x74, 0xc6, 0xe8, 0xfb, 0x93, 0x4f, 0x9f, 0x5e, 0xc0, 0xa6, 0x91, 0x1b,
        0xa4, 0x24, 0x82, 0xc3, 0x61, 0x07, 0x4c, 0x49, 0x4f, 0x53, 0xae, 0x5f,
        0x5d, 0x39, 0x36, 0xc0, 0x5b, 0x57, 0x54, 0x60, 0x10, 0x00, 0x00, 0xd1,
        0x68, 0xb6, 0x6d, 0xdb, 0x36, 0x22, 0xfa, 0x1f, 0x35, 0x75, 0x22, 0xec,
        0x31, 0xbc, 0x5d, 0x8f, 0x87, 0x53, 0xa2, 0x05, 0x8c, 0x2f, 0xcd, 0xa8,
        0xa7, 0xf3, 0xa3, 0xbd, 0x83, 0x8b, 0x2a, 0xc8, 0x58, 0xf5, 0xac, 0x80,
        0xe3, 0xfe, 0x66, 0xa4, 0x7c, 0x1b, 0x6c, 0xd1, 0xa9, 0xd8, 0x14, 0xd0,
        0xc5, 0xb5, 0x39, 0x71, 0x97, 0x19, 0x19, 0x1b
    };
    SkAutoDataUnref encoded(SkData::NewWithCopy(encodedWebP,
                                                sizeof(encodedWebP)));
    SkBitmap bm;

    bool success = SkInstallDiscardablePixelRef(encoded, &bm);

    REPORTER_ASSERT(reporter, success);
    if (!success) {
        return;
    }
    SkAutoLockPixels alp(bm);

    bool rightSize = ((kExpectedWidth == bm.width())
                      && (kExpectedHeight == bm.height()));
    REPORTER_ASSERT(reporter, rightSize);
    if (rightSize) {
        bool error = false;
        const SkColor* correctPixel = kExpectedPixels;
        for (int y = 0; y < bm.height(); ++y) {
            for (int x = 0; x < bm.width(); ++x) {
                error |= (*correctPixel != bm.getColor(x, y));
                ++correctPixel;
            }
        }
        REPORTER_ASSERT(reporter, !error);
    }
}

////////////////////////////////////////////////////////////////////////////////

// example of how Android will do this inside their BitmapFactory
static SkPixelRef* install_pixel_ref(SkBitmap* bitmap,
                                     SkStreamRewindable* stream,
                                     int sampleSize, bool ditherImage) {
    SkASSERT(bitmap != NULL);
    SkASSERT(stream != NULL);
    SkASSERT(stream->rewind());
    SkColorType colorType = bitmap->colorType();
    SkDecodingImageGenerator::Options opts(sampleSize, ditherImage, colorType);
    if (SkInstallDiscardablePixelRef(
                SkDecodingImageGenerator::Create(stream, opts), bitmap)) {
        return bitmap->pixelRef();
    }
    return NULL;
}
/**
 *  A test for the SkDecodingImageGenerator::Create and
 *  SkInstallDiscardablePixelRef functions.
 */
DEF_TEST(ImprovedBitmapFactory, reporter) {
    SkString pngFilename = GetResourcePath("randPixels.png");
    SkAutoTDelete<SkStreamRewindable> stream(SkStream::NewFromFile(pngFilename.c_str()));
    if (sk_exists(pngFilename.c_str())) {
        SkBitmap bm;
        SkAssertResult(bm.setInfo(SkImageInfo::MakeN32Premul(1, 1)));
        REPORTER_ASSERT(reporter,
            install_pixel_ref(&bm, stream.detach(), 1, true));
        SkAutoLockPixels alp(bm);
        REPORTER_ASSERT(reporter, bm.getPixels());
    }
}


////////////////////////////////////////////////////////////////////////////////

#if defined(SK_BUILD_FOR_ANDROID) || defined(SK_BUILD_FOR_UNIX)
static inline bool check_rounding(int value, int dividend, int divisor) {
    // returns true if the value is greater than floor(dividend/divisor)
    // and less than SkNextPow2(ceil(dividend - divisor))
    return (((divisor * value) > (dividend - divisor))
            && value <= SkNextPow2(((dividend - 1) / divisor) + 1));
}
#endif  // SK_BUILD_FOR_ANDROID || SK_BUILD_FOR_UNIX


#if SK_PMCOLOR_BYTE_ORDER(B,G,R,A)
    #define kBackwards_SkColorType kRGBA_8888_SkColorType
#elif SK_PMCOLOR_BYTE_ORDER(R,G,B,A)
    #define kBackwards_SkColorType kBGRA_8888_SkColorType
#else
    #error "SK_*32_SHFIT values must correspond to BGRA or RGBA byte order"
#endif

static inline const char* SkColorType_to_string(SkColorType colorType) {
    switch(colorType) {
        case kAlpha_8_SkColorType:   return "Alpha_8";
        case kRGB_565_SkColorType:   return "RGB_565";
        case kARGB_4444_SkColorType: return "ARGB_4444";
        case kN32_SkColorType:       return "N32";
        case kBackwards_SkColorType: return "Backwards";
        case kIndex_8_SkColorType:   return "Index_8";
        default:                     return "ERROR";
    }
}

static inline const char* options_colorType(
        const SkDecodingImageGenerator::Options& opts) {
    if (opts.fUseRequestedColorType) {
        return SkColorType_to_string(opts.fRequestedColorType);
    } else {
        return "(none)";
    }
}

static inline const char* yn(bool value) {
    if (value) {
        return "yes";
    } else {
        return "no";
    }
}

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
            SkDecodingImageGenerator::Create(encodedData, opts), &bm);
    } else {
        if (NULL == encodedStream) {
            return;
        }
        success = SkInstallDiscardablePixelRef(
            SkDecodingImageGenerator::Create(encodedStream->duplicate(), opts), &bm);
    }
    if (!success) {
        if (opts.fUseRequestedColorType
            && (kARGB_4444_SkColorType == opts.fRequestedColorType)) {
            return;  // Ignore known conversion inabilities.
        }
        // If we get here, it's a failure and we will need more
        // information about why it failed.
        ERRORF(reporter, "Bounds decode failed [sampleSize=%d dither=%s "
               "colorType=%s %s]", opts.fSampleSize, yn(opts.fDitherImage),
               options_colorType(opts), path.c_str());
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
    // The ImageDecoder API doesn't guarantee that SampleSize does
    // anything at all, but the decoders that this test excercises all
    // produce an output size in the following range:
    //    (((sample_size * out_size) > (in_size - sample_size))
    //     && out_size <= SkNextPow2(((in_size - 1) / sample_size) + 1));
    #endif  // SK_BUILD_FOR_ANDROID || SK_BUILD_FOR_UNIX
    SkAutoLockPixels alp(bm);
    if (bm.getPixels() == NULL) {
        ERRORF(reporter, "Pixel decode failed [sampleSize=%d dither=%s "
               "colorType=%s %s]", opts.fSampleSize, yn(opts.fDitherImage),
               options_colorType(opts), path.c_str());
        return;
    }

    SkColorType requestedColorType = opts.fRequestedColorType;
    REPORTER_ASSERT(reporter,
                    (!opts.fUseRequestedColorType)
                    || (bm.colorType() == requestedColorType));

    // Condition under which we should check the decoding results:
    if ((kN32_SkColorType == bm.colorType())
        && (!path.endsWith(".jpg"))  // lossy
        && (opts.fSampleSize == 1)) {  // scaled
        const SkColor* correctPixels = kExpectedPixels;
        SkASSERT(bm.height() == kExpectedHeight);
        SkASSERT(bm.width() == kExpectedWidth);
        int pixelErrors = 0;
        for (int y = 0; y < bm.height(); ++y) {
            for (int x = 0; x < bm.width(); ++x) {
                if (*correctPixels != bm.getColor(x, y)) {
                    ++pixelErrors;
                }
                ++correctPixels;
            }
        }
        if (pixelErrors != 0) {
            ERRORF(reporter, "Pixel-level mismatch (%d of %d) "
                   "[sampleSize=%d dither=%s colorType=%s %s]",
                   pixelErrors, kExpectedHeight * kExpectedWidth,
                   opts.fSampleSize, yn(opts.fDitherImage),
                   options_colorType(opts), path.c_str());
        }
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
        #if !defined(SK_BUILD_FOR_WIN)
        // TODO(halcanary): Find out why this fails sometimes.
        "randPixels.gif",
        #endif
    };

    SkString resourceDir = GetResourcePath();
    if (!sk_exists(resourceDir.c_str())) {
        return;
    }

    int scaleList[] = {1, 2, 3, 4};
    bool ditherList[] = {true, false};
    SkColorType colorList[] = {
        kAlpha_8_SkColorType,
        kRGB_565_SkColorType,
        kARGB_4444_SkColorType,  // Most decoders will fail on 4444.
        kN32_SkColorType
        // Note that indexed color is left out of the list.  Lazy
        // decoding doesn't do indexed color.
    };
    const bool useDataList[] = {true, false};

    for (size_t fidx = 0; fidx < SK_ARRAY_COUNT(files); ++fidx) {
        SkString path = SkOSPath::Join(resourceDir.c_str(), files[fidx]);
        if (!sk_exists(path.c_str())) {
            continue;
        }

        SkAutoDataUnref encodedData(SkData::NewFromFileName(path.c_str()));
        REPORTER_ASSERT(reporter, encodedData.get() != NULL);
        SkAutoTDelete<SkStreamRewindable> encodedStream(
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

DEF_TEST(DiscardablePixelRef_SecondLockColorTableCheck, r) {
    SkString resourceDir = GetResourcePath();
    SkString path = SkOSPath::Join(resourceDir.c_str(), "randPixels.gif");
    if (!sk_exists(path.c_str())) {
        return;
    }
    SkAutoDataUnref encoded(SkData::NewFromFileName(path.c_str()));
    SkBitmap bitmap;
    if (!SkInstallDiscardablePixelRef(
            SkDecodingImageGenerator::Create(
                    encoded, SkDecodingImageGenerator::Options()), &bitmap)) {
        #ifndef SK_BUILD_FOR_WIN
        ERRORF(r, "SkInstallDiscardablePixelRef [randPixels.gif] failed.");
        #endif
        return;
    }
    if (kIndex_8_SkColorType != bitmap.colorType()) {
        return;
    }
    {
        SkAutoLockPixels alp(bitmap);
        REPORTER_ASSERT(r, bitmap.getColorTable() && "first pass");
    }
    {
        SkAutoLockPixels alp(bitmap);
        REPORTER_ASSERT(r, bitmap.getColorTable() && "second pass");
    }
}


////////////////////////////////////////////////////////////////////////////////
namespace {
class SingleAllocator : public SkBitmap::Allocator {
public:
    SingleAllocator(void* p, size_t s) : fPixels(p), fSize(s) { }
    ~SingleAllocator() {}
    // If the pixels in fPixels are big enough, use them.
    bool allocPixelRef(SkBitmap* bm, SkColorTable* ct) override {
        SkASSERT(bm);
        if (bm->info().getSafeSize(bm->rowBytes()) <= fSize) {
            bm->setPixels(fPixels, ct);
            fPixels = NULL;
            fSize = 0;
            return true;
        }
        return bm->tryAllocPixels(NULL, ct);
    }
    bool ready() { return fPixels != NULL; }
private:
    void* fPixels;
    size_t fSize;
};
}  // namespace

/*  This tests for a bug in libjpeg where INT32 is typedefed to long
    and memory can be written to outside of the array. */
DEF_TEST(ImageDecoding_JpegOverwrite, r) {
    SkString resourceDir = GetResourcePath();
    SkString path = SkOSPath::Join(resourceDir.c_str(), "randPixels.jpg");
    SkAutoTDelete<SkStreamAsset> stream(
            SkStream::NewFromFile(path.c_str()));
    if (!stream.get()) {
        SkDebugf("\nPath '%s' missing.\n", path.c_str());
        return;
    }
    SkAutoTDelete<SkImageDecoder> decoder(SkImageDecoder::Factory(stream));
    if (NULL == decoder.get()) {
        ERRORF(r, "\nSkImageDecoder::Factory failed.\n");
        return;
    }
    SkAssertResult(stream->rewind());

    static const uint16_t sentinal = 0xBEEF;
    static const int pixelCount = 16;
    SkAutoTMalloc<uint16_t> pixels(pixelCount + 1);
    // pixels.get() should be 4-byte aligned.
    // This is necessary to reproduce the bug.

    pixels[pixelCount] = sentinal;  // This value should not be changed.

    SkAutoTUnref<SingleAllocator> allocator(
            SkNEW_ARGS(SingleAllocator,
                       ((void*)pixels.get(), sizeof(uint16_t) * pixelCount)));
    decoder->setAllocator(allocator);
    decoder->setSampleSize(2);
    SkBitmap bitmap;
    bool success = decoder->decode(stream, &bitmap, kRGB_565_SkColorType,
                                   SkImageDecoder::kDecodePixels_Mode) != SkImageDecoder::kFailure;
    REPORTER_ASSERT(r, success);
    REPORTER_ASSERT(r, !allocator->ready());  // Decoder used correct memory
    REPORTER_ASSERT(r, sentinal == pixels[pixelCount]);
}
