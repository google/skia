/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBitmapRegionDecoderPriv_DEFINED
#define SkBitmapRegionDecoderPriv_DEFINED

#include "include/core/SkRect.h"

enum SubsetType {
    kFullyInside_SubsetType,
    kPartiallyInside_SubsetType,
    kOutside_SubsetType,
};

/*
 * Corrects image subset offsets and dimensions in order to perform a valid decode.
 * Also indicates if the image subset should be placed at an offset within the
 * output bitmap.
 *
 * Values of output variables are undefined if the SubsetType is kInvalid.
 *
 * @param imageDims Original image dimensions.
 * @param subset    As input, the subset that the client requested.
 *                  As output, the image subset that we will decode.
 * @param outX      The left offset of the image subset within the output bitmap.
 * @param outY      The top offset of the image subset within the output bitmap.
 *
 * @return An indication of how the subset is contained in the image.
 *         If the return value is kInvalid, values of output variables are undefined.
 */
inline SubsetType adjust_subset_rect(const SkISize& imageDims, SkIRect* subset, int* outX,
        int* outY) {
    // These must be at least zero, we can't start decoding the image at a negative coordinate.
    int left = std::max(0, subset->fLeft);
    int top = std::max(0, subset->fTop);

    // If input offsets are less than zero, we decode to an offset location in the output bitmap.
    *outX = left - subset->fLeft;
    *outY = top - subset->fTop;

    // Make sure we don't decode pixels past the edge of the image or past the edge of the subset.
    int width = std::min(imageDims.width() - left, subset->width() - *outX);
    int height = std::min(imageDims.height() - top, subset->height() - *outY);
    if (width <= 0 || height <= 0) {
        return SubsetType::kOutside_SubsetType;
    }

    subset->setXYWH(left, top, width, height);
    if ((*outX != 0) || (*outY != 0) || (width != subset->width()) ||
            (height != subset->height())) {
        return SubsetType::kPartiallyInside_SubsetType;
    }

    return SubsetType::kFullyInside_SubsetType;
}

#endif // SkBitmapRegionDecoderPriv_DEFINED
