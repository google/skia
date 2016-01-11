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
#include "GrCaps.h"
#include "GrDrawContext.h"
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

static void convolve_gaussian_1d(GrDrawContext* drawContext,
                                 const GrClip& clip,
                                 const SkRect& dstRect,
                                 const SkPoint& srcOffset,
                                 GrTexture* texture,
                                 Gr1DKernelEffect::Direction direction,
                                 int radius,
                                 float sigma,
                                 bool useBounds,
                                 float bounds[2]) {
    GrPaint paint;
    SkAutoTUnref<GrFragmentProcessor> conv(GrConvolutionEffect::CreateGaussian(
        texture, direction, radius, sigma, useBounds, bounds));
    paint.addColorFragmentProcessor(conv);
    paint.setPorterDuffXPFactory(SkXfermode::kSrc_Mode);
    SkMatrix localMatrix = SkMatrix::MakeTrans(-srcOffset.x(), -srcOffset.y());
    drawContext->fillRectWithLocalMatrix(clip, paint, SkMatrix::I(), dstRect, localMatrix);
}

static void convolve_gaussian_2d(GrDrawContext* drawContext,
                                 const GrClip& clip,
                                 const SkRect& dstRect,
                                 const SkPoint& srcOffset,
                                 GrTexture* texture,
                                 int radiusX,
                                 int radiusY,
                                 SkScalar sigmaX,
                                 SkScalar sigmaY,
                                 const SkRect* srcBounds) {
    SkMatrix localMatrix = SkMatrix::MakeTrans(-srcOffset.x(), -srcOffset.y());
    SkISize size = SkISize::Make(2 * radiusX + 1,  2 * radiusY + 1);
    SkIPoint kernelOffset = SkIPoint::Make(radiusX, radiusY);
    GrPaint paint;
    SkIRect bounds;
    if (srcBounds) {
        srcBounds->roundOut(&bounds);
    } else {
        bounds.setEmpty();
    }

    SkAutoTUnref<GrFragmentProcessor> conv(GrMatrixConvolutionEffect::CreateGaussian(
            texture, bounds, size, 1.0, 0.0, kernelOffset,
            srcBounds ? GrTextureDomain::kDecal_Mode : GrTextureDomain::kIgnore_Mode,
            true, sigmaX, sigmaY));
    paint.addColorFragmentProcessor(conv);
    paint.setPorterDuffXPFactory(SkXfermode::kSrc_Mode);
    drawContext->fillRectWithLocalMatrix(clip, paint, SkMatrix::I(), dstRect, localMatrix);
}

