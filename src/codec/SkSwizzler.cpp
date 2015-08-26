/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCodecPriv.h"
#include "SkColorPriv.h"
#include "SkScaledCodec.h"
#include "SkSwizzler.h"
#include "SkTemplates.h"
#include "SkUtils.h"

SkSwizzler::ResultAlpha SkSwizzler::GetResult(uint8_t zeroAlpha,
                                              uint8_t maxAlpha) {
    // In the transparent case, this returns 0x0000
    // In the opaque case, this returns 0xFFFF
    // If the row is neither transparent nor opaque, returns something else
    return (((uint16_t) maxAlpha) << 8) | zeroAlpha;
}

// samples the row. Does not do anything else but sampling
static SkSwizzler::ResultAlpha sample565(void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src,
        int width, int deltaSrc, int offset, const SkPMColor ctable[]){

    src += offset;
    uint16_t* SK_RESTRICT dst = (uint16_t*) dstRow;
    for (int x = 0; x < width; x++) {
        dst[x] = src[1] << 8 | src[0];
        src += deltaSrc;
    }
    // 565 is always opaque
    return SkSwizzler::kOpaque_ResultAlpha;
}

// kBit
// These routines exclusively choose between white and black

#define GRAYSCALE_BLACK 0
#define GRAYSCALE_WHITE 0xFF


// same as swizzle_bit_to_index and swizzle_bit_to_n32 except for value assigned to dst[x]
static SkSwizzler::ResultAlpha swizzle_bit_to_grayscale(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int dstWidth,
        int deltaSrc, int offset, const SkPMColor* /*ctable*/) {

    uint8_t* SK_RESTRICT dst = (uint8_t*) dstRow;

    // increment src by byte offset and bitIndex by bit offset
    src += offset / 8;
    int bitIndex = offset % 8;
    uint8_t currByte = *src;

    dst[0] = ((currByte >> (7-bitIndex)) & 1) ? GRAYSCALE_WHITE : GRAYSCALE_BLACK;

    for (int x = 1; x < dstWidth; x++) {
        int bitOffset = bitIndex + deltaSrc;
        bitIndex = bitOffset % 8;
        currByte = *(src += bitOffset / 8);
        dst[x] = ((currByte >> (7-bitIndex)) & 1) ? GRAYSCALE_WHITE : GRAYSCALE_BLACK;
    }

    return SkSwizzler::kOpaque_ResultAlpha;
}

#undef GRAYSCALE_BLACK
#undef GRAYSCALE_WHITE

// same as swizzle_bit_to_grayscale and swizzle_bit_to_n32 except for value assigned to dst[x]
static SkSwizzler::ResultAlpha swizzle_bit_to_index(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int dstWidth,
        int deltaSrc, int offset, const SkPMColor* /*ctable*/) {
    uint8_t* SK_RESTRICT dst = (uint8_t*) dstRow;

    // increment src by byte offset and bitIndex by bit offset
    src += offset / 8;
    int bitIndex = offset % 8;
    uint8_t currByte = *src;

    dst[0] = ((currByte >> (7-bitIndex)) & 1);

    for (int x = 1; x < dstWidth; x++) {
        int bitOffset = bitIndex + deltaSrc;
        bitIndex = bitOffset % 8;
        currByte = *(src += bitOffset / 8);
        dst[x] = ((currByte >> (7-bitIndex)) & 1);
    }

    return SkSwizzler::kOpaque_ResultAlpha;
}

// same as swizzle_bit_to_grayscale and swizzle_bit_to_index except for value assigned to dst[x]
static SkSwizzler::ResultAlpha swizzle_bit_to_n32(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int dstWidth,
        int deltaSrc, int offset, const SkPMColor* /*ctable*/) {
    SkPMColor* SK_RESTRICT dst = (SkPMColor*) dstRow;

    // increment src by byte offset and bitIndex by bit offset
    src += offset / 8;
    int bitIndex = offset % 8;
    uint8_t currByte = *src;

    dst[0] = ((currByte >> (7 - bitIndex)) & 1) ? SK_ColorWHITE : SK_ColorBLACK;

    for (int x = 1; x < dstWidth; x++) {
        int bitOffset = bitIndex + deltaSrc;
        bitIndex = bitOffset % 8;
        currByte = *(src += bitOffset / 8);
        dst[x] = ((currByte >> (7 - bitIndex)) & 1) ? SK_ColorWHITE : SK_ColorBLACK;
    }

    return SkSwizzler::kOpaque_ResultAlpha;
}

