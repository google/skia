/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCodecPriv.h"
#include "SkColorPriv.h"
#include "SkOpts.h"
#include "SkSwizzler.h"
#include "SkTemplates.h"

// samples the row. Does not do anything else but sampling
static void sample565(void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src,
        int width, int bpp, int deltaSrc, int offset, const SkPMColor ctable[]){

    src += offset;
    uint16_t* SK_RESTRICT dst = (uint16_t*) dstRow;
    for (int x = 0; x < width; x++) {
        dst[x] = src[1] << 8 | src[0];
        src += deltaSrc;
    }
}

// TODO (msarett): Investigate SIMD optimizations for swizzle routines.

// kBit
// These routines exclusively choose between white and black

#define GRAYSCALE_BLACK 0
#define GRAYSCALE_WHITE 0xFF


// same as swizzle_bit_to_index and swizzle_bit_to_n32 except for value assigned to dst[x]
static void swizzle_bit_to_grayscale(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int dstWidth,
        int bpp, int deltaSrc, int offset, const SkPMColor* /*ctable*/) {

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
}

#undef GRAYSCALE_BLACK
#undef GRAYSCALE_WHITE

// same as swizzle_bit_to_grayscale and swizzle_bit_to_n32 except for value assigned to dst[x]
static void swizzle_bit_to_index(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int dstWidth,
        int bpp, int deltaSrc, int offset, const SkPMColor* /*ctable*/) {
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
}

// same as swizzle_bit_to_grayscale and swizzle_bit_to_index except for value assigned to dst[x]
static void swizzle_bit_to_n32(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int dstWidth,
        int bpp, int deltaSrc, int offset, const SkPMColor* /*ctable*/) {
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
}

#define RGB565_BLACK 0
#define RGB565_WHITE 0xFFFF

static void swizzle_bit_to_565(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int dstWidth,
        int bpp, int deltaSrc, int offset, const SkPMColor* /*ctable*/) {
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
}

#undef RGB565_BLACK
#undef RGB565_WHITE

// kIndex1, kIndex2, kIndex4

static void swizzle_small_index_to_index(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int dstWidth,
        int bpp, int deltaSrc, int offset, const SkPMColor ctable[]) {

    uint8_t* dst = (uint8_t*) dstRow;
    src += offset / 8;
    int bitIndex = offset % 8;
    uint8_t currByte = *src;
    const uint8_t mask = (1 << bpp) - 1;
    uint8_t index = (currByte >> (8 - bpp - bitIndex)) & mask;
    dst[0] = index;

    for (int x = 1; x < dstWidth; x++) {
        int bitOffset = bitIndex + deltaSrc;
        bitIndex = bitOffset % 8;
        currByte = *(src += bitOffset / 8);
        index = (currByte >> (8 - bpp - bitIndex)) & mask;
        dst[x] = index;
    }
}

static void swizzle_small_index_to_565(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int dstWidth,
        int bpp, int deltaSrc, int offset, const SkPMColor ctable[]) {

    uint16_t* dst = (uint16_t*) dstRow;
    src += offset / 8;
    int bitIndex = offset % 8;
    uint8_t currByte = *src;
    const uint8_t mask = (1 << bpp) - 1;
    uint8_t index = (currByte >> (8 - bpp - bitIndex)) & mask;
    dst[0] = SkPixel32ToPixel16(ctable[index]);

    for (int x = 1; x < dstWidth; x++) {
        int bitOffset = bitIndex + deltaSrc;
        bitIndex = bitOffset % 8;
        currByte = *(src += bitOffset / 8);
        index = (currByte >> (8 - bpp - bitIndex)) & mask;
        dst[x] = SkPixel32ToPixel16(ctable[index]);
    }
}

