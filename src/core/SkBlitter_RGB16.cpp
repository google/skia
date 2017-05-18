/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkArenaAlloc.h"
#include "SkBlitRow.h"
#include "SkCoreBlitters.h"
#include "SkColorPriv.h"
#include "SkDither.h"
#include "SkShader.h"
#include "SkUtils.h"
#include "SkUtilsArm.h"
#include "SkXfermodePriv.h"

#if defined(SK_ARM_HAS_NEON) && defined(SK_CPU_LENDIAN)
    #include <arm_neon.h>
extern void SkRGB16BlitterBlitV_neon(uint16_t* device,
                                     int height,
                                     size_t deviceRB,
                                     unsigned scale,
                                     uint32_t src32);
#else
    // if we don't have neon, then our black blitter is worth the extra code
    #define USE_BLACK_BLITTER
#endif

void sk_dither_memset16(uint16_t dst[], uint16_t value, uint16_t other,
                        int count) {
    if (count > 0) {
        // see if we need to write one short before we can cast to an 4byte ptr
        // (we do this subtract rather than (unsigned)dst so we don't get warnings
        //  on 64bit machines)
        if (((char*)dst - (char*)0) & 2) {
            *dst++ = value;
            count -= 1;
            SkTSwap(value, other);
        }

        // fast way to set [value,other] pairs
#ifdef SK_CPU_BENDIAN
        sk_memset32((uint32_t*)dst, (value << 16) | other, count >> 1);
#else
        sk_memset32((uint32_t*)dst, (other << 16) | value, count >> 1);
#endif

        if (count & 1) {
            dst[count - 1] = value;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

class SkRGB16_Blitter : public SkRasterBlitter {
public:
    SkRGB16_Blitter(const SkPixmap& device, const SkPaint& paint);
    void blitH(int x, int y, int width) override;
    virtual void blitAntiH(int x, int y, const SkAlpha* antialias,
                           const int16_t* runs) override;
    void blitV(int x, int y, int height, SkAlpha alpha) override;
    void blitRect(int x, int y, int width, int height) override;
    void blitMask(const SkMask&, const SkIRect&) override;
    const SkPixmap* justAnOpaqueColor(uint32_t*) override;

protected:
    SkPMColor   fSrcColor32;
    uint32_t    fExpandedRaw16;
    unsigned    fScale;
    uint16_t    fColor16;       // already scaled by fScale
    uint16_t    fRawColor16;    // unscaled
    uint16_t    fRawDither16;   // unscaled
    SkBool8     fDoDither;

    SkBlitRow::ColorProc16 fColorProc16;

    // illegal
    SkRGB16_Blitter& operator=(const SkRGB16_Blitter&);

    typedef SkRasterBlitter INHERITED;
};

class SkRGB16_Opaque_Blitter : public SkRGB16_Blitter {
public:
    SkRGB16_Opaque_Blitter(const SkPixmap& device, const SkPaint& paint);
    void blitH(int x, int y, int width) override;
    void blitAntiH(int x, int y, const SkAlpha* antialias, const int16_t* runs) override;
    void blitV(int x, int y, int height, SkAlpha alpha) override;
    void blitRect(int x, int y, int width, int height) override;
    void blitMask(const SkMask&, const SkIRect&) override;

private:
    typedef SkRGB16_Blitter INHERITED;
};

#ifdef USE_BLACK_BLITTER
class SkRGB16_Black_Blitter : public SkRGB16_Opaque_Blitter {
public:
    SkRGB16_Black_Blitter(const SkPixmap& device, const SkPaint& paint);
    void blitMask(const SkMask&, const SkIRect&) override;
    void blitAntiH(int x, int y, const SkAlpha* antialias, const int16_t* runs) override;

private:
    typedef SkRGB16_Opaque_Blitter INHERITED;
};
#endif

class SkRGB16_Shader_Blitter : public SkShaderBlitter {
public:
    SkRGB16_Shader_Blitter(const SkPixmap& device, const SkPaint& paint,
                           SkShader::Context* shaderContext);
    ~SkRGB16_Shader_Blitter() override;
    void blitH(int x, int y, int width) override;
    virtual void blitAntiH(int x, int y, const SkAlpha* antialias,
                           const int16_t* runs) override;
    void blitRect(int x, int y, int width, int height) override;

protected:
    SkPMColor*          fBuffer;
    SkBlitRow::Proc16   fOpaqueProc;
    SkBlitRow::Proc16   fAlphaProc;

private:
    // illegal
    SkRGB16_Shader_Blitter& operator=(const SkRGB16_Shader_Blitter&);

    typedef SkShaderBlitter INHERITED;
};

class SkRGB16_Shader_Xfermode_Blitter : public SkShaderBlitter {
public:
    SkRGB16_Shader_Xfermode_Blitter(const SkPixmap& device, const SkPaint& paint,
                                    SkShader::Context* shaderContext);
    ~SkRGB16_Shader_Xfermode_Blitter() override;
    void blitH(int x, int y, int width) override;
    virtual void blitAntiH(int x, int y, const SkAlpha* antialias,
                           const int16_t* runs) override;

private:
    SkXfermode* fXfermode;
    SkPMColor*  fBuffer;
    uint8_t*    fAAExpand;

    // illegal
    SkRGB16_Shader_Xfermode_Blitter& operator=(const SkRGB16_Shader_Xfermode_Blitter&);

    typedef SkShaderBlitter INHERITED;
};

///////////////////////////////////////////////////////////////////////////////
#ifdef USE_BLACK_BLITTER
SkRGB16_Black_Blitter::SkRGB16_Black_Blitter(const SkPixmap& device, const SkPaint& paint)
    : INHERITED(device, paint) {
    SkASSERT(paint.getShader() == nullptr);
    SkASSERT(paint.getColorFilter() == nullptr);
    SkASSERT(paint.isSrcOver());
    SkASSERT(paint.getColor() == SK_ColorBLACK);
}

#if 1
#define black_8_pixels(mask, dst)       \
    do {                                \
        if (mask & 0x80) dst[0] = 0;    \
        if (mask & 0x40) dst[1] = 0;    \
        if (mask & 0x20) dst[2] = 0;    \
        if (mask & 0x10) dst[3] = 0;    \
        if (mask & 0x08) dst[4] = 0;    \
        if (mask & 0x04) dst[5] = 0;    \
        if (mask & 0x02) dst[6] = 0;    \
        if (mask & 0x01) dst[7] = 0;    \
    } while (0)
#else
static inline black_8_pixels(U8CPU mask, uint16_t dst[])
{
    if (mask & 0x80) dst[0] = 0;
    if (mask & 0x40) dst[1] = 0;
    if (mask & 0x20) dst[2] = 0;
    if (mask & 0x10) dst[3] = 0;
    if (mask & 0x08) dst[4] = 0;
    if (mask & 0x04) dst[5] = 0;
    if (mask & 0x02) dst[6] = 0;
    if (mask & 0x01) dst[7] = 0;
}
#endif

#define SK_BLITBWMASK_NAME                  SkRGB16_Black_BlitBW
#define SK_BLITBWMASK_ARGS
#define SK_BLITBWMASK_BLIT8(mask, dst)      black_8_pixels(mask, dst)
#define SK_BLITBWMASK_GETADDR               writable_addr16
#define SK_BLITBWMASK_DEVTYPE               uint16_t
#include "SkBlitBWMaskTemplate.h"

void SkRGB16_Black_Blitter::blitMask(const SkMask& mask,
                                     const SkIRect& clip) {
    if (mask.fFormat == SkMask::kBW_Format) {
        SkRGB16_Black_BlitBW(fDevice, mask, clip);
    } else {
        uint16_t* SK_RESTRICT device = fDevice.writable_addr16(clip.fLeft, clip.fTop);
        const uint8_t* SK_RESTRICT alpha = mask.getAddr8(clip.fLeft, clip.fTop);
        unsigned width = clip.width();
        unsigned height = clip.height();
        size_t deviceRB = fDevice.rowBytes() - (width << 1);
        unsigned maskRB = mask.fRowBytes - width;

        SkASSERT((int)height > 0);
        SkASSERT((int)width > 0);
        SkASSERT((int)deviceRB >= 0);
        SkASSERT((int)maskRB >= 0);

        do {
            unsigned w = width;
            do {
                unsigned aa = *alpha++;
                *device = SkAlphaMulRGB16(*device, SkAlpha255To256(255 - aa));
                device += 1;
            } while (--w != 0);
            device = (uint16_t*)((char*)device + deviceRB);
            alpha += maskRB;
        } while (--height != 0);
    }
}

void SkRGB16_Black_Blitter::blitAntiH(int x, int y,
                                      const SkAlpha* SK_RESTRICT antialias,
                                      const int16_t* SK_RESTRICT runs) {
    uint16_t* SK_RESTRICT device = fDevice.writable_addr16(x, y);

    for (;;) {
        int count = runs[0];
        SkASSERT(count >= 0);
        if (count <= 0) {
            return;
        }
        runs += count;

        unsigned aa = antialias[0];
        antialias += count;
        if (aa) {
            if (aa == 255) {
                memset(device, 0, count << 1);
            } else {
                aa = SkAlpha255To256(255 - aa);
                do {
                    *device = SkAlphaMulRGB16(*device, aa);
                    device += 1;
                } while (--count != 0);
                continue;
            }
        }
        device += count;
    }
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

SkRGB16_Opaque_Blitter::SkRGB16_Opaque_Blitter(const SkPixmap& device, const SkPaint& paint)
    : INHERITED(device, paint) {}

void SkRGB16_Opaque_Blitter::blitH(int x, int y, int width) {
    SkASSERT(width > 0);
    SkASSERT(x + width <= fDevice.width());
    uint16_t* SK_RESTRICT device = fDevice.writable_addr16(x, y);
    uint16_t srcColor = fColor16;

    SkASSERT(fRawColor16 == srcColor);
    if (fDoDither) {
        uint16_t ditherColor = fRawDither16;
        if ((x ^ y) & 1) {
            SkTSwap(ditherColor, srcColor);
        }
        sk_dither_memset16(device, srcColor, ditherColor, width);
    } else {
        sk_memset16(device, srcColor, width);
    }
}

// return 1 or 0 from a bool
static inline int Bool2Int(int value) {
    return !!value;
}

void SkRGB16_Opaque_Blitter::blitAntiH(int x, int y,
                                       const SkAlpha* SK_RESTRICT antialias,
                                       const int16_t* SK_RESTRICT runs) {
    uint16_t* SK_RESTRICT device = fDevice.writable_addr16(x, y);
    uint16_t    srcColor = fRawColor16;
    uint32_t    srcExpanded = fExpandedRaw16;
    int         ditherInt = Bool2Int(fDoDither);
    uint16_t    ditherColor = fRawDither16;
    // if we have no dithering, this will always fail
    if ((x ^ y) & ditherInt) {
        SkTSwap(ditherColor, srcColor);
    }
    for (;;) {
        int count = runs[0];
        SkASSERT(count >= 0);
        if (count <= 0) {
            return;
        }
        runs += count;

        unsigned aa = antialias[0];
        antialias += count;
        if (aa) {
            if (aa == 255) {
                if (ditherInt) {
                    sk_dither_memset16(device, srcColor,
                                       ditherColor, count);
                } else {
                    sk_memset16(device, srcColor, count);
                }
            } else {
                // TODO: respect fDoDither
                unsigned scale5 = SkAlpha255To256(aa) >> 3;
                uint32_t src32 = srcExpanded * scale5;
                scale5 = 32 - scale5; // now we can use it on the device
                int n = count;
                do {
                    uint32_t dst32 = SkExpand_rgb_16(*device) * scale5;
                    *device++ = SkCompact_rgb_16((src32 + dst32) >> 5);
                } while (--n != 0);
                goto DONE;
            }
        }
        device += count;

    DONE:
        // if we have no dithering, this will always fail
        if (count & ditherInt) {
            SkTSwap(ditherColor, srcColor);
        }
    }
}

#define solid_8_pixels(mask, dst, color)    \
    do {                                    \
        if (mask & 0x80) dst[0] = color;    \
        if (mask & 0x40) dst[1] = color;    \
        if (mask & 0x20) dst[2] = color;    \
        if (mask & 0x10) dst[3] = color;    \
        if (mask & 0x08) dst[4] = color;    \
        if (mask & 0x04) dst[5] = color;    \
        if (mask & 0x02) dst[6] = color;    \
        if (mask & 0x01) dst[7] = color;    \
    } while (0)

#define SK_BLITBWMASK_NAME                  SkRGB16_BlitBW
#define SK_BLITBWMASK_ARGS                  , uint16_t color
#define SK_BLITBWMASK_BLIT8(mask, dst)      solid_8_pixels(mask, dst, color)
#define SK_BLITBWMASK_GETADDR               writable_addr16
#define SK_BLITBWMASK_DEVTYPE               uint16_t
#include "SkBlitBWMaskTemplate.h"

static U16CPU blend_compact(uint32_t src32, uint32_t dst32, unsigned scale5) {
    return SkCompact_rgb_16(dst32 + ((src32 - dst32) * scale5 >> 5));
}

void SkRGB16_Opaque_Blitter::blitMask(const SkMask& mask,
                                      const SkIRect& clip) {
    if (mask.fFormat == SkMask::kBW_Format) {
        SkRGB16_BlitBW(fDevice, mask, clip, fColor16);
        return;
    }

    uint16_t* SK_RESTRICT device = fDevice.writable_addr16(clip.fLeft, clip.fTop);
    const uint8_t* SK_RESTRICT alpha = mask.getAddr8(clip.fLeft, clip.fTop);
    int width = clip.width();
    int height = clip.height();
    size_t      deviceRB = fDevice.rowBytes() - (width << 1);
    unsigned    maskRB = mask.fRowBytes - width;
    uint32_t    expanded32 = fExpandedRaw16;

#if defined(SK_ARM_HAS_NEON) && defined(SK_CPU_LENDIAN)
#define    UNROLL    8
    do {
        int w = width;
        if (w >= UNROLL) {
            uint32x4_t color, dev_lo, dev_hi;
            uint32x4_t wn1, wn2, tmp;
            uint32x4_t vmask_g16, vmask_ng16;
            uint16x8_t valpha, vdev;
            uint16x4_t odev_lo, odev_hi, valpha_lo, valpha_hi;

            // prepare constants
            vmask_g16 = vdupq_n_u32(SK_G16_MASK_IN_PLACE);
            vmask_ng16 = vdupq_n_u32(~SK_G16_MASK_IN_PLACE);
            color = vdupq_n_u32(expanded32);

            do {
                // alpha is 8x8, widen and split to get a pair of 16x4
                valpha = vaddw_u8(vdupq_n_u16(1), vld1_u8(alpha));
                valpha = vshrq_n_u16(valpha, 3);
                valpha_lo = vget_low_u16(valpha);
                valpha_hi = vget_high_u16(valpha);

                // load pixels
                vdev = vld1q_u16(device);
                dev_lo = vmovl_u16(vget_low_u16(vdev));
                dev_hi = vmovl_u16(vget_high_u16(vdev));

                // unpack them in 32 bits
                dev_lo = (dev_lo & vmask_ng16) | vshlq_n_u32(dev_lo & vmask_g16, 16);
                dev_hi = (dev_hi & vmask_ng16) | vshlq_n_u32(dev_hi & vmask_g16, 16);

                // blend with color
                tmp = (color - dev_lo) * vmovl_u16(valpha_lo);
                tmp = vshrq_n_u32(tmp, 5);
                dev_lo += tmp;

                tmp = vmulq_u32(color - dev_hi, vmovl_u16(valpha_hi));
                tmp = vshrq_n_u32(tmp, 5);
                dev_hi += tmp;

                // re-compact
                wn1 = dev_lo & vmask_ng16;
                wn2 = vshrq_n_u32(dev_lo, 16) & vmask_g16;
                odev_lo = vmovn_u32(wn1 | wn2);

                wn1 = dev_hi & vmask_ng16;
                wn2 = vshrq_n_u32(dev_hi, 16) & vmask_g16;
                odev_hi = vmovn_u32(wn1 | wn2);

                // store
                vst1q_u16(device, vcombine_u16(odev_lo, odev_hi));

                device += UNROLL;
                alpha += UNROLL;
                w -= UNROLL;
            } while (w >= UNROLL);
        }

        // residuals
        while (w > 0) {
            *device = blend_compact(expanded32, SkExpand_rgb_16(*device),
                                    SkAlpha255To256(*alpha++) >> 3);
            device += 1;
            --w;
        }
        device = (uint16_t*)((char*)device + deviceRB);
        alpha += maskRB;
    } while (--height != 0);
#undef    UNROLL
#else   // non-neon code
    do {
        int w = width;
        do {
            *device = blend_compact(expanded32, SkExpand_rgb_16(*device),
                                    SkAlpha255To256(*alpha++) >> 3);
            device += 1;
        } while (--w != 0);
        device = (uint16_t*)((char*)device + deviceRB);
        alpha += maskRB;
    } while (--height != 0);
#endif
}

void SkRGB16_Opaque_Blitter::blitV(int x, int y, int height, SkAlpha alpha) {
    uint16_t* SK_RESTRICT device = fDevice.writable_addr16(x, y);
    size_t    deviceRB = fDevice.rowBytes();

    // TODO: respect fDoDither
    unsigned scale5 = SkAlpha255To256(alpha) >> 3;
    uint32_t src32 =  fExpandedRaw16 * scale5;
    scale5 = 32 - scale5;
#if defined(SK_ARM_HAS_NEON) && defined(SK_CPU_LENDIAN)
    SkRGB16BlitterBlitV_neon(device, height, deviceRB, scale5, src32);
#else
    do {
        uint32_t dst32 = SkExpand_rgb_16(*device) * scale5;
        *device = SkCompact_rgb_16((src32 + dst32) >> 5);
        device = (uint16_t*)((char*)device + deviceRB);
    } while (--height != 0);
#endif
}

void SkRGB16_Opaque_Blitter::blitRect(int x, int y, int width, int height) {
    SkASSERT(x + width <= fDevice.width() && y + height <= fDevice.height());
    uint16_t* SK_RESTRICT device = fDevice.writable_addr16(x, y);
    size_t      deviceRB = fDevice.rowBytes();
    uint16_t    color16 = fColor16;

    if (fDoDither) {
        uint16_t ditherColor = fRawDither16;
        if ((x ^ y) & 1) {
            SkTSwap(ditherColor, color16);
        }
        while (--height >= 0) {
            sk_dither_memset16(device, color16, ditherColor, width);
            SkTSwap(ditherColor, color16);
            device = (uint16_t*)((char*)device + deviceRB);
        }
    } else {  // no dither
        while (--height >= 0) {
            sk_memset16(device, color16, width);
            device = (uint16_t*)((char*)device + deviceRB);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

SkRGB16_Blitter::SkRGB16_Blitter(const SkPixmap& device, const SkPaint& paint)
    : INHERITED(device) {
    SkColor color = paint.getColor();

    fSrcColor32 = SkPreMultiplyColor(color);
    fScale = SkAlpha255To256(SkColorGetA(color));

    int r = SkColorGetR(color);
    int g = SkColorGetG(color);
    int b = SkColorGetB(color);

    fRawColor16 = fRawDither16 = SkPack888ToRGB16(r, g, b);
    // if we're dithered, use fRawDither16 to hold that.
    if ((fDoDither = paint.isDither()) != false) {
        fRawDither16 = SkDitherPack888ToRGB16(r, g, b);
    }

    fExpandedRaw16 = SkExpand_rgb_16(fRawColor16);

    fColor16 = SkPackRGB16( SkAlphaMul(r, fScale) >> (8 - SK_R16_BITS),
                            SkAlphaMul(g, fScale) >> (8 - SK_G16_BITS),
                            SkAlphaMul(b, fScale) >> (8 - SK_B16_BITS));

    // compute SkBlitRow::Procs
    unsigned flags = 0;

    if (SkGetPackedA32(fSrcColor32) < 0xFF) {
        flags |= SkBlitRow::kSrcPixelAlpha_Flag;
    }

    if (fDoDither) {
        flags |= SkBlitRow::kDither_Flag;
    }

    fColorProc16 = SkBlitRow::ColorFactory16(flags);
}

const SkPixmap* SkRGB16_Blitter::justAnOpaqueColor(uint32_t* value) {
    if (!fDoDither && 256 == fScale) {
        *value = fRawColor16;
        return &fDevice;
    }
    return nullptr;
}

void SkRGB16_Blitter::blitH(int x, int y, int width) {
    SkASSERT(width > 0);
    SkASSERT(x + width <= fDevice.width());
    uint16_t* SK_RESTRICT device = fDevice.writable_addr16(x, y);

    fColorProc16(device, fSrcColor32, width, x, y);
}

void SkRGB16_Blitter::blitAntiH(int x, int y,
                                const SkAlpha* SK_RESTRICT antialias,
                                const int16_t* SK_RESTRICT runs) {
    uint16_t* SK_RESTRICT device = fDevice.writable_addr16(x, y);
    uint32_t    srcExpanded = fExpandedRaw16;
    unsigned    scale = fScale;

    // TODO: respect fDoDither
    for (;;) {
        int count = runs[0];
        SkASSERT(count >= 0);
        if (count <= 0) {
            return;
        }
        runs += count;

        unsigned aa = antialias[0];
        antialias += count;
        if (aa) {
            unsigned scale5 = SkAlpha255To256(aa) * scale >> (8 + 3);
            uint32_t src32 =  srcExpanded * scale5;
            scale5 = 32 - scale5;
            do {
                uint32_t dst32 = SkExpand_rgb_16(*device) * scale5;
                *device++ = SkCompact_rgb_16((src32 + dst32) >> 5);
            } while (--count != 0);
            continue;
        }
        device += count;
    }
}

static inline void blend_8_pixels(U8CPU bw, uint16_t dst[], unsigned dst_scale,
                                  U16CPU srcColor) {
    if (bw & 0x80) dst[0] = srcColor + SkAlphaMulRGB16(dst[0], dst_scale);
    if (bw & 0x40) dst[1] = srcColor + SkAlphaMulRGB16(dst[1], dst_scale);
    if (bw & 0x20) dst[2] = srcColor + SkAlphaMulRGB16(dst[2], dst_scale);
    if (bw & 0x10) dst[3] = srcColor + SkAlphaMulRGB16(dst[3], dst_scale);
    if (bw & 0x08) dst[4] = srcColor + SkAlphaMulRGB16(dst[4], dst_scale);
    if (bw & 0x04) dst[5] = srcColor + SkAlphaMulRGB16(dst[5], dst_scale);
    if (bw & 0x02) dst[6] = srcColor + SkAlphaMulRGB16(dst[6], dst_scale);
    if (bw & 0x01) dst[7] = srcColor + SkAlphaMulRGB16(dst[7], dst_scale);
}

#define SK_BLITBWMASK_NAME                  SkRGB16_BlendBW
#define SK_BLITBWMASK_ARGS                  , unsigned dst_scale, U16CPU src_color
#define SK_BLITBWMASK_BLIT8(mask, dst)      blend_8_pixels(mask, dst, dst_scale, src_color)
#define SK_BLITBWMASK_GETADDR               writable_addr16
#define SK_BLITBWMASK_DEVTYPE               uint16_t
#include "SkBlitBWMaskTemplate.h"

void SkRGB16_Blitter::blitMask(const SkMask& mask,
                               const SkIRect& clip) {
    if (mask.fFormat == SkMask::kBW_Format) {
        SkRGB16_BlendBW(fDevice, mask, clip, 256 - fScale, fColor16);
        return;
    }

    uint16_t* SK_RESTRICT device = fDevice.writable_addr16(clip.fLeft, clip.fTop);
    const uint8_t* SK_RESTRICT alpha = mask.getAddr8(clip.fLeft, clip.fTop);
    int width = clip.width();
    int height = clip.height();
    size_t      deviceRB = fDevice.rowBytes() - (width << 1);
    unsigned    maskRB = mask.fRowBytes - width;
    uint32_t    color32 = fExpandedRaw16;

    unsigned scale256 = fScale;
    do {
        int w = width;
        do {
            unsigned aa = *alpha++;
            unsigned scale = SkAlpha255To256(aa) * scale256 >> (8 + 3);
            uint32_t src32 = color32 * scale;
            uint32_t dst32 = SkExpand_rgb_16(*device) * (32 - scale);
            *device++ = SkCompact_rgb_16((src32 + dst32) >> 5);
        } while (--w != 0);
        device = (uint16_t*)((char*)device + deviceRB);
        alpha += maskRB;
    } while (--height != 0);
}

void SkRGB16_Blitter::blitV(int x, int y, int height, SkAlpha alpha) {
    uint16_t* SK_RESTRICT device = fDevice.writable_addr16(x, y);
    size_t    deviceRB = fDevice.rowBytes();

    // TODO: respect fDoDither
    unsigned scale5 = SkAlpha255To256(alpha) * fScale >> (8 + 3);
    uint32_t src32 =  fExpandedRaw16 * scale5;
    scale5 = 32 - scale5;
#if defined(SK_ARM_HAS_NEON) && defined(SK_CPU_LENDIAN)
    SkRGB16BlitterBlitV_neon(device, height, deviceRB, scale5, src32);
#else
    do {
        uint32_t dst32 = SkExpand_rgb_16(*device) * scale5;
        *device = SkCompact_rgb_16((src32 + dst32) >> 5);
        device = (uint16_t*)((char*)device + deviceRB);
    } while (--height != 0);
#endif
}

void SkRGB16_Blitter::blitRect(int x, int y, int width, int height) {
    SkASSERT(x + width <= fDevice.width() && y + height <= fDevice.height());
    uint16_t* SK_RESTRICT device = fDevice.writable_addr16(x, y);
    size_t    deviceRB = fDevice.rowBytes();

    while (--height >= 0) {
        fColorProc16(device, fSrcColor32, width, x, y);
        device = (uint16_t*)((char*)device + deviceRB);
    }
}

///////////////////////////////////////////////////////////////////////////////

SkRGB16_Shader_Blitter::SkRGB16_Shader_Blitter(const SkPixmap& device,
                                               const SkPaint& paint,
                                               SkShader::Context* shaderContext)
    : INHERITED(device, paint, shaderContext)
{
    SkASSERT(paint.isSrcOver());

    fBuffer = (SkPMColor*)sk_malloc_throw(device.width() * sizeof(SkPMColor));

    // compute SkBlitRow::Procs
    unsigned flags = 0;

    uint32_t shaderFlags = fShaderFlags;
    // shaders take care of global alpha, so we never set it in SkBlitRow
    if (!(shaderFlags & SkShader::kOpaqueAlpha_Flag)) {
        flags |= SkBlitRow::kSrcPixelAlpha_Flag;
    }
    if (paint.isDither()) {
        flags |= SkBlitRow::kDither_Flag;
    }
    // used when we know our global alpha is 0xFF
    fOpaqueProc = SkBlitRow::Factory16(flags);
    // used when we know our global alpha is < 0xFF
    fAlphaProc  = SkBlitRow::Factory16(flags | SkBlitRow::kGlobalAlpha_Flag);
}

SkRGB16_Shader_Blitter::~SkRGB16_Shader_Blitter() {
    sk_free(fBuffer);
}

void SkRGB16_Shader_Blitter::blitH(int x, int y, int width) {
    SkASSERT(x + width <= fDevice.width());

    fShaderContext->shadeSpan(x, y, fBuffer, width);
    // shaders take care of global alpha, so we pass 0xFF (should be ignored)
    fOpaqueProc(fDevice.writable_addr16(x, y), fBuffer, width, 0xFF, x, y);
}

void SkRGB16_Shader_Blitter::blitRect(int x, int y, int width, int height) {
    SkShader::Context* shaderContext = fShaderContext;
    SkBlitRow::Proc16  proc = fOpaqueProc;
    SkPMColor*         buffer = fBuffer;
    uint16_t*          dst = fDevice.writable_addr16(x, y);
    size_t             dstRB = fDevice.rowBytes();

    if (fShaderFlags & SkShader::kConstInY32_Flag) {
        shaderContext->shadeSpan(x, y, buffer, width);
        do {
            proc(dst, buffer, width, 0xFF, x, y);
            y += 1;
            dst = (uint16_t*)((char*)dst + dstRB);
        } while (--height);
    } else {
        do {
            shaderContext->shadeSpan(x, y, buffer, width);
            proc(dst, buffer, width, 0xFF, x, y);
            y += 1;
            dst = (uint16_t*)((char*)dst + dstRB);
        } while (--height);
    }
}

static inline int count_nonzero_span(const int16_t runs[], const SkAlpha aa[]) {
    int count = 0;
    for (;;) {
        int n = *runs;
        if (n == 0 || *aa == 0) {
            break;
        }
        runs += n;
        aa += n;
        count += n;
    }
    return count;
}

void SkRGB16_Shader_Blitter::blitAntiH(int x, int y,
                                       const SkAlpha* SK_RESTRICT antialias,
                                       const int16_t* SK_RESTRICT runs) {
    SkShader::Context*     shaderContext = fShaderContext;
    SkPMColor* SK_RESTRICT span = fBuffer;
    uint16_t* SK_RESTRICT  device = fDevice.writable_addr16(x, y);

    for (;;) {
        int count = *runs;
        if (count <= 0) {
            break;
        }
        int aa = *antialias;
        if (0 == aa) {
            device += count;
            runs += count;
            antialias += count;
            x += count;
            continue;
        }

        int nonZeroCount = count + count_nonzero_span(runs + count, antialias + count);

        SkASSERT(nonZeroCount <= fDevice.width()); // don't overrun fBuffer
        shaderContext->shadeSpan(x, y, span, nonZeroCount);

        SkPMColor* localSpan = span;
        for (;;) {
            SkBlitRow::Proc16 proc = (aa == 0xFF) ? fOpaqueProc : fAlphaProc;
            proc(device, localSpan, count, aa, x, y);

            x += count;
            device += count;
            runs += count;
            antialias += count;
            nonZeroCount -= count;
            if (nonZeroCount == 0) {
                break;
            }
            localSpan += count;
            SkASSERT(nonZeroCount > 0);
            count = *runs;
            SkASSERT(count > 0);
            aa = *antialias;
        }
    }
}

///////////////////////////////////////////////////////////////////////

SkRGB16_Shader_Xfermode_Blitter::SkRGB16_Shader_Xfermode_Blitter(
                                const SkPixmap& device, const SkPaint& paint,
                                SkShader::Context* shaderContext)
    : INHERITED(device, paint, shaderContext)
{
    fXfermode = SkXfermode::Peek(paint.getBlendMode());
    SkASSERT(fXfermode);

    int width = device.width();
    fBuffer = (SkPMColor*)sk_malloc_throw((width + (SkAlign4(width) >> 2)) * sizeof(SkPMColor));
    fAAExpand = (uint8_t*)(fBuffer + width);
}

SkRGB16_Shader_Xfermode_Blitter::~SkRGB16_Shader_Xfermode_Blitter() {
    sk_free(fBuffer);
}

void SkRGB16_Shader_Xfermode_Blitter::blitH(int x, int y, int width) {
    SkASSERT(x + width <= fDevice.width());

    uint16_t*   device = fDevice.writable_addr16(x, y);
    SkPMColor*  span = fBuffer;

    fShaderContext->shadeSpan(x, y, span, width);
    fXfermode->xfer16(device, span, width, nullptr);
}

void SkRGB16_Shader_Xfermode_Blitter::blitAntiH(int x, int y,
                                const SkAlpha* SK_RESTRICT antialias,
                                const int16_t* SK_RESTRICT runs) {
    SkShader::Context*     shaderContext = fShaderContext;
    SkXfermode*            mode = fXfermode;
    SkPMColor* SK_RESTRICT span = fBuffer;
    uint8_t* SK_RESTRICT   aaExpand = fAAExpand;
    uint16_t* SK_RESTRICT  device = fDevice.writable_addr16(x, y);

    for (;;) {
        int count = *runs;
        if (count <= 0) {
            break;
        }
        int aa = *antialias;
        if (0 == aa) {
            device += count;
            runs += count;
            antialias += count;
            x += count;
            continue;
        }

        int nonZeroCount = count + count_nonzero_span(runs + count,
                                                      antialias + count);

        SkASSERT(nonZeroCount <= fDevice.width()); // don't overrun fBuffer
        shaderContext->shadeSpan(x, y, span, nonZeroCount);

        x += nonZeroCount;
        SkPMColor* localSpan = span;
        for (;;) {
            if (aa == 0xFF) {
                mode->xfer16(device, localSpan, count, nullptr);
            } else {
                SkASSERT(aa);
                memset(aaExpand, aa, count);
                mode->xfer16(device, localSpan, count, aaExpand);
            }
            device += count;
            runs += count;
            antialias += count;
            nonZeroCount -= count;
            if (nonZeroCount == 0) {
                break;
            }
            localSpan += count;
            SkASSERT(nonZeroCount > 0);
            count = *runs;
            SkASSERT(count > 0);
            aa = *antialias;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

SkBlitter* SkBlitter_ChooseD565(const SkPixmap& device, const SkPaint& paint,
        SkShader::Context* shaderContext,
        SkArenaAlloc* alloc) {
    SkASSERT(alloc != nullptr);

    SkBlitter* blitter;
    SkShader* shader = paint.getShader();
    bool is_srcover = paint.isSrcOver();

    // we require a shader if there is an xfermode, handled by our caller
    SkASSERT(is_srcover || shader);

    if (shader) {
        SkASSERT(shaderContext != nullptr);
        if (!is_srcover) {
            blitter = alloc->make<SkRGB16_Shader_Xfermode_Blitter>(device, paint,
                                                                          shaderContext);
        } else {
            blitter = alloc->make<SkRGB16_Shader_Blitter>(device, paint, shaderContext);
        }
    } else {
        // no shader, no xfermode, (and we always ignore colorfilter)
        SkColor color = paint.getColor();
        if (0 == SkColorGetA(color)) {
            blitter = alloc->make<SkNullBlitter>();
#ifdef USE_BLACK_BLITTER
        } else if (SK_ColorBLACK == color) {
            blitter = alloc->make<SkRGB16_Black_Blitter>(device, paint);
#endif
        } else if (0xFF == SkColorGetA(color)) {
            blitter = alloc->make<SkRGB16_Opaque_Blitter>(device, paint);
        } else {
            blitter = alloc->make<SkRGB16_Blitter>(device, paint);
        }
    }

    return blitter;
}
