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
        const SkIRect& desiredSubset, int sampleSize, SkColorType dstColorType,
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
    SkAlphaType dstAlphaType = fCodec->getInfo().alphaType();
    if (kOpaque_SkAlphaType != dstAlphaType) {
        dstAlphaType = requireUnpremul ? kUnpremul_SkAlphaType : kPremul_SkAlphaType;
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
    options.fColorPtr = colorPtr;
    options.fColorCount = colorCountPtr;
    options.fZeroInitialized = zeroInit;
    void* dst = bitmap->getAddr(scaledOutX, scaledOutY);

    // FIXME: skbug.com/4538
    // It is important that we use the rowBytes on the pixelRef.  They may not be
    // set properly on the bitmap.
    SkPixelRef* pr = SkRef(bitmap->pixelRef());
    size_t rowBytes = pr->rowBytes();
    bitmap->setInfo(outInfo, rowBytes);
    bitmap->setPixelRef(pr)->unref();
    bitmap->lockPixels();
    SkCodec::Result result = fCodec->getAndroidPixels(decodeInfo, dst, rowBytes, &options);
    if (SkCodec::kSuccess != result && SkCodec::kIncompleteInput != result) {
        SkCodecPrintf("Error: Could not get pixels.\n");
        return false;
    }

    return true;
}

bool SkBitmapRegionCodec::conversionSupported(SkColorType colorType) {
    // FIXME: Call virtual function when it lands.
    SkImageInfo info = SkImageInfo::Make(0, 0, colorType, fCodec->getInfo().alphaType(),
            fCodec->getInfo().profileType());
    return conversion_possible(info, fCodec->getInfo());
}
