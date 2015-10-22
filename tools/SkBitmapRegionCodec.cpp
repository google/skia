/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapRegionCodec.h"
#include "SkAndroidCodec.h"
#include "SkCodecPriv.h"
#include "SkCodecTools.h"

SkBitmapRegionCodec::SkBitmapRegionCodec(SkAndroidCodec* codec)
    : INHERITED(codec->getInfo().width(), codec->getInfo().height())
    , fCodec(codec)
{}

/*
 * Three differences from the Android version:
 *     Returns a skia bitmap instead of an Android bitmap.
 *     Android version attempts to reuse a recycled bitmap.
 *     Removed the options object and used parameters for color type and sample size.
 */
// FIXME: Should this function should take in SkIRect?
SkBitmap* SkBitmapRegionCodec::decodeRegion(int inputX, int inputY, int inputWidth, int inputHeight,
        int sampleSize, SkColorType dstColorType) {

    // Fix the input sampleSize if necessary.
    if (sampleSize < 1) {
        sampleSize = 1;
    }

    // The size of the output bitmap is determined by the size of the
    // requested subset, not by the size of the intersection of the subset
    // and the image dimensions.
    // If inputX is negative, we will need to place decoded pixels into the
    // output bitmap starting at a left offset.  Call this outX.
    // If outX is non-zero, subsetX must be zero.
    // If inputY is negative, we will need to place decoded pixels into the
    // output bitmap starting at a top offset.  Call this outY.
    // If outY is non-zero, subsetY must be zero.
    int outX;
    int outY;
    SkIRect subset = SkIRect::MakeXYWH(inputX, inputY, inputWidth, inputHeight);
    SubsetType type = adjust_subset_rect(fCodec->getInfo().dimensions(), &subset, &outX, &outY);
    if (SubsetType::kOutside_SubsetType == type) {
        return nullptr;
    }

    // Ask the codec for a scaled subset
    if (!fCodec->getSupportedSubset(&subset)) {
        SkCodecPrintf("Error: Could not get subset.\n");
        return nullptr;
    }
    SkISize scaledSize = fCodec->getSampledSubsetDimensions(sampleSize, subset);

    // Create the image info for the decode
    SkAlphaType dstAlphaType = fCodec->getInfo().alphaType();
    if (kUnpremul_SkAlphaType == dstAlphaType) {
        dstAlphaType = kPremul_SkAlphaType;
    }
    SkImageInfo decodeInfo = SkImageInfo::Make(scaledSize.width(), scaledSize.height(),
            dstColorType, dstAlphaType);

    // Construct a color table for the decode if necessary
    SkAutoTUnref<SkColorTable> colorTable(nullptr);
    SkPMColor* colorPtr = nullptr;
    int* colorCountPtr = nullptr;
    int maxColors = 256;
    SkPMColor colors[256];
    if (kIndex_8_SkColorType == dstColorType) {
        // TODO (msarett): This performs a copy that is unnecessary since
        //                 we have not yet initialized the color table.
        //                 And then we need to use a const cast to get
        //                 a pointer to the color table that we can
        //                 modify during the decode.  We could alternatively
        //                 perform the decode before creating the bitmap and
        //                 the color table.  We still would need to copy the
        //                 colors into the color table after the decode.
        colorTable.reset(new SkColorTable(colors, maxColors));
        colorPtr = const_cast<SkPMColor*>(colorTable->readColors());
        colorCountPtr = &maxColors;
    }

    // Initialize the destination bitmap
    SkAutoTDelete<SkBitmap> bitmap(new SkBitmap());
    int scaledOutX = 0;
    int scaledOutY = 0;
    int scaledOutWidth = scaledSize.width();
    int scaledOutHeight = scaledSize.height();
    if (SubsetType::kPartiallyInside_SubsetType == type) {
        scaledOutX = outX / sampleSize;
        scaledOutY = outY / sampleSize;
        // We need to be safe here because getSupportedSubset() may have modified the subset.
        const int extraX = SkTMax(0, inputWidth - outX - subset.width());
        const int extraY = SkTMax(0, inputHeight - outY - subset.height());
        const int scaledExtraX = extraX / sampleSize;
        const int scaledExtraY = extraY / sampleSize;
        scaledOutWidth += scaledOutX + scaledExtraX;
        scaledOutHeight += scaledOutY + scaledExtraY;
    }
    SkImageInfo outInfo = decodeInfo.makeWH(scaledOutWidth, scaledOutHeight);
    if (!bitmap->tryAllocPixels(outInfo, nullptr, colorTable.get())) {
        SkCodecPrintf("Error: Could not allocate pixels.\n");
        return nullptr;
    }

    // Zero the bitmap if the region is not completely within the image.
    // TODO (msarett): Can we make this faster by implementing it to only
    //                 zero parts of the image that we won't overwrite with
    //                 pixels?
    // TODO (msarett): This could be skipped if memory is zero initialized.
    //                 This would matter if this code is moved to Android and
    //                 uses Android bitmaps.
    if (SubsetType::kPartiallyInside_SubsetType == type) {
        void* pixels = bitmap->getPixels();
        size_t bytes = outInfo.getSafeSize(bitmap->rowBytes());
        memset(pixels, 0, bytes);
    }

    // Decode into the destination bitmap
    SkAndroidCodec::AndroidOptions options;
    options.fSampleSize = sampleSize;
    options.fSubset = &subset;
    options.fColorPtr = colorPtr;
    options.fColorCount = colorCountPtr;
    void* dst = bitmap->getAddr(scaledOutX, scaledOutY);
    size_t rowBytes = bitmap->rowBytes();
    SkCodec::Result result = fCodec->getAndroidPixels(decodeInfo, dst, rowBytes, &options);
    if (SkCodec::kSuccess != result && SkCodec::kIncompleteInput != result) {
        SkCodecPrintf("Error: Could not get pixels.\n");
        return nullptr;
    }

    return bitmap.detach();
}

bool SkBitmapRegionCodec::conversionSupported(SkColorType colorType) {
    // FIXME: Call virtual function when it lands.
    SkImageInfo info = SkImageInfo::Make(0, 0, colorType, fCodec->getInfo().alphaType(),
            fCodec->getInfo().profileType());
    return conversion_possible(info, fCodec->getInfo());
}
