/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCoreBlitters.h"
#include "SkColorPriv.h"
#include "SkShader.h"
#include "SkUtils.h"
#include "SkXfermode.h"
#include "SkBlitMask.h"

///////////////////////////////////////////////////////////////////////////////

static void SkARGB32_Blit32(const SkPixmap& device, const SkMask& mask,
                            const SkIRect& clip, SkPMColor srcColor) {
    U8CPU alpha = SkGetPackedA32(srcColor);
    unsigned flags = SkBlitRow::kSrcPixelAlpha_Flag32;
    if (alpha != 255) {
        flags |= SkBlitRow::kGlobalAlpha_Flag32;
    }
    SkBlitRow::Proc32 proc = SkBlitRow::Factory32(flags);

    int x = clip.fLeft;
    int y = clip.fTop;
    int width = clip.width();
    int height = clip.height();

    SkPMColor* dstRow = device.writable_addr32(x, y);
    const SkPMColor* srcRow = reinterpret_cast<const SkPMColor*>(mask.getAddr8(x, y));

    do {
        proc(dstRow, srcRow, width, alpha);
        dstRow = (SkPMColor*)((char*)dstRow + device.rowBytes());
        srcRow = (const SkPMColor*)((const char*)srcRow + mask.fRowBytes);
    } while (--height != 0);
}

//////////////////////////////////////////////////////////////////////////////////////

SkARGB32_Blitter::SkARGB32_Blitter(const SkPixmap& device, const SkPaint& paint)
        : INHERITED(device) {
    SkColor color = paint.getColor();
    fColor = color;

    fSrcA = SkColorGetA(color);
    unsigned scale = SkAlpha255To256(fSrcA);
    fSrcR = SkAlphaMul(SkColorGetR(color), scale);
    fSrcG = SkAlphaMul(SkColorGetG(color), scale);
    fSrcB = SkAlphaMul(SkColorGetB(color), scale);

    fPMColor = SkPackARGB32(fSrcA, fSrcR, fSrcG, fSrcB);
}

const SkPixmap* SkARGB32_Blitter::justAnOpaqueColor(uint32_t* value) {
    if (255 == fSrcA) {
        *value = fPMColor;
        return &fDevice;
    }
    return nullptr;
}

#if defined _WIN32 && _MSC_VER >= 1300  // disable warning : local variable used without having been initialized
#pragma warning ( push )
#pragma warning ( disable : 4701 )
#endif

void SkARGB32_Blitter::blitH(int x, int y, int width) {
    SkASSERT(x >= 0 && y >= 0 && x + width <= fDevice.width());

    uint32_t* device = fDevice.writable_addr32(x, y);
    SkBlitRow::Color32(device, device, width, fPMColor);
}

void SkARGB32_Blitter::blitAntiH(int x, int y, const SkAlpha antialias[],
                                 const int16_t runs[]) {
    if (fSrcA == 0) {
        return;
    }

    uint32_t    color = fPMColor;
    uint32_t*   device = fDevice.writable_addr32(x, y);
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
                uint32_t sc = SkAlphaMulQ(color, SkAlpha255To256(aa));
                SkBlitRow::Color32(device, device, count, sc);
            }
        }
        runs += count;
        antialias += count;
        device += count;
    }
}

void SkARGB32_Blitter::blitAntiH2(int x, int y, U8CPU a0, U8CPU a1) {
    uint32_t* device = fDevice.writable_addr32(x, y);
    SkDEBUGCODE((void)fDevice.writable_addr32(x + 1, y);)

    device[0] = SkBlendARGB32(fPMColor, device[0], a0);
    device[1] = SkBlendARGB32(fPMColor, device[1], a1);
}