#define RGB565_BLACK 0
#define RGB565_WHITE 0xFFFF

static SkSwizzler::ResultAlpha swizzle_bit_to_565(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int dstWidth,
        int deltaSrc, int offset, const SkPMColor* /*ctable*/) {
    uint16_t* SK_RESTRICT dst = (uint16_t*) dstRow;

    // increment src by byte offset and bitIndex by bit offset
    src += offset / 8;
    int bitIndex = offset % 8;
    uint8_t currByte = *src;

    dst[0] = ((currByte >> (7 - bitIndex)) & 1) ? RGB565_WHITE : RGB565_BLACK;

    for (int x = 1; x < dstWidth; x++) {
        int bitOffset = bitIndex + deltaSrc;
        bitIndex = bitOffset % 8;
        currByte = *(src += bitOffset / 8);
        dst[x] = ((currByte >> (7 - bitIndex)) & 1) ? RGB565_WHITE : RGB565_BLACK;
    }

    return SkSwizzler::kOpaque_ResultAlpha;
}

#undef RGB565_BLACK
#undef RGB565_WHITE

// kIndex1, kIndex2, kIndex4

static SkSwizzler::ResultAlpha swizzle_small_index_to_index(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int dstWidth,
        int bitsPerPixel, int offset, const SkPMColor ctable[]) {

    src += offset;
    uint8_t* SK_RESTRICT dst = (uint8_t*) dstRow;
    INIT_RESULT_ALPHA;
    const uint32_t pixelsPerByte = 8 / bitsPerPixel;
    const size_t rowBytes = compute_row_bytes_ppb(dstWidth, pixelsPerByte);
    const uint8_t mask = (1 << bitsPerPixel) - 1;
    int x = 0;
    for (uint32_t byte = 0; byte < rowBytes; byte++) {
        uint8_t pixelData = src[byte];
        for (uint32_t p = 0; p < pixelsPerByte && x < dstWidth; p++) {
            uint8_t index = (pixelData >> (8 - bitsPerPixel)) & mask;
            UPDATE_RESULT_ALPHA(ctable[index] >> SK_A32_SHIFT);
            dst[x] = index;
            pixelData <<= bitsPerPixel;
            x++;
        }
    }
    return COMPUTE_RESULT_ALPHA;
}

static SkSwizzler::ResultAlpha swizzle_small_index_to_565(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int dstWidth,
        int bitsPerPixel, int offset, const SkPMColor ctable[]) {

    src += offset;
    uint16_t* SK_RESTRICT dst = (uint16_t*) dstRow;
    const uint32_t pixelsPerByte = 8 / bitsPerPixel;
    const size_t rowBytes = compute_row_bytes_ppb(dstWidth, pixelsPerByte);
    const uint8_t mask = (1 << bitsPerPixel) - 1;
    int x = 0;
    for (uint32_t byte = 0; byte < rowBytes; byte++) {
        uint8_t pixelData = src[byte];
        for (uint32_t p = 0; p < pixelsPerByte && x < dstWidth; p++) {
            uint8_t index = (pixelData >> (8 - bitsPerPixel)) & mask;
            uint16_t c = SkPixel32ToPixel16(ctable[index]);
            dst[x] = c;
            pixelData <<= bitsPerPixel;
            x++;
        }
    }
    return SkSwizzler::kOpaque_ResultAlpha;
}

static SkSwizzler::ResultAlpha swizzle_small_index_to_n32(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int dstWidth,
        int bitsPerPixel, int offset, const SkPMColor ctable[]) {

    src += offset;
    SkPMColor* SK_RESTRICT dst = (SkPMColor*) dstRow;
    INIT_RESULT_ALPHA;
    const uint32_t pixelsPerByte = 8 / bitsPerPixel;
    const size_t rowBytes = compute_row_bytes_ppb(dstWidth, pixelsPerByte);
    const uint8_t mask = (1 << bitsPerPixel) - 1;
    int x = 0;
    for (uint32_t byte = 0; byte < rowBytes; byte++) {
        uint8_t pixelData = src[byte];
        for (uint32_t p = 0; p < pixelsPerByte && x < dstWidth; p++) {
            uint8_t index = (pixelData >> (8 - bitsPerPixel)) & mask;
            SkPMColor c = ctable[index];
            UPDATE_RESULT_ALPHA(c >> SK_A32_SHIFT);
            dst[x] = c;
            pixelData <<= bitsPerPixel;
            x++;
        }
    }
    return COMPUTE_RESULT_ALPHA;
}