static void swizzle_small_index_to_n32(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int dstWidth,
        int bpp, int deltaSrc, int offset, const SkPMColor ctable[]) {

    SkPMColor* dst = (SkPMColor*) dstRow;
    src += offset / 8;
    int bitIndex = offset % 8;
    uint8_t currByte = *src;
    const uint8_t mask = (1 << bpp) - 1;
    uint8_t index = (currByte >> (8 - bpp - bitIndex)) & mask;
    dst[0] = ctable[index];

    for (int x = 1; x < dstWidth; x++) {
        int bitOffset = bitIndex + deltaSrc;
        bitIndex = bitOffset % 8;
        currByte = *(src += bitOffset / 8);
        index = (currByte >> (8 - bpp - bitIndex)) & mask;
        dst[x] = ctable[index];
    }
}

// kIndex

static void swizzle_index_to_index(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int dstWidth,
        int bpp, int deltaSrc, int offset, const SkPMColor ctable[]) {

    src += offset;
    uint8_t* SK_RESTRICT dst = (uint8_t*) dstRow;
    if (1 == deltaSrc) {
        memcpy(dst, src, dstWidth);
    } else {
        for (int x = 0; x < dstWidth; x++) {
            dst[x] = *src;
            src += deltaSrc;
        }
    }
}

static void swizzle_index_to_n32(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int dstWidth,
        int bpp, int deltaSrc, int offset, const SkPMColor ctable[]) {

    src += offset;
    SkPMColor* SK_RESTRICT dst = (SkPMColor*)dstRow;
    for (int x = 0; x < dstWidth; x++) {
        SkPMColor c = ctable[*src];
        dst[x] = c;
        src += deltaSrc;
    }
}

static void swizzle_index_to_n32_skipZ(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int dstWidth,
        int bpp, int deltaSrc, int offset, const SkPMColor ctable[]) {

    src += offset;
    SkPMColor* SK_RESTRICT dst = (SkPMColor*)dstRow;
    for (int x = 0; x < dstWidth; x++) {
        SkPMColor c = ctable[*src];
        if (c != 0) {
            dst[x] = c;
        }
        src += deltaSrc;
    }
}

static void swizzle_index_to_565(
      void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int dstWidth,
      int bytesPerPixel, int deltaSrc, int offset, const SkPMColor ctable[]) {
    src += offset;
    uint16_t* SK_RESTRICT dst = (uint16_t*)dstRow;
    for (int x = 0; x < dstWidth; x++) {
        dst[x] = SkPixel32ToPixel16(ctable[*src]);
        src += deltaSrc;
    }
}

// kGray

static void swizzle_gray_to_n32(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int dstWidth,
        int bpp, int deltaSrc, int offset, const SkPMColor ctable[]) {

    src += offset;
    SkPMColor* SK_RESTRICT dst = (SkPMColor*)dstRow;
    for (int x = 0; x < dstWidth; x++) {
        dst[x] = SkPackARGB32NoCheck(0xFF, *src, *src, *src);
        src += deltaSrc;
    }
}

static void swizzle_gray_to_gray(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int dstWidth,
        int bpp, int deltaSrc, int offset, const SkPMColor ctable[]) {

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
}

static void swizzle_gray_to_565(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int dstWidth,
        int bytesPerPixel, int deltaSrc, int offset, const SkPMColor ctable[]) {

    src += offset;
    uint16_t* SK_RESTRICT dst = (uint16_t*)dstRow;
    for (int x = 0; x < dstWidth; x++) {
        dst[x] = SkPack888ToRGB16(src[0], src[0], src[0]);
        src += deltaSrc;
    }
}

// kBGRX

static void swizzle_bgrx_to_n32(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int dstWidth,
        int bpp, int deltaSrc, int offset, const SkPMColor ctable[]) {

    src += offset;
    SkPMColor* SK_RESTRICT dst = (SkPMColor*)dstRow;
    for (int x = 0; x < dstWidth; x++) {
        dst[x] = SkPackARGB32NoCheck(0xFF, src[2], src[1], src[0]);
        src += deltaSrc;
    }
}

static void swizzle_bgrx_to_565(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int dstWidth,
        int bpp, int deltaSrc, int offset, const SkPMColor ctable[]) {

    src += offset;
    uint16_t* SK_RESTRICT dst = (uint16_t*)dstRow;
    for (int x = 0; x < dstWidth; x++) {
        dst[x] = SkPack888ToRGB16(src[2], src[1], src[0]);
        src += deltaSrc;
    }
}

