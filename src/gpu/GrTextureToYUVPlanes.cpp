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
#include "GrDrawContext.h"
#include "GrPaint.h"
#include "GrTextureProvider.h"

namespace {
    using MakeFPProc = sk_sp<GrFragmentProcessor> (*)(sk_sp<GrFragmentProcessor>,
                                                      SkYUVColorSpace colorSpace);
};

static bool convert_texture(GrTexture* src, GrDrawContext* dst, int dstW, int dstH,
                            SkYUVColorSpace colorSpace, MakeFPProc proc) {

    SkScalar xScale = SkIntToScalar(src->width()) / dstW / src->width();
    SkScalar yScale = SkIntToScalar(src->height()) / dstH / src->height();
    GrTextureParams::FilterMode filter;
    if (dstW == src->width() && dstW == src->height()) {
        filter = GrTextureParams::kNone_FilterMode;
    } else {
        filter = GrTextureParams::kBilerp_FilterMode;
    }

    sk_sp<GrFragmentProcessor> fp(
            GrSimpleTextureEffect::Make(src, SkMatrix::MakeScale(xScale, yScale), filter));
    if (!fp) {
        return false;
    }
    fp = proc(std::move(fp), colorSpace);
    if (!fp) {
        return false;
    }
    GrPaint paint;
    paint.setPorterDuffXPFactory(SkXfermode::kSrc_Mode);
    paint.addColorFragmentProcessor(std::move(fp));
    dst->drawRect(GrNoClip(), paint, SkMatrix::I(), SkRect::MakeIWH(dstW, dstH));
    return true;
}

bool GrTextureToYUVPlanes(GrTexture* texture, const SkISize sizes[3], void* const planes[3],
                          const size_t rowBytes[3], SkYUVColorSpace colorSpace) {
    if (GrContext* context = texture->getContext()) {
        // Depending on the relative sizes of the y, u, and v planes we may do 1 to 3 draws/
        // readbacks.
        sk_sp<GrDrawContext> yuvDrawContext;
        sk_sp<GrDrawContext> yDrawContext;
        sk_sp<GrDrawContext> uvDrawContext;
        sk_sp<GrDrawContext> uDrawContext;
        sk_sp<GrDrawContext> vDrawContext;

        GrPixelConfig singleChannelPixelConfig;
        if (context->caps()->isConfigRenderable(kAlpha_8_GrPixelConfig, false)) {
            singleChannelPixelConfig = kAlpha_8_GrPixelConfig;
        } else {
            singleChannelPixelConfig = kRGBA_8888_GrPixelConfig;
        }

        // We issue draw(s) to convert from RGBA to Y, U, and V. All three planes may have different
        // sizes however we optimize for two other cases - all planes are the same (1 draw to YUV),
        // and U and V are the same but Y differs (2 draws, one for Y, one for UV).
        if (sizes[0] == sizes[1] && sizes[1] == sizes[2]) {
            yuvDrawContext = context->newDrawContext(SkBackingFit::kApprox,
                                                     sizes[0].fWidth, sizes[0].fHeight,
                                                     kRGBA_8888_GrPixelConfig);
            if (!yuvDrawContext) {
                return false;
            }
        } else {
            yDrawContext = context->newDrawContext(SkBackingFit::kApprox,
                                                   sizes[0].fWidth, sizes[0].fHeight,
                                                   singleChannelPixelConfig);
            if (!yDrawContext) {
                return false;
            }
            if (sizes[1] == sizes[2]) {
                // TODO: Add support for GL_RG when available.
                uvDrawContext = context->newDrawContext(SkBackingFit::kApprox,
                                                        sizes[1].fWidth, sizes[1].fHeight,
                                                        kRGBA_8888_GrPixelConfig);
                if (!uvDrawContext) {
                    return false;
                }
            } else {
                uDrawContext = context->newDrawContext(SkBackingFit::kApprox,
                                                       sizes[1].fWidth, sizes[1].fHeight,
                                                       singleChannelPixelConfig);
                vDrawContext = context->newDrawContext(SkBackingFit::kApprox,
                                                       sizes[2].fWidth, sizes[2].fHeight,
                                                       singleChannelPixelConfig);
                if (!uDrawContext || !vDrawContext) {
                    return false;
                }
            }
        }

        // Do all the draws before any readback.
        if (yuvDrawContext) {
            if (!convert_texture(texture, yuvDrawContext.get(),
                                 sizes[0].fWidth, sizes[0].fHeight,
                                 colorSpace, GrYUVEffect::MakeRGBToYUV)) {
                return false;
            }
        } else {
            SkASSERT(yDrawContext);
            if (!convert_texture(texture, yDrawContext.get(),
                                 sizes[0].fWidth, sizes[0].fHeight,
                                 colorSpace, GrYUVEffect::MakeRGBToY)) {
                return false;
            }
            if (uvDrawContext) {
                if (!convert_texture(texture, uvDrawContext.get(),
                                     sizes[1].fWidth, sizes[1].fHeight,
                                     colorSpace,  GrYUVEffect::MakeRGBToUV)) {
                    return false;
                }
            } else {
                SkASSERT(uDrawContext && vDrawContext);
                if (!convert_texture(texture, uDrawContext.get(),
                                     sizes[1].fWidth, sizes[1].fHeight,
                                     colorSpace, GrYUVEffect::MakeRGBToU)) {
                    return false;
                }
                if (!convert_texture(texture, vDrawContext.get(),
                                     sizes[2].fWidth, sizes[2].fHeight,
                                     colorSpace, GrYUVEffect::MakeRGBToV)) {
                    return false;
                }
            }
        }

        if (yuvDrawContext) {
            SkASSERT(sizes[0] == sizes[1] && sizes[1] == sizes[2]);
            sk_sp<GrTexture> yuvTex(yuvDrawContext->asTexture());
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
            SkASSERT(yDrawContext);
            sk_sp<GrTexture> yTex(yDrawContext->asTexture());
            SkASSERT(yTex);
            if (!yTex->readPixels(0, 0, sizes[0].fWidth, sizes[0].fHeight,
                                  kAlpha_8_GrPixelConfig, planes[0], rowBytes[0])) {
                return false;
            }
            if (uvDrawContext) {
                SkASSERT(sizes[1].fWidth == sizes[2].fWidth);
                sk_sp<GrTexture> uvTex(uvDrawContext->asTexture());
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
                SkASSERT(uDrawContext && vDrawContext);
                sk_sp<GrTexture> tex(uDrawContext->asTexture());
                SkASSERT(tex);
                if (!tex->readPixels(0, 0, sizes[1].fWidth, sizes[1].fHeight,
                                     kAlpha_8_GrPixelConfig, planes[1], rowBytes[1])) {
                    return false;
                }
                tex = vDrawContext->asTexture();
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
