/* libs/graphics/sgl/SkBlitter_ARGB32.cpp
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

#include "SkCoreBlitters.h"
#include "SkColorPriv.h"
#include "SkDither.h"
#include "SkShader.h"
#include "SkTemplatesPriv.h"
#include "SkUtils.h"
#include "SkXfermode.h"

inline SkPMColor SkBlendARGB4444(SkPMColor16 src, SkPMColor16 dst, U8CPU aa)
{
    SkASSERT((unsigned)aa <= 255);
    
    unsigned src_scale = SkAlpha255To256(aa) >> 4;
    unsigned dst_scale = SkAlpha15To16(15 - SkAlphaMul4(SkGetPackedA4444(src), src_scale));
    
    uint32_t src32 = SkExpand_4444(src) * src_scale;
    uint32_t dst32 = SkExpand_4444(dst) * dst_scale;
    return SkCompact_4444((src32 + dst32) >> 4);
}

///////////////////////////////////////////////////////////////////////////////

class SkARGB4444_Blitter : public SkRasterBlitter {
public:
    SkARGB4444_Blitter(const SkBitmap& device, const SkPaint& paint);
    virtual void blitH(int x, int y, int width);
    virtual void blitAntiH(int x, int y, const SkAlpha antialias[], const int16_t runs[]);
    virtual void blitV(int x, int y, int height, SkAlpha alpha);
    virtual void blitRect(int x, int y, int width, int height);
    virtual void blitMask(const SkMask&, const SkIRect&);
    virtual const SkBitmap* justAnOpaqueColor(uint32_t*);
    
protected:
    SkPMColor16 fPMColor16, fPMColor16Other;
    SkPMColor16 fRawColor16, fRawColor16Other;
    uint8_t     fScale16;
    
private:
    // illegal
    SkARGB4444_Blitter& operator=(const SkARGB4444_Blitter&);
    
    typedef SkRasterBlitter INHERITED;
};


SkARGB4444_Blitter::SkARGB4444_Blitter(const SkBitmap& device, const SkPaint& paint)
    : INHERITED(device)
{
    // cache premultiplied versions in 4444
    SkPMColor c = SkPreMultiplyColor(paint.getColor());
    fPMColor16 = SkPixel32ToPixel4444(c);
    if (paint.isDither()) {
        fPMColor16Other = SkDitherPixel32To4444(c);
    } else {
        fPMColor16Other = fPMColor16;
    }

    // cache raw versions in 4444
    fRawColor16 = SkPackARGB4444(0xFF >> 4, SkColorGetR(c) >> 4,
                                 SkColorGetG(c) >> 4, SkColorGetB(c) >> 4);
    if (paint.isDither()) {
        fRawColor16Other = SkDitherARGB32To4444(0xFF, SkColorGetR(c),
                                                SkColorGetG(c), SkColorGetB(c));
    } else {
        fRawColor16Other = fRawColor16;
    }
    
    // our dithered color will be the same or more opaque than the original
    // so use dithered to compute our scale
    SkASSERT(SkGetPackedA4444(fPMColor16Other) >= SkGetPackedA4444(fPMColor16));

    fScale16 = SkAlpha15To16(SkGetPackedA4444(fPMColor16Other));
    if (16 == fScale16) {
        // force the original to also be opaque
        fPMColor16 |= (0xF << SK_A4444_SHIFT);
    }
}

const SkBitmap* SkARGB4444_Blitter::justAnOpaqueColor(uint32_t* value)
{
    if (16 == fScale16) {
        *value = fPMColor16;
        return &fDevice;
    }
    return NULL;
}

static void src_over_4444(SkPMColor16 dst[], SkPMColor16 color,
                          SkPMColor16 other, unsigned invScale, int count)
{
    int twice = count >> 1;
    while (--twice >= 0) {
        *dst = color + SkAlphaMulQ4(*dst, invScale);
        dst++;
        *dst = other + SkAlphaMulQ4(*dst, invScale);
        dst++;
    }
    if (color & 1) {
        *dst = color + SkAlphaMulQ4(*dst, invScale);
    }
}

static inline uint32_t SkExpand_4444_Replicate(SkPMColor16 c)
{
    uint32_t c32 = SkExpand_4444(c);
    return c32 | (c32 << 4);
}

static void src_over_4444x(SkPMColor16 dst[], uint32_t color,
                           uint32_t other, unsigned invScale, int count)
{
    int twice = count >> 1;
    uint32_t tmp;
    while (--twice >= 0) {
        tmp = SkExpand_4444(*dst) * invScale;
        *dst++ = SkCompact_4444((color + tmp) >> 4);
        tmp = SkExpand_4444(*dst) * invScale;
        *dst++ = SkCompact_4444((other + tmp) >> 4);
    }
    if (color & 1) {
        tmp = SkExpand_4444(*dst) * invScale;
        *dst = SkCompact_4444((color + tmp) >> 4);
    }
}

void SkARGB4444_Blitter::blitH(int x, int y, int width)
{
    SkASSERT(x >= 0 && y >= 0 && x + width <= fDevice.width());
    
    if (0 == fScale16) {
        return;
    }
    
    SkPMColor16* device = fDevice.getAddr16(x, y);
    SkPMColor16  color = fPMColor16;
    SkPMColor16  other = fPMColor16Other;
    
    if ((x ^ y) & 1) {
        SkTSwap<SkPMColor16>(color, other);
    }
    
    if (16 == fScale16) {
        sk_dither_memset16(device, color, other, width);
    }
    else {
        src_over_4444x(device, SkExpand_4444_Replicate(color),
                       SkExpand_4444_Replicate(other),
                       16 - fScale16, width);
    }
}

void SkARGB4444_Blitter::blitV(int x, int y, int height, SkAlpha alpha)
{
    if (0 == alpha || 0 == fScale16) {
        return;
    }
    
    SkPMColor16* device = fDevice.getAddr16(x, y);
    SkPMColor16  color = fPMColor16;
    SkPMColor16  other = fPMColor16Other;
    unsigned     rb = fDevice.rowBytes();
    
    if ((x ^ y) & 1) {
        SkTSwap<SkPMColor16>(color, other);
    }

    if (16 == fScale16 && 255 == alpha) {
        while (--height >= 0) {
            *device = color;
            device = (SkPMColor16*)((char*)device + rb);
            SkTSwap<SkPMColor16>(color, other);
        }
    } else {
        unsigned alphaScale = SkAlpha255To256(alpha);
        uint32_t c32 = SkExpand_4444(color) * (alphaScale >> 4);
        // need to normalize the low nibble of each expanded component
        // so we don't overflow the add with d32
        c32 = SkCompact_4444(c32 >> 4);
        unsigned invScale = 16 - SkAlpha15To16(SkGetPackedA4444(c32));
        // now re-expand and replicate
        c32 = SkExpand_4444_Replicate(c32);

        while (--height >= 0) {
            uint32_t d32 = SkExpand_4444(*device) * invScale;
            *device = SkCompact_4444((c32 + d32) >> 4);
            device = (SkPMColor16*)((char*)device + rb);
        }
    }
}

void SkARGB4444_Blitter::blitRect(int x, int y, int width, int height)
{
    SkASSERT(x >= 0 && y >= 0 && x + width <= fDevice.width() && y + height <= fDevice.height());
    
    if (0 == fScale16) {
        return;
    }
    
    SkPMColor16* device = fDevice.getAddr16(x, y);
    SkPMColor16  color = fPMColor16;
    SkPMColor16  other = fPMColor16Other;
    
    if ((x ^ y) & 1) {
        SkTSwap<SkPMColor16>(color, other);
    }
    
    if (16 == fScale16) {
        while (--height >= 0) {
            sk_dither_memset16(device, color, other, width);
            device = (SkPMColor16*)((char*)device + fDevice.rowBytes());
            SkTSwap<SkPMColor16>(color, other);
        }
    } else {
        unsigned invScale = 16 - fScale16;

        uint32_t c32 = SkExpand_4444_Replicate(color);
        uint32_t o32 = SkExpand_4444_Replicate(other);
        while (--height >= 0) {
            src_over_4444x(device, c32, o32, invScale, width);
            device = (SkPMColor16*)((char*)device + fDevice.rowBytes());
            SkTSwap<uint32_t>(c32, o32);
        }
    }
}

void SkARGB4444_Blitter::blitAntiH(int x, int y, const SkAlpha antialias[], const int16_t runs[])
{
    if (0 == fScale16) {
        return;
    }
    
    SkPMColor16* device = fDevice.getAddr16(x, y);
    SkPMColor16  color = fPMColor16;
    SkPMColor16  other = fPMColor16Other;
    
    if ((x ^ y) & 1) {
        SkTSwap<SkPMColor16>(color, other);
    }
    
    for (;;) {
        int count = runs[0];
        SkASSERT(count >= 0);
        if (count <= 0) {
            return;
        }
        
        unsigned aa = antialias[0];
        if (aa) {
            if (0xFF == aa) {
                if (16 == fScale16) {
                    sk_dither_memset16(device, color, other, count);
                } else {
                    src_over_4444(device, color, other, 16 - fScale16, count);
                }
            } else {
                // todo: respect dithering
                aa = SkAlpha255To256(aa);   // FIX
                SkPMColor16 src = SkAlphaMulQ4(color, aa >> 4);
                unsigned dst_scale = SkAlpha15To16(15 - SkGetPackedA4444(src)); // FIX
                int n = count;
                do {
                    --n;
                    device[n] = src + SkAlphaMulQ4(device[n], dst_scale);
                } while (n > 0);
            }
        }

        runs += count;
        antialias += count;
        device += count;

        if (count & 1) {
            SkTSwap<SkPMColor16>(color, other);
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

#define SK_BLITBWMASK_NAME                  SkARGB4444_BlitBW
#define SK_BLITBWMASK_ARGS                  , SkPMColor16 color
#define SK_BLITBWMASK_BLIT8(mask, dst)      solid_8_pixels(mask, dst, color)
#define SK_BLITBWMASK_GETADDR               getAddr16
#define SK_BLITBWMASK_DEVTYPE               uint16_t
#include "SkBlitBWMaskTemplate.h"

#define blend_8_pixels(mask, dst, sc, dst_scale)                            \
do {                                                                    \
if (mask & 0x80) { dst[0] = sc + SkAlphaMulQ4(dst[0], dst_scale); }  \
if (mask & 0x40) { dst[1] = sc + SkAlphaMulQ4(dst[1], dst_scale); }  \
if (mask & 0x20) { dst[2] = sc + SkAlphaMulQ4(dst[2], dst_scale); }  \
if (mask & 0x10) { dst[3] = sc + SkAlphaMulQ4(dst[3], dst_scale); }  \
if (mask & 0x08) { dst[4] = sc + SkAlphaMulQ4(dst[4], dst_scale); }  \
if (mask & 0x04) { dst[5] = sc + SkAlphaMulQ4(dst[5], dst_scale); }  \
if (mask & 0x02) { dst[6] = sc + SkAlphaMulQ4(dst[6], dst_scale); }  \
if (mask & 0x01) { dst[7] = sc + SkAlphaMulQ4(dst[7], dst_scale); }  \
} while (0)

#define SK_BLITBWMASK_NAME                  SkARGB4444_BlendBW
#define SK_BLITBWMASK_ARGS                  , uint16_t sc, unsigned dst_scale
#define SK_BLITBWMASK_BLIT8(mask, dst)      blend_8_pixels(mask, dst, sc, dst_scale)
#define SK_BLITBWMASK_GETADDR               getAddr16
#define SK_BLITBWMASK_DEVTYPE               uint16_t
#include "SkBlitBWMaskTemplate.h"

void SkARGB4444_Blitter::blitMask(const SkMask& mask, const SkIRect& clip)
{
    SkASSERT(mask.fBounds.contains(clip));
    
    if (0 == fScale16) {
        return;
    }
    
    if (mask.fFormat == SkMask::kBW_Format) {
        if (16 == fScale16) {
            SkARGB4444_BlitBW(fDevice, mask, clip, fPMColor16);
        } else {
            SkARGB4444_BlendBW(fDevice, mask, clip, fPMColor16, 16 - fScale16);
        }
        return;
    }
    
    int x = clip.fLeft;
    int y = clip.fTop;
    int width = clip.width();
    int height = clip.height();
    
    SkPMColor16*    device = fDevice.getAddr16(x, y);
    const uint8_t*  alpha = mask.getAddr(x, y);
    SkPMColor16     srcColor = fPMColor16;
    unsigned        devRB = fDevice.rowBytes() - (width << 1);
    unsigned        maskRB = mask.fRowBytes - width;
    
    do {
        int w = width;
        do {
            unsigned aa = *alpha++;
            *device = SkBlendARGB4444(srcColor, *device, aa);
            device += 1;
        } while (--w != 0);
        device = (SkPMColor16*)((char*)device + devRB);
        alpha += maskRB;
    } while (--height != 0);
}

//////////////////////////////////////////////////////////////////////////////////////////

class SkARGB4444_Shader_Blitter : public SkShaderBlitter {
    SkXfermode*     fXfermode;
    SkBlitRow::Proc fOpaqueProc;
    SkBlitRow::Proc fAlphaProc;
    SkPMColor*      fBuffer;
    uint8_t*        fAAExpand;
public:
SkARGB4444_Shader_Blitter(const SkBitmap& device, const SkPaint& paint)
    : INHERITED(device, paint)
{
    const int width = device.width();
    fBuffer = (SkPMColor*)sk_malloc_throw(width * sizeof(SkPMColor) + width);
    fAAExpand = (uint8_t*)(fBuffer + width);
    
    (fXfermode = paint.getXfermode())->safeRef();
    
    unsigned flags = 0;
    if (!(fShader->getFlags() & SkShader::kOpaqueAlpha_Flag)) {
        flags |= SkBlitRow::kSrcPixelAlpha_Flag;
    }
    if (paint.isDither()) {
        flags |= SkBlitRow::kDither_Flag;
    }
    fOpaqueProc = SkBlitRow::Factory(flags, SkBitmap::kARGB_4444_Config);
    fAlphaProc = SkBlitRow::Factory(flags | SkBlitRow::kGlobalAlpha_Flag,
                                    SkBitmap::kARGB_4444_Config);
}

virtual ~SkARGB4444_Shader_Blitter()
{
    fXfermode->safeUnref();
    sk_free(fBuffer);
}

virtual void blitH(int x, int y, int width)
{
    SkASSERT(x >= 0 && y >= 0 && x + width <= fDevice.width());
    
    SkPMColor16* device = fDevice.getAddr16(x, y);    
    SkPMColor*   span = fBuffer;
    
    fShader->shadeSpan(x, y, span, width);
    if (fXfermode) {
        fXfermode->xfer4444(device, span, width, NULL);
    }
    else {
        fOpaqueProc(device, span, width, 0xFF, x, y);
    }
}

virtual void blitAntiH(int x, int y, const SkAlpha antialias[], const int16_t runs[])
{
    SkPMColor* SK_RESTRICT span = fBuffer;
    uint8_t* SK_RESTRICT aaExpand = fAAExpand;
    SkPMColor16* device = fDevice.getAddr16(x, y);
    SkShader*    shader = fShader;
    SkXfermode*  xfer = fXfermode;
    
    if (NULL != xfer) {
        for (;;) {
            int count = *runs;
            if (count <= 0)
                break;
            int aa = *antialias;
            if (aa) {
                shader->shadeSpan(x, y, span, count);
                if (255 == aa) {
                    xfer->xfer4444(device, span, count, NULL);
                } else {
                    const uint8_t* aaBuffer = antialias;
                    if (count > 1) {
                        memset(aaExpand, aa, count);
                        aaBuffer = aaExpand;
                    }
                    xfer->xfer4444(device, span, count, aaBuffer);
                }
            }
            device += count;
            runs += count;
            antialias += count;
            x += count;
        } 
    } else {    // no xfermode
        for (;;) {
            int count = *runs;
            if (count <= 0)
                break;
            int aa = *antialias;
            if (aa) {
                fShader->shadeSpan(x, y, span, count);
                if (255 == aa) {
                    fOpaqueProc(device, span, count, aa, x, y);
                } else {
                    fAlphaProc(device, span, count, aa, x, y);
                }
            }
            device += count;
            runs += count;
            antialias += count;
            x += count;
        } 
    }
}

private:
    typedef SkShaderBlitter INHERITED;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

SkBlitter* SkBlitter_ChooseD4444(const SkBitmap& device,
                                 const SkPaint& paint,
                                 void* storage, size_t storageSize)
{
    SkBlitter* blitter;

    if (paint.getShader()) {
        SK_PLACEMENT_NEW_ARGS(blitter, SkARGB4444_Shader_Blitter, storage, storageSize, (device, paint));
    } else {
        SK_PLACEMENT_NEW_ARGS(blitter, SkARGB4444_Blitter, storage, storageSize, (device, paint));
    }
    return blitter;
}