// kIndex

static SkSwizzler::ResultAlpha swizzle_index_to_index(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int dstWidth,
        int deltaSrc, int offset, const SkPMColor ctable[]) {

    src += offset;
    uint8_t* SK_RESTRICT dst = (uint8_t*) dstRow;
    INIT_RESULT_ALPHA;
    // TODO (msarett): Should we skip the loop here and guess that the row is opaque/not opaque?
    //                 SkScaledBitmap sampler just guesses that it is opaque.  This is dangerous
    //                 and probably wrong since gif and bmp (rarely) may have alpha.
    if (1 == deltaSrc) {
        // A non-zero offset is only used when sampling, meaning that deltaSrc will be
        // greater than 1. The below loop relies on the fact that src remains unchanged.
        SkASSERT(0 == offset);
        memcpy(dst, src, dstWidth);
        for (int x = 0; x < dstWidth; x++) {
            UPDATE_RESULT_ALPHA(ctable[src[x]] >> SK_A32_SHIFT);
        }
    } else {
        for (int x = 0; x < dstWidth; x++) {
            dst[x] = *src;
            UPDATE_RESULT_ALPHA(ctable[*src] >> SK_A32_SHIFT);
            src += deltaSrc;
        }
    }
    return COMPUTE_RESULT_ALPHA;
}

static SkSwizzler::ResultAlpha swizzle_index_to_n32(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int dstWidth,
        int deltaSrc, int offset, const SkPMColor ctable[]) {

    src += offset;
    SkPMColor* SK_RESTRICT dst = (SkPMColor*)dstRow;
    INIT_RESULT_ALPHA;
    for (int x = 0; x < dstWidth; x++) {
        SkPMColor c = ctable[*src];
        UPDATE_RESULT_ALPHA(c >> SK_A32_SHIFT);
        dst[x] = c;
        src += deltaSrc;
    }
    return COMPUTE_RESULT_ALPHA;
}

static SkSwizzler::ResultAlpha swizzle_index_to_n32_skipZ(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int dstWidth,
        int deltaSrc, int offset, const SkPMColor ctable[]) {

    src += offset;
    SkPMColor* SK_RESTRICT dst = (SkPMColor*)dstRow;
    INIT_RESULT_ALPHA;
    for (int x = 0; x < dstWidth; x++) {
        SkPMColor c = ctable[*src];
        UPDATE_RESULT_ALPHA(c >> SK_A32_SHIFT);
        if (c != 0) {
            dst[x] = c;
        }
        src += deltaSrc;
    }
    return COMPUTE_RESULT_ALPHA;
}

static SkSwizzler::ResultAlpha swizzle_index_to_565(
      void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int dstWidth,
      int bytesPerPixel, int offset, const SkPMColor ctable[]) {
    // FIXME: Support dithering? Requires knowing y, which I think is a bigger
    // change.
    src += offset;
    uint16_t* SK_RESTRICT dst = (uint16_t*)dstRow;
    for (int x = 0; x < dstWidth; x++) {
        dst[x] = SkPixel32ToPixel16(ctable[*src]);
        src += bytesPerPixel;
    }
    return SkSwizzler::kOpaque_ResultAlpha;
}


#undef A32_MASK_IN_PLACE

// kGray

static SkSwizzler::ResultAlpha swizzle_gray_to_n32(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int dstWidth,
        int deltaSrc, int offset, const SkPMColor ctable[]) {

    src += offset;
    SkPMColor* SK_RESTRICT dst = (SkPMColor*)dstRow;
    for (int x = 0; x < dstWidth; x++) {
        dst[x] = SkPackARGB32NoCheck(0xFF, *src, *src, *src);
        src += deltaSrc;
    }
    return SkSwizzler::kOpaque_ResultAlpha;
}

