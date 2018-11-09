/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Sk4px.h"
#include "SkBlitMask.h"
#include "SkColor.h"
#include "SkColorData.h"
#include "SkOpts.h"

SkBlitMask::BlitLCD16RowProc SkBlitMask::BlitLCD16RowFactory(bool isOpaque) {
    BlitLCD16RowProc proc = PlatformBlitRowProcs16(isOpaque);
    if (proc) {
        return proc;
    }

    if (isOpaque) {
        return  SkBlitLCD16OpaqueRow;
    } else {
        return  SkBlitLCD16Row;
    }
}

static void D32_LCD16_Proc(void* dst, size_t dstRB,
                           const void* mask, size_t maskRB,
                           SkColor color, int width, int height) {

    SkPMColor*        dstRow = (SkPMColor*)dst;
    const uint16_t* srcRow = (const uint16_t*)mask;
    SkPMColor       opaqueDst;

    SkBlitMask::BlitLCD16RowProc proc = nullptr;
    bool isOpaque = (0xFF == SkColorGetA(color));
    proc = SkBlitMask::BlitLCD16RowFactory(isOpaque);
    SkASSERT(proc != nullptr);

    if (isOpaque) {
        opaqueDst = SkPreMultiplyColor(color);
    } else {
        opaqueDst = 0;  // ignored
    }

    do {
        proc(dstRow, srcRow, color, width, opaqueDst);
        dstRow = (SkPMColor*)((char*)dstRow + dstRB);
        srcRow = (const uint16_t*)((const char*)srcRow + maskRB);
    } while (--height != 0);
}

bool SkBlitMask::BlitColor(const SkPixmap& device,
                           const SkMask& mask,
                           const SkIRect& clip,
                           SkColor color) {
    int x = clip.fLeft,
        y = clip.fTop;

    if (device.colorType() == kN32_SkColorType && mask.fFormat == SkMask::kA8_Format) {
        SkOpts::blit_mask_d32_a8(device.writable_addr32(x,y), device.rowBytes(),
                                 (const SkAlpha*)mask.getAddr(x,y), mask.fRowBytes,
                                 color, clip.width(), clip.height());
        return true;
    }

    if (device.colorType() == kN32_SkColorType && mask.fFormat == SkMask::kLCD16_Format) {
        D32_LCD16_Proc(device.writable_addr32(x,y), device.rowBytes(),
                       mask.getAddr(x,y), mask.fRowBytes,
                       color, clip.width(), clip.height());
        return true;
    }

    return false;
}

static void A8_RowProc_Blend(SkPMColor* dst, const void* vmask, const SkPMColor* src, int n) {
    auto mask = (const uint8_t*)vmask;

    Sk4px::MapDstSrcAlpha(n, dst, src, mask, [](const Sk4px& d, const Sk4px& s, const Sk4px& aa) {
        const auto s_aa = s.approxMulDiv255(aa);
        return s_aa + d.approxMulDiv255(s_aa.alphas().inv());
    });
}

static void A8_RowProc_Opaque(SkPMColor* dst, const void* vmask, const SkPMColor* src, int n) {
    auto mask = (const uint8_t*)vmask;

    Sk4px::MapDstSrcAlpha(n, dst, src, mask, [](const Sk4px& d, const Sk4px& s, const Sk4px& aa) {
        return (s * aa + d * aa.inv()).div255();
    });
}

static void LCD16_RowProc_Blend(SkPMColor* dst, const void* maskIn, const SkPMColor* src, int n) {

    auto src_alpha_blend = [](int s, int d, int sa, int m) {
        return d + SkAlphaMul(s - SkAlphaMul(sa, d), m);
    };

    auto upscale_31_to_255 = [](int v) {
        return (v << 3) | (v >> 2);
    };

    auto mask = (const uint16_t*)maskIn;
    for (int i = 0; i < n; ++i) {
        uint16_t m = mask[i];
        if (0 == m) {
            continue;
        }

        SkPMColor s = src[i];
        SkPMColor d = dst[i];

        int srcA = SkGetPackedA32(s);
        int srcR = SkGetPackedR32(s);
        int srcG = SkGetPackedG32(s);
        int srcB = SkGetPackedB32(s);

        srcA += srcA >> 7;

        // We're ignoring the least significant bit of the green coverage channel here.
        int maskR = SkGetPackedR16(m) >> (SK_R16_BITS - 5);
        int maskG = SkGetPackedG16(m) >> (SK_G16_BITS - 5);
        int maskB = SkGetPackedB16(m) >> (SK_B16_BITS - 5);

        // Scale up to 8-bit coverage to work with SkAlphaMul() in src_alpha_blend().
        maskR = upscale_31_to_255(maskR);
        maskG = upscale_31_to_255(maskG);
        maskB = upscale_31_to_255(maskB);

        // This LCD blit routine only works if the destination is opaque.
        dst[i] = SkPackARGB32(0xFF,
                              src_alpha_blend(srcR, SkGetPackedR32(d), srcA, maskR),
                              src_alpha_blend(srcG, SkGetPackedG32(d), srcA, maskG),
                              src_alpha_blend(srcB, SkGetPackedB32(d), srcA, maskB));
    }
}

static void LCD16_RowProc_Opaque(SkPMColor* dst, const void* vmask, const SkPMColor* src, int n) {
    auto mask = (const uint16_t*)vmask;

    for (int i = 0; i < n; ++i) {
        uint16_t m = mask[i];
        if (0 == m) {
            continue;
        }

        SkPMColor s = src[i];
        SkPMColor d = dst[i];

        int srcR = SkGetPackedR32(s);
        int srcG = SkGetPackedG32(s);
        int srcB = SkGetPackedB32(s);

        // We're ignoring the least significant bit of the green coverage channel here.
        int maskR = SkGetPackedR16(m) >> (SK_R16_BITS - 5);
        int maskG = SkGetPackedG16(m) >> (SK_G16_BITS - 5);
        int maskB = SkGetPackedB16(m) >> (SK_B16_BITS - 5);

        // Now upscale them to 0..32, so we can use SkBlend32.
        maskR = SkUpscale31To32(maskR);
        maskG = SkUpscale31To32(maskG);
        maskB = SkUpscale31To32(maskB);

        // This LCD blit routine only works if the destination is opaque.
        dst[i] = SkPackARGB32(0xFF,
                              SkBlend32(srcR, SkGetPackedR32(d), maskR),
                              SkBlend32(srcG, SkGetPackedG32(d), maskG),
                              SkBlend32(srcB, SkGetPackedB32(d), maskB));
    }
}

SkBlitMask::RowProc SkBlitMask::RowFactory(SkColorType ct,
                                           SkMask::Format format,
                                           RowFlags flags) {
    static const RowProc gProcs[] = {
        (RowProc)A8_RowProc_Blend,      (RowProc)A8_RowProc_Opaque,
        (RowProc)LCD16_RowProc_Blend,   (RowProc)LCD16_RowProc_Opaque,
    };

    switch (ct) {
        case kN32_SkColorType: {
            size_t index;
            switch (format) {
                case SkMask::kA8_Format:    index = 0; break;
                case SkMask::kLCD16_Format: index = 2; break;
                default:
                    return nullptr;
            }
            if (flags & kSrcIsOpaque_RowFlag) {
                index |= 1;
            }
            SkASSERT(index < SK_ARRAY_COUNT(gProcs));
            return gProcs[index];
        }
        default:
            break;
    }
    return nullptr;
}
