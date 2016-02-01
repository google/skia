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
    using CreateFPProc = const GrFragmentProcessor* (*)(const GrFragmentProcessor*,
                                                        SkYUVColorSpace colorSpace);
};

static bool convert_texture(GrTexture* src, GrDrawContext* dst, int dstW, int dstH,
                            SkYUVColorSpace colorSpace, CreateFPProc proc) {

    SkScalar xScale = SkIntToScalar(src->width()) / dstW / src->width();
    SkScalar yScale = SkIntToScalar(src->height()) / dstH / src->height();
    GrTextureParams::FilterMode filter;
    if (dstW == src->width() && dstW == src->height()) {
        filter = GrTextureParams::kNone_FilterMode;
    } else {
        filter = GrTextureParams::kBilerp_FilterMode;
    }

    SkAutoTUnref<const GrFragmentProcessor> fp(
            GrSimpleTextureEffect::Create(src, SkMatrix::MakeScale(xScale, yScale), filter));
    if (!fp) {
        return false;
    }
    fp.reset(proc(fp, colorSpace));
    if (!fp) {
        return false;
    }
    GrPaint paint;
    paint.setPorterDuffXPFactory(SkXfermode::kSrc_Mode);
    paint.addColorFragmentProcessor(fp);
    dst->drawRect(GrClip::WideOpen(), paint, SkMatrix::I(), SkRect::MakeIWH(dstW, dstH));
    return true;
}

bool GrTextureToYUVPlanes(GrTexture* texture, const SkISize sizes[3], void* const planes[3],
                          const size_t rowBytes[3], SkYUVColorSpace colorSpace) {
    if (GrContext* context = texture->getContext()) {
        // Depending on the relative sizes of the y, u, and v planes we may do 1 to 3 draws/
        // readbacks.
        SkAutoTUnref<GrTexture> yuvTex;
        SkAutoTUnref<GrTexture> yTex;
        SkAutoTUnref<GrTexture> uvTex;
        SkAutoTUnref<GrTexture> uTex;
        SkAutoTUnref<GrTexture> vTex;

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
            GrSurfaceDesc yuvDesc;
            yuvDesc.fConfig = kRGBA_8888_GrPixelConfig;
            yuvDesc.fFlags = kRenderTarget_GrSurfaceFlag;
            yuvDesc.fWidth = sizes[0].fWidth;
            yuvDesc.fHeight = sizes[0].fHeight;
            yuvTex.reset(context->textureProvider()->createApproxTexture(yuvDesc));
            if (!yuvTex) {
                return false;
            }
        } else {
            GrSurfaceDesc yDesc;
            yDesc.fConfig = singleChannelPixelConfig;
            yDesc.fFlags = kRenderTarget_GrSurfaceFlag;
            yDesc.fWidth = sizes[0].fWidth;
            yDesc.fHeight = sizes[0].fHeight;
            yTex.reset(context->textureProvider()->createApproxTexture(yDesc));
            if (!yTex) {
                return false;
            }
            if (sizes[1] == sizes[2]) {
                GrSurfaceDesc uvDesc;
                // TODO: Add support for GL_RG when available.
                uvDesc.fConfig = kRGBA_8888_GrPixelConfig;
                uvDesc.fFlags = kRenderTarget_GrSurfaceFlag;
                uvDesc.fWidth = sizes[1].fWidth;
                uvDesc.fHeight = sizes[1].fHeight;
                uvTex.reset(context->textureProvider()->createApproxTexture(uvDesc));
                if (!uvTex) {
                    return false;
                }
            } else {
                GrSurfaceDesc uvDesc;
                uvDesc.fConfig = singleChannelPixelConfig;
                uvDesc.fFlags = kRenderTarget_GrSurfaceFlag;
                uvDesc.fWidth = sizes[1].fWidth;
                uvDesc.fHeight = sizes[1].fHeight;
                uTex.reset(context->textureProvider()->createApproxTexture(uvDesc));
                uvDesc.fWidth = sizes[2].fWidth;
                uvDesc.fHeight = sizes[2].fHeight;
                vTex.reset(context->textureProvider()->createApproxTexture(uvDesc));
                if (!uTex || !vTex) {
                    return false;
                }
            }
        }

        // Do all the draws before any readback.
        if (yuvTex) {
            SkAutoTUnref<GrDrawContext> dc(context->drawContext(yuvTex->asRenderTarget()));
            if (!dc) {
                return false;
            }
            if (!convert_texture(texture, dc, sizes[0].fWidth, sizes[0].fHeight, colorSpace,
                                 GrYUVEffect::CreateRGBToYUV)) {
                return false;
            }

        } else {
            SkASSERT(yTex);
            SkAutoTUnref<GrDrawContext> dc(context->drawContext(yTex->asRenderTarget()));
            if (!dc) {
                return false;
            }
            if (!convert_texture(texture, dc, sizes[0].fWidth, sizes[0].fHeight, colorSpace,
                                 GrYUVEffect::CreateRGBToY)) {
                return false;
            }
            if (uvTex) {
                dc.reset(context->drawContext(uvTex->asRenderTarget()));
                if (!dc) {
                    return false;
                }
                if (!convert_texture(texture, dc, sizes[1].fWidth, sizes[1].fHeight,
                                     colorSpace,  GrYUVEffect::CreateRGBToUV)) {
                    return false;
                }
            } else {
                SkASSERT(uTex && vTex);
                dc.reset(context->drawContext(uTex->asRenderTarget()));
                if (!dc) {
                    return false;
                }
                if (!convert_texture(texture, dc, sizes[1].fWidth, sizes[1].fHeight,
                                     colorSpace, GrYUVEffect::CreateRGBToU)) {
                    return false;
                }
                dc.reset(context->drawContext(vTex->asRenderTarget()));
                if (!dc) {
                    return false;
                }
                if (!convert_texture(texture, dc, sizes[2].fWidth, sizes[2].fHeight,
                                     colorSpace, GrYUVEffect::CreateRGBToV)) {
                    return false;
                }
            }
        }

        if (yuvTex) {
            SkASSERT(sizes[0] == sizes[1] && sizes[1] == sizes[2]);
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
            SkASSERT(yTex);
            if (!yTex->readPixels(0, 0, sizes[0].fWidth, sizes[0].fHeight,
                                  kAlpha_8_GrPixelConfig, planes[0], rowBytes[0])) {
                return false;
            }
            if (uvTex) {
                SkASSERT(sizes[1].fWidth == sizes[2].fWidth);
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
                SkASSERT(uTex && vTex);
                if (!uTex->readPixels(0, 0, sizes[1].fWidth, sizes[1].fHeight,
                                      kAlpha_8_GrPixelConfig, planes[1], rowBytes[1])) {
                    return false;
                }
                if (!vTex->readPixels(0, 0, sizes[2].fWidth, sizes[2].fHeight,
                                      kAlpha_8_GrPixelConfig, planes[2], rowBytes[2])) {
                    return false;
                }
                return true;
            }
        }
    }
    return false;
}
