/* libs/graphics/sgl/SkBlitter_RGB16.cpp
**
** Copyright 2006, Google Inc.
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
#include "SkShader.h"
#include "SkUtils.h"
#include "SkXfermode.h"

#ifdef SK_DEBUG
    static unsigned RGB16Add(U16CPU a, U16CPU b)
    {
        SkASSERT(SkGetPackedR16(a) + SkGetPackedR16(b) <= SK_R16_MASK);
        SkASSERT(SkGetPackedG16(a) + SkGetPackedG16(b) <= SK_G16_MASK);
        SkASSERT(SkGetPackedB16(a) + SkGetPackedB16(b) <= SK_B16_MASK);

        return a + b;
    }
#else
    #define RGB16Add(a, b)  (a + b)
#endif

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

void SkRGB16_Black_Blitter::blitMask(const SkMask& mask, const SkRect16& clip)
{
    if (mask.fFormat == SkMask::kBW_Format)
    {
        SkRGB16_Black_BlitBW(fDevice, mask, clip);
    }
    else
    {
        uint16_t* device = fDevice.getAddr16(clip.fLeft, clip.fTop);
        const uint8_t* alpha = mask.getAddr(clip.fLeft, clip.fTop);
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
                if (aa)
                {
                    if (aa == 255)
                        *device = 0;
                    else
                        *device = SkToU16(SkAlphaMulRGB16(*device, SkAlpha255To256(255 - aa)));
                }
                device += 1;
            } while (--w != 0);
            device = (uint16_t*)((char*)device + deviceRB);
            alpha += maskRB;
        } while (--height != 0);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

SkRGB16_Blitter::SkRGB16_Blitter(const SkBitmap& device, const SkPaint& paint) : fDevice(device)
{
    uint32_t color = paint.getColor();

    fScale = SkAlpha255To256(SkColorGetA(color));

    fRawColor16 = SkPackRGB16(  SkColorGetR(color) >> (8 - SK_R16_BITS),
                                SkColorGetG(color) >> (8 - SK_G16_BITS),
                                SkColorGetB(color) >> (8 - SK_B16_BITS));

    fColor16 = SkPackRGB16( SkAlphaMul(SkColorGetR(color), fScale) >> (8 - SK_R16_BITS),
                            SkAlphaMul(SkColorGetG(color), fScale) >> (8 - SK_G16_BITS),
                            SkAlphaMul(SkColorGetB(color), fScale) >> (8 - SK_B16_BITS));
}

void SkRGB16_Blitter::blitH(int x, int y, int width)
{
    SkASSERT(width > 0);
    SkASSERT(x + width <= fDevice.width());

    if (fScale == 0)
        return;

    uint16_t*   device = fDevice.getAddr16(x, y);
    unsigned    srcColor = fColor16;

    if (fScale == 256)
    {
        sk_memset16(device, srcColor, width);
    }
    else
    {
        unsigned scale = 256 - fScale;
        do {
            *device = (uint16_t)RGB16Add(srcColor, SkAlphaMulRGB16(*device, scale));
            device += 1;
        } while (--width != 0);
    }
}

void SkRGB16_Blitter::blitAntiH(int x, int y, const SkAlpha antialias[], const S16 runs[])
{
    if (fScale == 0)
        return;

    uint16_t*   device = fDevice.getAddr16(x, y);
    uint16_t    srcColor = fColor16;
    unsigned    scale = fScale;

    if (scale == 256)
    {
        for (;;)
        {
            int count = runs[0];
            SkASSERT(count >= 0);
            if (count == 0)
                return;
            runs += count;

            unsigned aa = antialias[0];
            antialias += count;
            if (aa)
            {
                if (aa == 255)
                {
                    sk_memset16(device, srcColor, count);
                }
                else
                {
                    unsigned src = SkAlphaMulRGB16(srcColor, SkAlpha255To256(aa));
                    unsigned dst_scale = SkAlpha255To256(255 - aa);
                    do {
                        *device = (uint16_t)RGB16Add(src, SkAlphaMulRGB16(*device, dst_scale));
                        device += 1;
                    } while (--count != 0);
                    continue;
                }
            }
            device += count;
        }
    }
    else
    {
        for (;;)
        {
            int count = runs[0];
            SkASSERT(count >= 0);
            if (count == 0)
                return;
            runs += count;

            unsigned aa = antialias[0];
            antialias += count;
            if (aa)
            {
                unsigned src = SkAlphaMulRGB16(srcColor, SkAlpha255To256(aa));
                unsigned dst_scale = SkAlpha255To256(255 - SkAlphaMul(aa, scale));
                do {
                    *device = (uint16_t)RGB16Add(src, SkAlphaMulRGB16(*device, dst_scale));
                    device += 1;
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

static inline void blend_8_pixels(U8CPU bw, uint16_t dst[], unsigned dst_scale, U16CPU srcColor)
{
    if (bw & 0x80) dst[0] = SkToU16(srcColor + SkAlphaMulRGB16(dst[0], dst_scale));
    if (bw & 0x40) dst[1] = SkToU16(srcColor + SkAlphaMulRGB16(dst[1], dst_scale));
    if (bw & 0x20) dst[2] = SkToU16(srcColor + SkAlphaMulRGB16(dst[2], dst_scale));
    if (bw & 0x10) dst[3] = SkToU16(srcColor + SkAlphaMulRGB16(dst[3], dst_scale));
    if (bw & 0x08) dst[4] = SkToU16(srcColor + SkAlphaMulRGB16(dst[4], dst_scale));
    if (bw & 0x04) dst[5] = SkToU16(srcColor + SkAlphaMulRGB16(dst[5], dst_scale));
    if (bw & 0x02) dst[6] = SkToU16(srcColor + SkAlphaMulRGB16(dst[6], dst_scale));
    if (bw & 0x01) dst[7] = SkToU16(srcColor + SkAlphaMulRGB16(dst[7], dst_scale));
}

#define SK_BLITBWMASK_NAME                  SkRGB16_BlendBW
#define SK_BLITBWMASK_ARGS                  , unsigned dst_scale, U16CPU src_color
#define SK_BLITBWMASK_BLIT8(mask, dst)      blend_8_pixels(mask, dst, dst_scale, src_color)
#define SK_BLITBWMASK_GETADDR               getAddr16
#define SK_BLITBWMASK_DEVTYPE               uint16_t
#include "SkBlitBWMaskTemplate.h"

void SkRGB16_Blitter::blitMask(const SkMask& mask, const SkRect16& clip)
{
    if (fScale == 0)
        return;

    if (mask.fFormat == SkMask::kBW_Format)
    {
        if (fScale == 256)
            SkRGB16_BlitBW(fDevice, mask, clip, fColor16);
        else
            SkRGB16_BlendBW(fDevice, mask, clip, 256 - fScale, fColor16);
        return;
    }

    uint16_t* device = fDevice.getAddr16(clip.fLeft, clip.fTop);
    const uint8_t* alpha = mask.getAddr(clip.fLeft, clip.fTop);
    int width = clip.width();
    int height = clip.height();
    unsigned    maskRB = mask.fRowBytes;
    uint16_t    color16 = fRawColor16;
    unsigned    scale = fScale;
    unsigned    deviceRB = fDevice.rowBytes();

    if (scale == 256)
    {
        while (--height >= 0)
        {
            for (int i = width - 1; i >= 0; --i)
            {
                unsigned aa = alpha[i];
                if (aa)
                {
                    if (aa == 255)
                        device[i] = color16;
                    else
                    {
                        unsigned src_scale = SkAlpha255To256(aa);
                        unsigned dst_scale = SkAlpha255To256(255 - aa);
                        device[i] = (uint16_t)RGB16Add(SkAlphaMulRGB16(color16, src_scale), SkAlphaMulRGB16(device[i], dst_scale));
                    }
                }
            }
            device = (uint16_t*)((char*)device + deviceRB);
            alpha += maskRB;
        }
    }
    else    // scale < 256
    {
        while (--height >= 0)
        {
            for (int i = width - 1; i >= 0; --i)
            {
                unsigned aa = alpha[i];
                if (aa)
                {
                    aa = SkAlphaMul(aa, scale);
                    unsigned src_scale = SkAlpha255To256(aa);
                    unsigned dst_scale = SkAlpha255To256(255 - aa);
                    device[i] = (uint16_t)RGB16Add(SkAlphaMulRGB16(color16, src_scale), SkAlphaMulRGB16(device[i], dst_scale));
                }
            }
            device = (uint16_t*)((char*)device + deviceRB);
            alpha += maskRB;
        }
    }
}

void SkRGB16_Blitter::blitV(int x, int y, int height, SkAlpha alpha)
{
    if (fScale == 0)
        return;

    uint16_t*   device = fDevice.getAddr16(x, y);
    uint16_t    color16 = fColor16;
    unsigned    deviceRB = fDevice.rowBytes();

    if (alpha + fScale == (255 + 256))
    {
        do {
            device[0] = color16;
            device = (uint16_t*)((char*)device + deviceRB);
        } while (--height != 0);
    }
    else
    {
        unsigned scale = fScale;

        if (alpha < 255)
        {
            scale = SkAlphaMul(alpha, scale);
            color16 = SkToU16(SkAlphaMulRGB16(fRawColor16, scale));
        }
        scale = 256 - scale;
        do {
            *device = (uint16_t)RGB16Add(color16, SkAlphaMulRGB16(device[0], scale));
            device = (uint16_t*)((char*)device + deviceRB);
        } while (--height != 0);
    }
}

void SkRGB16_Blitter::blitRect(int x, int y, int width, int height)
{
    SkASSERT(x + width <= fDevice.width() && y + height <= fDevice.height());

    if (fScale == 0)
        return;

    uint16_t*   device = fDevice.getAddr16(x, y);
    unsigned    deviceRB = fDevice.rowBytes();
    uint16_t    color16 = fColor16;

    if (fScale == 256)
    {
        while (--height >= 0)
        {
            sk_memset16(device, color16, width);
            device = (uint16_t*)((char*)device + deviceRB);
        }
    }
    else
    {
        unsigned dst_scale = 256 - fScale;  // apply it to the dst

        while (--height >= 0)
        {
            for (int i = width - 1; i >= 0; --i)
                device[i] = SkToU16(color16 + SkAlphaMulRGB16(device[i], dst_scale));
            device = (uint16_t*)((char*)device + deviceRB);
        }
    }
}

///////////////////////////////////////////////////////////////////////

SkRGB16_Shader_Blitter::SkRGB16_Shader_Blitter(const SkBitmap& device, const SkPaint& paint) : fDevice(device)
{
    SkASSERT(paint.getXfermode() == NULL);

    fShader = paint.getShader();
    SkASSERT(fShader);
    fShader->ref();

    SkAutoUnref autoUnref(fShader);
    fBuffer = (SkPMColor*)sk_malloc_throw(device.width() * sizeof(SkPMColor));
    (void)autoUnref.detach();
}

SkRGB16_Shader_Blitter::~SkRGB16_Shader_Blitter()
{
    fShader->unref();
    sk_free(fBuffer);
}

#define BLEND_32_TO_16(srcA, src, dst)  \
    SkToU16(SkPixel32ToPixel16(src) + SkAlphaMulRGB16(dst, 256 - SkAlpha255To256(srcA)))

void SkRGB16_Shader_Blitter::blitH(int x, int y, int width)
{
    SkASSERT(x + width <= fDevice.width());

    uint16_t*   device = fDevice.getAddr16(x, y);
    SkShader*   shader = fShader;
    uint32_t    flags = fShader->getFlags();

    if (SkShader::CanCallShadeSpan16(flags))
    {
        int alpha = shader->getSpan16Alpha();
        if (0xFF == alpha)
            shader->shadeSpan16(x, y, device, width);
        else
        {
            uint16_t* span16 = (uint16_t*)fBuffer;
            shader->shadeSpan16(x, y, span16, width);
            SkBlendRGB16(span16, device, SkAlpha255To256(alpha), width);
        }
        return;
    }

    //  If we get here, we know we need the 32bit answer from the shader

    SkPMColor*  span = fBuffer;

    shader->shadeSpan(x, y, span, width);
    if (flags & SkShader::kOpaqueAlpha_Flag)
    {
        for (int i = width - 1; i >= 0; --i)
            device[i] = SkPixel32ToPixel16_ToU16(span[i]);
    }
    else
    {
        for (int i = 0; i < width; i++)
        {
            SkPMColor src = span[i];
            if (src)
            {
                unsigned srcA = SkGetPackedA32(src);
                if (srcA == 0xFF)
                    device[i] = SkPixel32ToPixel16_ToU16(src);
                else
                    device[i] = BLEND_32_TO_16(srcA, src, device[i]);
            }
        }
    }
}

static inline uint16_t aa_blendS32D16(SkPMColor src, U16CPU dst, int aa)
{
    SkASSERT((unsigned)aa <= 255);

    int src_scale = SkAlpha255To256(aa);
    int sa = SkGetPackedA32(src);
    int dst_scale = SkAlpha255To256(255 - SkAlphaMul(sa, src_scale));

    int dr = (SkPacked32ToR16(src) * src_scale + SkGetPackedR16(dst) * dst_scale) >> 8;
    int dg = (SkPacked32ToG16(src) * src_scale + SkGetPackedG16(dst) * dst_scale) >> 8;
    int db = (SkPacked32ToB16(src) * src_scale + SkGetPackedB16(dst) * dst_scale) >> 8;

    return SkPackRGB16(dr, dg, db);
}

static inline int count_nonzero_span(const int16_t runs[], const SkAlpha aa[])
{
    int count = 0;
    for (;;)
    {
        int n = *runs;
        if (n == 0 || *aa == 0)
            break;
        runs += n;
        aa += n;
        count += n;
    }
    return count;
}

void SkRGB16_Shader_Blitter::blitAntiH(int x, int y, const SkAlpha antialias[], const int16_t runs[])
{
    SkShader*   shader = fShader;
    SkPMColor*  span = fBuffer;
    uint16_t*   device = fDevice.getAddr16(x, y);
    uint32_t    flags = shader->getFlags();

    if (SkShader::CanCallShadeSpan16(flags))
    {
        int alpha = shader->getSpan16Alpha();
        uint16_t* span16 = (uint16_t*)span;

        if (0xFF == alpha)
        {
            for (;;)
            {
                int count = *runs;
                if (count == 0)
                    break;

                int aa = *antialias;
                if (aa == 255)
                    shader->shadeSpan16(x, y, device, count);   // go direct to the device!
                else if (aa)
                {
                    shader->shadeSpan16(x, y, span16, count);
                    SkBlendRGB16(span16, device, SkAlpha255To256(aa), count);
                }
                device += count;
                runs += count;
                antialias += count;
                x += count;
            }
        }
        else    // span alpha is < 255
        {
            alpha = SkAlpha255To256(alpha);
            for (;;)
            {
                int count = *runs;
                if (count == 0)
                    break;

                int aa = SkAlphaMul(*antialias, alpha);
                if (aa)
                {
                    shader->shadeSpan16(x, y, span16, count);
                    SkBlendRGB16(span16, device, SkAlpha255To256(aa), count);
                }

                device += count;
                runs += count;
                antialias += count;
                x += count;
            }
        }
        return;
    }

    // If we get here, take the 32bit shadeSpan case

    int opaque = flags & SkShader::kOpaqueAlpha_Flag;

    for (;;)
    {
        int count = *runs;
        if (count == 0)
            break;

        int aa = *antialias;
        if (aa == 0)
        {
            device += count;
            runs += count;
            antialias += count;
            x += count;
            continue;
        }

        int nonZeroCount = count + count_nonzero_span(runs + count, antialias + count);

        shader->shadeSpan(x, y, span, nonZeroCount);
        x += nonZeroCount;
        SkPMColor* localSpan = span;
        for (;;)
        {
            if (aa == 255)  // no antialiasing
            {
                if (opaque)
                {
                    for (int i = 0; i < count; i++)
                        device[i] = SkPixel32ToPixel16_ToU16(localSpan[i]);
                }
                else
                {
                    for (int i = 0; i < count; i++)
                    {
                        SkPMColor src = localSpan[i];
                        if (src)
                        {
                            unsigned srcA = SkGetPackedA32(src);
                            if (srcA == 0xFF)
                                device[i] = SkPixel32ToPixel16_ToU16(src);
                            else
                                device[i] = BLEND_32_TO_16(srcA, src, device[i]);
                        }
                    }
                }
            }
            else
            {
                for (int i = 0; i < count; i++)
                {
                    if (localSpan[i])
                        device[i] = aa_blendS32D16(localSpan[i], device[i], aa);
                }
            }

            device += count;
            runs += count;
            antialias += count;
            nonZeroCount -= count;
            if (nonZeroCount == 0)
                break;

            localSpan += count;
            SkASSERT(nonZeroCount > 0);
            count = *runs;
            SkASSERT(count > 0);
            aa = *antialias;
        }
    }
}

///////////////////////////////////////////////////////////////////////

SkRGB16_Shader_Xfermode_Blitter::SkRGB16_Shader_Xfermode_Blitter(const SkBitmap& device, const SkPaint& paint) : fDevice(device)
{
    fShader = paint.getShader();
    SkASSERT(fShader);
    fShader->ref();

    fXfermode = paint.getXfermode();
    SkASSERT(fXfermode);
    fXfermode->ref();

    int width = device.width();
    fBuffer = (SkPMColor*)sk_malloc_throw((width + (SkAlign4(width) >> 2)) * sizeof(SkPMColor));
    fAAExpand = (uint8_t*)(fBuffer + width);
}

SkRGB16_Shader_Xfermode_Blitter::~SkRGB16_Shader_Xfermode_Blitter()
{
    fXfermode->unref();
    fShader->unref();
    sk_free(fBuffer);
}

void SkRGB16_Shader_Xfermode_Blitter::blitH(int x, int y, int width)
{
    SkASSERT(x + width <= fDevice.width());

    uint16_t*   device = fDevice.getAddr16(x, y);
    SkPMColor*  span = fBuffer;

    fShader->shadeSpan(x, y, span, width);
    fXfermode->xfer16(device, span, width, NULL);
}

void SkRGB16_Shader_Xfermode_Blitter::blitAntiH(int x, int y, const SkAlpha antialias[], const S16 runs[])
{
    SkShader*   shader = fShader;
    SkXfermode* mode = fXfermode;
    SkPMColor*  span = fBuffer;
    uint8_t*    aaExpand = fAAExpand;
    uint16_t*   device = fDevice.getAddr16(x, y);

    for (;;)
    {
        int count = *runs;
        if (count == 0)
            break;

        int aa = *antialias;
        if (aa == 0)
        {
            device += count;
            runs += count;
            antialias += count;
            x += count;
            continue;
        }

        int nonZeroCount = count + count_nonzero_span(runs + count, antialias + count);

        shader->shadeSpan(x, y, span, nonZeroCount);
        x += nonZeroCount;
        SkPMColor* localSpan = span;
        for (;;)
        {
            if (aa == 0xFF)
                mode->xfer16(device, localSpan, count, NULL);
            else
            {
                SkASSERT(aa);
                memset(aaExpand, aa, count);
                mode->xfer16(device, localSpan, count, aaExpand);
            }
            device += count;
            runs += count;
            antialias += count;
            nonZeroCount -= count;
            if (nonZeroCount == 0)
                break;

            localSpan += count;
            SkASSERT(nonZeroCount > 0);
            count = *runs;
            SkASSERT(count > 0);
            aa = *antialias;
        }
    } 
}


