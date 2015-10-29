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
#include "GrGpuResourcePriv.h"
#include "GrResourceKey.h"
#include "GrTexture.h"
#include "GrTextureParams.h"
#include "GrTextureProvider.h"
#include "SkCanvas.h"
#include "SkGr.h"
#include "SkGrPriv.h"
#include "effects/GrTextureDomain.h"

typedef GrTextureProducer::CopyParams CopyParams;

//////////////////////////////////////////////////////////////////////////////

static GrTexture* copy_on_gpu(GrTexture* inputTexture, const SkIRect* subset,
                              const CopyParams& copyParams) {
    SkASSERT(!subset || !subset->isEmpty());
    GrContext* context = inputTexture->getContext();
    SkASSERT(context);
    const GrCaps* caps = context->caps();

    // Either it's a cache miss or the original wasn't cached to begin with.
    GrSurfaceDesc rtDesc = inputTexture->desc();
    rtDesc.fFlags = rtDesc.fFlags | kRenderTarget_GrSurfaceFlag;
    rtDesc.fWidth = copyParams.fWidth;
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

    // TODO: If no scaling is being performed then use copySurface.

    GrPaint paint;

    SkScalar sx;
    SkScalar sy;
    if (subset) {
        sx = 1.f / inputTexture->width();
        sy = 1.f / inputTexture->height();
    }

    if (copyParams.fFilter != GrTextureParams::kNone_FilterMode && subset &&
        (subset->width() != copyParams.fWidth || subset->height() != copyParams.fHeight)) {
        SkRect domain;
        domain.fLeft = (subset->fLeft + 0.5f) * sx;
        domain.fTop = (subset->fTop + 0.5f)* sy;
        domain.fRight = (subset->fRight - 0.5f) * sx;
        domain.fBottom = (subset->fBottom - 0.5f) * sy;
        // This would cause us to read values from outside the subset. Surely, the caller knows
        // better!
        SkASSERT(copyParams.fFilter != GrTextureParams::kMipMap_FilterMode);
        paint.addColorFragmentProcessor(
            GrTextureDomainEffect::Create(inputTexture, SkMatrix::I(), domain,
                                          GrTextureDomain::kClamp_Mode,
                                          copyParams.fFilter))->unref();
    } else {
        GrTextureParams params(SkShader::kClamp_TileMode, copyParams.fFilter);
        paint.addColorTextureProcessor(inputTexture, SkMatrix::I(), params);
    }

    SkRect localRect;
    if (subset) {
        localRect = SkRect::Make(*subset);
        localRect.fLeft *= sx;
        localRect.fTop *= sy;
        localRect.fRight *= sx;
        localRect.fBottom *= sy;
    } else {
        localRect = SkRect::MakeWH(1.f, 1.f);
    }

    SkAutoTUnref<GrDrawContext> drawContext(context->drawContext(copy->asRenderTarget()));
    if (!drawContext) {
        return nullptr;
    }

    SkRect dstRect = SkRect::MakeWH(SkIntToScalar(rtDesc.fWidth), SkIntToScalar(rtDesc.fHeight));
    drawContext->drawNonAARectToRect(GrClip::WideOpen(), paint, SkMatrix::I(), dstRect, localRect);
    return copy.detach();
}

GrTextureAdjuster::GrTextureAdjuster(GrTexture* original, const SkIRect& subset)
    : fOriginal(original) {
    if (subset.fLeft > 0 || subset.fTop > 0 ||
        subset.fRight < original->width() || subset.fBottom < original->height()) {
        fSubset.set(subset);
    }
}

GrTexture* GrTextureAdjuster::refTextureSafeForParams(const GrTextureParams& params,
                                                      SkIPoint* outOffset) {
    GrTexture* texture = this->originalTexture();
    GrContext* context = texture->getContext();
    CopyParams copyParams;
    const SkIRect* subset = this->subset();

    if (!context->getGpu()->makeCopyForTextureParams(texture->width(), texture->height(), params,
                                                     &copyParams)) {
        if (outOffset) {
            if (subset) {
                outOffset->set(subset->fLeft, subset->fRight);
            } else {
                outOffset->set(0, 0);
            }
        }
        return SkRef(texture);
    }
    GrUniqueKey key;
    this->makeCopyKey(copyParams, &key);
    if (key.isValid()) {
        GrTexture* result = context->textureProvider()->findAndRefTextureByUniqueKey(key);
        if (result) {
            return result;
        }
    }
    GrTexture* result = copy_on_gpu(texture, subset, copyParams);
    if (result) {
        if (key.isValid()) {
            result->resourcePriv().setUniqueKey(key);
            this->didCacheCopy(key);
        }
        if (outOffset) {
            outOffset->set(0, 0);
        }
    }
    return result;
}

//////////////////////////////////////////////////////////////////////////////

GrTexture* GrTextureMaker::refTextureForParams(GrContext* ctx, const GrTextureParams& params) {
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

GrTexture* GrTextureMaker::generateTextureForParams(GrContext* ctx, const CopyParams& copyParams) {
    SkAutoTUnref<GrTexture> original(this->refOriginalTexture(ctx));
    if (!original) {
        return nullptr;
    }
    return copy_on_gpu(original, nullptr, copyParams);
}
