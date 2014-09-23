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
#include "effects/GrMatrixConvolutionEffect.h"
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

static float adjust_sigma(float sigma, int maxTextureSize, int *scaleFactor, int *radius) {
    *scaleFactor = 1;
    while (sigma > MAX_BLUR_SIGMA) {
        *scaleFactor *= 2;
        sigma *= 0.5f;
        if (*scaleFactor > maxTextureSize) {
            *scaleFactor = maxTextureSize;
            sigma = MAX_BLUR_SIGMA;
        }
    }
    *radius = static_cast<int>(ceilf(sigma * 3.0f));
    SkASSERT(*radius <= GrConvolutionEffect::kMaxKernelRadius);
    return sigma;
}

static void convolve_gaussian_1d(GrContext* context,
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
    SkAutoTUnref<GrFragmentProcessor> conv(GrConvolutionEffect::CreateGaussian(
        texture, direction, radius, sigma, useBounds, bounds));
    paint.reset();
    paint.addColorProcessor(conv);
    context->drawRectToRect(paint, dstRect, srcRect);
}

static void convolve_gaussian_2d(GrContext* context,
                                 const SkRect& srcRect,
                                 const SkRect& dstRect,
                                 GrTexture* texture,
                                 int radiusX,
                                 int radiusY,
                                 SkScalar sigmaX,
                                 SkScalar sigmaY,
                                 bool useBounds,
                                 SkIRect bounds) {
    SkISize size = SkISize::Make(2 * radiusX + 1,  2 * radiusY + 1);
    SkIPoint kernelOffset = SkIPoint::Make(radiusX, radiusY);
    GrPaint paint;
    paint.reset();
    SkAutoTUnref<GrFragmentProcessor> conv(GrMatrixConvolutionEffect::CreateGaussian(
            texture, bounds, size, 1.0, 0.0, kernelOffset,
            useBounds ? GrTextureDomain::kClamp_Mode : GrTextureDomain::kIgnore_Mode,
            true, sigmaX, sigmaY));
    paint.reset();
    paint.addColorProcessor(conv);
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
        convolve_gaussian_1d(context, srcRect, dstRect, texture,
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
        convolve_gaussian_1d(context, srcRect, dstRect, texture,
                            direction, radius, sigma, true, bounds);
    } else {
        // Draw upper and lower margins with bounds; middle without.
        convolve_gaussian_1d(context, lowerSrcRect, lowerDstRect, texture,
                             direction, radius, sigma, true, bounds);
        convolve_gaussian_1d(context, upperSrcRect, upperDstRect, texture,
                             direction, radius, sigma, true, bounds);
        convolve_gaussian_1d(context, middleSrcRect, middleDstRect, texture,
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
    SkASSERT(context);

    GrContext::AutoRenderTarget art(context);

    GrContext::AutoMatrix am;
    am.setIdentity(context);

    SkIRect clearRect;
    int scaleFactorX, radiusX;
    int scaleFactorY, radiusY;
    int maxTextureSize = context->getMaxTextureSize();
    sigmaX = adjust_sigma(sigmaX, maxTextureSize, &scaleFactorX, &radiusX);
    sigmaY = adjust_sigma(sigmaY, maxTextureSize, &scaleFactorY, &radiusY);

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
            SkAutoTUnref<GrFragmentProcessor> fp(GrTextureDomainEffect::Create(
                srcTexture,
                matrix,
                domain,
                GrTextureDomain::kDecal_Mode,
                GrTextureParams::kBilerp_FilterMode));
            paint.addColorProcessor(fp);
        } else {
            GrTextureParams params(SkShader::kClamp_TileMode, GrTextureParams::kBilerp_FilterMode);
            paint.addColorTextureProcessor(srcTexture, matrix, params);
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

    // For really small blurs(Certainly no wider than 5x5 on desktop gpus) it is faster to just
    // launch a single non separable kernel vs two launches
    if (sigmaX > 0.0f && sigmaY > 0 &&
            (2 * radiusX + 1) * (2 * radiusY + 1) <= MAX_KERNEL_SIZE) {
        // We shouldn't be scaling because this is a small size blur
        SkASSERT((scaleFactorX == scaleFactorY) == 1);
        context->setRenderTarget(dstTexture->asRenderTarget());
        SkRect dstRect = SkRect::MakeWH(srcRect.width(), srcRect.height());
        convolve_gaussian_2d(context, srcRect, dstRect, srcTexture,
                radiusX, radiusY, sigmaX, sigmaY, cropToRect, srcIRect);
        srcTexture = dstTexture;
        srcRect = dstRect;
        SkTSwap(dstTexture, tempTexture);

    } else {
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
        paint.addColorTextureProcessor(srcTexture, matrix, params);

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
