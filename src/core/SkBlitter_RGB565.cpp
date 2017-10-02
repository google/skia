/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCoreBlitters.h"
#include "SkColorData.h"
#include "SkShader.h"
#include "SkUtils.h"
#include "SkXfermodePriv.h"
#include "SkBlitMask.h"
#include "SkColorData.h"

// Special version of SkBlitRow::Factory32 that knows we're in kSrc_Mode,
// instead of kSrcOver_Mode
static void D16_S32X_blend(uint16_t* SK_RESTRICT dst,
                           const SkPMColor* SK_RESTRICT src,
                           int count) {
    for (int i = 0; i < count; ++i) {
        dst[i] = SkPixel32ToPixel16(src[i]);
    }
}

SkRGB565_Shader_Blitter::SkRGB565_Shader_Blitter(const SkPixmap& device,
        const SkPaint& paint, SkShaderBase::Context* shaderContext)
    : INHERITED(device, paint, shaderContext)
{
    fBuffer = (SkPMColor*)sk_malloc_throw(device.width() * (sizeof(SkPMColor)));

    SkASSERT(paint.getBlendMode() == SkBlendMode::kSrcOver || paint.getBlendMode() == SkBlendMode::kSrc);

    int flags = 0;
    if (!(shaderContext->getFlags() & SkShaderBase::kOpaqueAlpha_Flag)) {
        flags |= SkBlitRow::kSrcPixelAlpha_Flag32;
    }

    SkBlitRow::Factory32(flags);
}

SkRGB565_Shader_Blitter::~SkRGB565_Shader_Blitter() {
    sk_free(fBuffer);
}

void SkRGB565_Shader_Blitter::blitH(int x, int y, int width) {
    SkASSERT(x >= 0 && y >= 0 && x + width <= fDevice.width());

    uint16_t* device = fDevice.writable_addr16(x, y);

    SkPMColor*  span = fBuffer;
    fShaderContext->shadeSpan(x, y, span, width);
    D16_S32X_blend(device, span, width);
}

void SkRGB565_Shader_Blitter::blitRect(int x, int y, int width, int height) {
    SkASSERT(x >= 0 && y >= 0 &&
             x + width <= fDevice.width() && y + height <= fDevice.height());

    uint16_t* device = fDevice.writable_addr16(x, y);
    size_t     deviceRB = fDevice.rowBytes();
    auto*      shaderContext = fShaderContext;
    SkPMColor* span = fBuffer;

    do {
        shaderContext->shadeSpan(x, y, span, width);
        D16_S32X_blend(device, span, width);
        y += 1;
        device = (uint16_t*)((char*)device + deviceRB);
    } while (--height > 0);
}

void SkRGB565_Shader_Blitter::blitAntiH(int x, int y, const SkAlpha antialias[],
                                        const int16_t runs[]) {
    SkASSERT(false); return;
#if 0
    SkPMColor* span = fBuffer;
    uint32_t*  device = fDevice.writable_addr32(x, y);
    auto*      shaderContext = fShaderContext;

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
               (shaderContext->getFlags() & SkShaderBase::kOpaqueAlpha_Flag)) {
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
#endif
}

void SkRGB565_Shader_Blitter::blitMask(const SkMask& mask, const SkIRect& clip) {
    SkASSERT(false); return;
#if 0
    // we only handle kA8 with an xfermode
    if (fXfermode && (SkMask::kA8_Format != mask.fFormat)) {
        this->INHERITED::blitMask(mask, clip);
        return;
    }

    SkASSERT(mask.fBounds.contains(clip));

    auto* shaderContext = fShaderContext;
    SkBlitMask::RowProc proc = nullptr;
    if (!fXfermode) {
        unsigned flags = 0;
        if (shaderContext->getFlags() & SkShaderBase::kOpaqueAlpha_Flag) {
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
#endif
}

void SkRGB565_Shader_Blitter::blitV(int x, int y, int height, SkAlpha alpha) {
    SkASSERT(false); return;
#if 0
    SkASSERT(x >= 0 && y >= 0 && y + height <= fDevice.height());

    uint32_t* device = fDevice.writable_addr32(x, y);
    size_t    deviceRB = fDevice.rowBytes();
    auto*     shaderContext = fShaderContext;

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
        if (255 == alpha) {
            do {
                shaderContext->shadeSpan(x, y, device, 1);
                y += 1;
                device = (uint32_t*)((char*)device + deviceRB);
            } while (--height > 0);
        } else {
            do {
                SkPMColor c;
                shaderContext->shadeSpan(x, y, &c, 1);
                *device = SkFourByteInterp(c, *device, alpha);
                y += 1;
                device = (uint32_t*)((char*)device + deviceRB);
            } while (--height > 0);
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
#endif
}
