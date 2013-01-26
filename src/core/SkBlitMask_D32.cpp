#include "SkBlitMask.h"
#include "SkColor.h"
#include "SkColorPriv.h"

static void D32_A8_Color(void* SK_RESTRICT dst, size_t dstRB,
                         const void* SK_RESTRICT maskPtr, size_t maskRB,
                         SkColor color, int width, int height) {
    SkPMColor pmc = SkPreMultiplyColor(color);
    size_t dstOffset = dstRB - (width << 2);
    size_t maskOffset = maskRB - width;
    SkPMColor* SK_RESTRICT device = (SkPMColor *)dst;
    const uint8_t* SK_RESTRICT mask = (const uint8_t*)maskPtr;

    do {
        int w = width;
        do {
            unsigned aa = *mask++;
            *device = SkBlendARGB32(pmc, *device, aa);
            device += 1;
        } while (--w != 0);
        device = (uint32_t*)((char*)device + dstOffset);
        mask += maskOffset;
    } while (--height != 0);
}

static void D32_A8_Opaque(void* SK_RESTRICT dst, size_t dstRB,
                          const void* SK_RESTRICT maskPtr, size_t maskRB,
                          SkColor color, int width, int height) {
    SkPMColor pmc = SkPreMultiplyColor(color);
    SkPMColor* SK_RESTRICT device = (SkPMColor*)dst;
    const uint8_t* SK_RESTRICT mask = (const uint8_t*)maskPtr;

    maskRB -= width;
    dstRB -= (width << 2);
    do {
        int w = width;
        do {
            unsigned aa = *mask++;
            *device = SkAlphaMulQ(pmc, SkAlpha255To256(aa)) + SkAlphaMulQ(*device, SkAlpha255To256(255 - aa));
            device += 1;
        } while (--w != 0);
        device = (uint32_t*)((char*)device + dstRB);
        mask += maskRB;
    } while (--height != 0);
}

static void D32_A8_Black(void* SK_RESTRICT dst, size_t dstRB,
                         const void* SK_RESTRICT maskPtr, size_t maskRB,
                         SkColor, int width, int height) {
    SkPMColor* SK_RESTRICT device = (SkPMColor*)dst;
    const uint8_t* SK_RESTRICT mask = (const uint8_t*)maskPtr;

    maskRB -= width;
    dstRB -= (width << 2);
    do {
        int w = width;
        do {
            unsigned aa = *mask++;
            *device = (aa << SK_A32_SHIFT) + SkAlphaMulQ(*device, SkAlpha255To256(255 - aa));
            device += 1;
        } while (--w != 0);
        device = (uint32_t*)((char*)device + dstRB);
        mask += maskRB;
    } while (--height != 0);
}

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

