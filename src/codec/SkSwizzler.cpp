/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorPriv.h"
#include "SkSwizzler.h"
#include "SkTemplates.h"

// index

#define A32_MASK_IN_PLACE   (SkPMColor)(SK_A32_MASK << SK_A32_SHIFT)

static bool swizzle_index_to_n32(void* SK_RESTRICT dstRow,
                                 const uint8_t* SK_RESTRICT src,
                                 int width, int deltaSrc, int, const SkPMColor ctable[]) {

    SkPMColor* SK_RESTRICT dst = (SkPMColor*)dstRow;
    SkPMColor cc = A32_MASK_IN_PLACE;
    for (int x = 0; x < width; x++) {
        SkPMColor c = ctable[*src];
        cc &= c;
        dst[x] = c;
        src += deltaSrc;
    }
    return cc != A32_MASK_IN_PLACE;
}

static bool swizzle_index_to_n32_skipZ(void* SK_RESTRICT dstRow,
                                      const uint8_t* SK_RESTRICT src,
                                      int width, int deltaSrc, int,
                                      const SkPMColor ctable[]) {

    SkPMColor* SK_RESTRICT dst = (SkPMColor*)dstRow;
    SkPMColor cc = A32_MASK_IN_PLACE;
    for (int x = 0; x < width; x++) {
        SkPMColor c = ctable[*src];
        cc &= c;
        if (c != 0) {
            dst[x] = c;
        }
        src += deltaSrc;
    }
    return cc != A32_MASK_IN_PLACE;
}

#undef A32_MASK_IN_PLACE

// n32
static bool swizzle_rgbx_to_n32(void* SK_RESTRICT dstRow,
                                const uint8_t* SK_RESTRICT src,
                                int width, int deltaSrc, int, const SkPMColor[]) {
    SkPMColor* SK_RESTRICT dst = (SkPMColor*)dstRow;
    for (int x = 0; x < width; x++) {
        dst[x] = SkPackARGB32(0xFF, src[0], src[1], src[2]);
        src += deltaSrc;
    }
    return false;
}

static bool swizzle_rgba_to_n32_premul(void* SK_RESTRICT dstRow,
                                       const uint8_t* SK_RESTRICT src,
                                       int width, int deltaSrc, int, const SkPMColor[]) {
    SkPMColor* SK_RESTRICT dst = (SkPMColor*)dstRow;
    unsigned alphaMask = 0xFF;
    for (int x = 0; x < width; x++) {
        unsigned alpha = src[3];
        dst[x] = SkPreMultiplyARGB(alpha, src[0], src[1], src[2]);
        src += deltaSrc;
        alphaMask &= alpha;
    }
    return alphaMask != 0xFF;
}

static bool swizzle_rgba_to_n32_unpremul(void* SK_RESTRICT dstRow,
                                         const uint8_t* SK_RESTRICT src,
                                         int width, int deltaSrc, int,
                                         const SkPMColor[]) {
    uint32_t* SK_RESTRICT dst = reinterpret_cast<uint32_t*>(dstRow);
    unsigned alphaMask = 0xFF;
    for (int x = 0; x < width; x++) {
        unsigned alpha = src[3];
        dst[x] = SkPackARGB32NoCheck(alpha, src[0], src[1], src[2]);
        src += deltaSrc;
        alphaMask &= alpha;
    }
    return alphaMask != 0xFF;
}

static bool swizzle_rgba_to_n32_premul_skipZ(void* SK_RESTRICT dstRow,
                                             const uint8_t* SK_RESTRICT src,
                                             int width, int deltaSrc, int,
                                             const SkPMColor[]) {
    SkPMColor* SK_RESTRICT dst = (SkPMColor*)dstRow;
    unsigned alphaMask = 0xFF;
    for (int x = 0; x < width; x++) {
        unsigned alpha = src[3];
        if (0 != alpha) {
            dst[x] = SkPreMultiplyARGB(alpha, src[0], src[1], src[2]);
        }
        src += deltaSrc;
        alphaMask &= alpha;
    }
    return alphaMask != 0xFF;
}

/**
    FIXME: This was my idea to cheat in order to continue taking advantage of skipping zeroes.
    This would be fine for drawing normally, but not for drawing with transfer modes. Being
    honest means we can draw correctly with transfer modes, with the cost of not being able
    to take advantage of Android's free unwritten pages. Something to keep in mind when we
    decide whether to switch to unpremul default.
static bool swizzle_rgba_to_n32_unpremul_skipZ(void* SK_RESTRICT dstRow,
                                               const uint8_t* SK_RESTRICT src,
                                               int width, int deltaSrc, int,
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

SkSwizzler* SkSwizzler::CreateSwizzler(SkSwizzler::SrcConfig sc, const SkPMColor* ctable,
                                       const SkImageInfo& info, void* dst,
                                       size_t dstRowBytes, bool skipZeroes) {
    if (info.colorType() == kUnknown_SkColorType) {
        return NULL;
    }
    if (info.minRowBytes() > dstRowBytes) {
        return  NULL;
    }
    if (kIndex == sc && NULL == ctable) {
        return NULL;
    }
    RowProc proc = NULL;
    switch (sc) {
        case kIndex:
            switch (info.colorType()) {
                case kN32_SkColorType:
                    // We assume the color premultiplied ctable (or not) as desired.
                    if (skipZeroes) {
                        proc = &swizzle_index_to_n32_skipZ;
                    } else {
                        proc = &swizzle_index_to_n32;
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
                default:
                    break;
            }
            break;
        case kRGBA:
            switch (info.colorType()) {
                case kN32_SkColorType:
                    if (info.alphaType() == kUnpremul_SkAlphaType) {
                        // Respect skipZeroes?
                        proc = &swizzle_rgba_to_n32_unpremul;
                    } else {
                        if (skipZeroes) {
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
        default:
            break;
    }
    if (NULL == proc) {
        return NULL;
    }
    return SkNEW_ARGS(SkSwizzler, (proc, ctable, BytesPerPixel(sc), info, dst, dstRowBytes));
}

SkSwizzler::SkSwizzler(RowProc proc, const SkPMColor* ctable, int srcBpp,
                       const SkImageInfo& info, void* dst, size_t rowBytes)
    : fRowProc(proc)
    , fColorTable(ctable)
    , fSrcPixelSize(srcBpp)
    , fDstInfo(info)
    , fDstRow(dst)
    , fDstRowBytes(rowBytes)
    , fCurrY(0)
{
}

bool SkSwizzler::next(const uint8_t* SK_RESTRICT src) {
    SkASSERT(fCurrY < fDstInfo.height());
    const bool hadAlpha = fRowProc(fDstRow, src, fDstInfo.width(), fSrcPixelSize,
                                   fCurrY, fColorTable);
    fCurrY++;
    fDstRow = SkTAddOffset<void>(fDstRow, fDstRowBytes);
    return hadAlpha;
}