static void convolve_gaussian(GrDrawContext* drawContext,
                              const GrClip& clip,
                              const SkRect& srcRect,
                              GrTexture* texture,
                              Gr1DKernelEffect::Direction direction,
                              int radius,
                              float sigma,
                              const SkRect* srcBounds,
                              const SkPoint& srcOffset) {
    float bounds[2] = { 0.0f, 1.0f };
    SkRect dstRect = SkRect::MakeWH(srcRect.width(), srcRect.height());
    if (!srcBounds) {
        convolve_gaussian_1d(drawContext, clip, dstRect, srcOffset, texture,
                             direction, radius, sigma, false, bounds);
        return;
    }
    SkRect midRect = *srcBounds, leftRect, rightRect;
    midRect.offset(srcOffset);
    SkIRect topRect, bottomRect;
    SkScalar rad = SkIntToScalar(radius);
    if (direction == Gr1DKernelEffect::kX_Direction) {
        bounds[0] = SkScalarToFloat(srcBounds->left()) / texture->width();
        bounds[1] = SkScalarToFloat(srcBounds->right()) / texture->width();
        SkRect::MakeLTRB(0, 0, dstRect.right(), midRect.top()).roundOut(&topRect);
        SkRect::MakeLTRB(0, midRect.bottom(), dstRect.right(), dstRect.bottom())
            .roundOut(&bottomRect);
        midRect.inset(rad, 0);
        leftRect = SkRect::MakeLTRB(0, midRect.top(), midRect.left(), midRect.bottom());
        rightRect =
            SkRect::MakeLTRB(midRect.right(), midRect.top(), dstRect.width(), midRect.bottom());
        dstRect.fTop = midRect.top();
        dstRect.fBottom = midRect.bottom();
    } else {
        bounds[0] = SkScalarToFloat(srcBounds->top()) / texture->height();
        bounds[1] = SkScalarToFloat(srcBounds->bottom()) / texture->height();
        SkRect::MakeLTRB(0, 0, midRect.left(), dstRect.bottom()).roundOut(&topRect);
        SkRect::MakeLTRB(midRect.right(), 0, dstRect.right(), dstRect.bottom())
            .roundOut(&bottomRect);;
        midRect.inset(0, rad);
        leftRect = SkRect::MakeLTRB(midRect.left(), 0, midRect.right(), midRect.top());
        rightRect =
            SkRect::MakeLTRB(midRect.left(), midRect.bottom(), midRect.right(), dstRect.height());
        dstRect.fLeft = midRect.left();
        dstRect.fRight = midRect.right();
    }
    if (!topRect.isEmpty()) {
        drawContext->clear(&topRect, 0, false);
    }

    if (!bottomRect.isEmpty()) {
        drawContext->clear(&bottomRect, 0, false);
    }
    if (midRect.isEmpty()) {
        // Blur radius covers srcBounds; use bounds over entire draw
        convolve_gaussian_1d(drawContext, clip, dstRect, srcOffset, texture,
                            direction, radius, sigma, true, bounds);
    } else {
        // Draw right and left margins with bounds; middle without.
        convolve_gaussian_1d(drawContext, clip, leftRect, srcOffset, texture,
                             direction, radius, sigma, true, bounds);
        convolve_gaussian_1d(drawContext, clip, rightRect, srcOffset, texture,
                             direction, radius, sigma, true, bounds);
        convolve_gaussian_1d(drawContext, clip, midRect, srcOffset, texture,
                             direction, radius, sigma, false, bounds);
    }
}

