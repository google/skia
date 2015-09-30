/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapRegionCanvas.h"
#include "SkCanvas.h"

SkBitmapRegionCanvas::SkBitmapRegionCanvas(SkCodec* decoder)
    : INHERITED(decoder->getInfo().width(), decoder->getInfo().height())
    , fDecoder(decoder)
{}

/*
 * Chooses the correct image subset offsets and dimensions for the partial decode.
 */
static inline void set_subset_region(int inputOffset, int inputDimension,
        int imageOriginalDimension, int* imageSubsetOffset, int* outOffset,
        int* imageSubsetDimension) {

    // This must be at least zero, we can't start decoding the image at a negative coordinate.
    *imageSubsetOffset = SkTMax(0, inputOffset);

    // If inputOffset is less than zero, we decode to an offset location in the output bitmap.
    *outOffset = *imageSubsetOffset - inputOffset;

    // Use imageSusetOffset to make sure we don't decode pixels past the edge of the image.
    // Use outOffset to make sure we don't decode pixels past the edge of the region.
    *imageSubsetDimension = SkTMin(imageOriginalDimension - *imageSubsetOffset,
            inputDimension - *outOffset);
}

/*
 * Returns a scaled dimension based on the original dimension and the sample size.
 * TODO: Share this implementation with SkScaledCodec.
 */
static int get_scaled_dimension(int srcDimension, int sampleSize) {
    if (sampleSize > srcDimension) {
        return 1;
    }
    return srcDimension / sampleSize;
}

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
        SkDebugf("Error: Color type not supported.\n");
        return nullptr;
    }

    // The client may not necessarily request a region that is fully within
    // the image.  We may need to do some calculation to determine what part
    // of the image to decode.

    // The left offset of the portion of the image we want, where zero
    // indicates the left edge of the image.
    int imageSubsetX;

    // The size of the output bitmap is determined by the size of the
    // requested region, not by the size of the intersection of the region
    // and the image dimensions.  If inputX is negative, we will need to
    // place decoded pixels into the output bitmap starting at a left offset.
    // If this is non-zero, imageSubsetX must be zero.
    int outX;

    // The width of the portion of the image that we will write to the output
    // bitmap.  If the region is not fully contained within the image, this
    // will not be the same as inputWidth.
    int imageSubsetWidth;
    set_subset_region(inputX, inputWidth, this->width(), &imageSubsetX, &outX, &imageSubsetWidth);

    // The top offset of the portion of the image we want, where zero
    // indicates the top edge of the image.
    int imageSubsetY;

    // The size of the output bitmap is determined by the size of the
    // requested region, not by the size of the intersection of the region
    // and the image dimensions.  If inputY is negative, we will need to
    // place decoded pixels into the output bitmap starting at a top offset.
    // If this is non-zero, imageSubsetY must be zero.
    int outY;

    // The height of the portion of the image that we will write to the output
    // bitmap.  If the region is not fully contained within the image, this
    // will not be the same as inputHeight.
    int imageSubsetHeight;
    set_subset_region(inputY, inputHeight, this->height(), &imageSubsetY, &outY,
            &imageSubsetHeight);

    if (imageSubsetWidth <= 0 || imageSubsetHeight <= 0) {
        SkDebugf("Error: Region must intersect part of the image.\n");
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
        SkDebugf("Error: Could not start scanline decoder.\n");
        return nullptr;
    }

    // Allocate a bitmap for the unscaled decode
    SkBitmap tmp;
    SkImageInfo tmpInfo = decodeInfo.makeWH(this->width(), imageSubsetHeight);
    if (!tmp.tryAllocPixels(tmpInfo)) {
        SkDebugf("Error: Could not allocate pixels.\n");
        return nullptr;
    }

    // Skip the unneeded rows
    if (SkCodec::kSuccess != fDecoder->skipScanlines(imageSubsetY)) {
        SkDebugf("Error: Failed to skip scanlines.\n");
        return nullptr;
    }

    // Decode the necessary rows
    SkCodec::Result result = fDecoder->getScanlines(tmp.getAddr(0, 0), imageSubsetHeight,
            tmp.rowBytes());
    switch (result) {
        case SkCodec::kSuccess:
        case SkCodec::kIncompleteInput:
            break;
        default:
            SkDebugf("Error: Failed to get scanlines.\n");
            return nullptr;
    }

    // Calculate the size of the output
    const int outWidth = get_scaled_dimension(inputWidth, sampleSize);
    const int outHeight = get_scaled_dimension(inputHeight, sampleSize);

    // Initialize the destination bitmap
    SkAutoTDelete<SkBitmap> bitmap(new SkBitmap());
    SkImageInfo dstInfo = decodeInfo.makeWH(outWidth, outHeight);
    if (!bitmap->tryAllocPixels(dstInfo)) {
        SkDebugf("Error: Could not allocate pixels.\n");
        return nullptr;
    }

    // Zero the bitmap if the region is not completely within the image.
    // TODO (msarett): Can we make this faster by implementing it to only
    //                 zero parts of the image that we won't overwrite with
    //                 pixels?
    // TODO (msarett): This could be skipped if memory is zero initialized.
    //                 This would matter if this code is moved to Android and
    //                 uses Android bitmaps.
    if (0 != outX || 0 != outY ||
            inputX + inputWidth > this->width() ||
            inputY + inputHeight > this->height()) {
        bitmap->eraseColor(0);
    }

    // Use a canvas to crop and scale to the destination bitmap
    SkCanvas canvas(*bitmap);
    // TODO (msarett): Maybe we can take advantage of the fact that SkRect uses floats?
    SkRect src = SkRect::MakeXYWH((SkScalar) imageSubsetX, (SkScalar) 0,
            (SkScalar) imageSubsetWidth, (SkScalar) imageSubsetHeight);
    SkRect dst = SkRect::MakeXYWH((SkScalar) (outX / sampleSize), (SkScalar) (outY / sampleSize),
            (SkScalar) get_scaled_dimension(imageSubsetWidth, sampleSize),
            (SkScalar) get_scaled_dimension(imageSubsetHeight, sampleSize));
    SkPaint paint;
    // Overwrite the dst with the src pixels
    paint.setXfermodeMode(SkXfermode::kSrc_Mode);
    // TODO (msarett): Test multiple filter qualities.  kNone is the default.
    canvas.drawBitmapRect(tmp, src, dst, &paint);

    return bitmap.detach();
}