void SkARGB32_Blitter::blitAntiV2(int x, int y, U8CPU a0, U8CPU a1) {
    uint32_t* device = fDevice.writable_addr32(x, y);
    SkDEBUGCODE((void)fDevice.writable_addr32(x, y + 1);)

    device[0] = SkBlendARGB32(fPMColor, device[0], a0);
    device = (uint32_t*)((char*)device + fDevice.rowBytes());
    device[0] = SkBlendARGB32(fPMColor, device[0], a1);
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
#define SK_BLITBWMASK_GETADDR               writable_addr32
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
#define SK_BLITBWMASK_GETADDR               writable_addr32
#define SK_BLITBWMASK_DEVTYPE               uint32_t
#include "SkBlitBWMaskTemplate.h"

void SkARGB32_Blitter::blitMask(const SkMask& mask, const SkIRect& clip) {
    SkASSERT(mask.fBounds.contains(clip));
    SkASSERT(fSrcA != 0xFF);

    if (fSrcA == 0) {
        return;
    }

    if (SkBlitMask::BlitColor(fDevice, mask, clip, fColor)) {
        return;
    }

    if (mask.fFormat == SkMask::kBW_Format) {
        SkARGB32_BlendBW(fDevice, mask, clip, fPMColor, SkAlpha255To256(255 - fSrcA));
    } else if (SkMask::kARGB32_Format == mask.fFormat) {
        SkARGB32_Blit32(fDevice, mask, clip, fPMColor);
    }
}

void SkARGB32_Opaque_Blitter::blitMask(const SkMask& mask,
                                       const SkIRect& clip) {
    SkASSERT(mask.fBounds.contains(clip));

    if (SkBlitMask::BlitColor(fDevice, mask, clip, fColor)) {
        return;
    }

    if (mask.fFormat == SkMask::kBW_Format) {
        SkARGB32_BlitBW(fDevice, mask, clip, fPMColor);
    } else if (SkMask::kARGB32_Format == mask.fFormat) {
        SkARGB32_Blit32(fDevice, mask, clip, fPMColor);
    }
}

void SkARGB32_Opaque_Blitter::blitAntiH2(int x, int y, U8CPU a0, U8CPU a1) {
    uint32_t* device = fDevice.writable_addr32(x, y);
    SkDEBUGCODE((void)fDevice.writable_addr32(x + 1, y);)

    device[0] = SkFastFourByteInterp(fPMColor, device[0], a0);
    device[1] = SkFastFourByteInterp(fPMColor, device[1], a1);
}

void SkARGB32_Opaque_Blitter::blitAntiV2(int x, int y, U8CPU a0, U8CPU a1) {
    uint32_t* device = fDevice.writable_addr32(x, y);
    SkDEBUGCODE((void)fDevice.writable_addr32(x, y + 1);)

    device[0] = SkFastFourByteInterp(fPMColor, device[0], a0);
    device = (uint32_t*)((char*)device + fDevice.rowBytes());
    device[0] = SkFastFourByteInterp(fPMColor, device[0], a1);
}

///////////////////////////////////////////////////////////////////////////////

void SkARGB32_Blitter::blitV(int x, int y, int height, SkAlpha alpha) {
    if (alpha == 0 || fSrcA == 0) {
        return;
    }

    uint32_t* device = fDevice.writable_addr32(x, y);
    uint32_t  color = fPMColor;

    if (alpha != 255) {
        color = SkAlphaMulQ(color, SkAlpha255To256(alpha));
    }

    unsigned dst_scale = 255 - SkGetPackedA32(color);
    size_t rowBytes = fDevice.rowBytes();
    while (--height >= 0) {
        device[0] = color + SkAlphaMulQ(device[0], dst_scale);
        device = (uint32_t*)((char*)device + rowBytes);
    }
}

void SkARGB32_Blitter::blitRect(int x, int y, int width, int height) {
    SkASSERT(x >= 0 && y >= 0 && x + width <= fDevice.width() && y + height <= fDevice.height());

    if (fSrcA == 0) {
        return;
    }

    uint32_t*   device = fDevice.writable_addr32(x, y);
    uint32_t    color = fPMColor;
    size_t      rowBytes = fDevice.rowBytes();

    while (--height >= 0) {
        SkBlitRow::Color32(device, device, width, color);
        device = (uint32_t*)((char*)device + rowBytes);
    }
}

#if defined _WIN32 && _MSC_VER >= 1300
#pragma warning ( pop )
#endif

///////////////////////////////////////////////////////////////////////

void SkARGB32_Black_Blitter::blitAntiH(int x, int y, const SkAlpha antialias[],
                                       const int16_t runs[]) {
    uint32_t*   device = fDevice.writable_addr32(x, y);
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

void SkARGB32_Black_Blitter::blitAntiH2(int x, int y, U8CPU a0, U8CPU a1) {
    uint32_t* device = fDevice.writable_addr32(x, y);
    SkDEBUGCODE((void)fDevice.writable_addr32(x + 1, y);)

    device[0] = (a0 << SK_A32_SHIFT) + SkAlphaMulQ(device[0], 256 - a0);
    device[1] = (a1 << SK_A32_SHIFT) + SkAlphaMulQ(device[1], 256 - a1);
}

void SkARGB32_Black_Blitter::blitAntiV2(int x, int y, U8CPU a0, U8CPU a1) {
    uint32_t* device = fDevice.writable_addr32(x, y);
    SkDEBUGCODE((void)fDevice.writable_addr32(x, y + 1);)

    device[0] = (a0 << SK_A32_SHIFT) + SkAlphaMulQ(device[0], 256 - a0);
    device = (uint32_t*)((char*)device + fDevice.rowBytes());
    device[0] = (a1 << SK_A32_SHIFT) + SkAlphaMulQ(device[0], 256 - a1);
}

///////////////////////////////////////////////////////////////////////////////

// Special version of SkBlitRow::Factory32 that knows we're in kSrc_Mode,
// instead of kSrcOver_Mode
static void blend_srcmode(SkPMColor* SK_RESTRICT device,
                          const SkPMColor* SK_RESTRICT span,
                          int count, U8CPU aa) {
    int aa256 = SkAlpha255To256(aa);
    for (int i = 0; i < count; ++i) {
        device[i] = SkFourByteInterp256(span[i], device[i], aa256);
    }
}

SkARGB32_Shader_Blitter::SkARGB32_Shader_Blitter(const SkPixmap& device,
        const SkPaint& paint, SkShader::Context* shaderContext)
    : INHERITED(device, paint, shaderContext)
{
    fBuffer = (SkPMColor*)sk_malloc_throw(device.width() * (sizeof(SkPMColor)));

    fXfermode = paint.getXfermode();
    SkSafeRef(fXfermode);

    int flags = 0;
    if (!(shaderContext->getFlags() & SkShader::kOpaqueAlpha_Flag)) {
        flags |= SkBlitRow::kSrcPixelAlpha_Flag32;
    }
    // we call this on the output from the shader
    fProc32 = SkBlitRow::Factory32(flags);
    // we call this on the output from the shader + alpha from the aa buffer
    fProc32Blend = SkBlitRow::Factory32(flags | SkBlitRow::kGlobalAlpha_Flag32);

    fShadeDirectlyIntoDevice = false;
    if (fXfermode == nullptr) {
        if (shaderContext->getFlags() & SkShader::kOpaqueAlpha_Flag) {
            fShadeDirectlyIntoDevice = true;
        }
    } else {
        SkXfermode::Mode mode;
        if (fXfermode->asMode(&mode)) {
            if (SkXfermode::kSrc_Mode == mode) {
                fShadeDirectlyIntoDevice = true;
                fProc32Blend = blend_srcmode;
            }
        }
    }

    fConstInY = SkToBool(shaderContext->getFlags() & SkShader::kConstInY32_Flag);
}

SkARGB32_Shader_Blitter::~SkARGB32_Shader_Blitter() {
    SkSafeUnref(fXfermode);
    sk_free(fBuffer);
}

void SkARGB32_Shader_Blitter::blitH(int x, int y, int width) {
    SkASSERT(x >= 0 && y >= 0 && x + width <= fDevice.width());

    uint32_t* device = fDevice.writable_addr32(x, y);

    if (fShadeDirectlyIntoDevice) {
        fShaderContext->shadeSpan(x, y, device, width);
    } else {
        SkPMColor*  span = fBuffer;
        fShaderContext->shadeSpan(x, y, span, width);
        if (fXfermode) {
            fXfermode->xfer32(device, span, width, nullptr);
        } else {
            fProc32(device, span, width, 255);
        }
    }
}

void SkARGB32_Shader_Blitter::blitRect(int x, int y, int width, int height) {
    SkASSERT(x >= 0 && y >= 0 &&
             x + width <= fDevice.width() && y + height <= fDevice.height());

    uint32_t*          device = fDevice.writable_addr32(x, y);
    size_t             deviceRB = fDevice.rowBytes();
    SkShader::Context* shaderContext = fShaderContext;
    SkPMColor*         span = fBuffer;

    if (fConstInY) {
        if (fShadeDirectlyIntoDevice) {
            // shade the first row directly into the device
            shaderContext->shadeSpan(x, y, device, width);
            span = device;
            while (--height > 0) {
                device = (uint32_t*)((char*)device + deviceRB);
                memcpy(device, span, width << 2);
            }
        } else {
            shaderContext->shadeSpan(x, y, span, width);
            SkXfermode* xfer = fXfermode;
            if (xfer) {
                do {
                    xfer->xfer32(device, span, width, nullptr);
                    y += 1;
                    device = (uint32_t*)((char*)device + deviceRB);
                } while (--height > 0);
            } else {
                SkBlitRow::Proc32 proc = fProc32;
                do {
                    proc(device, span, width, 255);
                    y += 1;
                    device = (uint32_t*)((char*)device + deviceRB);
                } while (--height > 0);
            }
        }
        return;
    }

    if (fShadeDirectlyIntoDevice) {
        void* ctx;
        SkShader::Context::ShadeProc shadeProc = shaderContext->asAShadeProc(&ctx);
        if (shadeProc) {
            do {
                shadeProc(ctx, x, y, device, width);
                y += 1;
                device = (uint32_t*)((char*)device + deviceRB);
            } while (--height > 0);
        } else {
            do {
                shaderContext->shadeSpan(x, y, device, width);
                y += 1;
                device = (uint32_t*)((char*)device + deviceRB);
            } while (--height > 0);
        }
    } else {
        SkXfermode* xfer = fXfermode;
        if (xfer) {
            do {
                shaderContext->shadeSpan(x, y, span, width);
                xfer->xfer32(device, span, width, nullptr);
                y += 1;
                device = (uint32_t*)((char*)device + deviceRB);
            } while (--height > 0);
        } else {
            SkBlitRow::Proc32 proc = fProc32;
            do {
                shaderContext->shadeSpan(x, y, span, width);
                proc(device, span, width, 255);
                y += 1;
                device = (uint32_t*)((char*)device + deviceRB);
            } while (--height > 0);
        }
    }
}

void SkARGB32_Shader_Blitter::blitAntiH(int x, int y, const SkAlpha antialias[],
                                        const int16_t runs[]) {
    SkPMColor*         span = fBuffer;
    uint32_t*          device = fDevice.writable_addr32(x, y);
    SkShader::Context* shaderContext = fShaderContext;

    if (fXfermode && !fShadeDirectlyIntoDevice) {
        for (;;) {
            SkXfermode* xfer = fXfermode;

            int count = *runs;
            if (count <= 0)
                break;
            int aa = *antialias;
            if (aa) {
                shaderContext->shadeSpan(x, y, span, count);
                if (aa == 255) {
                    xfer->xfer32(device, span, count, nullptr);
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
    } else if (fShadeDirectlyIntoDevice ||
               (shaderContext->getFlags() & SkShader::kOpaqueAlpha_Flag)) {
        for (;;) {
            int count = *runs;
            if (count <= 0) {
                break;
            }
            int aa = *antialias;
            if (aa) {
                if (aa == 255) {
                    // cool, have the shader draw right into the device
                    shaderContext->shadeSpan(x, y, device, count);
                } else {
                    shaderContext->shadeSpan(x, y, span, count);
                    fProc32Blend(device, span, count, aa);
                }
            }
            device += count;
            runs += count;
            antialias += count;
            x += count;
        }
    } else {
        for (;;) {
            int count = *runs;
            if (count <= 0) {
                break;
            }
            int aa = *antialias;
            if (aa) {
                shaderContext->shadeSpan(x, y, span, count);
                if (aa == 255) {
                    fProc32(device, span, count, 255);
                } else {
                    fProc32Blend(device, span, count, aa);
                }
            }
            device += count;
            runs += count;
            antialias += count;
            x += count;
        }
    }
}

void SkARGB32_Shader_Blitter::blitMask(const SkMask& mask, const SkIRect& clip) {
    // we only handle kA8 with an xfermode
    if (fXfermode && (SkMask::kA8_Format != mask.fFormat)) {
        this->INHERITED::blitMask(mask, clip);
        return;
    }

    SkASSERT(mask.fBounds.contains(clip));

    SkShader::Context*  shaderContext = fShaderContext;
    SkBlitMask::RowProc proc = nullptr;
    if (!fXfermode) {
        unsigned flags = 0;
        if (shaderContext->getFlags() & SkShader::kOpaqueAlpha_Flag) {
            flags |= SkBlitMask::kSrcIsOpaque_RowFlag;
        }
        proc = SkBlitMask::RowFactory(kN32_SkColorType, mask.fFormat,
                                      (SkBlitMask::RowFlags)flags);
        if (nullptr == proc) {
            this->INHERITED::blitMask(mask, clip);
            return;
        }
    }

    const int x = clip.fLeft;
    const int width = clip.width();
    int y = clip.fTop;
    int height = clip.height();

    char* dstRow = (char*)fDevice.writable_addr32(x, y);
    const size_t dstRB = fDevice.rowBytes();
    const uint8_t* maskRow = (const uint8_t*)mask.getAddr(x, y);
    const size_t maskRB = mask.fRowBytes;

    SkPMColor* span = fBuffer;

    if (fXfermode) {
        SkASSERT(SkMask::kA8_Format == mask.fFormat);
        SkXfermode* xfer = fXfermode;
        do {
            shaderContext->shadeSpan(x, y, span, width);
            xfer->xfer32(reinterpret_cast<SkPMColor*>(dstRow), span, width, maskRow);
            dstRow += dstRB;
            maskRow += maskRB;
            y += 1;
        } while (--height > 0);
    } else {
        do {
            shaderContext->shadeSpan(x, y, span, width);
            proc(reinterpret_cast<SkPMColor*>(dstRow), maskRow, span, width);
            dstRow += dstRB;
            maskRow += maskRB;
            y += 1;
        } while (--height > 0);
    }
}

void SkARGB32_Shader_Blitter::blitV(int x, int y, int height, SkAlpha alpha) {
    SkASSERT(x >= 0 && y >= 0 && y + height <= fDevice.height());

    uint32_t*          device = fDevice.writable_addr32(x, y);
    size_t             deviceRB = fDevice.rowBytes();
    SkShader::Context* shaderContext = fShaderContext;

    if (fConstInY) {
        SkPMColor c;
        shaderContext->shadeSpan(x, y, &c, 1);

        if (fShadeDirectlyIntoDevice) {
            if (255 == alpha) {
                do {
                    *device = c;
                    device = (uint32_t*)((char*)device + deviceRB);
                } while (--height > 0);
            } else {
                do {
                    *device = SkFourByteInterp(c, *device, alpha);
                    device = (uint32_t*)((char*)device + deviceRB);
                } while (--height > 0);
            }
        } else {
            SkXfermode* xfer = fXfermode;
            if (xfer) {
                do {
                    xfer->xfer32(device, &c, 1, &alpha);
                    device = (uint32_t*)((char*)device + deviceRB);
                } while (--height > 0);
            } else {
                SkBlitRow::Proc32 proc = (255 == alpha) ? fProc32 : fProc32Blend;
                do {
                    proc(device, &c, 1, alpha);
                    device = (uint32_t*)((char*)device + deviceRB);
                } while (--height > 0);
            }
        }
        return;
    }

    if (fShadeDirectlyIntoDevice) {
        void* ctx;
        SkShader::Context::ShadeProc shadeProc = shaderContext->asAShadeProc(&ctx);
        if (255 == alpha) {
            if (shadeProc) {
                do {
                    shadeProc(ctx, x, y, device, 1);
                    y += 1;
                    device = (uint32_t*)((char*)device + deviceRB);
                } while (--height > 0);
            } else {
                do {
                    shaderContext->shadeSpan(x, y, device, 1);
                    y += 1;
                    device = (uint32_t*)((char*)device + deviceRB);
                } while (--height > 0);
            }
        } else {    // alpha < 255
            SkPMColor c;
            if (shadeProc) {
                do {
                    shadeProc(ctx, x, y, &c, 1);
                    *device = SkFourByteInterp(c, *device, alpha);
                    y += 1;
                    device = (uint32_t*)((char*)device + deviceRB);
                } while (--height > 0);
            } else {
                do {
                    shaderContext->shadeSpan(x, y, &c, 1);
                    *device = SkFourByteInterp(c, *device, alpha);
                    y += 1;
                    device = (uint32_t*)((char*)device + deviceRB);
                } while (--height > 0);
            }
        }
    } else {
        SkPMColor* span = fBuffer;
        SkXfermode* xfer = fXfermode;
        if (xfer) {
            do {
                shaderContext->shadeSpan(x, y, span, 1);
                xfer->xfer32(device, span, 1, &alpha);
                y += 1;
                device = (uint32_t*)((char*)device + deviceRB);
            } while (--height > 0);
        } else {
            SkBlitRow::Proc32 proc = (255 == alpha) ? fProc32 : fProc32Blend;
            do {
                shaderContext->shadeSpan(x, y, span, 1);
                proc(device, span, 1, alpha);
                y += 1;
                device = (uint32_t*)((char*)device + deviceRB);
            } while (--height > 0);
        }
    }
}