// kBGRA

static void swizzle_bgra_to_n32_unpremul(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int dstWidth,
        int bpp, int deltaSrc, int offset, const SkPMColor ctable[]) {

    src += offset;
    SkPMColor* SK_RESTRICT dst = (SkPMColor*)dstRow;
    for (int x = 0; x < dstWidth; x++) {
        uint8_t alpha = src[3];
        dst[x] = SkPackARGB32NoCheck(alpha, src[2], src[1], src[0]);
        src += deltaSrc;
    }
}

static void swizzle_bgra_to_n32_premul(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int dstWidth,
        int bpp, int deltaSrc, int offset, const SkPMColor ctable[]) {

    src += offset;
    SkPMColor* SK_RESTRICT dst = (SkPMColor*)dstRow;
    for (int x = 0; x < dstWidth; x++) {
        uint8_t alpha = src[3];
        dst[x] = SkPremultiplyARGBInline(alpha, src[2], src[1], src[0]);
        src += deltaSrc;
    }
}

// kRGBX
static void swizzle_rgbx_to_n32(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int dstWidth,
        int bpp, int deltaSrc, int offset, const SkPMColor ctable[]) {

    src += offset;
    SkPMColor* SK_RESTRICT dst = (SkPMColor*)dstRow;
    for (int x = 0; x < dstWidth; x++) {
        dst[x] = SkPackARGB32(0xFF, src[0], src[1], src[2]);
        src += deltaSrc;
    }
}

static void swizzle_rgbx_to_565(
       void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int dstWidth,
       int bytesPerPixel, int deltaSrc, int offset, const SkPMColor ctable[]) {

    src += offset;
    uint16_t* SK_RESTRICT dst = (uint16_t*)dstRow;
    for (int x = 0; x < dstWidth; x++) {
        dst[x] = SkPack888ToRGB16(src[0], src[1], src[2]);
        src += deltaSrc;
    }
}

// kRGBA
static void swizzle_rgba_to_n32_premul(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int dstWidth,
        int bpp, int deltaSrc, int offset, const SkPMColor ctable[]) {

    src += offset;
    SkPMColor* SK_RESTRICT dst = (SkPMColor*)dstRow;
    for (int x = 0; x < dstWidth; x++) {
        unsigned alpha = src[3];
        dst[x] = SkPremultiplyARGBInline(alpha, src[0], src[1], src[2]);
        src += deltaSrc;
    }
}

static void fast_swizzle_rgba_to_n32_premul(
        void* dst, const uint8_t* src, int width, int bpp, int deltaSrc,
        int offset, const SkPMColor ctable[]) {

    // This function must not be called if we are sampling.  If we are not
    // sampling, deltaSrc should equal bpp.
    SkASSERT(deltaSrc == bpp);

#ifdef SK_PMCOLOR_IS_RGBA
    SkOpts::premul_xxxa((uint32_t*) dst, (const uint32_t*) (src + offset), width);
#else
    SkOpts::premul_swaprb_xxxa((uint32_t*) dst, (const uint32_t*) (src + offset), width);
#endif
}

static void swizzle_rgba_to_n32_unpremul(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int dstWidth,
        int bpp, int deltaSrc, int offset, const SkPMColor ctable[]) {

    src += offset;
    uint32_t* SK_RESTRICT dst = reinterpret_cast<uint32_t*>(dstRow);
    for (int x = 0; x < dstWidth; x++) {
        unsigned alpha = src[3];
        dst[x] = SkPackARGB32NoCheck(alpha, src[0], src[1], src[2]);
        src += deltaSrc;
    }
}

