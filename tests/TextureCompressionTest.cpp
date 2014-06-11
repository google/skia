/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkData.h"
#include "SkEndian.h"
#include "SkImageInfo.h"
#include "SkTextureCompressor.h"
#include "Test.h"

static const int kLATCBlockDimension = 4;
static const int kLATCEncodedBlockSize = 8;

/**
 * Make sure that we properly fail when we don't have multiple of four image dimensions.
 */
DEF_TEST(CompressLATCFailDimensions, reporter) {
    SkBitmap bitmap;
    static const int kWidth = 63;
    static const int kHeight = 63;
    SkImageInfo info = SkImageInfo::MakeA8(kWidth, kHeight);
    REPORTER_ASSERT(reporter, kWidth % kLATCBlockDimension != 0);
    REPORTER_ASSERT(reporter, kHeight % kLATCBlockDimension != 0);

    bool setInfoSuccess = bitmap.setInfo(info);
    REPORTER_ASSERT(reporter, setInfoSuccess);

    bool allocPixelsSuccess = bitmap.allocPixels(info);
    REPORTER_ASSERT(reporter, allocPixelsSuccess);
    bitmap.unlockPixels();

    const SkTextureCompressor::Format kLATCFormat = SkTextureCompressor::kLATC_Format;
    SkAutoDataUnref latcData(SkTextureCompressor::CompressBitmapToFormat(bitmap, kLATCFormat));
    REPORTER_ASSERT(reporter, NULL == latcData);
}

/**
 * Make sure that we properly fail when we don't have the correct bitmap type.
 * LATC compressed textures can only be created from A8 bitmaps.
 */
DEF_TEST(CompressLATCFailColorType, reporter) {
    SkBitmap bitmap;
    static const int kWidth = 64;
    static const int kHeight = 64;
    SkImageInfo info = SkImageInfo::MakeN32Premul(kWidth, kHeight);
    REPORTER_ASSERT(reporter, kWidth % kLATCBlockDimension == 0);
    REPORTER_ASSERT(reporter, kHeight % kLATCBlockDimension == 0);

    bool setInfoSuccess = bitmap.setInfo(info);
    REPORTER_ASSERT(reporter, setInfoSuccess);

    bool allocPixelsSuccess = bitmap.allocPixels(info);
    REPORTER_ASSERT(reporter, allocPixelsSuccess);
    bitmap.unlockPixels();

    const SkTextureCompressor::Format kLATCFormat = SkTextureCompressor::kLATC_Format;
    SkAutoDataUnref latcData(SkTextureCompressor::CompressBitmapToFormat(bitmap, kLATCFormat));
    REPORTER_ASSERT(reporter, NULL == latcData);
}

/**
 * Make sure that if we pass in a solid color bitmap that we get the appropriate results
 */
DEF_TEST(CompressLATC, reporter) {
    SkBitmap bitmap;
    static const int kWidth = 8;
    static const int kHeight = 8;
    SkImageInfo info = SkImageInfo::MakeA8(kWidth, kHeight);

    bool setInfoSuccess = bitmap.setInfo(info);
    REPORTER_ASSERT(reporter, setInfoSuccess);

    bool allocPixelsSuccess = bitmap.allocPixels(info);
    REPORTER_ASSERT(reporter, allocPixelsSuccess);
    bitmap.unlockPixels();

    REPORTER_ASSERT(reporter, kWidth % kLATCBlockDimension == 0);
    REPORTER_ASSERT(reporter, kHeight % kLATCBlockDimension == 0);
    const int numBlocks = (kWidth / kLATCBlockDimension) * (kHeight / kLATCBlockDimension);
    const size_t kSizeToBe = static_cast<size_t>(kLATCEncodedBlockSize * numBlocks);

    for (int lum = 0; lum < 256; ++lum) {
        bitmap.lockPixels();
        uint8_t* pixels = reinterpret_cast<uint8_t*>(bitmap.getPixels());
        REPORTER_ASSERT(reporter, NULL != pixels);

        for (int i = 0; i < kWidth*kHeight; ++i) {
            pixels[i] = lum;
        }
        bitmap.unlockPixels();

        const SkTextureCompressor::Format kLATCFormat = SkTextureCompressor::kLATC_Format;
        SkAutoDataUnref latcData(
            SkTextureCompressor::CompressBitmapToFormat(bitmap, kLATCFormat));
        REPORTER_ASSERT(reporter, NULL != latcData);
        REPORTER_ASSERT(reporter, kSizeToBe == latcData->size());

        // Make sure that it all matches a given block encoding. If the entire bitmap
        // is a single value, then the lower two bytes of the encoded data should be that
        // value. The remaining indices can be any value, and since we try to match the pixels
        // in the chosen palette in increasing index order, each one should be zero. Hence,
        // the correct encoding should be just the two luminance values in the bottom two
        // bytes of the block encoding.
        const uint64_t kConstColorEncoding = SkEndian_SwapLE64(lum | (lum << 8));
        const uint64_t* blockPtr = reinterpret_cast<const uint64_t*>(latcData->data());
        for (int i = 0; i < numBlocks; ++i) {
            REPORTER_ASSERT(reporter, blockPtr[i] == kConstColorEncoding);
        }
    }
}
