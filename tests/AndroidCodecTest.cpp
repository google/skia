/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Resources.h"
#include "SkAndroidCodec.h"
#include "SkBitmap.h"
#include "SkCodec.h"
#include "SkCodecImageGenerator.h"
#include "SkColor.h"
#include "SkData.h"
#include "SkEncodedImageFormat.h"
#include "SkImageGenerator.h"
#include "SkImageInfo.h"
#include "SkPixmapPriv.h"
#include "SkRefCnt.h"
#include "SkSize.h"
#include "SkString.h"
#include "SkTypes.h"
#include "Test.h"

#include <algorithm>
#include <memory>

static SkISize times(const SkISize& size, float factor) {
    return { (int) (size.width() * factor), (int) (size.height() * factor) };
}

static SkISize plus(const SkISize& size, int term) {
    return { size.width() + term, size.height() + term };
}

static bool invalid(const SkISize& size) {
    return size.width() < 1 || size.height() < 1;
}

DEF_TEST(AndroidCodec_computeSampleSize, r) {
    if (GetResourcePath().isEmpty()) {
        return;
    }
    for (const char* file : { "images/color_wheel.webp",
                              "images/ship.png",
                              "images/dog.jpg",
                              "images/color_wheel.gif",
                              "images/rle.bmp",
                              "images/google_chrome.ico",
                              "images/mandrill.wbmp",
#ifdef SK_CODEC_DECODES_RAW
                              "images/sample_1mp.dng",
#endif
                              }) {
        auto data = GetResourceAsData(file);
        if (!data) {
            ERRORF(r, "Could not get %s", file);
            continue;
        }

        auto codec = SkAndroidCodec::MakeFromCodec(SkCodec::MakeFromData(std::move(data)));
        if (!codec) {
            ERRORF(r, "Could not create codec for %s", file);
            continue;
        }

        const auto dims = codec->getInfo().dimensions();
        const SkISize downscales[] = {
            plus(dims, -1),
            times(dims, .15f),
            times(dims, .6f),
            { (int32_t) (dims.width() * .25f), (int32_t) (dims.height() * .75f ) },
            { 1,  1 },
            { 1,  2 },
            { 2,  1 },
            { 0, -1 },
            { dims.width(), dims.height() - 1 },
        };
        for (SkISize size : downscales) {
            const auto requested = size;
            const int computedSampleSize = codec->computeSampleSize(&size);
            REPORTER_ASSERT(r, size.width() >= 1 && size.height() >= 1);
            if (codec->getEncodedFormat() == SkEncodedImageFormat::kWEBP) {
                // WebP supports arbitrary down-scaling.
                REPORTER_ASSERT(r, size == requested || invalid(requested));
            } else if (computedSampleSize == 1) {
                REPORTER_ASSERT(r, size == dims);
            } else {
                REPORTER_ASSERT(r, computedSampleSize > 1);
                if (size.width() >= dims.width() || size.height() >= dims.height()) {
                    ERRORF(r, "File %s's computed sample size (%i) is bigger than"
                              " original? original: %i x %i\tsampled: %i x %i",
                              file, computedSampleSize, dims.width(), dims.height(),
                              size.width(), size.height());
                }
                REPORTER_ASSERT(r, size.width()  >= requested.width() &&
                                   size.height() >= requested.height());
                REPORTER_ASSERT(r, size.width()  <  dims.width() &&
                                   size.height() <  dims.height());
            }
        }

        const SkISize upscales[] = {
            dims, plus(dims, 5), times(dims, 2),
        };
        for (SkISize size : upscales) {
            const int computedSampleSize = codec->computeSampleSize(&size);
            REPORTER_ASSERT(r, computedSampleSize == 1);
            REPORTER_ASSERT(r, dims == size);
        }

        // This mimics how Android's ImageDecoder uses SkAndroidCodec. A client
        // can choose their dimensions based on calling getSampledDimensions,
        // but the ImageDecoder API takes an arbitrary size. It then uses
        // computeSampleSize to determine the best dimensions and sampleSize.
        // It should return the same dimensions. the sampleSize may be different
        // due to integer division.
        for (int sampleSize : { 1, 2, 3, 4, 8, 16, 32 }) {
            const SkISize sampledDims = codec->getSampledDimensions(sampleSize);
            SkISize size = sampledDims;
            const int computedSampleSize = codec->computeSampleSize(&size);
            if (sampledDims != size) {
                ERRORF(r, "File '%s'->getSampledDimensions(%i) yields computed"
                          " sample size of %i\n\tsampledDimensions: %i x %i\t"
                          "computed dimensions: %i x %i",
                          file, sampleSize, computedSampleSize,
                          sampledDims.width(), sampledDims.height(),
                          size.width(), size.height());
            }
        }
    }
}

