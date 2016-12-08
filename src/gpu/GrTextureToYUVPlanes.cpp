/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextureToYUVPlanes.h"
#include "effects/GrSimpleTextureEffect.h"
#include "effects/GrYUVEffect.h"
#include "GrClip.h"
#include "GrContext.h"
#include "GrRenderTargetContext.h"
#include "GrPaint.h"
#include "GrTextureProvider.h"

namespace {
    using MakeFPProc = sk_sp<GrFragmentProcessor> (*)(sk_sp<GrFragmentProcessor>,
                                                      SkYUVColorSpace colorSpace);
};

static bool convert_texture(GrTexture* src, GrRenderTargetContext* dst, int dstW, int dstH,
                            SkYUVColorSpace colorSpace, MakeFPProc proc) {

    SkScalar xScale = SkIntToScalar(src->width()) / dstW / src->width();
    SkScalar yScale = SkIntToScalar(src->height()) / dstH / src->height();
    GrSamplerParams::FilterMode filter;
    if (dstW == src->width() && dstW == src->height()) {
        filter = GrSamplerParams::kNone_FilterMode;
    } else {
        filter = GrSamplerParams::kBilerp_FilterMode;
    }

    sk_sp<GrFragmentProcessor> fp(
            GrSimpleTextureEffect::Make(src, nullptr, SkMatrix::MakeScale(xScale, yScale), filter));
    if (!fp) {
        return false;
    }
    fp = proc(std::move(fp), colorSpace);
    if (!fp) {
        return false;
    }
    GrPaint paint;
    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
    paint.addColorFragmentProcessor(std::move(fp));
    dst->drawRect(GrNoClip(), paint, SkMatrix::I(), SkRect::MakeIWH(dstW, dstH));
    return true;
}

