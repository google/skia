/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkShader.h"
#include "include/private/SkColorData.h"
#include "include/private/SkColorData.h"
#include "src/core/SkCoreBlitters.h"
#include "src/core/SkXfermodePriv.h"

#include "include/private/SkNx.h"

static void D16_S32X_src(uint16_t dst[], const SkPMColor src[], int count, uint8_t coverage) {
    SkASSERT(coverage == 0xFF);
    for (int i = 0; i < count; ++i) {
        dst[i] = SkPixel32ToPixel16(src[i]);
    }
}

static void D16_S32X_src_coverage(uint16_t dst[], const SkPMColor src[], int count,
                                  uint8_t coverage) {
    switch (coverage) {
        case 0: break;
        case 0xFF:
            for (int i = 0; i < count; ++i) {
                dst[i] = SkPixel32ToPixel16(src[i]);
            }
            break;
        default:
            unsigned scale = coverage + (coverage >> 7);
            for (int i = 0; i < count; ++i) {
                dst[i] = SkSrcOver32To16(SkAlphaMulQ(src[i], scale), dst[i]);
            }
            break;
    }
}

static void D16_S32A_srcover(uint16_t dst[], const SkPMColor src[], int count, uint8_t coverage) {
    SkASSERT(coverage == 0xFF);
    for (int i = 0; i < count; ++i) {
        dst[i] = SkSrcOver32To16(src[i], dst[i]);
    }
}

static void D16_S32A_srcover_coverage(uint16_t dst[], const SkPMColor src[], int count,
                                      uint8_t coverage) {
    switch (coverage) {
        case 0: break;
        case 0xFF:
            for (int i = 0; i < count; ++i) {
                dst[i] = SkSrcOver32To16(src[i], dst[i]);
            }
            break;
        default:
            unsigned scale = coverage + (coverage >> 7);
            for (int i = 0; i < count; ++i) {
                dst[i] = SkSrcOver32To16(SkAlphaMulQ(src[i], scale), dst[i]);
            }
            break;
    }
}

bool SkRGB565_Shader_Blitter::Supports(const SkPixmap& device, const SkPaint& paint) {
    if (device.colorType() != kRGB_565_SkColorType) {
        return false;
    }
    if (device.colorSpace()) {
        return false;
    }
    if (paint.getBlendMode() != SkBlendMode::kSrcOver &&
        paint.getBlendMode() != SkBlendMode::kSrc) {
        return false;
    }
    if (paint.isDither()) {
        return false;
    }
    return true;
}

SkRGB565_Shader_Blitter::SkRGB565_Shader_Blitter(const SkPixmap& device,
        const SkPaint& paint, SkShaderBase::Context* shaderContext)
    : INHERITED(device, paint, shaderContext)
{
    SkASSERT(shaderContext);
    SkASSERT(Supports(device, paint));

    fBuffer = (SkPMColor*)sk_malloc_throw(device.width() * (sizeof(SkPMColor)));

    bool isOpaque = SkToBool(shaderContext->getFlags() & SkShaderBase::kOpaqueAlpha_Flag);

    if (paint.getBlendMode() == SkBlendMode::kSrc || isOpaque) {
        fBlend = D16_S32X_src;
        fBlendCoverage = D16_S32X_src_coverage;
    } else {    // srcover
        fBlend = isOpaque ? D16_S32X_src : D16_S32A_srcover;
        fBlendCoverage = isOpaque ? D16_S32X_src_coverage : D16_S32A_srcover_coverage;
    }
}

SkRGB565_Shader_Blitter::~SkRGB565_Shader_Blitter() {
    sk_free(fBuffer);
}

void SkRGB565_Shader_Blitter::blitH(int x, int y, int width) {
    SkASSERT(x >= 0 && y >= 0 && x + width <= fDevice.width());

    uint16_t* device = fDevice.writable_addr16(x, y);

    SkPMColor*  span = fBuffer;
    fShaderContext->shadeSpan(x, y, span, width);
    fBlend(device, span, width, 0xFF);
}

void SkRGB565_Shader_Blitter::blitAntiH(int x, int y, const SkAlpha coverage[],
                                        const int16_t runs[]) {
    SkPMColor* span = fBuffer;
    uint16_t*  device = fDevice.writable_addr16(x, y);
    auto*      shaderContext = fShaderContext;

    for (;;) {
        int count = *runs;
        if (count <= 0) {
            break;
        }
        int aa = *coverage;
        if (aa) {
            shaderContext->shadeSpan(x, y, span, count);
            fBlendCoverage(device, span, count, aa);
        }
        device += count;
        runs += count;
        coverage += count;
        x += count;
    }
}
