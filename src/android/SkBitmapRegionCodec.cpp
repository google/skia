/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAndroidCodec.h"
#include "SkBitmapRegionCodec.h"
#include "SkBitmapRegionDecoderPriv.h"
#include "SkCodecPriv.h"
#include "SkPixelRef.h"

SkBitmapRegionCodec::SkBitmapRegionCodec(SkAndroidCodec* codec)
    : INHERITED(codec->getInfo().width(), codec->getInfo().height())
    , fCodec(codec)
{}

bool SkBitmapRegionCodec::decodeRegion(SkBitmap* bitmap, SkBRDAllocator* allocator,
        const SkIRect& desiredSubset, int sampleSize, SkColorType prefColorType,
        bool requireUnpremul) {

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
    SkIRect subset = desiredSubset;
    SubsetType type = adjust_subset_rect(fCodec->getInfo().dimensions(), &subset, &outX, &outY);
    if (SubsetType::kOutside_SubsetType == type) {
        return false;
    }

    // Ask the codec for a scaled subset
    if (!fCodec->getSupportedSubset(&subset)) {
        SkCodecPrintf("Error: Could not get subset.\n");
        return false;
    }
    SkISize scaledSize = fCodec->getSampledSubsetDimensions(sampleSize, subset);

    // Create the image info for the decode
    SkColorType dstColorType = fCodec->computeOutputColorType(prefColorType);
    SkAlphaType dstAlphaType = fCodec->computeOutputAlphaType(requireUnpremul);
    SkImageInfo decodeInfo = fCodec->getInfo().makeWH(scaledSize.width(), scaledSize.height())
                                              .makeColorType(dstColorType)
                                              .makeAlphaType(dstAlphaType);

    // Construct a color table for the decode if necessary
    SkAutoTUnref<SkColorTable> colorTable(nullptr);
    int maxColors = 256;
    SkPMColor colors[256];
    if (kIndex_8_SkColorType == dstColorType) {
        colorTable.reset(new SkColorTable(colors, maxColors));
    }

    // Initialize the destination bitmap
    int scaledOutX = 0;
    int scaledOutY = 0;
    int scaledOutWidth = scaledSize.width();
    int scaledOutHeight = scaledSize.height();
    if (SubsetType::kPartiallyInside_SubsetType == type) {
        scaledOutX = outX / sampleSize;
        scaledOutY = outY / sampleSize;
        // We need to be safe here because getSupportedSubset() may have modified the subset.
        const int extraX = SkTMax(0, desiredSubset.width() - outX - subset.width());
        const int extraY = SkTMax(0, desiredSubset.height() - outY - subset.height());
        const int scaledExtraX = extraX / sampleSize;
        const int scaledExtraY = extraY / sampleSize;
        scaledOutWidth += scaledOutX + scaledExtraX;
        scaledOutHeight += scaledOutY + scaledExtraY;
    }
    SkImageInfo outInfo = decodeInfo.makeWH(scaledOutWidth, scaledOutHeight);
    if (kGray_8_SkColorType == dstColorType) {
        // The legacy implementations of BitmapFactory and BitmapRegionDecoder
        // used kAlpha8 for grayscale images (before kGray8 existed).  While
        // the codec recognizes kGray8, we need to decode into a kAlpha8
        // bitmap in order to avoid a behavior change.
        outInfo = outInfo.makeColorType(kAlpha_8_SkColorType).makeAlphaType(kPremul_SkAlphaType);
    }
    bitmap->setInfo(outInfo);
    if (!bitmap->tryAllocPixels(allocator, colorTable.get())) {
        SkCodecPrintf("Error: Could not allocate pixels.\n");
        return false;
    }

    // Zero the bitmap if the region is not completely within the image.
    // TODO (msarett): Can we make this faster by implementing it to only
    //                 zero parts of the image that we won't overwrite with
    //                 pixels?
    SkCodec::ZeroInitialized zeroInit = allocator ? allocator->zeroInit() :
            SkCodec::kNo_ZeroInitialized;
    if (SubsetType::kPartiallyInside_SubsetType == type &&
            SkCodec::kNo_ZeroInitialized == zeroInit) {
        void* pixels = bitmap->getPixels();
        size_t bytes = outInfo.getSafeSize(bitmap->rowBytes());
        memset(pixels, 0, bytes);
    }

    // Decode into the destination bitmap
    SkAndroidCodec::AndroidOptions options;
    options.fSampleSize = sampleSize;
    options.fSubset = &subset;
    options.fColorPtr = colors;
    options.fColorCount = &maxColors;
    options.fZeroInitialized = zeroInit;
    void* dst = bitmap->getAddr(scaledOutX, scaledOutY);

    SkCodec::Result result = fCodec->getAndroidPixels(decodeInfo, dst, bitmap->rowBytes(),
            &options);
    if (SkCodec::kSuccess != result && SkCodec::kIncompleteInput != result) {
        SkCodecPrintf("Error: Could not get pixels.\n");
        return false;
    }

    // Intialize the color table
    if (kIndex_8_SkColorType == dstColorType) {
        colorTable->dangerous_overwriteColors(colors, maxColors);
    }

    return true;
}

bool SkBitmapRegionCodec::conversionSupported(SkColorType colorType) {
    return conversion_possible(fCodec->getInfo().makeColorType(colorType), fCodec->getInfo());
}
