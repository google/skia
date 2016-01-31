/*
 * Copyright 2016 Google Inc.
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

//////////////////////////////////////////////////////////////////////////////////////

SkARGB32_Shader4f_Blitter::SkARGB32_Shader4f_Blitter(const SkPixmap& device,
        const SkPaint& paint, SkShader::Context* shaderContext)
    : INHERITED(device, paint, shaderContext)
{
    const uint32_t shaderFlags = shaderContext->getFlags();

    SkASSERT(shaderFlags & SkShader::kSupports4f_Flag);

    fBuffer = (SkPM4f*)sk_malloc_throw(device.width() * (sizeof(SkPM4f)));

    fState.fXfer = SkSafeRef(paint.getXfermode());
    fState.fFlags = 0;
    if (shaderFlags & SkShader::kOpaqueAlpha_Flag) {
        fState.fFlags |= SkXfermode::kSrcIsOpaque_PM4fFlag;
    }
    if (device.info().isSRGB()) {
        fState.fFlags |= SkXfermode::kDstIsSRGB_PM4fFlag;
    }
    if (fState.fXfer) {
        fProc1 = fState.fXfer->getPM4fProc1(fState.fFlags);
        fProcN = fState.fXfer->getPM4fProcN(fState.fFlags);
    } else {
        fProc1 = SkXfermode::GetPM4fProc1(SkXfermode::kSrcOver_Mode, fState.fFlags);
        fProcN = SkXfermode::GetPM4fProcN(SkXfermode::kSrcOver_Mode, fState.fFlags);
    }

    fConstInY = SkToBool(shaderFlags & SkShader::kConstInY32_Flag);
}

SkARGB32_Shader4f_Blitter::~SkARGB32_Shader4f_Blitter() {
    SkSafeUnref(fState.fXfer);
    sk_free(fBuffer);
}

void SkARGB32_Shader4f_Blitter::blitH(int x, int y, int width) {
    SkASSERT(x >= 0 && y >= 0 && x + width <= fDevice.width());

    uint32_t* device = fDevice.writable_addr32(x, y);
    fShaderContext->shadeSpan4f(x, y, fBuffer, width);
    fProcN(fState, device, fBuffer, width, nullptr);
}

void SkARGB32_Shader4f_Blitter::blitRect(int x, int y, int width, int height) {
    SkASSERT(x >= 0 && y >= 0 &&
             x + width <= fDevice.width() && y + height <= fDevice.height());

    uint32_t*   device = fDevice.writable_addr32(x, y);
    size_t      deviceRB = fDevice.rowBytes();

    if (fConstInY) {
        fShaderContext->shadeSpan4f(x, y, fBuffer, width);
        do {
            fProcN(fState, device, fBuffer, width, nullptr);
            y += 1;
            device = (uint32_t*)((char*)device + deviceRB);
        } while (--height > 0);
    } else {
        do {
            fShaderContext->shadeSpan4f(x, y, fBuffer, width);
            fProcN(fState, device, fBuffer, width, nullptr);
            y += 1;
            device = (uint32_t*)((char*)device + deviceRB);
        } while (--height > 0);
    }
}

void SkARGB32_Shader4f_Blitter::blitAntiH(int x, int y, const SkAlpha antialias[],
                                        const int16_t runs[]) {
    uint32_t* device = fDevice.writable_addr32(x, y);

    for (;;) {
        int count = *runs;
        if (count <= 0) {
            break;
        }
        int aa = *antialias;
        if (aa) {
            fShaderContext->shadeSpan4f(x, y, fBuffer, count);
            if (aa == 255) {
                fProcN(fState, device, fBuffer, count, nullptr);
            } else {
                // count is almost always 1
                for (int i = count - 1; i >= 0; --i) {
                    fProcN(fState, &device[i], &fBuffer[i], 1, antialias);
                }
            }
        }
        device += count;
        runs += count;
        antialias += count;
        x += count;
    }
}

void SkARGB32_Shader4f_Blitter::blitMask(const SkMask& mask, const SkIRect& clip) {
    // we only handle kA8
    if (SkMask::kA8_Format != mask.fFormat) {
        this->INHERITED::blitMask(mask, clip);
        return;
    }

    SkASSERT(mask.fBounds.contains(clip));

    const int x = clip.fLeft;
    const int width = clip.width();
    int y = clip.fTop;
    int height = clip.height();

    char* dstRow = (char*)fDevice.writable_addr32(x, y);
    const size_t dstRB = fDevice.rowBytes();
    const uint8_t* maskRow = (const uint8_t*)mask.getAddr(x, y);
    const size_t maskRB = mask.fRowBytes;

    do {
        fShaderContext->shadeSpan4f(x, y, fBuffer, width);
        fProcN(fState, reinterpret_cast<SkPMColor*>(dstRow), fBuffer, width, maskRow);
        dstRow += dstRB;
        maskRow += maskRB;
        y += 1;
    } while (--height > 0);
}

void SkARGB32_Shader4f_Blitter::blitV(int x, int y, int height, SkAlpha alpha) {
    SkASSERT(x >= 0 && y >= 0 && y + height <= fDevice.height());

    uint32_t*   device = fDevice.writable_addr32(x, y);
    size_t      deviceRB = fDevice.rowBytes();

    if (fConstInY) {
        fShaderContext->shadeSpan4f(x, y, fBuffer, 1);
        do {
            fProcN(fState, device, fBuffer, 1, &alpha);
            device = (uint32_t*)((char*)device + deviceRB);
        } while (--height > 0);
    } else {
        do {
            fShaderContext->shadeSpan4f(x, y, fBuffer, 1);
            fProcN(fState, device, fBuffer, 1, &alpha);
            y += 1;
            device = (uint32_t*)((char*)device + deviceRB);
        } while (--height > 0);
    }
}
