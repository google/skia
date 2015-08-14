/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCodecPriv.h"
#include "SkColorPriv.h"
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

// kBit
// These routines exclusively choose between white and black

#define GRAYSCALE_BLACK 0
#define GRAYSCALE_WHITE 0xFF

static SkSwizzler::ResultAlpha swizzle_bit_to_grayscale(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int width,
        int /*bitsPerPixel*/, const SkPMColor* /*ctable*/) {
    uint8_t* SK_RESTRICT dst = (uint8_t*) dstRow;

    // Determine how many full bytes are in the row
    int bytesInRow = width >> 3;
    int i;
    for (i = 0; i < bytesInRow; i++) {
        U8CPU currByte = src[i];
        for (int j = 0; j < 8; j++) {
            dst[j] = ((currByte >> (7 - j)) & 1) ? GRAYSCALE_WHITE : GRAYSCALE_BLACK;
        }
        dst += 8;
    }

    // Finish the remaining bits
    width &= 7;
    if (width > 0) {
        U8CPU currByte = src[i];
        for (int j = 0; j < width; j++) {
            dst[j] = ((currByte >> 7) & 1) ? GRAYSCALE_WHITE : GRAYSCALE_BLACK;
            currByte <<= 1;
        }
    }
    return SkSwizzler::kOpaque_ResultAlpha;
}

#undef GRAYSCALE_BLACK
#undef GRAYSCALE_WHITE

static SkSwizzler::ResultAlpha swizzle_bit_to_index(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int width,
        int /*bitsPerPixel*/, const SkPMColor* /*ctable*/) {
    uint8_t* SK_RESTRICT dst = (uint8_t*) dstRow;

    // Determine how many full bytes are in the row
    int bytesInRow = width >> 3;
    int i;
    for (i = 0; i < bytesInRow; i++) {
        U8CPU currByte = src[i];
        for (int j = 0; j < 8; j++) {
            dst[j] = (currByte >> (7 - j)) & 1;
        }
        dst += 8;
    }

    // Finish the remaining bits
    width &= 7;
    if (width > 0) {
        U8CPU currByte = src[i];
        for (int j = 0; j < width; j++) {
            dst[j] = ((currByte >> 7) & 1);
            currByte <<= 1;
        }
    }
    return SkSwizzler::kOpaque_ResultAlpha;
}

static SkSwizzler::ResultAlpha swizzle_bit_to_n32(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int width,
        int /*bitsPerPixel*/, const SkPMColor* /*ctable*/) {
    SkPMColor* SK_RESTRICT dst = (SkPMColor*) dstRow;

    // Determine how many full bytes are in the row
    int bytesInRow = width >> 3;
    int i;
    for (i = 0; i < bytesInRow; i++) {
        U8CPU currByte = src[i];
        for (int j = 0; j < 8; j++) {
            dst[j] = ((currByte >> (7 - j)) & 1) ? SK_ColorWHITE : SK_ColorBLACK;
        }
        dst += 8;
    }

    // Finish the remaining bits
    width &= 7;
    if (width > 0) {
        U8CPU currByte = src[i];
        for (int j = 0; j < width; j++) {
            dst[j] = ((currByte >> 7) & 1) ? SK_ColorWHITE : SK_ColorBLACK;
            currByte <<= 1;
        }
    }
    return SkSwizzler::kOpaque_ResultAlpha;
}

// kIndex1, kIndex2, kIndex4

static SkSwizzler::ResultAlpha swizzle_small_index_to_index(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int width,
        int bitsPerPixel, const SkPMColor ctable[]) {

    uint8_t* SK_RESTRICT dst = (uint8_t*) dstRow;
    INIT_RESULT_ALPHA;
    const uint32_t pixelsPerByte = 8 / bitsPerPixel;
    const size_t rowBytes = compute_row_bytes_ppb(width, pixelsPerByte);
    const uint8_t mask = (1 << bitsPerPixel) - 1;
    int x = 0;
    for (uint32_t byte = 0; byte < rowBytes; byte++) {
        uint8_t pixelData = src[byte];
        for (uint32_t p = 0; p < pixelsPerByte && x < width; p++) {
            uint8_t index = (pixelData >> (8 - bitsPerPixel)) & mask;
            UPDATE_RESULT_ALPHA(ctable[index] >> SK_A32_SHIFT);
            dst[x] = index;
            pixelData <<= bitsPerPixel;
            x++;
        }
    }
    return COMPUTE_RESULT_ALPHA;
}