static SkSwizzler::ResultAlpha swizzle_gray_to_gray(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int dstWidth,
        int deltaSrc, int offset, const SkPMColor ctable[]) {

    src += offset;
    uint8_t* SK_RESTRICT dst = (uint8_t*) dstRow;
    if (1 == deltaSrc) {
        memcpy(dstRow, src, dstWidth);
    } else {
        for (int x = 0; x < dstWidth; x++) {
            dst[x] = src[0];
            src += deltaSrc;
        }
    }
    return SkSwizzler::kOpaque_ResultAlpha;
}

static SkSwizzler::ResultAlpha swizzle_gray_to_565(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int dstWidth,
        int bytesPerPixel, int offset, const SkPMColor ctable[]) {
    // FIXME: Support dithering?
    src += offset;
    uint16_t* SK_RESTRICT dst = (uint16_t*)dstRow;
    for (int x = 0; x < dstWidth; x++) {
        dst[x] = SkPack888ToRGB16(src[0], src[0], src[0]);
        src += bytesPerPixel;
    }
    return SkSwizzler::kOpaque_ResultAlpha;
}

// kBGRX

static SkSwizzler::ResultAlpha swizzle_bgrx_to_n32(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int dstWidth,
        int deltaSrc, int offset, const SkPMColor ctable[]) {

    src += offset;
    SkPMColor* SK_RESTRICT dst = (SkPMColor*)dstRow;
    for (int x = 0; x < dstWidth; x++) {
        dst[x] = SkPackARGB32NoCheck(0xFF, src[2], src[1], src[0]);
        src += deltaSrc;
    }
    return SkSwizzler::kOpaque_ResultAlpha;
}

static SkSwizzler::ResultAlpha swizzle_bgrx_to_565(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int dstWidth,
        int deltaSrc, int offset, const SkPMColor ctable[]) {
    // FIXME: Support dithering?
    src += offset;
    uint16_t* SK_RESTRICT dst = (uint16_t*)dstRow;
    for (int x = 0; x < dstWidth; x++) {
        dst[x] = SkPack888ToRGB16(src[2], src[1], src[0]);
        src += deltaSrc;
    }
    return SkSwizzler::kOpaque_ResultAlpha;
}

// kBGRA

static SkSwizzler::ResultAlpha swizzle_bgra_to_n32_unpremul(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int dstWidth,
        int deltaSrc, int offset, const SkPMColor ctable[]) {

    src += offset;
    SkPMColor* SK_RESTRICT dst = (SkPMColor*)dstRow;
    INIT_RESULT_ALPHA;
    for (int x = 0; x < dstWidth; x++) {
        uint8_t alpha = src[3];
        UPDATE_RESULT_ALPHA(alpha);
        dst[x] = SkPackARGB32NoCheck(alpha, src[2], src[1], src[0]);
        src += deltaSrc;
    }
    return COMPUTE_RESULT_ALPHA;
}

static SkSwizzler::ResultAlpha swizzle_bgra_to_n32_premul(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int dstWidth,
        int deltaSrc, int offset, const SkPMColor ctable[]) {

    src += offset;
    SkPMColor* SK_RESTRICT dst = (SkPMColor*)dstRow;
    INIT_RESULT_ALPHA;
    for (int x = 0; x < dstWidth; x++) {
        uint8_t alpha = src[3];
        UPDATE_RESULT_ALPHA(alpha);
        dst[x] = SkPreMultiplyARGB(alpha, src[2], src[1], src[0]);
        src += deltaSrc;
    }
    return COMPUTE_RESULT_ALPHA;
}

// kRGBX
static SkSwizzler::ResultAlpha swizzle_rgbx_to_n32(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int dstWidth,
        int deltaSrc, int offset, const SkPMColor ctable[]) {

    src += offset;
    SkPMColor* SK_RESTRICT dst = (SkPMColor*)dstRow;
    for (int x = 0; x < dstWidth; x++) {
        dst[x] = SkPackARGB32(0xFF, src[0], src[1], src[2]);
        src += deltaSrc;
    }
    return SkSwizzler::kOpaque_ResultAlpha;
}

static SkSwizzler::ResultAlpha swizzle_rgbx_to_565(
       void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int dstWidth,
       int bytesPerPixel, int offset, const SkPMColor ctable[]) {
    // FIXME: Support dithering?
    src += offset;
    uint16_t* SK_RESTRICT dst = (uint16_t*)dstRow;
    for (int x = 0; x < dstWidth; x++) {
        dst[x] = SkPack888ToRGB16(src[0], src[1], src[2]);
        src += bytesPerPixel;
    }
    return SkSwizzler::kOpaque_ResultAlpha;
}