// kCMYK
//
// CMYK is stored as four bytes per pixel.
//
// We will implement a crude conversion from CMYK -> RGB using formulas
// from easyrgb.com.
//
// CMYK -> CMY
// C = C * (1 - K) + K
// M = M * (1 - K) + K
// Y = Y * (1 - K) + K
//
// libjpeg actually gives us inverted CMYK, so we must subtract the
// original terms from 1.
// CMYK -> CMY
// C = (1 - C) * (1 - (1 - K)) + (1 - K)
// M = (1 - M) * (1 - (1 - K)) + (1 - K)
// Y = (1 - Y) * (1 - (1 - K)) + (1 - K)
//
// Simplifying the above expression.
// CMYK -> CMY
// C = 1 - CK
// M = 1 - MK
// Y = 1 - YK
//
// CMY -> RGB
// R = (1 - C) * 255
// G = (1 - M) * 255
// B = (1 - Y) * 255
//
// Therefore the full conversion is below.  This can be verified at
// www.rapidtables.com (assuming inverted CMYK).
// CMYK -> RGB
// R = C * K * 255
// G = M * K * 255
// B = Y * K * 255
//
// As a final note, we have treated the CMYK values as if they were on
// a scale from 0-1, when in fact they are 8-bit ints scaling from 0-255.
// We must divide each CMYK component by 255 to obtain the true conversion
// we should perform.
// CMYK -> RGB
// R = C * K / 255
// G = M * K / 255
// B = Y * K / 255
static void swizzle_cmyk_to_n32(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int dstWidth,
        int bpp, int deltaSrc, int offset, const SkPMColor ctable[]) {

    src += offset;
    SkPMColor* SK_RESTRICT dst = (SkPMColor*)dstRow;
    for (int x = 0; x < dstWidth; x++) {
        const uint8_t r = SkMulDiv255Round(src[0], src[3]);
        const uint8_t g = SkMulDiv255Round(src[1], src[3]);
        const uint8_t b = SkMulDiv255Round(src[2], src[3]);

        dst[x] = SkPackARGB32NoCheck(0xFF, r, g, b);
        src += deltaSrc;
    }
}

static void swizzle_cmyk_to_565(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int dstWidth,
        int bpp, int deltaSrc, int offset, const SkPMColor ctable[]) {

    src += offset;
    uint16_t* SK_RESTRICT dst = (uint16_t*)dstRow;
    for (int x = 0; x < dstWidth; x++) {
        const uint8_t r = SkMulDiv255Round(src[0], src[3]);
        const uint8_t g = SkMulDiv255Round(src[1], src[3]);
        const uint8_t b = SkMulDiv255Round(src[2], src[3]);

        dst[x] = SkPack888ToRGB16(r, g, b);
        src += deltaSrc;
    }
}

template <SkSwizzler::RowProc proc>
void SkSwizzler::SkipLeading8888ZerosThen(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int dstWidth,
        int bpp, int deltaSrc, int offset, const SkPMColor ctable[]) {
    SkASSERT(!ctable);

    auto src32 = (const uint32_t*)(src+offset);
    auto dst32 = (uint32_t*)dstRow;

    // This may miss opportunities to skip when the output is premultiplied,
    // e.g. for a src pixel 0x00FFFFFF which is not zero but becomes zero after premultiplication.
    while (dstWidth > 0 && *src32 == 0x00000000) {
        dstWidth--;
        dst32++;
        src32 += deltaSrc/4;
    }
    proc(dst32, (const uint8_t*)src32, dstWidth, bpp, deltaSrc, 0, ctable);
}

