/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapCache.h"
#include "SkImage_Gpu.h"
#include "GrContext.h"
#include "GrDrawContext.h"
#include "effects/GrYUVtoRGBEffect.h"
#include "SkCanvas.h"
#include "SkGpuDevice.h"
#include "SkPixelRef.h"

SkImage_Gpu::SkImage_Gpu(int w, int h, uint32_t uniqueID, SkAlphaType at, GrTexture* tex,
                         int sampleCountForNewSurfaces, SkSurface::Budgeted budgeted)
    : INHERITED(w, h, uniqueID, NULL)
    , fTexture(SkRef(tex))
    , fSampleCountForNewSurfaces(sampleCountForNewSurfaces)
    , fAlphaType(at)
    , fBudgeted(budgeted)
    , fAddedRasterVersionToCache(false)
    {}

SkImage_Gpu::~SkImage_Gpu() {
    if (fAddedRasterVersionToCache.load()) {
        SkNotifyBitmapGenIDIsStale(this->uniqueID());
    }
}

SkSurface* SkImage_Gpu::onNewSurface(const SkImageInfo& info, const SkSurfaceProps& props) const {
    GrTexture* tex = this->getTexture();
    SkASSERT(tex);
    GrContext* ctx = tex->getContext();
    if (!ctx) {
        // the texture may have been abandoned, so we have to check
        return NULL;
    }
    // TODO: Change signature of onNewSurface to take a budgeted param.
    const SkSurface::Budgeted budgeted = SkSurface::kNo_Budgeted;
    return SkSurface::NewRenderTarget(ctx, budgeted, info, fSampleCountForNewSurfaces, &props);
}

extern void SkTextureImageApplyBudgetedDecision(SkImage* image) {
    if (as_IB(image)->getTexture()) {
        ((SkImage_Gpu*)image)->applyBudgetDecision();
    }
}

SkShader* SkImage_Gpu::onNewShader(SkShader::TileMode tileX, SkShader::TileMode tileY,
                                   const SkMatrix* localMatrix) const {
    SkBitmap bm;
    GrWrapTextureInBitmap(fTexture, this->width(), this->height(), this->isOpaque(), &bm);
    return SkShader::CreateBitmapShader(bm, tileX, tileY, localMatrix);
}

bool SkImage_Gpu::getROPixels(SkBitmap* dst) const {
    if (SkBitmapCache::Find(this->uniqueID(), dst)) {
        SkASSERT(dst->getGenerationID() == this->uniqueID());
        SkASSERT(dst->isImmutable());
        SkASSERT(dst->getPixels());
        return true;
    }

    SkAlphaType at = this->isOpaque() ? kOpaque_SkAlphaType : kPremul_SkAlphaType;
    if (!dst->tryAllocPixels(SkImageInfo::MakeN32(this->width(), this->height(), at))) {
        return false;
    }
    if (!fTexture->readPixels(0, 0, dst->width(), dst->height(), kSkia8888_GrPixelConfig,
                              dst->getPixels(), dst->rowBytes())) {
        return false;
    }

    dst->pixelRef()->setImmutableWithID(this->uniqueID());
    SkBitmapCache::Add(this->uniqueID(), *dst);
    fAddedRasterVersionToCache.store(true);
    return true;
}

bool SkImage_Gpu::isOpaque() const {
    return GrPixelConfigIsOpaque(fTexture->config()) || fAlphaType == kOpaque_SkAlphaType;
}

static void apply_premul(const SkImageInfo& info, void* pixels, size_t rowBytes) {
    switch (info.colorType()) {
        case kRGBA_8888_SkColorType:
        case kBGRA_8888_SkColorType:
            break;
        default:
            return; // nothing to do
    }

    // SkColor is not necesarily RGBA or BGRA, but it is one of them on little-endian,
    // and in either case, the alpha-byte is always in the same place, so we can safely call
    // SkPreMultiplyColor()
    //
    SkColor* row = (SkColor*)pixels;
    for (int y = 0; y < info.height(); ++y) {
        for (int x = 0; x < info.width(); ++x) {
            row[x] = SkPreMultiplyColor(row[x]);
        }
    }
}

