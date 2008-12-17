/* libs/graphics/sgl/SkBlitter_RGB16.cpp
**
** Copyright 2006, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/

#include "SkBlitRow.h"
#include "SkCoreBlitters.h"
#include "SkColorPriv.h"
#include "SkDither.h"
#include "SkShader.h"
#include "SkUtils.h"
#include "SkXfermode.h"

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

SkRGB16_Black_Blitter::SkRGB16_Black_Blitter(const SkBitmap& device, const SkPaint& paint)
    : SkRGB16_Blitter(device, paint) {
    SkASSERT(paint.getShader() == NULL);
    SkASSERT(paint.getColorFilter() == NULL);
    SkASSERT(paint.getXfermode() == NULL);
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
#define SK_BLITBWMASK_GETADDR               getAddr16
#define SK_BLITBWMASK_DEVTYPE               uint16_t
#include "SkBlitBWMaskTemplate.h"

void SkRGB16_Black_Blitter::blitMask(const SkMask& SK_RESTRICT mask,
                                     const SkIRect& SK_RESTRICT clip)
                                     SK_RESTRICT {
    if (mask.fFormat == SkMask::kBW_Format) {
        SkRGB16_Black_BlitBW(fDevice, mask, clip);
    } else {
        uint16_t* SK_RESTRICT device = fDevice.getAddr16(clip.fLeft, clip.fTop);
        const uint8_t* SK_RESTRICT alpha = mask.getAddr(clip.fLeft, clip.fTop);
        unsigned width = clip.width();
        unsigned height = clip.height();
        unsigned deviceRB = fDevice.rowBytes() - (width << 1);
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
                                      const int16_t* SK_RESTRICT runs)
                                      SK_RESTRICT {
    uint16_t* SK_RESTRICT device = fDevice.getAddr16(x, y);

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

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

SkRGB16_Blitter::SkRGB16_Blitter(const SkBitmap& device, const SkPaint& paint)
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
    
    fColor16 = SkPackRGB16( SkAlphaMul(r, fScale) >> (8 - SK_R16_BITS),
                            SkAlphaMul(g, fScale) >> (8 - SK_G16_BITS),
                            SkAlphaMul(b, fScale) >> (8 - SK_B16_BITS));
}

const SkBitmap* SkRGB16_Blitter::justAnOpaqueColor(uint32_t* value) {
    if (!fDoDither && 256 == fScale) {
        *value = fRawColor16;
        return &fDevice;
    }
    return NULL;
}

void SkRGB16_Blitter::blitH(int x, int y, int width) SK_RESTRICT {
    SkASSERT(width > 0);
    SkASSERT(x + width <= fDevice.width());

    if (fScale == 0) {
        return;
    }

    uint16_t* SK_RESTRICT device = fDevice.getAddr16(x, y);
    uint16_t srcColor = fColor16;

    if (256 == fScale) {
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
    } else {
        // TODO: respect fDoDither
        SkPMColor src32 = fSrcColor32;
        do {
            *device = SkSrcOver32To16(src32, *device);
            device += 1;
        } while (--width != 0);
    }
}

// return 1 or 0 from a bool
static int Bool2Int(bool value) {
    return !!value;
}

void SkRGB16_Blitter::blitAntiH(int x, int y,
                                const SkAlpha* SK_RESTRICT antialias,
                                const int16_t* SK_RESTRICT runs) SK_RESTRICT {
    if (fScale == 0) {
        return;
    }

    uint16_t* SK_RESTRICT device = fDevice.getAddr16(x, y);
    uint16_t    srcColor = fRawColor16;
    unsigned    scale = fScale;
    int         ditherInt = Bool2Int(fDoDither);

    if (256 == scale) {
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
                    uint32_t src32 = SkExpand_rgb_16(srcColor) * scale5;
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
    } else {
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
                uint32_t src32 =  SkExpand_rgb_16(srcColor) * scale5;
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
}

//////////////////////////////////////////////////////////////////////////////////////

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
#define SK_BLITBWMASK_GETADDR               getAddr16
#define SK_BLITBWMASK_DEVTYPE               uint16_t
#include "SkBlitBWMaskTemplate.h"

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
#define SK_BLITBWMASK_GETADDR               getAddr16
#define SK_BLITBWMASK_DEVTYPE               uint16_t
#include "SkBlitBWMaskTemplate.h"

static U16CPU blend_compact(uint32_t src32, uint32_t dst32, unsigned scale5) {
    return SkCompact_rgb_16(dst32 + ((src32 - dst32) * scale5 >> 5));
}

void SkRGB16_Blitter::blitMask(const SkMask& SK_RESTRICT mask,
                               const SkIRect& SK_RESTRICT clip) SK_RESTRICT {
    if (fScale == 0) {
        return;
    }
    if (mask.fFormat == SkMask::kBW_Format) {
        if (fScale == 256) {
            SkRGB16_BlitBW(fDevice, mask, clip, fColor16);
        } else {
            SkRGB16_BlendBW(fDevice, mask, clip, 256 - fScale, fColor16);
        }
        return;
    }

    uint16_t* SK_RESTRICT device = fDevice.getAddr16(clip.fLeft, clip.fTop);
    const uint8_t* SK_RESTRICT alpha = mask.getAddr(clip.fLeft, clip.fTop);
    int width = clip.width();
    int height = clip.height();
    unsigned    deviceRB = fDevice.rowBytes() - (width << 1);
    unsigned    maskRB = mask.fRowBytes - width;
    uint32_t    color32 = SkExpand_rgb_16(fRawColor16);

    if (256 == fScale) {
        do {
            int w = width;
            do {
                *device = blend_compact(color32, SkExpand_rgb_16(*device),
                                        SkAlpha255To256(*alpha++) >> 3);
                device += 1;
            } while (--w != 0);
            device = (uint16_t*)((char*)device + deviceRB);
            alpha += maskRB;
        } while (--height != 0);
    } else {   // scale < 256
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
}

void SkRGB16_Blitter::blitV(int x, int y, int height, SkAlpha alpha) {
    if (fScale == 0) {
        return;
    }
    uint16_t* SK_RESTRICT device = fDevice.getAddr16(x, y);
    uint16_t    color16 = fRawColor16;
    unsigned    deviceRB = fDevice.rowBytes();

    if (alpha + fScale == (255 + 256)) {
        if (fDoDither) {
            uint16_t ditherColor = fRawDither16;
            if ((x ^ y) & 1) {
                SkTSwap(ditherColor, color16);
            }
            do {
                device[0] = color16;
                device = (uint16_t*)((char*)device + deviceRB);
                SkTSwap(ditherColor, color16);
            } while (--height != 0);
        } else {
            do {
                device[0] = color16;
                device = (uint16_t*)((char*)device + deviceRB);
            } while (--height != 0);
        }
    } else {
        // TODO: respect fDoDither
        unsigned scale5 = SkAlpha255To256(alpha) * fScale >> (8 + 3);
        uint32_t src32 =  SkExpand_rgb_16(color16) * scale5;
        scale5 = 32 - scale5;
        do {
            uint32_t dst32 = SkExpand_rgb_16(*device) * scale5;
            *device = SkCompact_rgb_16((src32 + dst32) >> 5);
            device = (uint16_t*)((char*)device + deviceRB);
        } while (--height != 0);
    }
}

void SkRGB16_Blitter::blitRect(int x, int y, int width, int height) {
    SkASSERT(x + width <= fDevice.width() && y + height <= fDevice.height());

    if (fScale == 0) {
        return;
    }
    uint16_t* SK_RESTRICT device = fDevice.getAddr16(x, y);
    unsigned    deviceRB = fDevice.rowBytes();
    uint16_t    color16 = fColor16;

    if (256 == fScale) {
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
    } else {
        SkPMColor src32 = fSrcColor32;
        while (--height >= 0) {
            for (int i = width - 1; i >= 0; --i) {
                device[i] = SkSrcOver32To16(src32, device[i]);
            }
            device = (uint16_t*)((char*)device + deviceRB);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

SkRGB16_Shader16_Blitter::SkRGB16_Shader16_Blitter(const SkBitmap& device,
                                                   const SkPaint& paint)
    : SkRGB16_Shader_Blitter(device, paint) {
    SkASSERT(SkShader::CanCallShadeSpan16(fShader->getFlags()));
}

void SkRGB16_Shader16_Blitter::blitH(int x, int y, int width) SK_RESTRICT {
    SkASSERT(x + width <= fDevice.width());

    uint16_t* SK_RESTRICT device = fDevice.getAddr16(x, y);
    SkShader*   shader = fShader;

    int alpha = shader->getSpan16Alpha();
    if (0xFF == alpha) {
        shader->shadeSpan16(x, y, device, width);
    } else {
        uint16_t* span16 = (uint16_t*)fBuffer;
        shader->shadeSpan16(x, y, span16, width);
        SkBlendRGB16(span16, device, SkAlpha255To256(alpha), width);
    }
}

void SkRGB16_Shader16_Blitter::blitAntiH(int x, int y,
                                         const SkAlpha* SK_RESTRICT antialias,
                                         const int16_t* SK_RESTRICT runs)
                                         SK_RESTRICT {
    SkShader*   shader = fShader;
    SkPMColor* SK_RESTRICT span = fBuffer;
    uint16_t* SK_RESTRICT device = fDevice.getAddr16(x, y);

    int alpha = shader->getSpan16Alpha();
    uint16_t* span16 = (uint16_t*)span;

    if (0xFF == alpha) {
        for (;;) {
            int count = *runs;
            if (count <= 0) {
                break;
            }
            SkASSERT(count <= fDevice.width()); // don't overrun fBuffer

            int aa = *antialias;
            if (aa == 255) {
                // go direct to the device!
                shader->shadeSpan16(x, y, device, count);
            } else if (aa) {
                shader->shadeSpan16(x, y, span16, count);
                SkBlendRGB16(span16, device, SkAlpha255To256(aa), count);
            }
            device += count;
            runs += count;
            antialias += count;
            x += count;
        }
    } else {  // span alpha is < 255
        alpha = SkAlpha255To256(alpha);
        for (;;) {
            int count = *runs;
            if (count <= 0) {
                break;
            }
            SkASSERT(count <= fDevice.width()); // don't overrun fBuffer

            int aa = SkAlphaMul(*antialias, alpha);
            if (aa) {
                shader->shadeSpan16(x, y, span16, count);
                SkBlendRGB16(span16, device, SkAlpha255To256(aa), count);
            }

            device += count;
            runs += count;
            antialias += count;
            x += count;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

SkRGB16_Shader_Blitter::SkRGB16_Shader_Blitter(const SkBitmap& device,
                                               const SkPaint& paint)
: INHERITED(device, paint) {
    SkASSERT(paint.getXfermode() == NULL);

    fBuffer = (SkPMColor*)sk_malloc_throw(device.width() * sizeof(SkPMColor));

    // compute SkBlitRow::Procs
    unsigned flags = 0;
    
    uint32_t shaderFlags = fShader->getFlags();
    // shaders take care of global alpha, so we never set it in SkBlitRow
    if (!(shaderFlags & SkShader::kOpaqueAlpha_Flag)) {
        flags |= SkBlitRow::kSrcPixelAlpha_Flag;
        }
    // don't dither if the shader is really 16bit
    if (paint.isDither() && !(shaderFlags & SkShader::kIntrinsicly16_Flag)) {
        flags |= SkBlitRow::kDither_Flag;
    }
    // used when we know our global alpha is 0xFF
    fOpaqueProc = SkBlitRow::Factory(flags, SkBitmap::kRGB_565_Config);
    // used when we know our global alpha is < 0xFF
    fAlphaProc  = SkBlitRow::Factory(flags | SkBlitRow::kGlobalAlpha_Flag,
                                     SkBitmap::kRGB_565_Config);
}

SkRGB16_Shader_Blitter::~SkRGB16_Shader_Blitter() {
    sk_free(fBuffer);
}

void SkRGB16_Shader_Blitter::blitH(int x, int y, int width) {
    SkASSERT(x + width <= fDevice.width());

    fShader->shadeSpan(x, y, fBuffer, width);
    // shaders take care of global alpha, so we pass 0xFF (should be ignored)
    fOpaqueProc(fDevice.getAddr16(x, y), fBuffer, width, 0xFF, x, y);
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
                                       const int16_t* SK_RESTRICT runs)
                                       SK_RESTRICT {
    SkShader*   shader = fShader;
    SkPMColor* SK_RESTRICT span = fBuffer;
    uint16_t* SK_RESTRICT device = fDevice.getAddr16(x, y);

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
        shader->shadeSpan(x, y, span, nonZeroCount);

        SkPMColor* localSpan = span;
        for (;;) {
            SkBlitRow::Proc proc = (aa == 0xFF) ? fOpaqueProc : fAlphaProc;
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
                                const SkBitmap& device, const SkPaint& paint)
: INHERITED(device, paint) {
    fXfermode = paint.getXfermode();
    SkASSERT(fXfermode);
    fXfermode->ref();

    int width = device.width();
    fBuffer = (SkPMColor*)sk_malloc_throw((width + (SkAlign4(width) >> 2)) * sizeof(SkPMColor));
    fAAExpand = (uint8_t*)(fBuffer + width);
}

SkRGB16_Shader_Xfermode_Blitter::~SkRGB16_Shader_Xfermode_Blitter() {
    fXfermode->unref();
    sk_free(fBuffer);
}

void SkRGB16_Shader_Xfermode_Blitter::blitH(int x, int y, int width) {
    SkASSERT(x + width <= fDevice.width());

    uint16_t*   device = fDevice.getAddr16(x, y);
    SkPMColor*  span = fBuffer;

    fShader->shadeSpan(x, y, span, width);
    fXfermode->xfer16(device, span, width, NULL);
}

void SkRGB16_Shader_Xfermode_Blitter::blitAntiH(int x, int y,
                                const SkAlpha* SK_RESTRICT antialias,
                                const int16_t* SK_RESTRICT runs) SK_RESTRICT {
    SkShader*   shader = fShader;
    SkXfermode* mode = fXfermode;
    SkPMColor* SK_RESTRICT span = fBuffer;
    uint8_t* SK_RESTRICT aaExpand = fAAExpand;
    uint16_t* SK_RESTRICT device = fDevice.getAddr16(x, y);

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
        shader->shadeSpan(x, y, span, nonZeroCount);

        x += nonZeroCount;
        SkPMColor* localSpan = span;
        for (;;) {
            if (aa == 0xFF) {
                mode->xfer16(device, localSpan, count, NULL);
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

////////////////////////

#if 0
static inline uint16_t aa_blendS32D16(SkPMColor src, U16CPU dst, int aa
#ifdef DITHER_SHADER
                                      , int dither
#endif
                                      )
{
    SkASSERT((unsigned)aa <= 255);
    
    int src_scale = SkAlpha255To256(aa);
    int sa = SkGetPackedA32(src);
    int dst_scale = SkAlpha255To256(255 - SkAlphaMul(sa, src_scale));
    
#ifdef DITHER_SHADER
    int sr = SkGetPackedR32(src);
    int sg = SkGetPackedG32(src);
    int sb = SkGetPackedB32(src);
    sr = SkDITHER_R32To16(sr, dither);
    sg = SkDITHER_G32To16(sg, dither);
    sb = SkDITHER_B32To16(sb, dither);
#else
    int sr = SkPacked32ToR16(src);
    int sg = SkPacked32ToG16(src);
    int sb = SkPacked32ToB16(src);
#endif
    
    int dr = (sr * src_scale + SkGetPackedR16(dst) * dst_scale) >> 8;
    int dg = (sg * src_scale + SkGetPackedG16(dst) * dst_scale) >> 8;
    int db = (sb * src_scale + SkGetPackedB16(dst) * dst_scale) >> 8;
    
    return SkPackRGB16(dr, dg, db);
}
#endif

