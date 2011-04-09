/* libs/graphics/sgl/SkBlitter_A8.cpp
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
#include "SkXfermode.h"

SkA8_Blitter::SkA8_Blitter(const SkBitmap& device, const SkPaint& paint)
        : INHERITED(device) {
    fSrcA = paint.getAlpha();
}

const SkBitmap* SkA8_Blitter::justAnOpaqueColor(uint32_t* value) {
    if (255 == fSrcA) {
        *value = 255;
        return &fDevice;
    }
    return NULL;
}

void SkA8_Blitter::blitH(int x, int y, int width) {
    SkASSERT(x >= 0 && y >= 0 &&
             (unsigned)(x + width) <= (unsigned)fDevice.width());

    if (fSrcA == 0) {
        return;
    }

    uint8_t* device = fDevice.getAddr8(x, y);

    if (fSrcA == 255) {
        memset(device, 0xFF, width);
    } else {
        unsigned scale = 256 - SkAlpha255To256(fSrcA);
        unsigned srcA = fSrcA;

        for (int i = 0; i < width; i++) {
            device[i] = SkToU8(srcA + SkAlphaMul(device[i], scale));
        }
    }
}

void SkA8_Blitter::blitAntiH(int x, int y, const SkAlpha antialias[],
                             const int16_t runs[]) {
    if (fSrcA == 0) {
        return;
    }

    uint8_t*    device = fDevice.getAddr8(x, y);
    unsigned    srcA = fSrcA;

    for (;;) {
        int count = runs[0];
        SkASSERT(count >= 0);
        if (count == 0) {
            return;
        }
        unsigned aa = antialias[0];

        if (aa == 255 && srcA == 255) {
            memset(device, 0xFF, count);
        } else {
            unsigned sa = SkAlphaMul(srcA, SkAlpha255To256(aa));
            unsigned scale = 256 - sa;

            for (int i = 0; i < count; i++) {
                device[i] = SkToU8(sa + SkAlphaMul(device[i], scale));
            }
        }
        runs += count;
        antialias += count;
        device += count;
    }
}

/////////////////////////////////////////////////////////////////////////////////////

#define solid_8_pixels(mask, dst)           \
    do {                                    \
        if (mask & 0x80) dst[0] = 0xFF;     \
        if (mask & 0x40) dst[1] = 0xFF;     \
        if (mask & 0x20) dst[2] = 0xFF;     \
        if (mask & 0x10) dst[3] = 0xFF;     \
        if (mask & 0x08) dst[4] = 0xFF;     \
        if (mask & 0x04) dst[5] = 0xFF;     \
        if (mask & 0x02) dst[6] = 0xFF;     \
        if (mask & 0x01) dst[7] = 0xFF;     \
    } while (0)

#define SK_BLITBWMASK_NAME                  SkA8_BlitBW
#define SK_BLITBWMASK_ARGS
#define SK_BLITBWMASK_BLIT8(mask, dst)      solid_8_pixels(mask, dst)
#define SK_BLITBWMASK_GETADDR               getAddr8
#define SK_BLITBWMASK_DEVTYPE               uint8_t
#include "SkBlitBWMaskTemplate.h"

static inline void blend_8_pixels(U8CPU bw, uint8_t dst[], U8CPU sa,
                                  unsigned dst_scale) {
    if (bw & 0x80) dst[0] = SkToU8(sa + SkAlphaMul(dst[0], dst_scale));
    if (bw & 0x40) dst[1] = SkToU8(sa + SkAlphaMul(dst[1], dst_scale));
    if (bw & 0x20) dst[2] = SkToU8(sa + SkAlphaMul(dst[2], dst_scale));
    if (bw & 0x10) dst[3] = SkToU8(sa + SkAlphaMul(dst[3], dst_scale));
    if (bw & 0x08) dst[4] = SkToU8(sa + SkAlphaMul(dst[4], dst_scale));
    if (bw & 0x04) dst[5] = SkToU8(sa + SkAlphaMul(dst[5], dst_scale));
    if (bw & 0x02) dst[6] = SkToU8(sa + SkAlphaMul(dst[6], dst_scale));
    if (bw & 0x01) dst[7] = SkToU8(sa + SkAlphaMul(dst[7], dst_scale));
}

#define SK_BLITBWMASK_NAME                  SkA8_BlendBW
#define SK_BLITBWMASK_ARGS                  , U8CPU sa, unsigned dst_scale
#define SK_BLITBWMASK_BLIT8(mask, dst)      blend_8_pixels(mask, dst, sa, dst_scale)
#define SK_BLITBWMASK_GETADDR               getAddr8
#define SK_BLITBWMASK_DEVTYPE               uint8_t
#include "SkBlitBWMaskTemplate.h"

void SkA8_Blitter::blitMask(const SkMask& mask, const SkIRect& clip) {
    if (fSrcA == 0) {
        return;
    }

    if (mask.fFormat == SkMask::kBW_Format) {
        if (fSrcA == 0xFF) {
            SkA8_BlitBW(fDevice, mask, clip);
        } else {
            SkA8_BlendBW(fDevice, mask, clip, fSrcA,
                         SkAlpha255To256(255 - fSrcA));
        }
        return;
    }

    int x = clip.fLeft;
    int y = clip.fTop;
    int width = clip.width();
    int height = clip.height();
    uint8_t* device = fDevice.getAddr8(x, y);
    const uint8_t* alpha = mask.getAddr(x, y);
    unsigned    srcA = fSrcA;

    while (--height >= 0) {
        for (int i = width - 1; i >= 0; --i) {
            unsigned sa;
            // scale our src by the alpha value
            {
                int aa = alpha[i];
                if (aa == 0) {
                    continue;
                }
                if (aa == 255) {
                    if (srcA == 255) {
                        device[i] = 0xFF;
                        continue;
                    }
                    sa = srcA;
                } else {
                    sa = SkAlphaMul(srcA, SkAlpha255To256(aa));
                }
            }

            int scale = 256 - SkAlpha255To256(sa);
            device[i] = SkToU8(sa + SkAlphaMul(device[i], scale));
        }
        device += fDevice.rowBytes();
        alpha += mask.fRowBytes;
    }
}

///////////////////////////////////////////////////////////////////////////////

void SkA8_Blitter::blitV(int x, int y, int height, SkAlpha alpha) {
    if (fSrcA == 0) {
        return;
    }

    unsigned sa = SkAlphaMul(fSrcA, SkAlpha255To256(alpha));
    uint8_t* device = fDevice.getAddr8(x, y);
    int      rowBytes = fDevice.rowBytes();

    if (sa == 0xFF) {
        for (int i = 0; i < height; i++) {
            *device = SkToU8(sa);
            device += rowBytes;
        }
    } else {
        unsigned scale = 256 - SkAlpha255To256(sa);

        for (int i = 0; i < height; i++) {
            *device = SkToU8(sa + SkAlphaMul(*device, scale));
            device += rowBytes;
        }
    }
}

void SkA8_Blitter::blitRect(int x, int y, int width, int height) {
    SkASSERT(x >= 0 && y >= 0 &&
             (unsigned)(x + width) <= (unsigned)fDevice.width() &&
             (unsigned)(y + height) <= (unsigned)fDevice.height());

    if (fSrcA == 0) {
        return;
    }

    uint8_t*    device = fDevice.getAddr8(x, y);
    unsigned    srcA = fSrcA;

    if (srcA == 255) {
        while (--height >= 0) {
            memset(device, 0xFF, width);
            device += fDevice.rowBytes();
        }
    } else {
        unsigned scale = 256 - SkAlpha255To256(srcA);

        while (--height >= 0) {
            for (int i = 0; i < width; i++) {
                device[i] = SkToU8(srcA + SkAlphaMul(device[i], scale));
            }
            device += fDevice.rowBytes();
        }
    }
}

///////////////////////////////////////////////////////////////////////

SkA8_Shader_Blitter::SkA8_Shader_Blitter(const SkBitmap& device, const SkPaint& paint)
    : INHERITED(device, paint) {
    if ((fXfermode = paint.getXfermode()) != NULL) {
        fXfermode->ref();
        SkASSERT(fShader);
    }

    int width = device.width();
    fBuffer = (SkPMColor*)sk_malloc_throw(sizeof(SkPMColor) * (width + (SkAlign4(width) >> 2)));
    fAAExpand = (uint8_t*)(fBuffer + width);
}

SkA8_Shader_Blitter::~SkA8_Shader_Blitter() {
    if (fXfermode) SkSafeUnref(fXfermode);
    sk_free(fBuffer);
}

void SkA8_Shader_Blitter::blitH(int x, int y, int width) {
    SkASSERT(x >= 0 && y >= 0 &&
             (unsigned)(x + width) <= (unsigned)fDevice.width());

    uint8_t* device = fDevice.getAddr8(x, y);

    if ((fShader->getFlags() & SkShader::kOpaqueAlpha_Flag) && !fXfermode) {
        memset(device, 0xFF, width);
    } else {
        SkPMColor*  span = fBuffer;

        fShader->shadeSpan(x, y, span, width);
        if (fXfermode) {
            fXfermode->xferA8(device, span, width, NULL);
        } else {
            for (int i = width - 1; i >= 0; --i) {
                unsigned    srcA = SkGetPackedA32(span[i]);
                unsigned    scale = 256 - SkAlpha255To256(srcA);

                device[i] = SkToU8(srcA + SkAlphaMul(device[i], scale));
            }
        }
    }
}

static inline uint8_t aa_blend8(SkPMColor src, U8CPU da, int aa) {
    SkASSERT((unsigned)aa <= 255);

    int src_scale = SkAlpha255To256(aa);
    int sa = SkGetPackedA32(src);
    int dst_scale = 256 - SkAlphaMul(sa, src_scale);

    return SkToU8((sa * src_scale + da * dst_scale) >> 8);
}

void SkA8_Shader_Blitter::blitAntiH(int x, int y, const SkAlpha antialias[],
                                    const int16_t runs[]) {
    SkShader*   shader = fShader;
    SkXfermode* mode = fXfermode;
    uint8_t*    aaExpand = fAAExpand;
    SkPMColor*  span = fBuffer;
    uint8_t*    device = fDevice.getAddr8(x, y);
    int         opaque = fShader->getFlags() & SkShader::kOpaqueAlpha_Flag;

    for (;;) {
        int count = *runs;
        if (count == 0) {
            break;
        }
        int aa = *antialias;
        if (aa) {
            if (opaque && aa == 255 && mode == NULL) {
                memset(device, 0xFF, count);
            } else {
                shader->shadeSpan(x, y, span, count);
                if (mode) {
                    memset(aaExpand, aa, count);
                    mode->xferA8(device, span, count, aaExpand);
                } else {
                    for (int i = count - 1; i >= 0; --i) {
                        device[i] = aa_blend8(span[i], device[i], aa);
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

void SkA8_Shader_Blitter::blitMask(const SkMask& mask, const SkIRect& clip) {
    if (mask.fFormat == SkMask::kBW_Format) {
        this->INHERITED::blitMask(mask, clip);
        return;
    }

    int x = clip.fLeft;
    int y = clip.fTop;
    int width = clip.width();
    int height = clip.height();
    uint8_t* device = fDevice.getAddr8(x, y);
    const uint8_t* alpha = mask.getAddr(x, y);

    SkPMColor*  span = fBuffer;

    while (--height >= 0) {
        fShader->shadeSpan(x, y, span, width);
        if (fXfermode) {
            fXfermode->xferA8(device, span, width, alpha);
        }

        y += 1;
        device += fDevice.rowBytes();
        alpha += mask.fRowBytes;
    }
}