GrTexture* GaussianBlur(GrContext* context,
                        GrTexture* srcTexture,
                        bool canClobberSrc,
                        const SkRect& dstBounds,
                        const SkRect* srcBounds,
                        float sigmaX,
                        float sigmaY) {
    SkASSERT(context);
    SkIRect clearRect;
    int scaleFactorX, radiusX;
    int scaleFactorY, radiusY;
    int maxTextureSize = context->caps()->maxTextureSize();
    sigmaX = adjust_sigma(sigmaX, maxTextureSize, &scaleFactorX, &radiusX);
    sigmaY = adjust_sigma(sigmaY, maxTextureSize, &scaleFactorY, &radiusY);

    SkPoint srcOffset = SkPoint::Make(-dstBounds.x(), -dstBounds.y());
    SkRect localDstBounds = SkRect::MakeWH(dstBounds.width(), dstBounds.height());
    SkRect localSrcBounds;
    SkRect srcRect;
    if (srcBounds) {
        srcRect = localSrcBounds = *srcBounds;
        srcRect.offset(srcOffset);
        srcBounds = &localSrcBounds;
    } else {
        srcRect = localDstBounds;
    }

    scale_rect(&srcRect, 1.0f / scaleFactorX, 1.0f / scaleFactorY);
    srcRect.roundOut(&srcRect);
    scale_rect(&srcRect, static_cast<float>(scaleFactorX),
                         static_cast<float>(scaleFactorY));

    // setup new clip
    GrClip clip(localDstBounds);

    SkASSERT(kBGRA_8888_GrPixelConfig == srcTexture->config() ||
             kRGBA_8888_GrPixelConfig == srcTexture->config() ||
             kAlpha_8_GrPixelConfig == srcTexture->config());

    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fWidth = SkScalarFloorToInt(dstBounds.width());
    desc.fHeight = SkScalarFloorToInt(dstBounds.height());
    desc.fConfig = srcTexture->config();

    GrTexture* dstTexture;
    GrTexture* tempTexture;
    SkAutoTUnref<GrTexture> temp1, temp2;

    temp1.reset(context->textureProvider()->createApproxTexture(desc));
    dstTexture = temp1.get();
    if (canClobberSrc) {
        tempTexture = srcTexture;
    } else {
        temp2.reset(context->textureProvider()->createApproxTexture(desc));
        tempTexture = temp2.get();
    }

    if (nullptr == dstTexture || nullptr == tempTexture) {
        return nullptr;
    }

    SkAutoTUnref<GrDrawContext> srcDrawContext;

    for (int i = 1; i < scaleFactorX || i < scaleFactorY; i *= 2) {
        GrPaint paint;
        SkMatrix matrix;
        matrix.setIDiv(srcTexture->width(), srcTexture->height());
        SkRect dstRect(srcRect);
        if (srcBounds && i == 1) {
            SkRect domain;
            matrix.mapRect(&domain, *srcBounds);
            domain.inset((i < scaleFactorX) ? SK_ScalarHalf / srcTexture->width() : 0.0f,
                         (i < scaleFactorY) ? SK_ScalarHalf / srcTexture->height() : 0.0f);
            SkAutoTUnref<const GrFragmentProcessor> fp(GrTextureDomainEffect::Create(
                srcTexture,
                matrix,
                domain,
                GrTextureDomain::kDecal_Mode,
                GrTextureParams::kBilerp_FilterMode));
            paint.addColorFragmentProcessor(fp);
            srcRect.offset(-srcOffset);
            srcOffset.set(0, 0);
        } else {
            GrTextureParams params(SkShader::kClamp_TileMode, GrTextureParams::kBilerp_FilterMode);
            paint.addColorTextureProcessor(srcTexture, matrix, params);
        }
        paint.setPorterDuffXPFactory(SkXfermode::kSrc_Mode);
        scale_rect(&dstRect, i < scaleFactorX ? 0.5f : 1.0f,
                             i < scaleFactorY ? 0.5f : 1.0f);

        SkAutoTUnref<GrDrawContext> dstDrawContext(
                                             context->drawContext(dstTexture->asRenderTarget()));
        if (!dstDrawContext) {
            return nullptr;
        }
        dstDrawContext->fillRectToRect(clip, paint, SkMatrix::I(), dstRect, srcRect);

        srcDrawContext.swap(dstDrawContext);
        srcRect = dstRect;
        srcTexture = dstTexture;
        SkTSwap(dstTexture, tempTexture);
        localSrcBounds = srcRect;
    }

    // For really small blurs (certainly no wider than 5x5 on desktop gpus) it is faster to just
    // launch a single non separable kernel vs two launches
    srcRect = localDstBounds;
    if (sigmaX > 0.0f && sigmaY > 0.0f &&
            (2 * radiusX + 1) * (2 * radiusY + 1) <= MAX_KERNEL_SIZE) {
        // We shouldn't be scaling because this is a small size blur
        SkASSERT((1 == scaleFactorX) && (1 == scaleFactorY));

        SkAutoTUnref<GrDrawContext> dstDrawContext(
                                             context->drawContext(dstTexture->asRenderTarget()));
        if (!dstDrawContext) {
            return nullptr;
        }
        convolve_gaussian_2d(dstDrawContext, clip, srcRect, srcOffset,
                             srcTexture, radiusX, radiusY, sigmaX, sigmaY, srcBounds);

        srcDrawContext.swap(dstDrawContext);
        srcRect.offsetTo(0, 0);
        srcTexture = dstTexture;
        SkTSwap(dstTexture, tempTexture);

    } else {
        scale_rect(&srcRect, 1.0f / scaleFactorX, 1.0f / scaleFactorY);
        srcRect.roundOut(&srcRect);
        const SkIRect srcIRect = srcRect.roundOut();
        if (sigmaX > 0.0f) {
            if (scaleFactorX > 1) {
                // TODO: if we pass in the source draw context we don't need this here
                if (!srcDrawContext) {
                    srcDrawContext.reset(context->drawContext(srcTexture->asRenderTarget()));
                    if (!srcDrawContext) {
                        return nullptr;
                    }        
                }

                // Clear out a radius to the right of the srcRect to prevent the
                // X convolution from reading garbage.
                clearRect = SkIRect::MakeXYWH(srcIRect.fRight, srcIRect.fTop,
                                              radiusX, srcIRect.height());
                srcDrawContext->clear(&clearRect, 0x0, false);
            }

            SkAutoTUnref<GrDrawContext> dstDrawContext(
                                             context->drawContext(dstTexture->asRenderTarget()));
            if (!dstDrawContext) {
                return nullptr;
            }
            convolve_gaussian(dstDrawContext, clip, srcRect,
                              srcTexture, Gr1DKernelEffect::kX_Direction, radiusX, sigmaX,
                              srcBounds, srcOffset);
            srcDrawContext.swap(dstDrawContext);
            srcTexture = dstTexture;
            srcRect.offsetTo(0, 0);
            SkTSwap(dstTexture, tempTexture);
            localSrcBounds = srcRect;
            srcOffset.set(0, 0);
        }

        if (sigmaY > 0.0f) {
            if (scaleFactorY > 1 || sigmaX > 0.0f) {
                // TODO: if we pass in the source draw context we don't need this here
                if (!srcDrawContext) {
                    srcDrawContext.reset(context->drawContext(srcTexture->asRenderTarget()));
                    if (!srcDrawContext) {
                        return nullptr;
                    }        
                }

                // Clear out a radius below the srcRect to prevent the Y
                // convolution from reading garbage.
                clearRect = SkIRect::MakeXYWH(srcIRect.fLeft, srcIRect.fBottom,
                                              srcIRect.width(), radiusY);
                srcDrawContext->clear(&clearRect, 0x0, false);
            }

            SkAutoTUnref<GrDrawContext> dstDrawContext(
                                               context->drawContext(dstTexture->asRenderTarget()));
            if (!dstDrawContext) {
                return nullptr;
            }
            convolve_gaussian(dstDrawContext, clip, srcRect,
                              srcTexture, Gr1DKernelEffect::kY_Direction, radiusY, sigmaY,
                              srcBounds, srcOffset);

            srcDrawContext.swap(dstDrawContext);
            srcTexture = dstTexture;
            srcRect.offsetTo(0, 0);
            SkTSwap(dstTexture, tempTexture);
        }
    }
    const SkIRect srcIRect = srcRect.roundOut();

    if (scaleFactorX > 1 || scaleFactorY > 1) {
        SkASSERT(srcDrawContext);

        // Clear one pixel to the right and below, to accommodate bilinear
        // upsampling.
        clearRect = SkIRect::MakeXYWH(srcIRect.fLeft, srcIRect.fBottom,
                                      srcIRect.width() + 1, 1);
        srcDrawContext->clear(&clearRect, 0x0, false);
        clearRect = SkIRect::MakeXYWH(srcIRect.fRight, srcIRect.fTop,
                                      1, srcIRect.height());
        srcDrawContext->clear(&clearRect, 0x0, false);
        SkMatrix matrix;
        matrix.setIDiv(srcTexture->width(), srcTexture->height());

        GrPaint paint;
        // FIXME:  this should be mitchell, not bilinear.
        GrTextureParams params(SkShader::kClamp_TileMode, GrTextureParams::kBilerp_FilterMode);
        paint.addColorTextureProcessor(srcTexture, matrix, params);
        paint.setPorterDuffXPFactory(SkXfermode::kSrc_Mode);

        SkRect dstRect(srcRect);
        scale_rect(&dstRect, (float) scaleFactorX, (float) scaleFactorY);

        SkAutoTUnref<GrDrawContext> dstDrawContext(
                                context->drawContext(dstTexture->asRenderTarget()));
        if (!dstDrawContext) {
            return nullptr;
        }
        dstDrawContext->fillRectToRect(clip, paint, SkMatrix::I(), dstRect, srcRect);

        srcDrawContext.swap(dstDrawContext);
        srcRect = dstRect;
        srcTexture = dstTexture;
        SkTSwap(dstTexture, tempTexture);
    }

    return SkRef(srcTexture);
}
#endif

}
