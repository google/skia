/*
 * Copyright 2015 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCodecPriv_DEFINED
#define SkCodecPriv_DEFINED

#include "SkColorTable.h"
#include "SkImageInfo.h"
#include "SkSwizzler.h"
#include "SkTypes.h"
#include "SkUtils.h"

/*
 *
 * Helper routine for alpha result codes
 *
 */
#define INIT_RESULT_ALPHA                       \
    uint8_t zeroAlpha = 0;                      \
    uint8_t maxAlpha = 0xFF;

#define UPDATE_RESULT_ALPHA(alpha)              \
    zeroAlpha |= (alpha);                       \
    maxAlpha  &= (alpha);

#define COMPUTE_RESULT_ALPHA                    \
    SkSwizzler::GetResult(zeroAlpha, maxAlpha);

static inline bool valid_alpha(SkAlphaType dstAlpha, SkAlphaType srcAlpha) {
    // Check for supported alpha types
    if (srcAlpha != dstAlpha) {
        if (kOpaque_SkAlphaType == srcAlpha) {
            // If the source is opaque, we must decode to opaque
            return false;
        }

        // The source is not opaque
        switch (dstAlpha) {
            case kPremul_SkAlphaType:
            case kUnpremul_SkAlphaType:
                // The source is not opaque, so either of these is okay
                break;
            default:
                // We cannot decode a non-opaque image to opaque (or unknown)
                return false;
        }
    }
    return true;
}

/*
 * If there is a color table, get a pointer to the colors, otherwise return NULL
 */
static const SkPMColor* get_color_ptr(SkColorTable* colorTable) {
     return NULL != colorTable ? colorTable->readColors() : NULL;
}

/*
 *
 * Copy the codec color table back to the client when kIndex8 color type is requested
 */
static inline void copy_color_table(const SkImageInfo& dstInfo, SkColorTable* colorTable,
        SkPMColor* inputColorPtr, int* inputColorCount) {
    if (kIndex_8_SkColorType == dstInfo.colorType()) {
        SkASSERT(NULL != inputColorPtr);
        SkASSERT(NULL != inputColorCount);
        SkASSERT(NULL != colorTable);
        memcpy(inputColorPtr, colorTable->readColors(), *inputColorCount * 4);
    }
}

/*
 * Compute row bytes for an image using pixels per byte
 */
static inline size_t compute_row_bytes_ppb(int width, uint32_t pixelsPerByte) {
    return (width + pixelsPerByte - 1) / pixelsPerByte;
}

/*
 * Compute row bytes for an image using bytes per pixel
 */
static inline size_t compute_row_bytes_bpp(int width, uint32_t bytesPerPixel) {
    return width * bytesPerPixel;
}

/*
 * Compute row bytes for an image
 */
static inline size_t compute_row_bytes(int width, uint32_t bitsPerPixel) {
    if (bitsPerPixel < 16) {
        SkASSERT(0 == 8 % bitsPerPixel);
        const uint32_t pixelsPerByte = 8 / bitsPerPixel;
        return compute_row_bytes_ppb(width, pixelsPerByte);
    } else {
        SkASSERT(0 == bitsPerPixel % 8);
        const uint32_t bytesPerPixel = bitsPerPixel / 8;
        return compute_row_bytes_bpp(width, bytesPerPixel);
    }
}

/*
 * Get a byte from a buffer
 * This method is unsafe, the caller is responsible for performing a check
 */
static inline uint8_t get_byte(uint8_t* buffer, uint32_t i) {
    return buffer[i];
}

/*
 * Get a short from a buffer
 * This method is unsafe, the caller is responsible for performing a check
 */
static inline uint16_t get_short(uint8_t* buffer, uint32_t i) {
    uint16_t result;
    memcpy(&result, &(buffer[i]), 2);
#ifdef SK_CPU_BENDIAN
    return SkEndianSwap16(result);
#else
    return result;
#endif
}

/*
 * Get an int from a buffer
 * This method is unsafe, the caller is responsible for performing a check
 */
static inline uint32_t get_int(uint8_t* buffer, uint32_t i) {
    uint32_t result;
    memcpy(&result, &(buffer[i]), 4);
#ifdef SK_CPU_BENDIAN
    return SkEndianSwap32(result);
#else
    return result;
#endif
}

#ifdef SK_PRINT_CODEC_MESSAGES
    #define SkCodecPrintf SkDebugf
#else
    #define SkCodecPrintf(...)
#endif

#endif // SkCodecPriv_DEFINED
