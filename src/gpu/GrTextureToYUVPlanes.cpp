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
#include "GrPaint.h"
#include "GrRenderTargetContext.h"
#include "GrResourceProvider.h"

namespace {
    using MakeFPProc = sk_sp<GrFragmentProcessor> (*)(sk_sp<GrFragmentProcessor>,
                                                      SkYUVColorSpace colorSpace);
};

static bool convert_proxy(sk_sp<GrTextureProxy> src,
                          GrRenderTargetContext* dst, int dstW, int dstH,
                          SkYUVColorSpace colorSpace, MakeFPProc proc) {

    SkScalar xScale = SkIntToScalar(src->width()) / dstW;
    SkScalar yScale = SkIntToScalar(src->height()) / dstH;
    GrSamplerParams::FilterMode filter;
    if (dstW == src->width() && dstW == src->height()) {
        filter = GrSamplerParams::kNone_FilterMode;
    } else {
        filter = GrSamplerParams::kBilerp_FilterMode;
    }

    GrResourceProvider* resourceProvider = dst->resourceProvider();

    sk_sp<GrFragmentProcessor> fp(GrSimpleTextureEffect::Make(resourceProvider, std::move(src),
                                                              nullptr,
                                                              SkMatrix::MakeScale(xScale, yScale),
                                                              filter));
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
    dst->drawRect(GrNoClip(), std::move(paint), GrAA::kNo, SkMatrix::I(),
                  SkRect::MakeIWH(dstW, dstH));
    return true;
}

bool GrTextureToYUVPlanes(GrContext* context, sk_sp<GrTextureProxy> proxy,
                          const SkISize sizes[3], void* const planes[3],
                          const size_t rowBytes[3], SkYUVColorSpace colorSpace) {
    if (!context) {
        return false;
    }

    {
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
            if (!convert_proxy(std::move(proxy), yuvRenderTargetContext.get(),
                               sizes[0].fWidth, sizes[0].fHeight,
                               colorSpace, GrYUVEffect::MakeRGBToYUV)) {
                return false;
            }
        } else {
            SkASSERT(yRenderTargetContext);
            if (!convert_proxy(proxy, yRenderTargetContext.get(),
                               sizes[0].fWidth, sizes[0].fHeight,
                               colorSpace, GrYUVEffect::MakeRGBToY)) {
                return false;
            }
            if (uvRenderTargetContext) {
                if (!convert_proxy(std::move(proxy), uvRenderTargetContext.get(),
                                   sizes[1].fWidth, sizes[1].fHeight,
                                   colorSpace,  GrYUVEffect::MakeRGBToUV)) {
                    return false;
                }
            } else {
                SkASSERT(uRenderTargetContext && vRenderTargetContext);
                if (!convert_proxy(proxy, uRenderTargetContext.get(),
                                   sizes[1].fWidth, sizes[1].fHeight,
                                   colorSpace, GrYUVEffect::MakeRGBToU)) {
                    return false;
                }
                if (!convert_proxy(std::move(proxy), vRenderTargetContext.get(),
                                   sizes[2].fWidth, sizes[2].fHeight,
                                   colorSpace, GrYUVEffect::MakeRGBToV)) {
                    return false;
                }
            }
        }

        if (yuvRenderTargetContext) {
            SkASSERT(sizes[0] == sizes[1] && sizes[1] == sizes[2]);
            SkISize yuvSize = sizes[0];
            // We have no kRGB_888 pixel format, so readback rgba and then copy three channels.
            SkAutoSTMalloc<128 * 128, uint32_t> tempYUV(yuvSize.fWidth * yuvSize.fHeight);

            const SkImageInfo ii = SkImageInfo::Make(yuvSize.fWidth, yuvSize.fHeight,
                                                     kRGBA_8888_SkColorType, kOpaque_SkAlphaType);
            if (!yuvRenderTargetContext->readPixels(ii, tempYUV.get(), 0, 0, 0)) {
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

            SkImageInfo ii = SkImageInfo::MakeA8(sizes[0].fWidth, sizes[0].fHeight);
            if (!yRenderTargetContext->readPixels(ii, planes[0], rowBytes[0], 0, 0)) {
                return false;
            }

            if (uvRenderTargetContext) {
                SkASSERT(sizes[1].fWidth == sizes[2].fWidth);
                SkISize uvSize = sizes[1];
                // We have no kRG_88 pixel format, so readback rgba and then copy two channels.
                SkAutoSTMalloc<128 * 128, uint32_t> tempUV(uvSize.fWidth * uvSize.fHeight);

                ii = SkImageInfo::Make(uvSize.fWidth, uvSize.fHeight,
                                       kRGBA_8888_SkColorType, kOpaque_SkAlphaType);

                if (!uvRenderTargetContext->readPixels(ii, tempUV.get(), 0, 0, 0)) {
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

                ii = SkImageInfo::MakeA8(sizes[1].fWidth, sizes[1].fHeight);
                if (!uRenderTargetContext->readPixels(ii, planes[1], rowBytes[1], 0, 0)) {
                    return false;
                }

                ii = SkImageInfo::MakeA8(sizes[2].fWidth, sizes[2].fHeight);
                if (!vRenderTargetContext->readPixels(ii, planes[2], rowBytes[2], 0, 0)) {
                    return false;
                }

                return true;
            }
        }
    }
    return false;
}