SkSwizzler* SkSwizzler::CreateSwizzler(SkSwizzler::SrcConfig sc,
                                       const SkPMColor* ctable,
                                       const SkImageInfo& dstInfo,
                                       const SkCodec::Options& options,
                                       const SkIRect* frame) {
    if (dstInfo.colorType() == kUnknown_SkColorType || kUnknown == sc) {
        return nullptr;
    }
    if ((kIndex == sc || kIndex4 == sc || kIndex2 == sc || kIndex1 == sc)
            && nullptr == ctable) {
        return nullptr;
    }
    RowProc fastProc = nullptr;
    RowProc proc = nullptr;
    SkCodec::ZeroInitialized zeroInit = options.fZeroInitialized;
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
                        if (SkCodec::kYes_ZeroInitialized == zeroInit) {
                            proc = &SkipLeading8888ZerosThen<swizzle_rgba_to_n32_unpremul>;
                        } else {
                            proc = &swizzle_rgba_to_n32_unpremul;
                        }
                    } else {
                        if (SkCodec::kYes_ZeroInitialized == zeroInit) {
                            proc = &SkipLeading8888ZerosThen<swizzle_rgba_to_n32_premul>;
                            fastProc = &SkipLeading8888ZerosThen<fast_swizzle_rgba_to_n32_premul>;
                        } else {
                            proc = &swizzle_rgba_to_n32_premul;
                            fastProc = &fast_swizzle_rgba_to_n32_premul;
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
            break;
        case kCMYK:
            switch (dstInfo.colorType()) {
                case kN32_SkColorType:
                    proc = &swizzle_cmyk_to_n32;
                    break;
                case kRGB_565_SkColorType:
                    proc = &swizzle_cmyk_to_565;
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }

    // Store bpp in bytes if it is an even multiple, otherwise use bits
    int srcBPP = SkIsAlign8(BitsPerPixel(sc)) ? BytesPerPixel(sc) : BitsPerPixel(sc);
    int dstBPP = SkColorTypeBytesPerPixel(dstInfo.colorType());

    int srcOffset = 0;
    int srcWidth = dstInfo.width();
    int dstOffset = 0;
    int dstWidth = srcWidth;
    if (options.fSubset) {
        // We do not currently support subset decodes for image types that may have
        // frames (gif).
        SkASSERT(!frame);
        srcOffset = options.fSubset->left();
        srcWidth = options.fSubset->width();
        dstWidth = srcWidth;
    } else if (frame) {
        dstOffset = frame->left();
        srcWidth = frame->width();
    }

    return new SkSwizzler(fastProc, proc, ctable, srcOffset, srcWidth, dstOffset, dstWidth,
            srcBPP, dstBPP);
}

SkSwizzler::SkSwizzler(RowProc fastProc, RowProc proc, const SkPMColor* ctable, int srcOffset,
        int srcWidth, int dstOffset, int dstWidth, int srcBPP, int dstBPP)
    : fFastProc(fastProc)
    , fProc(proc)
    , fColorTable(ctable)
    , fSrcOffset(srcOffset)
    , fDstOffset(dstOffset)
    , fSrcOffsetUnits(srcOffset * srcBPP)
    , fDstOffsetBytes(dstOffset * dstBPP)
    , fSrcWidth(srcWidth)
    , fDstWidth(dstWidth)
    , fSwizzleWidth(srcWidth)
    , fAllocatedWidth(dstWidth)
    , fSampleX(1)
    , fSrcBPP(srcBPP)
    , fDstBPP(dstBPP)
{}

int SkSwizzler::onSetSampleX(int sampleX) {
    SkASSERT(sampleX > 0); // Surely there is an upper limit? Should there be
                           // way to report failure?
    fSampleX = sampleX;
    fSrcOffsetUnits = (get_start_coord(sampleX) + fSrcOffset) * fSrcBPP;
    fDstOffsetBytes = (fDstOffset / sampleX) * fDstBPP;
    fSwizzleWidth = get_scaled_dimension(fSrcWidth, sampleX);
    fAllocatedWidth = get_scaled_dimension(fDstWidth, sampleX);

    // The optimized swizzler routines do not (yet) support sampling.
    fFastProc = nullptr;

    return fAllocatedWidth;
}

void SkSwizzler::swizzle(void* dst, const uint8_t* SK_RESTRICT src) {
    SkASSERT(nullptr != dst && nullptr != src);
    RowProc proc = fFastProc ? fFastProc : fProc;
    proc(SkTAddOffset<void>(dst, fDstOffsetBytes), src, fSwizzleWidth, fSrcBPP,
            fSampleX * fSrcBPP, fSrcOffsetUnits, fColorTable);
}