bool SkImage_Gpu::onReadPixels(const SkImageInfo& info, void* pixels, size_t rowBytes,
                               int srcX, int srcY) const {
    GrPixelConfig config = SkImageInfo2GrPixelConfig(info.colorType(), info.alphaType(),
                                                     info.profileType());
    uint32_t flags = 0;
    if (kUnpremul_SkAlphaType == info.alphaType() && kPremul_SkAlphaType == fAlphaType) {
        // let the GPU perform this transformation for us
        flags = GrContext::kUnpremul_PixelOpsFlag;
    }
    if (!fTexture->readPixels(srcX, srcY, info.width(), info.height(), config,
                              pixels, rowBytes, flags)) {
        return false;
    }
    // do we have to manually fix-up the alpha channel?
    //      src         dst
    //      unpremul    premul      fix manually
    //      premul      unpremul    done by kUnpremul_PixelOpsFlag
    // all other combos need to change.
    //
    // Should this be handled by Ganesh? todo:?
    //
    if (kPremul_SkAlphaType == info.alphaType() && kUnpremul_SkAlphaType == fAlphaType) {
        apply_premul(info, pixels, rowBytes);
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static SkImage* new_wrapped_texture_common(GrContext* ctx, const GrBackendTextureDesc& desc,
                                           SkAlphaType at, GrWrapOwnership ownership,
                                           SkImage::TextureReleaseProc releaseProc,
                                           SkImage::ReleaseContext releaseCtx) {
    if (desc.fWidth <= 0 || desc.fHeight <= 0) {
        return NULL;
    }
    SkAutoTUnref<GrTexture> tex(ctx->textureProvider()->wrapBackendTexture(desc, ownership));
    if (!tex) {
        return NULL;
    }
    if (releaseProc) {
        tex->setRelease(releaseProc, releaseCtx);
    }

    const SkSurface::Budgeted budgeted = SkSurface::kNo_Budgeted;
    return SkNEW_ARGS(SkImage_Gpu,
                      (desc.fWidth, desc.fHeight, kNeedNewImageUniqueID, at, tex, 0, budgeted));

}

SkImage* SkImage::NewFromTexture(GrContext* ctx, const GrBackendTextureDesc& desc, SkAlphaType at,
                                 TextureReleaseProc releaseP, ReleaseContext releaseC) {
    return new_wrapped_texture_common(ctx, desc, at, kBorrow_GrWrapOwnership, releaseP, releaseC);
}

SkImage* SkImage::NewFromAdoptedTexture(GrContext* ctx, const GrBackendTextureDesc& desc,
                                        SkAlphaType at) {
    return new_wrapped_texture_common(ctx, desc, at, kAdopt_GrWrapOwnership, NULL, NULL);
}

SkImage* SkImage::NewFromTextureCopy(GrContext* ctx, const GrBackendTextureDesc& desc,
                                     SkAlphaType at) {
    if (desc.fWidth <= 0 || desc.fHeight <= 0) {
        return NULL;
    }

    SkAutoTUnref<GrTexture> src(ctx->textureProvider()->wrapBackendTexture(
        desc, kBorrow_GrWrapOwnership));
    if (!src) {
        return NULL;
    }

    const bool isBudgeted = true;
    SkAutoTUnref<GrTexture> dst(GrDeepCopyTexture(src, isBudgeted));
    if (!dst) {
        return NULL;
    }

    const SkSurface::Budgeted budgeted = SkSurface::kYes_Budgeted;
    const int sampleCount = 0;  // todo: make this an explicit parameter to newSurface()?
    return SkNEW_ARGS(SkImage_Gpu, (desc.fWidth, desc.fHeight, kNeedNewImageUniqueID,
                                    at, dst, sampleCount, budgeted));
}

SkImage* SkImage::NewFromYUVTexturesCopy(GrContext* ctx , SkYUVColorSpace colorSpace,
                                         const GrBackendObject yuvTextureHandles[3],
                                         const SkISize yuvSizes[3],
                                         GrSurfaceOrigin origin) {
    const SkSurface::Budgeted budgeted = SkSurface::kYes_Budgeted;

    if (yuvSizes[0].fWidth <= 0 || yuvSizes[0].fHeight <= 0 ||
        yuvSizes[1].fWidth <= 0 || yuvSizes[1].fHeight <= 0 ||
        yuvSizes[2].fWidth <= 0 || yuvSizes[2].fHeight <= 0) {
        return NULL;
    }
    static const GrPixelConfig kConfig = kAlpha_8_GrPixelConfig;
    GrBackendTextureDesc yDesc;
    yDesc.fConfig = kConfig;
    yDesc.fOrigin = origin;
    yDesc.fSampleCnt = 0;
    yDesc.fTextureHandle = yuvTextureHandles[0];
    yDesc.fWidth = yuvSizes[0].fWidth;
    yDesc.fHeight = yuvSizes[0].fHeight;

    GrBackendTextureDesc uDesc;
    uDesc.fConfig = kConfig;
    uDesc.fOrigin = origin;
    uDesc.fSampleCnt = 0;
    uDesc.fTextureHandle = yuvTextureHandles[1];
    uDesc.fWidth = yuvSizes[1].fWidth;
    uDesc.fHeight = yuvSizes[1].fHeight;

    GrBackendTextureDesc vDesc;
    vDesc.fConfig = kConfig;
    vDesc.fOrigin = origin;
    vDesc.fSampleCnt = 0;
    vDesc.fTextureHandle = yuvTextureHandles[2];
    vDesc.fWidth = yuvSizes[2].fWidth;
    vDesc.fHeight = yuvSizes[2].fHeight;

    SkAutoTUnref<GrTexture> yTex(ctx->textureProvider()->wrapBackendTexture(
        yDesc, kBorrow_GrWrapOwnership));
    SkAutoTUnref<GrTexture> uTex(ctx->textureProvider()->wrapBackendTexture(
        uDesc, kBorrow_GrWrapOwnership));
    SkAutoTUnref<GrTexture> vTex(ctx->textureProvider()->wrapBackendTexture(
        vDesc, kBorrow_GrWrapOwnership));
    if (!yTex || !uTex || !vTex) {
        return NULL;
    }

    GrSurfaceDesc dstDesc;
    // Needs to be a render target in order to draw to it for the yuv->rgb conversion.
    dstDesc.fFlags = kRenderTarget_GrSurfaceFlag;
    dstDesc.fOrigin = origin;
    dstDesc.fWidth = yuvSizes[0].fWidth;
    dstDesc.fHeight = yuvSizes[0].fHeight;
    dstDesc.fConfig = kRGBA_8888_GrPixelConfig;
    dstDesc.fSampleCnt = 0;

    SkAutoTUnref<GrTexture> dst(ctx->textureProvider()->createTexture(dstDesc, true));
    if (!dst) {
        return NULL;
    }

    GrPaint paint;
    paint.setPorterDuffXPFactory(SkXfermode::kSrc_Mode);
    paint.addColorProcessor(GrYUVtoRGBEffect::Create(paint.getProcessorDataManager(), yTex, uTex,
                                                     vTex, yuvSizes, colorSpace))->unref();

    const SkRect rect = SkRect::MakeWH(SkIntToScalar(dstDesc.fWidth),
                                       SkIntToScalar(dstDesc.fHeight));
    GrDrawContext* drawContext = ctx->drawContext();
    drawContext->drawRect(dst->asRenderTarget(), GrClip::WideOpen(), paint, SkMatrix::I(), rect);
    ctx->flushSurfaceWrites(dst);
    return SkNEW_ARGS(SkImage_Gpu, (dstDesc.fWidth, dstDesc.fHeight, kNeedNewImageUniqueID,
                                    kOpaque_SkAlphaType, dst, 0, budgeted));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

GrTexture* GrDeepCopyTexture(GrTexture* src, bool budgeted) {
    GrContext* ctx = src->getContext();

    GrSurfaceDesc desc = src->desc();
    GrTexture* dst = ctx->textureProvider()->createTexture(desc, budgeted, NULL, 0);
    if (!dst) {
        return NULL;
    }
    
    const SkIRect srcR = SkIRect::MakeWH(desc.fWidth, desc.fHeight);
    const SkIPoint dstP = SkIPoint::Make(0, 0);
    ctx->copySurface(dst, src, srcR, dstP, GrContext::kFlushWrites_PixelOp);
    return dst;
}

