/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapRegionCanvas.h"
#include "SkBitmapRegionDecoderPriv.h"
#include "SkCanvas.h"
#include "SkCodecPriv.h"

SkBitmapRegionCanvas::SkBitmapRegionCanvas(SkCodec* decoder)
    : INHERITED(decoder->getInfo().width(), decoder->getInfo().height())
    , fDecoder(decoder)
{}

bool SkBitmapRegionCanvas::decodeRegion(SkBitmap* bitmap, SkBRDAllocator* allocator,
        const SkIRect& desiredSubset, int sampleSize, SkColorType dstColorType,
        bool requireUnpremul) {
    // Reject color types not supported by this method
    if (kIndex_8_SkColorType == dstColorType || kGray_8_SkColorType == dstColorType) {
        SkCodecPrintf("Error: Color type not supported.\n");
        return false;
    }

    // Reject requests for unpremultiplied alpha
    if (requireUnpremul) {
        SkCodecPrintf("Error: Alpha type not supported.\n");
        return false;
    }
    SkAlphaType dstAlphaType = fDecoder->getInfo().alphaType();
    if (kUnpremul_SkAlphaType == dstAlphaType) {
        dstAlphaType = kPremul_SkAlphaType;
    }

    // FIXME: Can we add checks and support kIndex8 or unpremultiplied alpha in special cases?

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
    SubsetType type = adjust_subset_rect(fDecoder->getInfo().dimensions(), &subset, &outX, &outY);
    if (SubsetType::kOutside_SubsetType == type) {
        return false;
    }

    // Create the image info for the decode
    SkImageInfo decodeInfo = SkImageInfo::Make(this->width(), this->height(),
            dstColorType, dstAlphaType);

    // Start the scanline decoder
    SkCodec::Result r = fDecoder->startScanlineDecode(decodeInfo);
    if (SkCodec::kSuccess != r) {
        SkCodecPrintf("Error: Could not start scanline decoder.\n");
        return false;
    }

    // Allocate a bitmap for the unscaled decode
    SkBitmap tmp;
    SkImageInfo tmpInfo = decodeInfo.makeWH(this->width(), subset.height());
    if (!tmp.tryAllocPixels(tmpInfo)) {
        SkCodecPrintf("Error: Could not allocate pixels.\n");
        return false;
    }

    // Skip the unneeded rows
    if (!fDecoder->skipScanlines(subset.y())) {
        SkCodecPrintf("Error: Failed to skip scanlines.\n");
        return false;
    }

    // Decode the necessary rows
    fDecoder->getScanlines(tmp.getAddr(0, 0), subset.height(), tmp.rowBytes());

    // Calculate the size of the output
    const int outWidth = get_scaled_dimension(desiredSubset.width(), sampleSize);
    const int outHeight = get_scaled_dimension(desiredSubset.height(), sampleSize);

    // Initialize the destination bitmap
    SkImageInfo dstInfo = decodeInfo.makeWH(outWidth, outHeight);
    bitmap->setInfo(dstInfo, dstInfo.minRowBytes());
    if (!bitmap->tryAllocPixels(allocator, nullptr)) {
        SkCodecPrintf("Error: Could not allocate pixels.\n");
        return false;
    }

    // Zero the bitmap if the region is not completely within the image.
    // TODO (msarett): Can we make this faster by implementing it to only
    //                 zero parts of the image that we won't overwrite with
    //                 pixels?
    if (SubsetType::kPartiallyInside_SubsetType == type) {
        SkCodec::ZeroInitialized zeroInit = allocator ? allocator->zeroInit() :
                    SkCodec::kNo_ZeroInitialized;
        if (SkCodec::kNo_ZeroInitialized == zeroInit) {
            bitmap->eraseColor(0);
        }
    }

    // Use a canvas to crop and scale to the destination bitmap
    SkCanvas canvas(*bitmap);
    // TODO (msarett): Maybe we can take advantage of the fact that SkRect uses floats?
    SkRect src = SkRect::MakeXYWH((SkScalar) subset.x(), (SkScalar) 0,
            (SkScalar) subset.width(), (SkScalar) subset.height());
    SkRect dst = SkRect::MakeXYWH((SkScalar) (outX / sampleSize), (SkScalar) (outY / sampleSize),
            (SkScalar) get_scaled_dimension(subset.width(), sampleSize),
            (SkScalar) get_scaled_dimension(subset.height(), sampleSize));
    SkPaint paint;
    // Overwrite the dst with the src pixels
    paint.setXfermodeMode(SkXfermode::kSrc_Mode);
    // TODO (msarett): Test multiple filter qualities.  kNone is the default.
    canvas.drawBitmapRect(tmp, src, dst, &paint);

    return true;
}

bool SkBitmapRegionCanvas::conversionSupported(SkColorType colorType) {
    // SkCanvas does not draw to these color types.
    if (kIndex_8_SkColorType == colorType || kGray_8_SkColorType == colorType) {
        return false;
    }

    // FIXME: Call virtual function when it lands.
    SkImageInfo info = SkImageInfo::Make(0, 0, colorType, fDecoder->getInfo().alphaType(),
            fDecoder->getInfo().profileType());
    return conversion_possible(info, fDecoder->getInfo());
}