// kRGBA
static SkSwizzler::ResultAlpha swizzle_rgba_to_n32_premul(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int dstWidth,
        int deltaSrc, int offset, const SkPMColor ctable[]) {

    src += offset;
    SkPMColor* SK_RESTRICT dst = (SkPMColor*)dstRow;
    INIT_RESULT_ALPHA;
    for (int x = 0; x < dstWidth; x++) {
        unsigned alpha = src[3];
        UPDATE_RESULT_ALPHA(alpha);
        dst[x] = SkPreMultiplyARGB(alpha, src[0], src[1], src[2]);
        src += deltaSrc;
    }
    return COMPUTE_RESULT_ALPHA;
}

static SkSwizzler::ResultAlpha swizzle_rgba_to_n32_unpremul(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int dstWidth,
        int deltaSrc, int offset, const SkPMColor ctable[]) {

    src += offset;
    uint32_t* SK_RESTRICT dst = reinterpret_cast<uint32_t*>(dstRow);
    INIT_RESULT_ALPHA;
    for (int x = 0; x < dstWidth; x++) {
        unsigned alpha = src[3];
        UPDATE_RESULT_ALPHA(alpha);
        dst[x] = SkPackARGB32NoCheck(alpha, src[0], src[1], src[2]);
        src += deltaSrc;
    }
    return COMPUTE_RESULT_ALPHA;
}

static SkSwizzler::ResultAlpha swizzle_rgba_to_n32_premul_skipZ(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int dstWidth,
        int deltaSrc, int offset, const SkPMColor ctable[]) {

    src += offset;
    SkPMColor* SK_RESTRICT dst = (SkPMColor*)dstRow;
    INIT_RESULT_ALPHA;
    for (int x = 0; x < dstWidth; x++) {
        unsigned alpha = src[3];
        UPDATE_RESULT_ALPHA(alpha);
        if (0 != alpha) {
            dst[x] = SkPreMultiplyARGB(alpha, src[0], src[1], src[2]);
        }
        src += deltaSrc;
    }
    return COMPUTE_RESULT_ALPHA;
}

/**
    FIXME: This was my idea to cheat in order to continue taking advantage of skipping zeroes.
    This would be fine for drawing normally, but not for drawing with transfer modes. Being
    honest means we can draw correctly with transfer modes, with the cost of not being able
    to take advantage of Android's free unwritten pages. Something to keep in mind when we
    decide whether to switch to unpremul default.
static bool swizzle_rgba_to_n32_unpremul_skipZ(void* SK_RESTRICT dstRow,
                                               const uint8_t* SK_RESTRICT src,
                                               int dstWidth, int bitsPerPixel, int offset,
                                               const SkPMColor[]) {
    src += offset;
    SkPMColor* SK_RESTRICT dst = (SkPMColor*)dstRow;
    unsigned alphaMask = 0xFF;
    for (int x = 0; x < dstWidth; x++) {
        unsigned alpha = src[3];
        // NOTE: We cheat here. The caller requested unpremul and skip zeroes. It's possible
        // the color components are not zero, but we skip them anyway, meaning they'll remain
        // zero (implied by the request to skip zeroes).
        if (0 != alpha) {
            dst[x] = SkPackARGB32NoCheck(alpha, src[0], src[1], src[2]);
        }
        src += deltaSrc;
        alphaMask &= alpha;
    }
    return alphaMask != 0xFF;
}
*/

