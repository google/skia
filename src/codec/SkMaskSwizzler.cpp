/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCodecPriv.h"
#include "SkColorPriv.h"
#include "SkMaskSwizzler.h"

/*
 *
 * Row procedure for masked color components with 16 bits per pixel
 *
 */
static SkSwizzler::ResultAlpha swizzle_mask16_to_n32(
        void* dstRow, const uint8_t* srcRow, int width, SkMasks* masks) {

    // Use the masks to decode to the destination
    uint16_t* srcPtr = (uint16_t*) srcRow;
    SkPMColor* dstPtr = (SkPMColor*) dstRow;
    for (int i = 0; i < width; i++) {
        uint16_t p = srcPtr[i];
        uint8_t red = masks->getRed(p);
        uint8_t green = masks->getGreen(p);
        uint8_t blue = masks->getBlue(p);
        dstPtr[i] = SkPackARGB32NoCheck(0xFF, red, green, blue);
    }
    return SkSwizzler::kOpaque_ResultAlpha;
}

/*
 *
 * Row procedure for masked color components with 16 bits per pixel with alpha
 *
 */
static SkSwizzler::ResultAlpha swizzle_mask16_alpha_to_n32(
        void* dstRow, const uint8_t* srcRow, int width, SkMasks* masks) {

    // Use the masks to decode to the destination
    uint16_t* srcPtr = (uint16_t*) srcRow;
    SkPMColor* dstPtr = (SkPMColor*) dstRow;
    INIT_RESULT_ALPHA;
    for (int i = 0; i < width; i++) {
        uint16_t p = srcPtr[i];
        uint8_t red = masks->getRed(p);
        uint8_t green = masks->getGreen(p);
        uint8_t blue = masks->getBlue(p);
        uint8_t alpha = masks->getAlpha(p);
        UPDATE_RESULT_ALPHA(alpha);
        dstPtr[i] = SkPackARGB32NoCheck(alpha, red, green, blue);
    }
    return COMPUTE_RESULT_ALPHA;
}

/*
 *
 * Row procedure for masked color components with 24 bits per pixel
 *
 */
static SkSwizzler::ResultAlpha swizzle_mask24_to_n32(
        void* dstRow, const uint8_t* srcRow, int width, SkMasks* masks) {

    // Use the masks to decode to the destination
    SkPMColor* dstPtr = (SkPMColor*) dstRow;
    for (int i = 0; i < 3*width; i += 3) {
        uint32_t p = srcRow[i] | (srcRow[i + 1] << 8) | srcRow[i + 2] << 16;
        uint8_t red = masks->getRed(p);
        uint8_t green = masks->getGreen(p);
        uint8_t blue = masks->getBlue(p);
        dstPtr[i/3] = SkPackARGB32NoCheck(0xFF, red, green, blue);
    }
    return SkSwizzler::kOpaque_ResultAlpha;
}

/*
 *
 * Row procedure for masked color components with 24 bits per pixel with alpha
 *
 */
static SkSwizzler::ResultAlpha swizzle_mask24_alpha_to_n32(
        void* dstRow, const uint8_t* srcRow, int width, SkMasks* masks) {

    // Use the masks to decode to the destination
    SkPMColor* dstPtr = (SkPMColor*) dstRow;
    INIT_RESULT_ALPHA;
    for (int i = 0; i < 3*width; i += 3) {
        uint32_t p = srcRow[i] | (srcRow[i + 1] << 8) | srcRow[i + 2] << 16;
        uint8_t red = masks->getRed(p);
        uint8_t green = masks->getGreen(p);
        uint8_t blue = masks->getBlue(p);
        uint8_t alpha = masks->getAlpha(p);
        UPDATE_RESULT_ALPHA(alpha);
        dstPtr[i/3] = SkPackARGB32NoCheck(alpha, red, green, blue);
    }
    return COMPUTE_RESULT_ALPHA;
}

/*
 *
 * Row procedure for masked color components with 32 bits per pixel
 *
 */
static SkSwizzler::ResultAlpha swizzle_mask32_to_n32(
        void* dstRow, const uint8_t* srcRow, int width, SkMasks* masks) {

    // Use the masks to decode to the destination
    uint32_t* srcPtr = (uint32_t*) srcRow;
    SkPMColor* dstPtr = (SkPMColor*) dstRow;
    for (int i = 0; i < width; i++) {
        uint32_t p = srcPtr[i];
        uint8_t red = masks->getRed(p);
        uint8_t green = masks->getGreen(p);
        uint8_t blue = masks->getBlue(p);
        dstPtr[i] = SkPackARGB32NoCheck(0xFF, red, green, blue);
    }
    return SkSwizzler::kOpaque_ResultAlpha;
}

/*
 *
 * Row procedure for masked color components with 32 bits per pixel
 *
 */
static SkSwizzler::ResultAlpha swizzle_mask32_alpha_to_n32(
        void* dstRow, const uint8_t* srcRow, int width, SkMasks* masks) {

    // Use the masks to decode to the destination
    uint32_t* srcPtr = (uint32_t*) srcRow;
    SkPMColor* dstPtr = (SkPMColor*) dstRow;
    INIT_RESULT_ALPHA;
    for (int i = 0; i < width; i++) {
        uint32_t p = srcPtr[i];
        uint8_t red = masks->getRed(p);
        uint8_t green = masks->getGreen(p);
        uint8_t blue = masks->getBlue(p);
        uint8_t alpha = masks->getAlpha(p);
        UPDATE_RESULT_ALPHA(alpha);
        dstPtr[i] = SkPackARGB32NoCheck(alpha, red, green, blue);
    }
    return COMPUTE_RESULT_ALPHA;
}

/*
 *
 * Create a new mask swizzler
 *
 */
SkMaskSwizzler* SkMaskSwizzler::CreateMaskSwizzler(
        const SkImageInfo& imageInfo, SkMasks* masks, uint32_t bitsPerPixel) {

    // Choose the appropriate row procedure
    RowProc proc = NULL;
    uint32_t alphaMask = masks->getAlphaMask();
    switch (bitsPerPixel) {
        case 16:
            if (0 == alphaMask) {
                proc = &swizzle_mask16_to_n32;
            } else {
                proc = &swizzle_mask16_alpha_to_n32;
            }
            break;
        case 24:
            if (0 == alphaMask) {
                proc = &swizzle_mask24_to_n32;
            } else {
                proc = &swizzle_mask24_alpha_to_n32;
            }
            break;
        case 32:
            if (0 == alphaMask) {
                proc = &swizzle_mask32_to_n32;
            } else {
                proc = &swizzle_mask32_alpha_to_n32;
            }
            break;
        default:
            SkASSERT(false);
            return NULL;
    }
    return SkNEW_ARGS(SkMaskSwizzler, (imageInfo, masks, proc));
}

/*
 *
 * Constructor for mask swizzler
 *
 */
SkMaskSwizzler::SkMaskSwizzler(const SkImageInfo& imageInfo,
                               SkMasks* masks, RowProc proc)
    : fImageInfo(imageInfo)
    , fMasks(masks)
    , fRowProc(proc)
{}

/*
 *
 * Swizzle the next row
 *
 */
SkSwizzler::ResultAlpha SkMaskSwizzler::next(void* dst,
                                             const uint8_t* src) {
    return fRowProc(dst, src, fImageInfo.width(), fMasks);
}