static void D32_LCD16_Proc(void* SK_RESTRICT dst, size_t dstRB,
                           const void* SK_RESTRICT mask, size_t maskRB,
                           SkColor color, int width, int height) {

    SkPMColor*        dstRow = (SkPMColor*)dst;
    const uint16_t* srcRow = (const uint16_t*)mask;
    SkPMColor       opaqueDst;

    SkBlitMask::BlitLCD16RowProc proc = NULL;
    bool isOpaque = (0xFF == SkColorGetA(color));
    proc = SkBlitMask::BlitLCD16RowFactory(isOpaque);
    SkASSERT(proc != NULL);

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

///////////////////////////////////////////////////////////////////////////////

static void blit_lcd32_opaque_row(SkPMColor* SK_RESTRICT dst,
                                  const SkPMColor* SK_RESTRICT src,
                                  SkColor color, int width) {
    int srcR = SkColorGetR(color);
    int srcG = SkColorGetG(color);
    int srcB = SkColorGetB(color);

    for (int i = 0; i < width; i++) {
        SkPMColor mask = src[i];
        if (0 == mask) {
            continue;
        }

        SkPMColor d = dst[i];

        int maskR = SkGetPackedR32(mask);
        int maskG = SkGetPackedG32(mask);
        int maskB = SkGetPackedB32(mask);

        // Now upscale them to 0..256, so we can use SkAlphaBlend
        maskR = SkAlpha255To256(maskR);
        maskG = SkAlpha255To256(maskG);
        maskB = SkAlpha255To256(maskB);

        int dstR = SkGetPackedR32(d);
        int dstG = SkGetPackedG32(d);
        int dstB = SkGetPackedB32(d);

        // LCD blitting is only supported if the dst is known/required
        // to be opaque
        dst[i] = SkPackARGB32(0xFF,
                              SkAlphaBlend(srcR, dstR, maskR),
                              SkAlphaBlend(srcG, dstG, maskG),
                              SkAlphaBlend(srcB, dstB, maskB));
    }
}

static void blit_lcd32_row(SkPMColor* SK_RESTRICT dst,
                           const SkPMColor* SK_RESTRICT src,
                           SkColor color, int width) {
    int srcA = SkColorGetA(color);
    int srcR = SkColorGetR(color);
    int srcG = SkColorGetG(color);
    int srcB = SkColorGetB(color);

    srcA = SkAlpha255To256(srcA);

    for (int i = 0; i < width; i++) {
        SkPMColor mask = src[i];
        if (0 == mask) {
            continue;
        }

        SkPMColor d = dst[i];

        int maskR = SkGetPackedR32(mask);
        int maskG = SkGetPackedG32(mask);
        int maskB = SkGetPackedB32(mask);

        // Now upscale them to 0..256, so we can use SkAlphaBlend
        maskR = SkAlpha255To256(maskR);
        maskG = SkAlpha255To256(maskG);
        maskB = SkAlpha255To256(maskB);

        maskR = maskR * srcA >> 8;
        maskG = maskG * srcA >> 8;
        maskB = maskB * srcA >> 8;

        int dstR = SkGetPackedR32(d);
        int dstG = SkGetPackedG32(d);
        int dstB = SkGetPackedB32(d);

        // LCD blitting is only supported if the dst is known/required
        // to be opaque
        dst[i] = SkPackARGB32(0xFF,
                              SkAlphaBlend(srcR, dstR, maskR),
                              SkAlphaBlend(srcG, dstG, maskG),
                              SkAlphaBlend(srcB, dstB, maskB));
    }
}

static void D32_LCD32_Blend(void* SK_RESTRICT dst, size_t dstRB,
                            const void* SK_RESTRICT mask, size_t maskRB,
                            SkColor color, int width, int height) {
    SkASSERT(height > 0);
    SkPMColor* SK_RESTRICT dstRow = (SkPMColor*)dst;
    const SkPMColor* SK_RESTRICT srcRow = (const SkPMColor*)mask;

    do {
        blit_lcd32_row(dstRow, srcRow, color, width);
        dstRow = (SkPMColor*)((char*)dstRow + dstRB);
        srcRow = (const SkPMColor*)((const char*)srcRow + maskRB);
    } while (--height != 0);
}

static void D32_LCD32_Opaque(void* SK_RESTRICT dst, size_t dstRB,
                             const void* SK_RESTRICT mask, size_t maskRB,
                             SkColor color, int width, int height) {
    SkASSERT(height > 0);
    SkPMColor* SK_RESTRICT dstRow = (SkPMColor*)dst;
    const SkPMColor* SK_RESTRICT srcRow = (const SkPMColor*)mask;

    do {
        blit_lcd32_opaque_row(dstRow, srcRow, color, width);
        dstRow = (SkPMColor*)((char*)dstRow + dstRB);
        srcRow = (const SkPMColor*)((const char*)srcRow + maskRB);
    } while (--height != 0);
}

///////////////////////////////////////////////////////////////////////////////

static SkBlitMask::ColorProc D32_A8_Factory(SkColor color) {
    if (SK_ColorBLACK == color) {
        return D32_A8_Black;
    } else if (0xFF == SkColorGetA(color)) {
        return D32_A8_Opaque;
    } else {
        return D32_A8_Color;
    }
}

static SkBlitMask::ColorProc D32_LCD32_Factory(SkColor color) {
    return (0xFF == SkColorGetA(color)) ? D32_LCD32_Opaque : D32_LCD32_Blend;
}

SkBlitMask::ColorProc SkBlitMask::ColorFactory(SkBitmap::Config config,
                                               SkMask::Format format,
                                               SkColor color) {
    ColorProc proc = PlatformColorProcs(config, format, color);
    if (proc) {
        return proc;
    }

    switch (config) {
        case SkBitmap::kARGB_8888_Config:
            switch (format) {
                case SkMask::kA8_Format:
                    return D32_A8_Factory(color);
                case SkMask::kLCD16_Format:
                    return D32_LCD16_Proc;
                case SkMask::kLCD32_Format:
                    return D32_LCD32_Factory(color);
                default:
                    break;
            }
            break;
        default:
            break;
    }
    return NULL;
}

bool SkBlitMask::BlitColor(const SkBitmap& device, const SkMask& mask,
                           const SkIRect& clip, SkColor color) {
    ColorProc proc = ColorFactory(device.config(), mask.fFormat, color);
    if (proc) {
        int x = clip.fLeft;
        int y = clip.fTop;
        proc(device.getAddr32(x, y), device.rowBytes(), mask.getAddr(x, y),
             mask.fRowBytes, color, clip.width(), clip.height());
        return true;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void BW_RowProc_Blend(SkPMColor* SK_RESTRICT dst,
                             const uint8_t* SK_RESTRICT mask,
                             const SkPMColor* SK_RESTRICT src, int count) {
    int i, octuple = (count + 7) >> 3;
    for (i = 0; i < octuple; ++i) {
        int m = *mask++;
        if (m & 0x80) { dst[0] = SkPMSrcOver(src[0], dst[0]); }
        if (m & 0x40) { dst[1] = SkPMSrcOver(src[1], dst[1]); }
        if (m & 0x20) { dst[2] = SkPMSrcOver(src[2], dst[2]); }
        if (m & 0x10) { dst[3] = SkPMSrcOver(src[3], dst[3]); }
        if (m & 0x08) { dst[4] = SkPMSrcOver(src[4], dst[4]); }
        if (m & 0x04) { dst[5] = SkPMSrcOver(src[5], dst[5]); }
        if (m & 0x02) { dst[6] = SkPMSrcOver(src[6], dst[6]); }
        if (m & 0x01) { dst[7] = SkPMSrcOver(src[7], dst[7]); }
        src += 8;
        dst += 8;
    }
    count &= 7;
    if (count > 0) {
        int m = *mask;
        do {
            if (m & 0x80) { dst[0] = SkPMSrcOver(src[0], dst[0]); }
            m <<= 1;
            src += 1;
            dst += 1;
        } while (--count > 0);
    }
}

static void BW_RowProc_Opaque(SkPMColor* SK_RESTRICT dst,
                              const uint8_t* SK_RESTRICT mask,
                              const SkPMColor* SK_RESTRICT src, int count) {
    int i, octuple = (count + 7) >> 3;
    for (i = 0; i < octuple; ++i) {
        int m = *mask++;
        if (m & 0x80) { dst[0] = src[0]; }
        if (m & 0x40) { dst[1] = src[1]; }
        if (m & 0x20) { dst[2] = src[2]; }
        if (m & 0x10) { dst[3] = src[3]; }
        if (m & 0x08) { dst[4] = src[4]; }
        if (m & 0x04) { dst[5] = src[5]; }
        if (m & 0x02) { dst[6] = src[6]; }
        if (m & 0x01) { dst[7] = src[7]; }
        src += 8;
        dst += 8;
    }
    count &= 7;
    if (count > 0) {
        int m = *mask;
        do {
            if (m & 0x80) { dst[0] = SkPMSrcOver(src[0], dst[0]); }
            m <<= 1;
            src += 1;
            dst += 1;
        } while (--count > 0);
    }
}

static void A8_RowProc_Blend(SkPMColor* SK_RESTRICT dst,
                             const uint8_t* SK_RESTRICT mask,
                             const SkPMColor* SK_RESTRICT src, int count) {
    for (int i = 0; i < count; ++i) {
        if (mask[i]) {
            dst[i] = SkBlendARGB32(src[i], dst[i], mask[i]);
        }
    }
}

// expand the steps that SkAlphaMulQ performs, but this way we can
//  exand.. add.. combine
// instead of
// expand..combine add expand..combine
//
#define EXPAND0(v, m, s)    ((v) & (m)) * (s)
#define EXPAND1(v, m, s)    (((v) >> 8) & (m)) * (s)
#define COMBINE(e0, e1, m)  ((((e0) >> 8) & (m)) | ((e1) & ~(m)))

static void A8_RowProc_Opaque(SkPMColor* SK_RESTRICT dst,
                              const uint8_t* SK_RESTRICT mask,
                              const SkPMColor* SK_RESTRICT src, int count) {
#if 0 // suppress warning
    const uint32_t rbmask = gMask_00FF00FF;
#endif
    for (int i = 0; i < count; ++i) {
        int m = mask[i];
        if (m) {
            m += (m >> 7);
#if 1
            // this is slightly slower than the expand/combine version, but it
            // is much closer to the old results, so we use it for now to reduce
            // rebaselining.
            dst[i] = SkAlphaMulQ(src[i], m) + SkAlphaMulQ(dst[i], 256 - m);
#else
            uint32_t v = src[i];
            uint32_t s0 = EXPAND0(v, rbmask, m);
            uint32_t s1 = EXPAND1(v, rbmask, m);
            v = dst[i];
            uint32_t d0 = EXPAND0(v, rbmask, m);
            uint32_t d1 = EXPAND1(v, rbmask, m);
            dst[i] = COMBINE(s0 + d0, s1 + d1, rbmask);
#endif
        }
    }
}

static int upscale31To255(int value) {
    value = (value << 3) | (value >> 2);
    return value;
}

static int mul(int a, int b) {
    return a * b >> 8;
}

static int src_alpha_blend(int src, int dst, int srcA, int mask) {

    return dst + mul(src - mul(srcA, dst), mask);
}

static void LCD16_RowProc_Blend(SkPMColor* SK_RESTRICT dst,
                                const uint16_t* SK_RESTRICT mask,
                                const SkPMColor* SK_RESTRICT src, int count) {
    for (int i = 0; i < count; ++i) {
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

        /*  We want all of these in 5bits, hence the shifts in case one of them
         *  (green) is 6bits.
         */
        int maskR = SkGetPackedR16(m) >> (SK_R16_BITS - 5);
        int maskG = SkGetPackedG16(m) >> (SK_G16_BITS - 5);
        int maskB = SkGetPackedB16(m) >> (SK_B16_BITS - 5);

        maskR = upscale31To255(maskR);
        maskG = upscale31To255(maskG);
        maskB = upscale31To255(maskB);

        int dstR = SkGetPackedR32(d);
        int dstG = SkGetPackedG32(d);
        int dstB = SkGetPackedB32(d);

        // LCD blitting is only supported if the dst is known/required
        // to be opaque
        dst[i] = SkPackARGB32(0xFF,
                              src_alpha_blend(srcR, dstR, srcA, maskR),
                              src_alpha_blend(srcG, dstG, srcA, maskG),
                              src_alpha_blend(srcB, dstB, srcA, maskB));
    }
}

static void LCD16_RowProc_Opaque(SkPMColor* SK_RESTRICT dst,
                                 const uint16_t* SK_RESTRICT mask,
                                 const SkPMColor* SK_RESTRICT src, int count) {
    for (int i = 0; i < count; ++i) {
        uint16_t m = mask[i];
        if (0 == m) {
            continue;
        }

        SkPMColor s = src[i];
        SkPMColor d = dst[i];

        int srcR = SkGetPackedR32(s);
        int srcG = SkGetPackedG32(s);
        int srcB = SkGetPackedB32(s);

        /*  We want all of these in 5bits, hence the shifts in case one of them
         *  (green) is 6bits.
         */
        int maskR = SkGetPackedR16(m) >> (SK_R16_BITS - 5);
        int maskG = SkGetPackedG16(m) >> (SK_G16_BITS - 5);
        int maskB = SkGetPackedB16(m) >> (SK_B16_BITS - 5);

        // Now upscale them to 0..32, so we can use blend32
        maskR = SkUpscale31To32(maskR);
        maskG = SkUpscale31To32(maskG);
        maskB = SkUpscale31To32(maskB);

        int dstR = SkGetPackedR32(d);
        int dstG = SkGetPackedG32(d);
        int dstB = SkGetPackedB32(d);

        // LCD blitting is only supported if the dst is known/required
        // to be opaque
        dst[i] = SkPackARGB32(0xFF,
                              SkBlend32(srcR, dstR, maskR),
                              SkBlend32(srcG, dstG, maskG),
                              SkBlend32(srcB, dstB, maskB));
    }
}

static void LCD32_RowProc_Blend(SkPMColor* SK_RESTRICT dst,
                                const SkPMColor* SK_RESTRICT mask,
                                const SkPMColor* SK_RESTRICT src, int count) {
    for (int i = 0; i < count; ++i) {
        SkPMColor m = mask[i];
        if (0 == m) {
            continue;
        }

        SkPMColor s = src[i];
        int srcA = SkGetPackedA32(s);
        int srcR = SkGetPackedR32(s);
        int srcG = SkGetPackedG32(s);
        int srcB = SkGetPackedB32(s);

        srcA = SkAlpha255To256(srcA);

        SkPMColor d = dst[i];

        int maskR = SkGetPackedR32(m);
        int maskG = SkGetPackedG32(m);
        int maskB = SkGetPackedB32(m);

        // Now upscale them to 0..256
        maskR = SkAlpha255To256(maskR);
        maskG = SkAlpha255To256(maskG);
        maskB = SkAlpha255To256(maskB);

        int dstR = SkGetPackedR32(d);
        int dstG = SkGetPackedG32(d);
        int dstB = SkGetPackedB32(d);

        // LCD blitting is only supported if the dst is known/required
        // to be opaque
        dst[i] = SkPackARGB32(0xFF,
                              src_alpha_blend(srcR, dstR, srcA, maskR),
                              src_alpha_blend(srcG, dstG, srcA, maskG),
                              src_alpha_blend(srcB, dstB, srcA, maskB));
    }
}

static void LCD32_RowProc_Opaque(SkPMColor* SK_RESTRICT dst,
                                 const SkPMColor* SK_RESTRICT mask,
                                 const SkPMColor* SK_RESTRICT src, int count) {
    for (int i = 0; i < count; ++i) {
        SkPMColor m = mask[i];
        if (0 == m) {
            continue;
        }

        SkPMColor s = src[i];
        SkPMColor d = dst[i];

        int maskR = SkGetPackedR32(m);
        int maskG = SkGetPackedG32(m);
        int maskB = SkGetPackedB32(m);

        int srcR = SkGetPackedR32(s);
        int srcG = SkGetPackedG32(s);
        int srcB = SkGetPackedB32(s);

        int dstR = SkGetPackedR32(d);
        int dstG = SkGetPackedG32(d);
        int dstB = SkGetPackedB32(d);

        // Now upscale them to 0..256, so we can use SkAlphaBlend
        maskR = SkAlpha255To256(maskR);
        maskG = SkAlpha255To256(maskG);
        maskB = SkAlpha255To256(maskB);

        // LCD blitting is only supported if the dst is known/required
        // to be opaque
        dst[i] = SkPackARGB32(0xFF,
                              SkAlphaBlend(srcR, dstR, maskR),
                              SkAlphaBlend(srcG, dstG, maskG),
                              SkAlphaBlend(srcB, dstB, maskB));
    }
}

SkBlitMask::RowProc SkBlitMask::RowFactory(SkBitmap::Config config,
                                           SkMask::Format format,
                                           RowFlags flags) {
// make this opt-in until chrome can rebaseline
    RowProc proc = PlatformRowProcs(config, format, flags);
    if (proc) {
        return proc;
    }

    static const RowProc gProcs[] = {
        // need X coordinate to handle BW
        false ? (RowProc)BW_RowProc_Blend : NULL, // suppress unused warning
        false ? (RowProc)BW_RowProc_Opaque : NULL, // suppress unused warning
        (RowProc)A8_RowProc_Blend,      (RowProc)A8_RowProc_Opaque,
        (RowProc)LCD16_RowProc_Blend,   (RowProc)LCD16_RowProc_Opaque,
        (RowProc)LCD32_RowProc_Blend,   (RowProc)LCD32_RowProc_Opaque,
    };

    int index;
    switch (config) {
        case SkBitmap::kARGB_8888_Config:
            switch (format) {
                case SkMask::kBW_Format:    index = 0; break;
                case SkMask::kA8_Format:    index = 2; break;
                case SkMask::kLCD16_Format: index = 4; break;
                case SkMask::kLCD32_Format: index = 6; break;
                default:
                    return NULL;
            }
            if (flags & kSrcIsOpaque_RowFlag) {
                index |= 1;
            }
            SkASSERT((size_t)index < SK_ARRAY_COUNT(gProcs));
            return gProcs[index];
        default:
            break;
    }
    return NULL;
}