bool GrTextureToYUVPlanes(GrTexture* texture, const SkISize sizes[3], void* const planes[3],
                          const size_t rowBytes[3], SkYUVColorSpace colorSpace) {
    if (GrContext* context = texture->getContext()) {
        // Depending on the relative sizes of the y, u, and v planes we may do 1 to 3 draws/
        // readbacks.
        sk_sp<GrRenderTargetContext> yuvRenderTargetContext;
        sk_sp<GrRenderTargetContext> yRenderTargetContext;
        sk_sp<GrRenderTargetContext> uvRenderTargetContext;
        sk_sp<GrRenderTargetContext> uRenderTargetContext;
        sk_sp<GrRenderTargetContext> vRenderTargetContext;

        // We issue draw(s) to convert from RGBA to Y, U, and V. All three planes may have different
        // sizes however we optimize for two other cases - all planes are the same (1 draw to YUV),
        // and U and V are the same but Y differs (2 draws, one for Y, one for UV).
        if (sizes[0] == sizes[1] && sizes[1] == sizes[2]) {
            yuvRenderTargetContext = context->makeRenderTargetContextWithFallback(
                                                                           SkBackingFit::kApprox,
                                                                           sizes[0].fWidth,
                                                                           sizes[0].fHeight,
                                                                           kRGBA_8888_GrPixelConfig,
                                                                           nullptr);
            if (!yuvRenderTargetContext) {
                return false;
            }
        } else {
            yRenderTargetContext = context->makeRenderTargetContextWithFallback(
                                                                             SkBackingFit::kApprox,
                                                                             sizes[0].fWidth,
                                                                             sizes[0].fHeight,
                                                                             kAlpha_8_GrPixelConfig,
                                                                             nullptr);
            if (!yRenderTargetContext) {
                return false;
            }
            if (sizes[1] == sizes[2]) {
                // TODO: Add support for GL_RG when available.
                uvRenderTargetContext = context->makeRenderTargetContextWithFallback(
                                                                           SkBackingFit::kApprox,
                                                                           sizes[1].fWidth,
                                                                           sizes[1].fHeight,
                                                                           kRGBA_8888_GrPixelConfig,
                                                                           nullptr);
                if (!uvRenderTargetContext) {
                    return false;
                }
            } else {
                uRenderTargetContext = context->makeRenderTargetContextWithFallback(
                                                                             SkBackingFit::kApprox,
                                                                             sizes[1].fWidth,
                                                                             sizes[1].fHeight,
                                                                             kAlpha_8_GrPixelConfig,
                                                                             nullptr);
                vRenderTargetContext = context->makeRenderTargetContextWithFallback(
                                                                             SkBackingFit::kApprox,
                                                                             sizes[2].fWidth,
                                                                             sizes[2].fHeight,
                                                                             kAlpha_8_GrPixelConfig,
                                                                             nullptr);
                if (!uRenderTargetContext || !vRenderTargetContext) {
                    return false;
                }
            }
        }

        // Do all the draws before any readback.
        if (yuvRenderTargetContext) {
            if (!convert_texture(texture, yuvRenderTargetContext.get(),
                                 sizes[0].fWidth, sizes[0].fHeight,
                                 colorSpace, GrYUVEffect::MakeRGBToYUV)) {
                return false;
            }
        } else {
            SkASSERT(yRenderTargetContext);
            if (!convert_texture(texture, yRenderTargetContext.get(),
                                 sizes[0].fWidth, sizes[0].fHeight,
                                 colorSpace, GrYUVEffect::MakeRGBToY)) {
                return false;
            }
            if (uvRenderTargetContext) {
                if (!convert_texture(texture, uvRenderTargetContext.get(),
                                     sizes[1].fWidth, sizes[1].fHeight,
                                     colorSpace,  GrYUVEffect::MakeRGBToUV)) {
                    return false;
                }
            } else {
                SkASSERT(uRenderTargetContext && vRenderTargetContext);
                if (!convert_texture(texture, uRenderTargetContext.get(),
                                     sizes[1].fWidth, sizes[1].fHeight,
                                     colorSpace, GrYUVEffect::MakeRGBToU)) {
                    return false;
                }
                if (!convert_texture(texture, vRenderTargetContext.get(),
                                     sizes[2].fWidth, sizes[2].fHeight,
                                     colorSpace, GrYUVEffect::MakeRGBToV)) {
                    return false;
                }
            }
        }

        if (yuvRenderTargetContext) {
            SkASSERT(sizes[0] == sizes[1] && sizes[1] == sizes[2]);
            sk_sp<GrTexture> yuvTex(yuvRenderTargetContext->asTexture());
            SkASSERT(yuvTex);
            SkISize yuvSize = sizes[0];
            // We have no kRGB_888 pixel format, so readback rgba and then copy three channels.
            SkAutoSTMalloc<128 * 128, uint32_t> tempYUV(yuvSize.fWidth * yuvSize.fHeight);
            if (!yuvTex->readPixels(0, 0, yuvSize.fWidth, yuvSize.fHeight,
                                    kRGBA_8888_GrPixelConfig, tempYUV.get(), 0)) {
                return false;
            }
            size_t yRowBytes = rowBytes[0] ? rowBytes[0] : yuvSize.fWidth;
            size_t uRowBytes = rowBytes[1] ? rowBytes[1] : yuvSize.fWidth;
            size_t vRowBytes = rowBytes[2] ? rowBytes[2] : yuvSize.fWidth;
            if (yRowBytes < (size_t)yuvSize.fWidth || uRowBytes < (size_t)yuvSize.fWidth ||
                vRowBytes < (size_t)yuvSize.fWidth) {
                return false;
            }
            for (int j = 0; j < yuvSize.fHeight; ++j) {
                for (int i = 0; i < yuvSize.fWidth; ++i) {
                    // These writes could surely be made more efficient.
                    uint32_t y = GrColorUnpackR(tempYUV.get()[j * yuvSize.fWidth + i]);
                    uint32_t u = GrColorUnpackG(tempYUV.get()[j * yuvSize.fWidth + i]);
                    uint32_t v = GrColorUnpackB(tempYUV.get()[j * yuvSize.fWidth + i]);
                    uint8_t* yLoc = ((uint8_t*)planes[0]) + j * yRowBytes + i;
                    uint8_t* uLoc = ((uint8_t*)planes[1]) + j * uRowBytes + i;
                    uint8_t* vLoc = ((uint8_t*)planes[2]) + j * vRowBytes + i;
                    *yLoc = y;
                    *uLoc = u;
                    *vLoc = v;
                }
            }
            return true;
        } else {
            SkASSERT(yRenderTargetContext);
            sk_sp<GrTexture> yTex(yRenderTargetContext->asTexture());
            SkASSERT(yTex);
            if (!yTex->readPixels(0, 0, sizes[0].fWidth, sizes[0].fHeight,
                                  kAlpha_8_GrPixelConfig, planes[0], rowBytes[0])) {
                return false;
            }
            if (uvRenderTargetContext) {
                SkASSERT(sizes[1].fWidth == sizes[2].fWidth);
                sk_sp<GrTexture> uvTex(uvRenderTargetContext->asTexture());
                SkASSERT(uvTex);
                SkISize uvSize = sizes[1];
                // We have no kRG_88 pixel format, so readback rgba and then copy two channels.
                SkAutoSTMalloc<128 * 128, uint32_t> tempUV(uvSize.fWidth * uvSize.fHeight);
                if (!uvTex->readPixels(0, 0, uvSize.fWidth, uvSize.fHeight,
                                       kRGBA_8888_GrPixelConfig, tempUV.get(), 0)) {
                    return false;
                }

                size_t uRowBytes = rowBytes[1] ? rowBytes[1] : uvSize.fWidth;
                size_t vRowBytes = rowBytes[2] ? rowBytes[2] : uvSize.fWidth;
                if (uRowBytes < (size_t)uvSize.fWidth || vRowBytes < (size_t)uvSize.fWidth) {
                    return false;
                }
                for (int j = 0; j < uvSize.fHeight; ++j) {
                    for (int i = 0; i < uvSize.fWidth; ++i) {
                        // These writes could surely be made more efficient.
                        uint32_t u = GrColorUnpackR(tempUV.get()[j * uvSize.fWidth + i]);
                        uint32_t v = GrColorUnpackG(tempUV.get()[j * uvSize.fWidth + i]);
                        uint8_t* uLoc = ((uint8_t*)planes[1]) + j * uRowBytes + i;
                        uint8_t* vLoc = ((uint8_t*)planes[2]) + j * vRowBytes + i;
                        *uLoc = u;
                        *vLoc = v;
                    }
                }
                return true;
            } else {
                SkASSERT(uRenderTargetContext && vRenderTargetContext);
                sk_sp<GrTexture> tex(uRenderTargetContext->asTexture());
                SkASSERT(tex);
                if (!tex->readPixels(0, 0, sizes[1].fWidth, sizes[1].fHeight,
                                     kAlpha_8_GrPixelConfig, planes[1], rowBytes[1])) {
                    return false;
                }
                tex = vRenderTargetContext->asTexture();
                SkASSERT(tex);
                if (!tex->readPixels(0, 0, sizes[2].fWidth, sizes[2].fHeight,
                                     kAlpha_8_GrPixelConfig, planes[2], rowBytes[2])) {
                    return false;
                }
                return true;
            }
        }
    }
    return false;
}
