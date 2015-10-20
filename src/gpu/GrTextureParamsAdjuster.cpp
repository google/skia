/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextureParamsAdjuster.h"

#include "GrCaps.h"
#include "GrContext.h"
#include "GrDrawContext.h"
#include "GrGpu.h"
#include "GrTexture.h"
#include "GrTextureParams.h"
#include "GrTextureProvider.h"
#include "SkCanvas.h"
#include "SkGr.h"
#include "SkGrPriv.h"

typedef GrTextureParamsAdjuster::CopyParams CopyParams;

static GrTexture* copy_on_gpu(GrTexture* inputTexture, const CopyParams& copyParams) {
    GrContext* context = inputTexture->getContext();
    SkASSERT(context);
    const GrCaps* caps = context->caps();

    // Either it's a cache miss or the original wasn't cached to begin with.
    GrSurfaceDesc rtDesc = inputTexture->desc();
    rtDesc.fFlags =  rtDesc.fFlags | kRenderTarget_GrSurfaceFlag;
    rtDesc.fWidth  = copyParams.fWidth;
    rtDesc.fHeight = copyParams.fHeight;
    rtDesc.fConfig = GrMakePixelConfigUncompressed(rtDesc.fConfig);

    // If the config isn't renderable try converting to either A8 or an 32 bit config. Otherwise,
    // fail.
    if (!caps->isConfigRenderable(rtDesc.fConfig, false)) {
        if (GrPixelConfigIsAlphaOnly(rtDesc.fConfig)) {
            if (caps->isConfigRenderable(kAlpha_8_GrPixelConfig, false)) {
                rtDesc.fConfig = kAlpha_8_GrPixelConfig;
            } else if (caps->isConfigRenderable(kSkia8888_GrPixelConfig, false)) {
                rtDesc.fConfig = kSkia8888_GrPixelConfig;
            } else {
                return nullptr;
            }
        } else if (kRGB_GrColorComponentFlags ==
                   (kRGB_GrColorComponentFlags & GrPixelConfigComponentMask(rtDesc.fConfig))) {
            if (caps->isConfigRenderable(kSkia8888_GrPixelConfig, false)) {
                rtDesc.fConfig = kSkia8888_GrPixelConfig;
            } else {
                return nullptr;
            }
        } else {
            return nullptr;
        }
    }

    SkAutoTUnref<GrTexture> copy(context->textureProvider()->createTexture(rtDesc, true));
    if (!copy) {
        return nullptr;
    }

    GrPaint paint;

    // If filtering is not desired then we want to ensure all texels in the resampled image are
    // copies of texels from the original.
    GrTextureParams params(SkShader::kClamp_TileMode, copyParams.fFilter);
    paint.addColorTextureProcessor(inputTexture, SkMatrix::I(), params);

    SkRect rect = SkRect::MakeWH(SkIntToScalar(rtDesc.fWidth), SkIntToScalar(rtDesc.fHeight));
    SkRect localRect = SkRect::MakeWH(1.f, 1.f);

    SkAutoTUnref<GrDrawContext> drawContext(context->drawContext(copy->asRenderTarget()));
    if (!drawContext) {
        return nullptr;
    }

    drawContext->drawNonAARectToRect(GrClip::WideOpen(), paint, SkMatrix::I(), rect, localRect);
    return copy.detach();
}

static SkBitmap copy_on_cpu(const SkBitmap& bmp, const CopyParams& copyParams) {
    SkBitmap stretched;
    stretched.allocN32Pixels(copyParams.fWidth, copyParams.fHeight);
    SkCanvas canvas(stretched);
    SkPaint paint;
    switch (copyParams.fFilter) {
        case GrTextureParams::kNone_FilterMode:
            paint.setFilterQuality(kNone_SkFilterQuality);
            break;
        case GrTextureParams::kBilerp_FilterMode:
            paint.setFilterQuality(kLow_SkFilterQuality);
            break;
        case GrTextureParams::kMipMap_FilterMode:
            paint.setFilterQuality(kMedium_SkFilterQuality);
            break;
    }
    SkRect dstRect = SkRect::MakeWH(SkIntToScalar(copyParams.fWidth),
                                    SkIntToScalar(copyParams.fHeight));
    canvas.drawBitmapRect(bmp, dstRect, &paint);
    return stretched;
}

GrTexture* GrTextureParamsAdjuster::refTextureForParams(GrContext* ctx,
                                                        const GrTextureParams& params) {
    CopyParams copyParams;
    if (!ctx->getGpu()->makeCopyForTextureParams(this->width(), this->height(), params,
                                                 &copyParams)) {
        return this->refOriginalTexture(ctx);
    }
    GrUniqueKey copyKey;
    this->makeCopyKey(copyParams, &copyKey);
    if (copyKey.isValid()) {
        GrTexture* result = ctx->textureProvider()->findAndRefTextureByUniqueKey(copyKey);
        if (result) {
            return result;
        }
    }

    GrTexture* result = this->generateTextureForParams(ctx, copyParams);
    if (!result) {
        return nullptr;
    }

    if (copyKey.isValid()) {
        ctx->textureProvider()->assignUniqueKeyToTexture(copyKey, result);
        this->didCacheCopy(copyKey);
    }
    return result;
}

GrTexture* GrTextureParamsAdjuster::generateTextureForParams(GrContext* ctx,
                                                             const CopyParams& copyParams) {
    if ((this->width() < ctx->caps()->minTextureSize() ||
         this->height() < ctx->caps()->minTextureSize()) && !this->peekOriginalTexture())
    {
        // we can't trust our ability to use HW to perform the stretch, so we request
        // a raster instead, and perform the stretch on the CPU.
        SkBitmap bitmap;
        if (!this->getROBitmap(&bitmap)) {
            return nullptr;
        }
        SkBitmap stretchedBmp = copy_on_cpu(bitmap, copyParams);
        return GrUploadBitmapToTexture(ctx, stretchedBmp);
    } else {
        SkAutoTUnref<GrTexture> original(this->refOriginalTexture(ctx));
        if (!original) {
            return nullptr;
        }
        return copy_on_gpu(original, copyParams);
    }
}
