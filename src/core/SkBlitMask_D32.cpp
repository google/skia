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

///////////////////////////////////////////////////////////////////////////////

static inline int upscale31To32(int value) {
    SkASSERT((unsigned)value <= 31);
    return value + (value >> 4);
}

static inline int blend32(int src, int dst, int scale) {
    SkASSERT((unsigned)src <= 0xFF);
    SkASSERT((unsigned)dst <= 0xFF);
    SkASSERT((unsigned)scale <= 32);
    return dst + ((src - dst) * scale >> 5);
}

static void blit_lcd16_row(SkPMColor dst[], const uint16_t src[],
                           SkColor color, int width, SkPMColor) {
    int srcA = SkColorGetA(color);
    int srcR = SkColorGetR(color);
    int srcG = SkColorGetG(color);
    int srcB = SkColorGetB(color);
    
    srcA = SkAlpha255To256(srcA);
    
    for (int i = 0; i < width; i++) {
        uint16_t mask = src[i];
        if (0 == mask) {
            continue;
        }
        
        SkPMColor d = dst[i];
        
        /*  We want all of these in 5bits, hence the shifts in case one of them
         *  (green) is 6bits.
         */
        int maskR = SkGetPackedR16(mask) >> (SK_R16_BITS - 5);
        int maskG = SkGetPackedG16(mask) >> (SK_G16_BITS - 5);
        int maskB = SkGetPackedB16(mask) >> (SK_B16_BITS - 5);
        
        // Now upscale them to 0..32, so we can use blend32
        maskR = upscale31To32(maskR);
        maskG = upscale31To32(maskG);
        maskB = upscale31To32(maskB);
        
        maskR = maskR * srcA >> 8;
        maskG = maskG * srcA >> 8;
        maskB = maskB * srcA >> 8;
        
        int dstR = SkGetPackedR32(d);
        int dstG = SkGetPackedG32(d);
        int dstB = SkGetPackedB32(d);
        
        // LCD blitting is only supported if the dst is known/required
        // to be opaque
        dst[i] = SkPackARGB32(0xFF,
                              blend32(srcR, dstR, maskR),
                              blend32(srcG, dstG, maskG),
                              blend32(srcB, dstB, maskB));
    }
}

static void blit_lcd16_opaque_row(SkPMColor dst[], const uint16_t src[],
                                  SkColor color, int width, SkPMColor opaqueDst) {
    int srcR = SkColorGetR(color);
    int srcG = SkColorGetG(color);
    int srcB = SkColorGetB(color);
    
    for (int i = 0; i < width; i++) {
        uint16_t mask = src[i];
        if (0 == mask) {
            continue;
        }
        if (0xFFFF == mask) {
            dst[i] = opaqueDst;
            continue;
        }
        
        SkPMColor d = dst[i];
        
        /*  We want all of these in 5bits, hence the shifts in case one of them
         *  (green) is 6bits.
         */
        int maskR = SkGetPackedR16(mask) >> (SK_R16_BITS - 5);
        int maskG = SkGetPackedG16(mask) >> (SK_G16_BITS - 5);
        int maskB = SkGetPackedB16(mask) >> (SK_B16_BITS - 5);
        
        // Now upscale them to 0..32, so we can use blend32
        maskR = upscale31To32(maskR);
        maskG = upscale31To32(maskG);
        maskB = upscale31To32(maskB);
        
        int dstR = SkGetPackedR32(d);
        int dstG = SkGetPackedG32(d);
        int dstB = SkGetPackedB32(d);
        
        // LCD blitting is only supported if the dst is known/required
        // to be opaque
        dst[i] = SkPackARGB32(0xFF,
                              blend32(srcR, dstR, maskR),
                              blend32(srcG, dstG, maskG),
                              blend32(srcB, dstB, maskB));
    }
}

static void D32_LCD16_Proc(void* SK_RESTRICT dst, size_t dstRB,
                           const void* SK_RESTRICT mask, size_t maskRB,
                           SkColor color, int width, int height) {
    
    SkPMColor*		dstRow = (SkPMColor*)dst;
    const uint16_t* srcRow = (const uint16_t*)mask;
    SkPMColor       opaqueDst;
    
    void (*proc)(SkPMColor dst[], const uint16_t src[],
                 SkColor color, int width, SkPMColor);
    if (0xFF == SkColorGetA(color)) {
        proc = blit_lcd16_opaque_row;
        opaqueDst = SkPreMultiplyColor(color);
    } else {
        proc = blit_lcd16_row;
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

static SkBlitMask::Proc D32_A8_Factory(SkColor color) {
    if (SK_ColorBLACK == color) {
        return D32_A8_Black;
    } else if (0xFF == SkColorGetA(color)) {
        return D32_A8_Opaque;
    } else {
        return D32_A8_Color;
    }
}

static SkBlitMask::Proc D32_LCD32_Factory(SkColor color) {
    return (0xFF == SkColorGetA(color)) ? D32_LCD32_Opaque : D32_LCD32_Blend;
}

SkBlitMask::Proc SkBlitMask::Factory(SkBitmap::Config config,
                                     SkMask::Format format, SkColor color) {
    SkBlitMask::Proc proc = PlatformProcs(config, format, color);
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
    Proc proc = Factory(device.config(), mask.fFormat, color);
    if (proc) {
        int x = clip.fLeft;
        int y = clip.fTop;
        proc(device.getAddr32(x, y), device.rowBytes(), mask.getAddr(x, y),
             mask.fRowBytes, color, clip.width(), clip.height());
        return true;
    }
    return false;
}


