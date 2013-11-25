/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkGpuBlurUtils.h"

#include "SkRect.h"

#if SK_SUPPORT_GPU
#include "effects/GrConvolutionEffect.h"
#include "effects/GrTextureDomainEffect.h"
#include "GrContext.h"
#endif

namespace SkGpuBlurUtils {

#if SK_SUPPORT_GPU

#define MAX_BLUR_SIGMA 4.0f

static void scale_rect(SkRect* rect, float xScale, float yScale) {
    rect->fLeft   = SkScalarMul(rect->fLeft,   xScale);
    rect->fTop    = SkScalarMul(rect->fTop,    yScale);
    rect->fRight  = SkScalarMul(rect->fRight,  xScale);
    rect->fBottom = SkScalarMul(rect->fBottom, yScale);
}

static float adjust_sigma(float sigma, int *scaleFactor, int *radius) {
    *scaleFactor = 1;
    while (sigma > MAX_BLUR_SIGMA) {
        *scaleFactor *= 2;
        sigma *= 0.5f;
    }
    *radius = static_cast<int>(ceilf(sigma * 3.0f));
    SkASSERT(*radius <= GrConvolutionEffect::kMaxKernelRadius);
    return sigma;
}

static void convolve_gaussian_pass(GrContext* context,
                                   const SkRect& srcRect,
                                   const SkRect& dstRect,
                                   GrTexture* texture,
                                   Gr1DKernelEffect::Direction direction,
                                   int radius,
                                   float sigma,
                                   bool useBounds,
                                   float bounds[2]) {
    GrPaint paint;
    paint.reset();
    SkAutoTUnref<GrEffectRef> conv(GrConvolutionEffect::CreateGaussian(
        texture, direction, radius, sigma, useBounds, bounds));
    paint.reset();
    paint.addColorEffect(conv);
    context->drawRectToRect(paint, dstRect, srcRect);
}

static void convolve_gaussian(GrContext* context,
                              const SkRect& srcRect,
                              const SkRect& dstRect,
                              GrTexture* texture,
                              Gr1DKernelEffect::Direction direction,
                              int radius,
                              float sigma,
                              bool cropToSrcRect) {
    float bounds[2] = { 0.0f, 1.0f };
    if (!cropToSrcRect) {
        convolve_gaussian_pass(context, srcRect, dstRect, texture,
                          direction, radius, sigma, false, bounds);
        return;
    }
    SkRect lowerSrcRect = srcRect, lowerDstRect = dstRect;
    SkRect middleSrcRect = srcRect, middleDstRect = dstRect;
    SkRect upperSrcRect = srcRect, upperDstRect = dstRect;
    SkScalar size;
    SkScalar rad = SkIntToScalar(radius);
    if (direction == Gr1DKernelEffect::kX_Direction) {
        bounds[0] = SkScalarToFloat(srcRect.left()) / texture->width();
        bounds[1] = SkScalarToFloat(srcRect.right()) / texture->width();
        size = srcRect.width();
        lowerSrcRect.fRight = srcRect.left() + rad;
        lowerDstRect.fRight = dstRect.left() + rad;
        upperSrcRect.fLeft = srcRect.right() - rad;
        upperDstRect.fLeft = dstRect.right() - rad;
        middleSrcRect.inset(rad, 0);
        middleDstRect.inset(rad, 0);
    } else {
        bounds[0] = SkScalarToFloat(srcRect.top()) / texture->height();
        bounds[1] = SkScalarToFloat(srcRect.bottom()) / texture->height();
        size = srcRect.height();
        lowerSrcRect.fBottom = srcRect.top() + rad;
        lowerDstRect.fBottom = dstRect.top() + rad;
        upperSrcRect.fTop = srcRect.bottom() - rad;
        upperDstRect.fTop = dstRect.bottom() - rad;
        middleSrcRect.inset(0, rad);
        middleDstRect.inset(0, rad);
    }
    if (radius >= size * SK_ScalarHalf) {
        // Blur radius covers srcRect; use bounds over entire draw
        convolve_gaussian_pass(context, srcRect, dstRect, texture,
                          direction, radius, sigma, true, bounds);
    } else {
        // Draw upper and lower margins with bounds; middle without.
        convolve_gaussian_pass(context, lowerSrcRect, lowerDstRect, texture,
                          direction, radius, sigma, true, bounds);
        convolve_gaussian_pass(context, upperSrcRect, upperDstRect, texture,
                          direction, radius, sigma, true, bounds);
        convolve_gaussian_pass(context, middleSrcRect, middleDstRect, texture,
                          direction, radius, sigma, false, bounds);
    }
}

GrTexture* GaussianBlur(GrContext* context,
                        GrTexture* srcTexture,
                        bool canClobberSrc,
                        const SkRect& rect,
                        bool cropToRect,
                        float sigmaX,
                        float sigmaY) {
    SkASSERT(NULL != context);

    GrContext::AutoRenderTarget art(context);

    GrContext::AutoMatrix am;
    am.setIdentity(context);

    SkIRect clearRect;
    int scaleFactorX, radiusX;
    int scaleFactorY, radiusY;
    sigmaX = adjust_sigma(sigmaX, &scaleFactorX, &radiusX);
    sigmaY = adjust_sigma(sigmaY, &scaleFactorY, &radiusY);

    SkRect srcRect(rect);
    scale_rect(&srcRect, 1.0f / scaleFactorX, 1.0f / scaleFactorY);
    srcRect.roundOut();
    scale_rect(&srcRect, static_cast<float>(scaleFactorX),
                         static_cast<float>(scaleFactorY));

    GrContext::AutoClip acs(context, SkRect::MakeWH(srcRect.width(), srcRect.height()));

    SkASSERT(kBGRA_8888_GrPixelConfig == srcTexture->config() ||
             kRGBA_8888_GrPixelConfig == srcTexture->config() ||
             kAlpha_8_GrPixelConfig == srcTexture->config());

    GrTextureDesc desc;
    desc.fFlags = kRenderTarget_GrTextureFlagBit | kNoStencil_GrTextureFlagBit;
    desc.fWidth = SkScalarFloorToInt(srcRect.width());
    desc.fHeight = SkScalarFloorToInt(srcRect.height());
    desc.fConfig = srcTexture->config();

    GrAutoScratchTexture temp1, temp2;
    GrTexture* dstTexture = temp1.set(context, desc);
    GrTexture* tempTexture = canClobberSrc ? srcTexture : temp2.set(context, desc);
    if (NULL == dstTexture || NULL == tempTexture) {
        return NULL;
    }

    for (int i = 1; i < scaleFactorX || i < scaleFactorY; i *= 2) {
        GrPaint paint;
        SkMatrix matrix;
        matrix.setIDiv(srcTexture->width(), srcTexture->height());
        context->setRenderTarget(dstTexture->asRenderTarget());
        SkRect dstRect(srcRect);
        if (cropToRect && i == 1) {
            dstRect.offset(-dstRect.fLeft, -dstRect.fTop);
            SkRect domain;
            matrix.mapRect(&domain, rect);
            domain.inset(i < scaleFactorX ? SK_ScalarHalf / srcTexture->width() : 0.0f,
                         i < scaleFactorY ? SK_ScalarHalf / srcTexture->height() : 0.0f);
            SkAutoTUnref<GrEffectRef> effect(GrTextureDomainEffect::Create(
                srcTexture,
                matrix,
                domain,
                GrTextureDomainEffect::kDecal_WrapMode,
                GrTextureParams::kBilerp_FilterMode));
            paint.addColorEffect(effect);
        } else {
            GrTextureParams params(SkShader::kClamp_TileMode, GrTextureParams::kBilerp_FilterMode);
            paint.addColorTextureEffect(srcTexture, matrix, params);
        }
        scale_rect(&dstRect, i < scaleFactorX ? 0.5f : 1.0f,
                             i < scaleFactorY ? 0.5f : 1.0f);
        context->drawRectToRect(paint, dstRect, srcRect);
        srcRect = dstRect;
        srcTexture = dstTexture;
        SkTSwap(dstTexture, tempTexture);
    }

    SkIRect srcIRect;
    srcRect.roundOut(&srcIRect);

    if (sigmaX > 0.0f) {
        if (scaleFactorX > 1) {
            // Clear out a radius to the right of the srcRect to prevent the
            // X convolution from reading garbage.
            clearRect = SkIRect::MakeXYWH(srcIRect.fRight, srcIRect.fTop,
                                          radiusX, srcIRect.height());
            context->clear(&clearRect, 0x0, false);
        }
        context->setRenderTarget(dstTexture->asRenderTarget());
        SkRect dstRect = SkRect::MakeWH(srcRect.width(), srcRect.height());
        convolve_gaussian(context, srcRect, dstRect, srcTexture,
                          Gr1DKernelEffect::kX_Direction, radiusX, sigmaX, cropToRect);
        srcTexture = dstTexture;
        srcRect = dstRect;
        SkTSwap(dstTexture, tempTexture);
    }

    if (sigmaY > 0.0f) {
        if (scaleFactorY > 1 || sigmaX > 0.0f) {
            // Clear out a radius below the srcRect to prevent the Y
            // convolution from reading garbage.
            clearRect = SkIRect::MakeXYWH(srcIRect.fLeft, srcIRect.fBottom,
                                          srcIRect.width(), radiusY);
            context->clear(&clearRect, 0x0, false);
        }

        context->setRenderTarget(dstTexture->asRenderTarget());
        SkRect dstRect = SkRect::MakeWH(srcRect.width(), srcRect.height());
        convolve_gaussian(context, srcRect, dstRect, srcTexture,
                          Gr1DKernelEffect::kY_Direction, radiusY, sigmaY, cropToRect);
        srcTexture = dstTexture;
        srcRect = dstRect;
        SkTSwap(dstTexture, tempTexture);
    }

    if (scaleFactorX > 1 || scaleFactorY > 1) {
        // Clear one pixel to the right and below, to accommodate bilinear
        // upsampling.
        clearRect = SkIRect::MakeXYWH(srcIRect.fLeft, srcIRect.fBottom,
                                      srcIRect.width() + 1, 1);
        context->clear(&clearRect, 0x0, false);
        clearRect = SkIRect::MakeXYWH(srcIRect.fRight, srcIRect.fTop,
                                      1, srcIRect.height());
        context->clear(&clearRect, 0x0, false);
        SkMatrix matrix;
        matrix.setIDiv(srcTexture->width(), srcTexture->height());
        context->setRenderTarget(dstTexture->asRenderTarget());

        GrPaint paint;
        // FIXME:  this should be mitchell, not bilinear.
        GrTextureParams params(SkShader::kClamp_TileMode, GrTextureParams::kBilerp_FilterMode);
        paint.addColorTextureEffect(srcTexture, matrix, params);

        SkRect dstRect(srcRect);
        scale_rect(&dstRect, (float) scaleFactorX, (float) scaleFactorY);
        context->drawRectToRect(paint, dstRect, srcRect);
        srcRect = dstRect;
        srcTexture = dstTexture;
        SkTSwap(dstTexture, tempTexture);
    }
    if (srcTexture == temp1.texture()) {
        return temp1.detach();
    } else if (srcTexture == temp2.texture()) {
        return temp2.detach();
    } else {
        srcTexture->ref();
        return srcTexture;
    }
}
#endif

}