SkSwizzler* SkSwizzler::CreateSwizzler(SkSwizzler::SrcConfig sc,
                                       const SkPMColor* ctable,
                                       const SkImageInfo& dstInfo, 
                                       SkCodec::ZeroInitialized zeroInit, 
                                       const SkImageInfo& srcInfo) {
    if (dstInfo.colorType() == kUnknown_SkColorType || kUnknown == sc) {
        return NULL;
    }
    if ((kIndex == sc || kIndex4 == sc || kIndex2 == sc || kIndex1 == sc)
            && NULL == ctable) {
        return NULL;
    }
    RowProc proc = NULL;

    switch (sc) {
        case kBit:
            switch (dstInfo.colorType()) {
                case kN32_SkColorType:
                    proc = &swizzle_bit_to_n32;
                    break;
                case kIndex_8_SkColorType:
                    proc = &swizzle_bit_to_index;
                    break;
                case kRGB_565_SkColorType:
                    proc = &swizzle_bit_to_565;
                    break;
                case kGray_8_SkColorType:
                    proc = &swizzle_bit_to_grayscale;
                    break;
                default:
                    break;
            }
            break;
        case kIndex1:
        case kIndex2:
        case kIndex4:
            switch (dstInfo.colorType()) {
                case kN32_SkColorType:
                    proc = &swizzle_small_index_to_n32;
                    break;
                case kRGB_565_SkColorType:
                    proc = &swizzle_small_index_to_565;
                    break;
                case kIndex_8_SkColorType:
                    proc = &swizzle_small_index_to_index;
                    break;
                default:
                    break;
            }
            break;
        case kIndex:
            switch (dstInfo.colorType()) {
                case kN32_SkColorType:
                    // We assume the color premultiplied ctable (or not) as desired.
                    if (SkCodec::kYes_ZeroInitialized == zeroInit) {
                        proc = &swizzle_index_to_n32_skipZ;
                        break;
                    } else {
                        proc = &swizzle_index_to_n32;
                        break;
                    }
                    break;
                case kRGB_565_SkColorType:
                    proc = &swizzle_index_to_565;
                    break;
                case kIndex_8_SkColorType:
                    proc = &swizzle_index_to_index;
                    break;
                default:
                    break;
            }
            break;
        case kGray:
            switch (dstInfo.colorType()) {
                case kN32_SkColorType:
                    proc = &swizzle_gray_to_n32;
                    break;
                case kGray_8_SkColorType:
                    proc = &swizzle_gray_to_gray;
                    break;
                case kRGB_565_SkColorType:
                    proc = &swizzle_gray_to_565;
                    break;
                default:
                    break;
            }
            break;
        case kBGR:
        case kBGRX:
            switch (dstInfo.colorType()) {
                case kN32_SkColorType:
                    proc = &swizzle_bgrx_to_n32;
                    break;
                case kRGB_565_SkColorType:
                    proc = &swizzle_bgrx_to_565;
                    break;
                default:
                    break;
            }
            break;
        case kBGRA:
            switch (dstInfo.colorType()) {
                case kN32_SkColorType:
                    switch (dstInfo.alphaType()) {
                        case kUnpremul_SkAlphaType:
                            proc = &swizzle_bgra_to_n32_unpremul;
                            break;
                        case kPremul_SkAlphaType:
                            proc = &swizzle_bgra_to_n32_premul;
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
            break;
        case kRGBX:
            // TODO: Support other swizzles.
            switch (dstInfo.colorType()) {
                case kN32_SkColorType:
                    proc = &swizzle_rgbx_to_n32;
                    break;
                case kRGB_565_SkColorType:
                    proc = &swizzle_rgbx_to_565;
                default:
                    break;
            }
            break;
        case kRGBA:
            switch (dstInfo.colorType()) {
                case kN32_SkColorType:
                    if (dstInfo.alphaType() == kUnpremul_SkAlphaType) {
                        // Respect zeroInit?
                        proc = &swizzle_rgba_to_n32_unpremul;
                    } else {
                        if (SkCodec::kYes_ZeroInitialized == zeroInit) {
                            proc = &swizzle_rgba_to_n32_premul_skipZ;
                        } else {
                            proc = &swizzle_rgba_to_n32_premul;
                        }
                    }
                    break;
                default:
                    break;
            }
            break;
        case kRGB:
            switch (dstInfo.colorType()) {
                case kN32_SkColorType:
                    proc = &swizzle_rgbx_to_n32;
                    break;
                default:
                    break;
            }
            break;
        case kRGB_565:
            switch (dstInfo.colorType()) {
                case kRGB_565_SkColorType:
                    proc = &sample565;
                    break;
                default:
                    break;
            }
        default:
            break;
    }
    if (NULL == proc) {
        return NULL;
    }

    // Store deltaSrc in bytes if it is an even multiple, otherwise use bits
    int deltaSrc = SkIsAlign8(BitsPerPixel(sc)) ? BytesPerPixel(sc) : BitsPerPixel(sc);

    // get sampleX based on srcInfo and dstInfo dimensions
    int sampleX;
    SkScaledCodec::ComputeSampleSize(dstInfo, srcInfo, &sampleX, NULL);

    return new SkSwizzler(proc, ctable, deltaSrc, dstInfo, sampleX);
}

SkSwizzler::SkSwizzler(RowProc proc, const SkPMColor* ctable,
                       int deltaSrc, const SkImageInfo& info, int sampleX)
    : fRowProc(proc)
    , fColorTable(ctable)
    , fDeltaSrc(deltaSrc)
    , fDstInfo(info)
    , fSampleX(sampleX)
    , fX0(sampleX == 1 ? 0 : sampleX >> 1)
{
    // check that fX0 is less than original width
    SkASSERT(fX0 >= 0 && fX0 < fDstInfo.width() * fSampleX);
}

SkSwizzler::ResultAlpha SkSwizzler::swizzle(void* dst, const uint8_t* SK_RESTRICT src) {
    SkASSERT(NULL != dst && NULL != src);
    return fRowProc(dst, src, fDstInfo.width(), fSampleX * fDeltaSrc, fX0 * fDeltaSrc, fColorTable);
}

void SkSwizzler::Fill(void* dstStartRow, const SkImageInfo& dstInfo, size_t dstRowBytes,
        uint32_t numRows, uint32_t colorOrIndex, const SkPMColor* colorTable) {
    SkASSERT(dstStartRow != NULL);
    SkASSERT(numRows <= (uint32_t) dstInfo.height());

    // Calculate bytes to fill.  We use getSafeSize since the last row may not be padded.
    const size_t bytesToFill = dstInfo.makeWH(dstInfo.width(), numRows).getSafeSize(dstRowBytes);

    // Use the proper memset routine to fill the remaining bytes
    switch(dstInfo.colorType()) {
        case kN32_SkColorType:
            // Assume input is an index if we have a color table
            uint32_t color;
            if (NULL != colorTable) {
                SkASSERT(colorOrIndex == (uint8_t) colorOrIndex);
                color = colorTable[colorOrIndex];
            // Otherwise, assume the input is a color
            } else {
                color = colorOrIndex;
            }

            // We must fill row by row in the case of unaligned row bytes
            if (SkIsAlign4((size_t) dstStartRow) && SkIsAlign4(dstRowBytes)) {
                sk_memset32((uint32_t*) dstStartRow, color,
                        (uint32_t) bytesToFill / sizeof(SkPMColor));
            } else {
                // This is an unlikely, slow case
                SkCodecPrintf("Warning: Strange number of row bytes, fill will be slow.\n");
                uint32_t* dstRow = (uint32_t*) dstStartRow;
                for (uint32_t row = 0; row < numRows; row++) {
                    for (int32_t col = 0; col < dstInfo.width(); col++) {
                        dstRow[col] = color;
                    }
                    dstRow = SkTAddOffset<uint32_t>(dstRow, dstRowBytes);
                }
            }
            break;
        // On an index destination color type, always assume the input is an index
        case kIndex_8_SkColorType:
            SkASSERT(colorOrIndex == (uint8_t) colorOrIndex);
            memset(dstStartRow, colorOrIndex, bytesToFill);
            break;
        case kGray_8_SkColorType:
            // If the destination is kGray, the caller passes in an 8-bit color.
            // We will not assert that the high bits of colorOrIndex must be zeroed.
            // This allows us to take advantage of the fact that the low 8 bits of an
            // SKPMColor may be a valid a grayscale color.  For example, the low 8
            // bits of SK_ColorBLACK are identical to the grayscale representation
            // for black. 
            memset(dstStartRow, (uint8_t) colorOrIndex, bytesToFill);
            break;
        case kRGB_565_SkColorType:
            // If the destination is k565, the caller passes in a 16-bit color.
            // We will not assert that the high bits of colorOrIndex must be zeroed.
            // This allows us to take advantage of the fact that the low 16 bits of an
            // SKPMColor may be a valid a 565 color.  For example, the low 16
            // bits of SK_ColorBLACK are identical to the 565 representation
            // for black.
            memset(dstStartRow, (uint16_t) colorOrIndex, bytesToFill);
            break;
        default:
            SkCodecPrintf("Error: Unsupported dst color type for fill().  Doing nothing.\n");
            SkASSERT(false);
            break;
    }
}
