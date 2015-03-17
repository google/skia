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

SkSwizzler::ResultAlpha SkSwizzler::GetResult(uint8_t zeroAlpha,
                                              uint8_t maxAlpha) {
    // In the transparent case, this returns 0x0000
    // In the opaque case, this returns 0xFFFF
    // If the row is neither transparent nor opaque, returns something else
    return (((uint16_t) maxAlpha) << 8) | zeroAlpha;
}

// kIndex1, kIndex2, kIndex4

static SkSwizzler::ResultAlpha swizzle_small_index_to_n32(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int width,
        int bitsPerPixel, int y, const SkPMColor ctable[]) {

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

static SkSwizzler::ResultAlpha swizzle_index_to_n32(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int width,
        int bytesPerPixel, int y, const SkPMColor ctable[]) {

    SkPMColor* SK_RESTRICT dst = (SkPMColor*)dstRow;
    INIT_RESULT_ALPHA;
    for (int x = 0; x < width; x++) {
        SkPMColor c = ctable[*src];
        UPDATE_RESULT_ALPHA(c >> SK_A32_SHIFT);
        dst[x] = c;
        src++;
    }
    return COMPUTE_RESULT_ALPHA;
}

static SkSwizzler::ResultAlpha swizzle_index_to_n32_skipZ(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int width,
        int bytesPerPixel, int y, const SkPMColor ctable[]) {

    SkPMColor* SK_RESTRICT dst = (SkPMColor*)dstRow;
    INIT_RESULT_ALPHA;
    for (int x = 0; x < width; x++) {
        SkPMColor c = ctable[*src];
        UPDATE_RESULT_ALPHA(c >> SK_A32_SHIFT);
        if (c != 0) {
            dst[x] = c;
        }
        src++;
    }
    return COMPUTE_RESULT_ALPHA;
}

#undef A32_MASK_IN_PLACE

static SkSwizzler::ResultAlpha swizzle_bgrx_to_n32(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int width,
        int bytesPerPixel, int y, const SkPMColor ctable[]) {

    SkPMColor* SK_RESTRICT dst = (SkPMColor*)dstRow;
    for (int x = 0; x < width; x++) {
        dst[x] = SkPackARGB32NoCheck(0xFF, src[2], src[1], src[0]);
        src += bytesPerPixel;
    }
    return SkSwizzler::kOpaque_ResultAlpha;
}

// kBGRA

static SkSwizzler::ResultAlpha swizzle_bgra_to_n32(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int width,
        int bytesPerPixel, int y, const SkPMColor ctable[]) {

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

// n32
static SkSwizzler::ResultAlpha swizzle_rgbx_to_n32(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int width,
        int bytesPerPixel, int y, const SkPMColor ctable[]) {

    SkPMColor* SK_RESTRICT dst = (SkPMColor*)dstRow;
    for (int x = 0; x < width; x++) {
        dst[x] = SkPackARGB32(0xFF, src[0], src[1], src[2]);
        src += bytesPerPixel;
    }
    return SkSwizzler::kOpaque_ResultAlpha;
}

static SkSwizzler::ResultAlpha swizzle_rgba_to_n32_premul(
        void* SK_RESTRICT dstRow, const uint8_t* SK_RESTRICT src, int width,
        int bytesPerPixel, int y, const SkPMColor ctable[]) {

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
        int bytesPerPixel, int y, const SkPMColor ctable[]) {

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
        int bytesPerPixel, int y, const SkPMColor ctable[]) {

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
                                       const SkImageInfo& info, void* dst,
                                       size_t dstRowBytes,
                                       SkImageGenerator::ZeroInitialized zeroInit) {
    if (kUnknown_SkColorType == info.colorType()) {
        return NULL;
    }
    if (info.minRowBytes() > dstRowBytes) {
        return  NULL;
    }
    if ((kIndex == sc || kIndex4 == sc || kIndex2 == sc || kIndex1 == sc)
            && NULL == ctable) {
        return NULL;
    }
    RowProc proc = NULL;
    switch (sc) {
        case kIndex1:
        case kIndex2:
        case kIndex4:
            switch (info.colorType()) {
                case kN32_SkColorType:
                    proc = &swizzle_small_index_to_n32;
                    break;
                default:
                    break;
            }
            break;
        case kIndex:
            switch (info.colorType()) {
                case kN32_SkColorType:
                    // We assume the color premultiplied ctable (or not) as desired.
                    if (SkImageGenerator::kYes_ZeroInitialized == zeroInit) {
                        proc = &swizzle_index_to_n32_skipZ;
                    } else {
                        proc = &swizzle_index_to_n32;
                    }
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
                    proc = &swizzle_bgra_to_n32;
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
                        if (SkImageGenerator::kYes_ZeroInitialized == zeroInit) {
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
    return SkNEW_ARGS(SkSwizzler, (proc, ctable, deltaSrc, info, dst,
                                   dstRowBytes));
}

SkSwizzler::SkSwizzler(RowProc proc, const SkPMColor* ctable,
                       int deltaSrc, const SkImageInfo& info, void* dst,
                       size_t rowBytes)
    : fRowProc(proc)
    , fColorTable(ctable)
    , fDeltaSrc(deltaSrc)
    , fDstInfo(info)
    , fDstRow(dst)
    , fDstRowBytes(rowBytes)
    , fCurrY(0)
{
    SkDEBUGCODE(fNextMode = kUninitialized_NextMode);
}

SkSwizzler::ResultAlpha SkSwizzler::next(const uint8_t* SK_RESTRICT src) {
    SkASSERT(0 <= fCurrY && fCurrY < fDstInfo.height());
    SkASSERT(kDesignateRow_NextMode != fNextMode);
    SkDEBUGCODE(fNextMode = kConsecutive_NextMode);

    // Decode a row
    const ResultAlpha result = fRowProc(fDstRow, src, fDstInfo.width(),
            fDeltaSrc, fCurrY, fColorTable);

    // Move to the next row and return the result
    fDstRow = SkTAddOffset<void>(fDstRow, fDstRowBytes);
    return result;
}

SkSwizzler::ResultAlpha SkSwizzler::next(const uint8_t* SK_RESTRICT src,
        int y) {
    SkASSERT(0 <= y && y < fDstInfo.height());
    SkASSERT(kConsecutive_NextMode != fNextMode);
    SkDEBUGCODE(fNextMode = kDesignateRow_NextMode);
    
    // Choose the row
    void* row = SkTAddOffset<void>(fDstRow, y*fDstRowBytes);

    // Decode the row
    return fRowProc(row, src, fDstInfo.width(), fDeltaSrc, fCurrY,
            fColorTable);
}
