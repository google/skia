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
#include "SkShader.h"
#include "SkUtils.h"
#include "SkXfermode.h"

SkARGB32_Blitter::SkARGB32_Blitter(const SkBitmap& device, const SkPaint& paint)
        : INHERITED(device) {
    uint32_t color = paint.getColor();

    fSrcA = SkColorGetA(color);
    unsigned scale = SkAlpha255To256(fSrcA);
    fSrcR = SkAlphaMul(SkColorGetR(color), scale);
    fSrcG = SkAlphaMul(SkColorGetG(color), scale);
    fSrcB = SkAlphaMul(SkColorGetB(color), scale);

    fPMColor = SkPackARGB32(fSrcA, fSrcR, fSrcG, fSrcB);
}

const SkBitmap* SkARGB32_Blitter::justAnOpaqueColor(uint32_t* value) {
    if (255 == fSrcA) {
        *value = fPMColor;
        return &fDevice;
    }
    return NULL;
}

#if defined _WIN32 && _MSC_VER >= 1300  // disable warning : local variable used without having been initialized
#pragma warning ( push )
#pragma warning ( disable : 4701 )
#endif

void SkARGB32_Blitter::blitH(int x, int y, int width) {
    SkASSERT(x >= 0 && y >= 0 && x + width <= fDevice.width());

    if (fSrcA == 0) {
        return;
    }

    uint32_t* device = fDevice.getAddr32(x, y);

    if (fSrcA == 255) {
        sk_memset32(device, fPMColor, width);
    } else {
        uint32_t color = fPMColor;
        unsigned dst_scale = SkAlpha255To256(255 - fSrcA);
        uint32_t prevDst = ~device[0];  // so we always fail the test the first time
        uint32_t result SK_INIT_TO_AVOID_WARNING;

        for (int i = 0; i < width; i++) {
            uint32_t currDst = device[i];
            if (currDst != prevDst) {
                result = color + SkAlphaMulQ(currDst, dst_scale);
                prevDst = currDst;
            }
            device[i] = result;
        }
    }
}