static SkSwizzler::ResultAlpha swizzle_small_index_to_n32(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int width,
        int bitsPerPixel, const SkPMColor ctable[]) {

    SkPMColor* SK_RESTRICT dst = (SkPMColor*) dstRow;
    INIT_RESULT_ALPHA;
    const uint32_t pixelsPerByte = 8 / bitsPerPixel;
    const size_t rowBytes = compute_row_bytes_ppb(width, pixelsPerByte);
    const uint8_t mask = (1 << bitsPerPixel) - 1;
    int x = 0;
    for (uint32_t byte = 0; byte < rowBytes; byte++) {
        uint8_t pixelData = src[byte];
        for (uint32_t p = 0; p < pixelsPerByte && x < width; p++) {
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
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int width,
        int bytesPerPixel, const SkPMColor ctable[]) {

    uint8_t* SK_RESTRICT dst = (uint8_t*) dstRow;
    memcpy(dst, src, width);
    // TODO (msarett): Should we skip the loop here and guess that the row is opaque/not opaque?
    //                 SkScaledBitmap sampler just guesses that it is opaque.  This is dangerous
    //                 and probably wrong since gif and bmp (rarely) may have alpha.
    INIT_RESULT_ALPHA;
    for (int x = 0; x < width; x++) {
        UPDATE_RESULT_ALPHA(ctable[src[x]] >> SK_A32_SHIFT);
    }
    return COMPUTE_RESULT_ALPHA;
}

static SkSwizzler::ResultAlpha swizzle_index_to_n32(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int width,
        int bytesPerPixel, const SkPMColor ctable[]) {

    SkPMColor* SK_RESTRICT dst = (SkPMColor*)dstRow;
    INIT_RESULT_ALPHA;
    for (int x = 0; x < width; x++) {
        SkPMColor c = ctable[src[x]];
        UPDATE_RESULT_ALPHA(c >> SK_A32_SHIFT);
        dst[x] = c;
    }
    return COMPUTE_RESULT_ALPHA;
}

static SkSwizzler::ResultAlpha swizzle_index_to_n32_skipZ(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int width,
        int bytesPerPixel, const SkPMColor ctable[]) {

    SkPMColor* SK_RESTRICT dst = (SkPMColor*)dstRow;
    INIT_RESULT_ALPHA;
    for (int x = 0; x < width; x++) {
        SkPMColor c = ctable[src[x]];
        UPDATE_RESULT_ALPHA(c >> SK_A32_SHIFT);
        if (c != 0) {
            dst[x] = c;
        }
    }
    return COMPUTE_RESULT_ALPHA;
}

static SkSwizzler::ResultAlpha swizzle_index_to_565(
      void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int width,
      int bytesPerPixel, const SkPMColor ctable[]) {
    // FIXME: Support dithering? Requires knowing y, which I think is a bigger
    // change.
    uint16_t* SK_RESTRICT dst = (uint16_t*)dstRow;
    for (int x = 0; x < width; x++) {
        dst[x] = SkPixel32ToPixel16(ctable[*src]);
        src += bytesPerPixel;
    }
    return SkSwizzler::kOpaque_ResultAlpha;
}


#undef A32_MASK_IN_PLACE

// kGray

static SkSwizzler::ResultAlpha swizzle_gray_to_n32(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int width,
        int bytesPerPixel, const SkPMColor ctable[]) {

    SkPMColor* SK_RESTRICT dst = (SkPMColor*)dstRow;
    for (int x = 0; x < width; x++) {
        dst[x] = SkPackARGB32NoCheck(0xFF, src[x], src[x], src[x]);
    }
    return SkSwizzler::kOpaque_ResultAlpha;
}

static SkSwizzler::ResultAlpha swizzle_gray_to_gray(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int width,
        int bytesPerPixel, const SkPMColor ctable[]) {
    memcpy(dstRow, src, width);
    return SkSwizzler::kOpaque_ResultAlpha;
}

static SkSwizzler::ResultAlpha swizzle_gray_to_565(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int width,
        int bytesPerPixel, const SkPMColor ctable[]) {
    // FIXME: Support dithering?
    uint16_t* SK_RESTRICT dst = (uint16_t*)dstRow;
    for (int x = 0; x < width; x++) {
        dst[x] = SkPack888ToRGB16(src[0], src[0], src[0]);
        src += bytesPerPixel;
    }
    return SkSwizzler::kOpaque_ResultAlpha;
}

// kBGRX

static SkSwizzler::ResultAlpha swizzle_bgrx_to_n32(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int width,
        int bytesPerPixel, const SkPMColor ctable[]) {

    SkPMColor* SK_RESTRICT dst = (SkPMColor*)dstRow;
    for (int x = 0; x < width; x++) {
        dst[x] = SkPackARGB32NoCheck(0xFF, src[2], src[1], src[0]);
        src += bytesPerPixel;
    }
    return SkSwizzler::kOpaque_ResultAlpha;
}

// kBGRA

static SkSwizzler::ResultAlpha swizzle_bgra_to_n32_unpremul(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int width,
        int bytesPerPixel, const SkPMColor ctable[]) {

    SkPMColor* SK_RESTRICT dst = (SkPMColor*)dstRow;
    INIT_RESULT_ALPHA;
    for (int x = 0; x < width; x++) {
        uint8_t alpha = src[3];
        UPDATE_RESULT_ALPHA(alpha);
        dst[x] = SkPackARGB32NoCheck(alpha, src[2], src[1], src[0]);
        src += bytesPerPixel;
    }
    return COMPUTE_RESULT_ALPHA;
}

static SkSwizzler::ResultAlpha swizzle_bgra_to_n32_premul(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int width,
        int bytesPerPixel, const SkPMColor ctable[]) {

    SkPMColor* SK_RESTRICT dst = (SkPMColor*)dstRow;
    INIT_RESULT_ALPHA;
    for (int x = 0; x < width; x++) {
        uint8_t alpha = src[3];
        UPDATE_RESULT_ALPHA(alpha);
        dst[x] = SkPreMultiplyARGB(alpha, src[2], src[1], src[0]);
        src += bytesPerPixel;
    }
    return COMPUTE_RESULT_ALPHA;
}

// kRGBX
static SkSwizzler::ResultAlpha swizzle_rgbx_to_n32(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int width,
        int bytesPerPixel, const SkPMColor ctable[]) {

    SkPMColor* SK_RESTRICT dst = (SkPMColor*)dstRow;
    for (int x = 0; x < width; x++) {
        dst[x] = SkPackARGB32(0xFF, src[0], src[1], src[2]);
        src += bytesPerPixel;
    }
    return SkSwizzler::kOpaque_ResultAlpha;
}

static SkSwizzler::ResultAlpha swizzle_rgbx_to_565(
       void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int width,
       int bytesPerPixel, const SkPMColor ctable[]) {
    // FIXME: Support dithering?
    uint16_t* SK_RESTRICT dst = (uint16_t*)dstRow;
    for (int x = 0; x < width; x++) {
        dst[x] = SkPack888ToRGB16(src[0], src[1], src[2]);
        src += bytesPerPixel;
    }
    return SkSwizzler::kOpaque_ResultAlpha;
}


// kRGBA
static SkSwizzler::ResultAlpha swizzle_rgba_to_n32_premul(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int width,
        int bytesPerPixel, const SkPMColor ctable[]) {

    SkPMColor* SK_RESTRICT dst = (SkPMColor*)dstRow;
    INIT_RESULT_ALPHA;
    for (int x = 0; x < width; x++) {
        unsigned alpha = src[3];
        UPDATE_RESULT_ALPHA(alpha);
        dst[x] = SkPreMultiplyARGB(alpha, src[0], src[1], src[2]);
        src += bytesPerPixel;
    }
    return COMPUTE_RESULT_ALPHA;
}

static SkSwizzler::ResultAlpha swizzle_rgba_to_n32_unpremul(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int width,
        int bytesPerPixel, const SkPMColor ctable[]) {

    uint32_t* SK_RESTRICT dst = reinterpret_cast<uint32_t*>(dstRow);
    INIT_RESULT_ALPHA;
    for (int x = 0; x < width; x++) {
        unsigned alpha = src[3];
        UPDATE_RESULT_ALPHA(alpha);
        dst[x] = SkPackARGB32NoCheck(alpha, src[0], src[1], src[2]);
        src += bytesPerPixel;
    }
    return COMPUTE_RESULT_ALPHA;
}

static SkSwizzler::ResultAlpha swizzle_rgba_to_n32_premul_skipZ(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int width,
        int bytesPerPixel, const SkPMColor ctable[]) {

    SkPMColor* SK_RESTRICT dst = (SkPMColor*)dstRow;
    INIT_RESULT_ALPHA;
    for (int x = 0; x < width; x++) {
        unsigned alpha = src[3];
        UPDATE_RESULT_ALPHA(alpha);
        if (0 != alpha) {
            dst[x] = SkPreMultiplyARGB(alpha, src[0], src[1], src[2]);
        }
        src += bytesPerPixel;
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
                                               int width, int bitsPerPixel,
                                               const SkPMColor[]) {
    SkPMColor* SK_RESTRICT dst = (SkPMColor*)dstRow;
    unsigned alphaMask = 0xFF;
    for (int x = 0; x < width; x++) {
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
                                       const SkImageInfo& info,
                                       SkCodec::ZeroInitialized zeroInit) {
    if (info.colorType() == kUnknown_SkColorType || kUnknown == sc) {
        return NULL;
    }
    if ((kIndex == sc || kIndex4 == sc || kIndex2 == sc || kIndex1 == sc)
            && NULL == ctable) {
        return NULL;
    }
    RowProc proc = NULL;
    switch (sc) {
        case kBit:
            switch (info.colorType()) {
                case kN32_SkColorType:
                    proc = &swizzle_bit_to_n32;
                    break;
                case kIndex_8_SkColorType:
                    proc = &swizzle_bit_to_index;
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
            switch (info.colorType()) {
                case kN32_SkColorType:
                    proc = &swizzle_small_index_to_n32;
                    break;
                case kIndex_8_SkColorType:
                    proc = &swizzle_small_index_to_index;
                    break;
                default:
                    break;
            }
            break;
        case kIndex:
            switch (info.colorType()) {
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
            switch (info.colorType()) {
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
            switch (info.colorType()) {
                case kN32_SkColorType:
                    proc = &swizzle_bgrx_to_n32;
                    break;
                default:
                    break;
            }
            break;
        case kBGRA:
            switch (info.colorType()) {
                case kN32_SkColorType:
                    switch (info.alphaType()) {
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
            switch (info.colorType()) {
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
            switch (info.colorType()) {
                case kN32_SkColorType:
                    if (info.alphaType() == kUnpremul_SkAlphaType) {
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
            switch (info.colorType()) {
                case kN32_SkColorType:
                    proc = &swizzle_rgbx_to_n32;
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
    if (NULL == proc) {
        return NULL;
    }

    // Store deltaSrc in bytes if it is an even multiple, otherwise use bits
    int deltaSrc = SkIsAlign8(BitsPerPixel(sc)) ? BytesPerPixel(sc) :
            BitsPerPixel(sc);
    return SkNEW_ARGS(SkSwizzler, (proc, ctable, deltaSrc, info));
}

SkSwizzler::SkSwizzler(RowProc proc, const SkPMColor* ctable,
                       int deltaSrc, const SkImageInfo& info)
    : fRowProc(proc)
    , fColorTable(ctable)
    , fDeltaSrc(deltaSrc)
    , fDstInfo(info)
{}

SkSwizzler::ResultAlpha SkSwizzler::swizzle(void* dst, const uint8_t* SK_RESTRICT src) {
    SkASSERT(NULL != dst && NULL != src);
    return fRowProc(dst, src, fDstInfo.width(), fDeltaSrc, fColorTable);
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
