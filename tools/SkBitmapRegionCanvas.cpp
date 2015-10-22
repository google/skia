/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapRegionCanvas.h"
#include "SkCanvas.h"
#include "SkCodecPriv.h"
#include "SkCodecTools.h"

SkBitmapRegionCanvas::SkBitmapRegionCanvas(SkCodec* decoder)
    : INHERITED(decoder->getInfo().width(), decoder->getInfo().height())
    , fDecoder(decoder)
{}

/*
 * Three differences from the Android version:
 *     Returns a Skia bitmap instead of an Android bitmap.
 *     Android version attempts to reuse a recycled bitmap.
 *     Removed the options object and used parameters for color type and
 *     sample size.
 */
SkBitmap* SkBitmapRegionCanvas::decodeRegion(int inputX, int inputY,
                                             int inputWidth, int inputHeight,
                                             int sampleSize,
                                             SkColorType dstColorType) {
    // Reject color types not supported by this method
    if (kIndex_8_SkColorType == dstColorType || kGray_8_SkColorType == dstColorType) {
        SkCodecPrintf("Error: Color type not supported.\n");
        return nullptr;
    }

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
    SubsetType type = adjust_subset_rect(fDecoder->getInfo().dimensions(), &subset, &outX, &outY);
    if (SubsetType::kOutside_SubsetType == type) {
        return nullptr;
    }

    // Create the image info for the decode
    SkAlphaType dstAlphaType = fDecoder->getInfo().alphaType();
    if (kUnpremul_SkAlphaType == dstAlphaType) {
        dstAlphaType = kPremul_SkAlphaType;
    }
    SkImageInfo decodeInfo = SkImageInfo::Make(this->width(), this->height(),
            dstColorType, dstAlphaType);

    // Start the scanline decoder
    SkCodec::Result r = fDecoder->startScanlineDecode(decodeInfo);
    if (SkCodec::kSuccess != r) {
        SkCodecPrintf("Error: Could not start scanline decoder.\n");
        return nullptr;
    }

    // Allocate a bitmap for the unscaled decode
    SkBitmap tmp;
    SkImageInfo tmpInfo = decodeInfo.makeWH(this->width(), subset.height());
    if (!tmp.tryAllocPixels(tmpInfo)) {
        SkCodecPrintf("Error: Could not allocate pixels.\n");
        return nullptr;
    }

    // Skip the unneeded rows
    if (!fDecoder->skipScanlines(subset.y())) {
        SkCodecPrintf("Error: Failed to skip scanlines.\n");
        return nullptr;
    }

    // Decode the necessary rows
    fDecoder->getScanlines(tmp.getAddr(0, 0), subset.height(), tmp.rowBytes());

    // Calculate the size of the output
    const int outWidth = get_scaled_dimension(inputWidth, sampleSize);
    const int outHeight = get_scaled_dimension(inputHeight, sampleSize);

    // Initialize the destination bitmap
    SkAutoTDelete<SkBitmap> bitmap(new SkBitmap());
    SkImageInfo dstInfo = decodeInfo.makeWH(outWidth, outHeight);
    if (!bitmap->tryAllocPixels(dstInfo)) {
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
        bitmap->eraseColor(0);
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

    return bitmap.detach();
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