DEF_TEST(AndroidCodec_orientation, r) {
    if (GetResourcePath().isEmpty()) {
        return;
    }

    for (const char* ext : { "jpg", "webp" })
    for (char i = '1'; i <= '8'; ++i) {
        SkString path = SkStringPrintf("images/orientation/%c.%s", i, ext);
        auto data = GetResourceAsData(path.c_str());
        auto gen = SkCodecImageGenerator::MakeFromEncodedCodec(data);
        if (!gen) {
            ERRORF(r, "failed to decode %s", path.c_str());
            return;
        }

        // Dimensions after adjusting for the origin.
        const SkISize expectedDims = { 100, 80 };

        // SkCodecImageGenerator automatically adjusts for the origin.
        REPORTER_ASSERT(r, gen->getInfo().dimensions() == expectedDims);

        auto androidCodec = SkAndroidCodec::MakeFromCodec(SkCodec::MakeFromData(data));
        if (!androidCodec) {
            ERRORF(r, "failed to decode %s", path.c_str());
            return;
        }

        // SkAndroidCodec does not adjust for the origin by default. Dimensions may be reversed.
        if (SkPixmapPriv::ShouldSwapWidthHeight(androidCodec->codec()->getOrigin())) {
            auto swappedDims = SkPixmapPriv::SwapWidthHeight(androidCodec->getInfo()).dimensions();
            REPORTER_ASSERT(r, expectedDims == swappedDims);
        } else {
            REPORTER_ASSERT(r, expectedDims == androidCodec->getInfo().dimensions());
        }

        // Passing kRespect adjusts for the origin.
        androidCodec = SkAndroidCodec::MakeFromCodec(SkCodec::MakeFromData(std::move(data)),
                SkAndroidCodec::ExifOrientationBehavior::kRespect);
        auto info = androidCodec->getInfo();
        REPORTER_ASSERT(r, info.dimensions() == expectedDims);

        SkBitmap fromGenerator;
        fromGenerator.allocPixels(info);
        REPORTER_ASSERT(r, gen->getPixels(info, fromGenerator.getPixels(),
                                          fromGenerator.rowBytes()));

        SkBitmap fromAndroidCodec;
        fromAndroidCodec.allocPixels(info);
        auto result = androidCodec->getPixels(info, fromAndroidCodec.getPixels(),
                                              fromAndroidCodec.rowBytes());
        REPORTER_ASSERT(r, result == SkCodec::kSuccess);

        for (int i = 0; i < info.width();  ++i)
        for (int j = 0; j < info.height(); ++j) {
            SkColor c1 = *fromGenerator   .getAddr32(i, j);
            SkColor c2 = *fromAndroidCodec.getAddr32(i, j);
            if (c1 != c2) {
                ERRORF(r, "Bitmaps for %s do not match starting at position %i, %i\n"
                          "\tfromGenerator: %x\tfromAndroidCodec: %x", path.c_str(), i, j,
                          c1, c2);
                return;
            }
        }
    }
}