void SkARGB32_Blitter::blitAntiH(int x, int y, const SkAlpha antialias[],
                                 const int16_t runs[]) {
    if (fSrcA == 0) {
        return;
    }

    uint32_t    color = fPMColor;
    uint32_t*   device = fDevice.getAddr32(x, y);
    unsigned    opaqueMask = fSrcA; // if fSrcA is 0xFF, then we will catch the fast opaque case

    for (;;) {
        int count = runs[0];
        SkASSERT(count >= 0);
        if (count <= 0) {
            return;
        }
        unsigned aa = antialias[0];
        if (aa) {
            if ((opaqueMask & aa) == 255) {
                sk_memset32(device, color, count);
            } else {
                uint32_t sc = SkAlphaMulQ(color, aa);
                unsigned dst_scale = 255 - SkGetPackedA32(sc);
                int n = count;
                do {
                    --n;
                    device[n] = sc + SkAlphaMulQ(device[n], dst_scale);
                } while (n > 0);
            }
        }
        runs += count;
        antialias += count;
        device += count;
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

#define SK_BLITBWMASK_NAME                  SkARGB32_BlitBW
#define SK_BLITBWMASK_ARGS                  , SkPMColor color
#define SK_BLITBWMASK_BLIT8(mask, dst)      solid_8_pixels(mask, dst, color)
#define SK_BLITBWMASK_GETADDR               getAddr32
#define SK_BLITBWMASK_DEVTYPE               uint32_t
#include "SkBlitBWMaskTemplate.h"

#define blend_8_pixels(mask, dst, sc, dst_scale)                            \
    do {                                                                    \
        if (mask & 0x80) { dst[0] = sc + SkAlphaMulQ(dst[0], dst_scale); }  \
        if (mask & 0x40) { dst[1] = sc + SkAlphaMulQ(dst[1], dst_scale); }  \
        if (mask & 0x20) { dst[2] = sc + SkAlphaMulQ(dst[2], dst_scale); }  \
        if (mask & 0x10) { dst[3] = sc + SkAlphaMulQ(dst[3], dst_scale); }  \
        if (mask & 0x08) { dst[4] = sc + SkAlphaMulQ(dst[4], dst_scale); }  \
        if (mask & 0x04) { dst[5] = sc + SkAlphaMulQ(dst[5], dst_scale); }  \
        if (mask & 0x02) { dst[6] = sc + SkAlphaMulQ(dst[6], dst_scale); }  \
        if (mask & 0x01) { dst[7] = sc + SkAlphaMulQ(dst[7], dst_scale); }  \
    } while (0)

#define SK_BLITBWMASK_NAME                  SkARGB32_BlendBW
#define SK_BLITBWMASK_ARGS                  , uint32_t sc, unsigned dst_scale
#define SK_BLITBWMASK_BLIT8(mask, dst)      blend_8_pixels(mask, dst, sc, dst_scale)
#define SK_BLITBWMASK_GETADDR               getAddr32
#define SK_BLITBWMASK_DEVTYPE               uint32_t
#include "SkBlitBWMaskTemplate.h"

void SkARGB32_Blitter::blitMask(const SkMask& mask, const SkIRect& clip) {
    SkASSERT(mask.fBounds.contains(clip));
    SkASSERT(fSrcA != 0xFF);

    if (fSrcA == 0) {
        return;
    }

    if (mask.fFormat == SkMask::kBW_Format) {
        SkARGB32_BlendBW(fDevice, mask, clip, fPMColor, SkAlpha255To256(255 - fSrcA));
        return;
    }

    int x = clip.fLeft;
    int y = clip.fTop;
    int width = clip.width();
    int height = clip.height();

    uint32_t*       device = fDevice.getAddr32(x, y);
    const uint8_t*  alpha = mask.getAddr(x, y);
    uint32_t        srcColor = fPMColor;
    unsigned        devRB = fDevice.rowBytes() - (width << 2);
    unsigned        maskRB = mask.fRowBytes - width;

    do {
        int w = width;
        do {
            unsigned aa = *alpha++;
            *device = SkBlendARGB32(srcColor, *device, aa);
            device += 1;
        } while (--w != 0);
        device = (uint32_t*)((char*)device + devRB);
        alpha += maskRB;
    } while (--height != 0);
}

void SkARGB32_Opaque_Blitter::blitMask(const SkMask& mask,
                                       const SkIRect& clip) {
    SkASSERT(mask.fBounds.contains(clip));

    if (mask.fFormat == SkMask::kBW_Format) {
        SkARGB32_BlitBW(fDevice, mask, clip, fPMColor);
        return;
    }

    int x = clip.fLeft;
    int y = clip.fTop;
    int width = clip.width();
    int height = clip.height();

    uint32_t*       device = fDevice.getAddr32(x, y);
    const uint8_t*  alpha = mask.getAddr(x, y);
    uint32_t        srcColor = fPMColor;
    unsigned        devRB = fDevice.rowBytes() - (width << 2);
    unsigned        maskRB = mask.fRowBytes - width;

    do {
        int w = width;
        do {
            unsigned aa = *alpha++;
            *device = SkAlphaMulQ(srcColor, SkAlpha255To256(aa)) + SkAlphaMulQ(*device, SkAlpha255To256(255 - aa));
            device += 1;
        } while (--w != 0);
        device = (uint32_t*)((char*)device + devRB);
        alpha += maskRB;
    } while (--height != 0);
}

//////////////////////////////////////////////////////////////////////////////////////

void SkARGB32_Blitter::blitV(int x, int y, int height, SkAlpha alpha) {
    if (alpha == 0 || fSrcA == 0) {
        return;
    }

    uint32_t* device = fDevice.getAddr32(x, y);
    uint32_t  color = fPMColor;

    if (alpha != 255) {
        color = SkAlphaMulQ(color, SkAlpha255To256(alpha));
    }

    unsigned dst_scale = 255 - SkGetPackedA32(color);
    uint32_t prevDst = ~device[0];
    uint32_t result  SK_INIT_TO_AVOID_WARNING;
    uint32_t rowBytes = fDevice.rowBytes();

    while (--height >= 0) {
        uint32_t dst = device[0];
        if (dst != prevDst) {
            result = color + SkAlphaMulQ(dst, dst_scale);
            prevDst = dst;
        }
        device[0] = result;
        device = (uint32_t*)((char*)device + rowBytes);
    }
}

void SkARGB32_Blitter::blitRect(int x, int y, int width, int height) {
    SkASSERT(x >= 0 && y >= 0 && x + width <= fDevice.width() && y + height <= fDevice.height());

    if (fSrcA == 0) {
        return;
    }

    uint32_t*   device = fDevice.getAddr32(x, y);
    uint32_t    color = fPMColor;

    if (fSrcA == 255) {
        while (--height >= 0) {
            sk_memset32(device, color, width);
            device = (uint32_t*)((char*)device + fDevice.rowBytes());
        }
    } else {
        unsigned dst_scale = SkAlpha255To256(255 - fSrcA);

        while (--height >= 0) {
            uint32_t prevDst = ~device[0];
            uint32_t result SK_INIT_TO_AVOID_WARNING;

            for (int i = 0; i < width; i++) {
                uint32_t dst = device[i];
                if (dst != prevDst) {
                    result = color + SkAlphaMulQ(dst, dst_scale);
                    prevDst = dst;
                }
                device[i] = result;
            }
            device = (uint32_t*)((char*)device + fDevice.rowBytes());
        }
    }
}

#if defined _WIN32 && _MSC_VER >= 1300
#pragma warning ( pop )
#endif

///////////////////////////////////////////////////////////////////////

void SkARGB32_Black_Blitter::blitMask(const SkMask& mask, const SkIRect& clip) {
    SkASSERT(mask.fBounds.contains(clip));

    if (mask.fFormat == SkMask::kBW_Format) {
        SkPMColor black = (SkPMColor)(SK_A32_MASK << SK_A32_SHIFT);

        SkARGB32_BlitBW(fDevice, mask, clip, black);
    } else {
        uint32_t*       device = fDevice.getAddr32(clip.fLeft, clip.fTop);
        const uint8_t*  alpha = mask.getAddr(clip.fLeft, clip.fTop);
        unsigned        width = clip.width();
        unsigned        height = clip.height();
        unsigned        deviceRB = fDevice.rowBytes() - (width << 2);
        unsigned        maskRB = mask.fRowBytes - width;

        SkASSERT((int)height > 0);
        SkASSERT((int)width > 0);
        SkASSERT((int)deviceRB >= 0);
        SkASSERT((int)maskRB >= 0);

        do {
            unsigned w = width;
            do {
                unsigned aa = *alpha++;
                *device = (aa << SK_A32_SHIFT) + SkAlphaMulQ(*device, SkAlpha255To256(255 - aa));
                device += 1;
            } while (--w != 0);
            device = (uint32_t*)((char*)device + deviceRB);
            alpha += maskRB;
        } while (--height != 0);
    }
}

void SkARGB32_Black_Blitter::blitAntiH(int x, int y, const SkAlpha antialias[],
                                       const int16_t runs[]) {
    uint32_t*   device = fDevice.getAddr32(x, y);
    SkPMColor   black = (SkPMColor)(SK_A32_MASK << SK_A32_SHIFT);

    for (;;) {
        int count = runs[0];
        SkASSERT(count >= 0);
        if (count <= 0) {
            return;
        }
        unsigned aa = antialias[0];
        if (aa) {
            if (aa == 255) {
                sk_memset32(device, black, count);
            } else {
                SkPMColor src = aa << SK_A32_SHIFT;
                unsigned dst_scale = 256 - aa;
                int n = count;
                do {
                    --n;
                    device[n] = src + SkAlphaMulQ(device[n], dst_scale);
                } while (n > 0);
            }
        }
        runs += count;
        antialias += count;
        device += count;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////

SkARGB32_Shader_Blitter::SkARGB32_Shader_Blitter(const SkBitmap& device,
                                                 const SkPaint& paint)
        : INHERITED(device, paint) {
    fBuffer = (SkPMColor*)sk_malloc_throw(device.width() * (sizeof(SkPMColor)));

    (fXfermode = paint.getXfermode())->safeRef();
}

SkARGB32_Shader_Blitter::~SkARGB32_Shader_Blitter() {
    fXfermode->safeUnref();
    sk_free(fBuffer);
}

void SkARGB32_Shader_Blitter::blitH(int x, int y, int width) {
    SkASSERT(x >= 0 && y >= 0 && x + width <= fDevice.width());

    uint32_t*   device = fDevice.getAddr32(x, y);

    if (fXfermode == NULL && (fShader->getFlags() & SkShader::kOpaqueAlpha_Flag)) {
        fShader->shadeSpan(x, y, device, width);
    } else {
        SkPMColor*  span = fBuffer;
        fShader->shadeSpan(x, y, span, width);
        if (fXfermode) {
            fXfermode->xfer32(device, span, width, NULL);
        } else {
            for (int i = 0; i < width; i++) {
                uint32_t src = span[i];
                if (src) {
                    unsigned srcA = SkGetPackedA32(src);
                    if (srcA != 0xFF) {
                        src += SkAlphaMulQ(device[i], SkAlpha255To256(255 - srcA));
                    }
                    device[i] = src;
                }
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////

void SkARGB32_Shader_Blitter::blitAntiH(int x, int y, const SkAlpha antialias[],
                                        const int16_t runs[]) {
    SkPMColor*  span = fBuffer;
    uint32_t*   device = fDevice.getAddr32(x, y);
    SkShader*   shader = fShader;

    if (fXfermode) {
        for (;;) {
            SkXfermode* xfer = fXfermode;

            int count = *runs;
            if (count <= 0)
                break;
            int aa = *antialias;
            if (aa) {
                shader->shadeSpan(x, y, span, count);
                if (aa == 255) {
                    xfer->xfer32(device, span, count, NULL);
                } else {
                    // count is almost always 1
                    for (int i = count - 1; i >= 0; --i) {
                        xfer->xfer32(&device[i], &span[i], 1, antialias);
                    }
                }
            }
            device += count;
            runs += count;
            antialias += count;
            x += count;
        } 
    } else if (fShader->getFlags() & SkShader::kOpaqueAlpha_Flag) {
        for (;;) {
            int count = *runs;
            if (count <= 0) {
                break;
            }
            int aa = *antialias;
            if (aa) {
                if (aa == 255) {  // cool, have the shader draw right into the device
                    shader->shadeSpan(x, y, device, count);
                } else {
                    shader->shadeSpan(x, y, span, count);
                    for (int i = count - 1; i >= 0; --i) {
                        if (span[i]) {
                            device[i] = SkBlendARGB32(span[i], device[i], aa);
                        }
                    }
                }
            }
            device += count;
            runs += count;
            antialias += count;
            x += count;
        } 
    } else {    // no xfermode but we are not opaque
        for (;;) {
            int count = *runs;
            if (count <= 0) {
                break;
            }
            int aa = *antialias;
            if (aa) {
                fShader->shadeSpan(x, y, span, count);
                if (aa == 255) {
                    for (int i = count - 1; i >= 0; --i) {
                        if (span[i]) {
                            device[i] = SkPMSrcOver(span[i], device[i]);
                        }
                    }
                } else {
                    for (int i = count - 1; i >= 0; --i) {
                        if (span[i]) {
                            device[i] = SkBlendARGB32(span[i], device[i], aa);
                        }
                    }
                }
            }
            device += count;
            runs += count;
            antialias += count;
            x += count;
        } 
    }
}

