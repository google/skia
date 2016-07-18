/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAutoPixmapStorage.h"
#include "GrCaps.h"
#include "GrContext.h"
#include "GrDrawContext.h"
#include "GrImageIDTextureAdjuster.h"
#include "effects/GrYUVEffect.h"
#include "SkCanvas.h"
#include "SkBitmapCache.h"
#include "SkGrPixelRef.h"
#include "SkGrPriv.h"
#include "SkImage_Gpu.h"
#include "SkMipMap.h"
#include "SkPixelRef.h"

SkImage_Gpu::SkImage_Gpu(int w, int h, uint32_t uniqueID, SkAlphaType at, GrTexture* tex,
                         SkBudgeted budgeted)
    : INHERITED(w, h, uniqueID)
    , fTexture(SkRef(tex))
    , fAlphaType(at)
    , fBudgeted(budgeted)
    , fAddedRasterVersionToCache(false)
{
    SkASSERT(tex->width() == w);
    SkASSERT(tex->height() == h);
}

SkImage_Gpu::~SkImage_Gpu() {
    if (fAddedRasterVersionToCache.load()) {
        SkNotifyBitmapGenIDIsStale(this->uniqueID());
    }
}

extern void SkTextureImageApplyBudgetedDecision(SkImage* image) {
    if (as_IB(image)->peekTexture()) {
        ((SkImage_Gpu*)image)->applyBudgetDecision();
    }
}

static SkImageInfo make_info(int w, int h, bool isOpaque) {
    return SkImageInfo::MakeN32(w, h, isOpaque ? kOpaque_SkAlphaType : kPremul_SkAlphaType);
}

bool SkImage_Gpu::getROPixels(SkBitmap* dst, CachingHint chint) const {
    if (SkBitmapCache::Find(this->uniqueID(), dst)) {
        SkASSERT(dst->getGenerationID() == this->uniqueID());
        SkASSERT(dst->isImmutable());
        SkASSERT(dst->getPixels());
        return true;
    }

    if (!dst->tryAllocPixels(make_info(this->width(), this->height(), this->isOpaque()))) {
        return false;
    }
    if (!fTexture->readPixels(0, 0, dst->width(), dst->height(), kSkia8888_GrPixelConfig,
                              dst->getPixels(), dst->rowBytes())) {
        return false;
    }

    dst->pixelRef()->setImmutableWithID(this->uniqueID());
    if (kAllow_CachingHint == chint) {
        SkBitmapCache::Add(this->uniqueID(), *dst);
        fAddedRasterVersionToCache.store(true);
    }
    return true;
}

bool SkImage_Gpu::asBitmapForImageFilters(SkBitmap* bitmap) const {
    bitmap->setInfo(make_info(this->width(), this->height(), this->isOpaque()));
    bitmap->setPixelRef(new SkGrPixelRef(bitmap->info(), fTexture))->unref();
    bitmap->pixelRef()->setImmutableWithID(this->uniqueID());
    return true;
}

GrTexture* SkImage_Gpu::asTextureRef(GrContext* ctx, const GrTextureParams& params,
                                     SkSourceGammaTreatment gammaTreatment) const {
    return GrImageTextureAdjuster(as_IB(this)).refTextureSafeForParams(params, gammaTreatment,
                                                                       nullptr);
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
                               int srcX, int srcY, CachingHint) const {
    GrPixelConfig config = SkImageInfo2GrPixelConfig(info, *fTexture->getContext()->caps());
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

sk_sp<SkImage> SkImage_Gpu::onMakeSubset(const SkIRect& subset) const {
    GrContext* ctx = fTexture->getContext();
    GrSurfaceDesc desc = fTexture->desc();
    desc.fWidth = subset.width();
    desc.fHeight = subset.height();

    sk_sp<GrTexture> subTx(ctx->textureProvider()->createTexture(desc, fBudgeted));
    if (!subTx) {
        return nullptr;
    }
    ctx->copySurface(subTx.get(), fTexture, subset, SkIPoint::Make(0, 0));
    return sk_make_sp<SkImage_Gpu>(desc.fWidth, desc.fHeight, kNeedNewImageUniqueID,
                                   fAlphaType, subTx.get(), fBudgeted);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static sk_sp<SkImage> new_wrapped_texture_common(GrContext* ctx, const GrBackendTextureDesc& desc,
                                                 SkAlphaType at, GrWrapOwnership ownership,
                                                 SkImage::TextureReleaseProc releaseProc,
                                                 SkImage::ReleaseContext releaseCtx) {
    if (desc.fWidth <= 0 || desc.fHeight <= 0) {
        return nullptr;
    }
    SkAutoTUnref<GrTexture> tex(ctx->textureProvider()->wrapBackendTexture(desc, ownership));
    if (!tex) {
        return nullptr;
    }
    if (releaseProc) {
        tex->setRelease(releaseProc, releaseCtx);
    }

    const SkBudgeted budgeted = SkBudgeted::kNo;
    return sk_make_sp<SkImage_Gpu>(desc.fWidth, desc.fHeight, kNeedNewImageUniqueID,
                                   at, tex, budgeted);
}

sk_sp<SkImage> SkImage::MakeFromTexture(GrContext* ctx, const GrBackendTextureDesc& desc,
                                        SkAlphaType at, TextureReleaseProc releaseP,
                                        ReleaseContext releaseC) {
    return new_wrapped_texture_common(ctx, desc, at, kBorrow_GrWrapOwnership, releaseP, releaseC);
}

sk_sp<SkImage> SkImage::MakeFromAdoptedTexture(GrContext* ctx, const GrBackendTextureDesc& desc,
                                               SkAlphaType at) {
    return new_wrapped_texture_common(ctx, desc, at, kAdopt_GrWrapOwnership, nullptr, nullptr);
}

sk_sp<SkImage> SkImage::MakeFromTextureCopy(GrContext* ctx, const GrBackendTextureDesc& desc,
                                            SkAlphaType at) {
    if (desc.fWidth <= 0 || desc.fHeight <= 0) {
        return nullptr;
    }

    SkAutoTUnref<GrTexture> src(ctx->textureProvider()->wrapBackendTexture(
        desc, kBorrow_GrWrapOwnership));
    if (!src) {
        return nullptr;
    }

    SkAutoTUnref<GrTexture> dst(GrDeepCopyTexture(src, SkBudgeted::kYes));
    if (!dst) {
        return nullptr;
    }

    return sk_make_sp<SkImage_Gpu>(desc.fWidth, desc.fHeight, kNeedNewImageUniqueID, at, dst,
                                   SkBudgeted::kYes);
}

static sk_sp<SkImage> make_from_yuv_textures_copy(GrContext* ctx, SkYUVColorSpace colorSpace,
                                                  bool nv12,
                                                  const GrBackendObject yuvTextureHandles[],
                                                  const SkISize yuvSizes[],
                                                  GrSurfaceOrigin origin) {
    const SkBudgeted budgeted = SkBudgeted::kYes;

    if (yuvSizes[0].fWidth <= 0 || yuvSizes[0].fHeight <= 0 || yuvSizes[1].fWidth <= 0 ||
        yuvSizes[1].fHeight <= 0) {
        return nullptr;
    }
    if (!nv12 && (yuvSizes[2].fWidth <= 0 || yuvSizes[2].fHeight <= 0)) {
        return nullptr;
    }

    const GrPixelConfig kConfig = nv12 ? kRGBA_8888_GrPixelConfig : kAlpha_8_GrPixelConfig;

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

    sk_sp<GrTexture> yTex(
        ctx->textureProvider()->wrapBackendTexture(yDesc, kBorrow_GrWrapOwnership));
    sk_sp<GrTexture> uTex(
        ctx->textureProvider()->wrapBackendTexture(uDesc, kBorrow_GrWrapOwnership));
    sk_sp<GrTexture> vTex;
    if (nv12) {
        vTex = uTex;
    } else {
        GrBackendTextureDesc vDesc;
        vDesc.fConfig = kConfig;
        vDesc.fOrigin = origin;
        vDesc.fSampleCnt = 0;
        vDesc.fTextureHandle = yuvTextureHandles[2];
        vDesc.fWidth = yuvSizes[2].fWidth;
        vDesc.fHeight = yuvSizes[2].fHeight;

        vTex = sk_sp<GrTexture>(
            ctx->textureProvider()->wrapBackendTexture(vDesc, kBorrow_GrWrapOwnership));
    }
    if (!yTex || !uTex || !vTex) {
        return nullptr;
    }

    const int width = yuvSizes[0].fWidth;
    const int height = yuvSizes[0].fHeight;

    // Needs to be a render target in order to draw to it for the yuv->rgb conversion.
    sk_sp<GrDrawContext> drawContext(ctx->newDrawContext(SkBackingFit::kExact,
                                                         width, height,
                                                         kRGBA_8888_GrPixelConfig,
                                                         0,
                                                         origin));
    if (!drawContext) {
        return nullptr;
    }

    GrPaint paint;
    paint.setPorterDuffXPFactory(SkXfermode::kSrc_Mode);
    paint.addColorFragmentProcessor(
        GrYUVEffect::MakeYUVToRGB(yTex.get(), uTex.get(), vTex.get(), yuvSizes, colorSpace, nv12));

    const SkRect rect = SkRect::MakeWH(SkIntToScalar(width), SkIntToScalar(height));

    drawContext->drawRect(GrNoClip(), paint, SkMatrix::I(), rect);
    ctx->flushSurfaceWrites(drawContext->accessRenderTarget());
    return sk_make_sp<SkImage_Gpu>(width, height, kNeedNewImageUniqueID,
                                   kOpaque_SkAlphaType,
                                   drawContext->asTexture().get(), budgeted);
}

sk_sp<SkImage> SkImage::MakeFromYUVTexturesCopy(GrContext* ctx, SkYUVColorSpace colorSpace,
                                                const GrBackendObject yuvTextureHandles[3],
                                                const SkISize yuvSizes[3], GrSurfaceOrigin origin) {
    return make_from_yuv_textures_copy(ctx, colorSpace, false, yuvTextureHandles, yuvSizes, origin);
}

sk_sp<SkImage> SkImage::MakeFromNV12TexturesCopy(GrContext* ctx, SkYUVColorSpace colorSpace,
                                                 const GrBackendObject yuvTextureHandles[2],
                                                 const SkISize yuvSizes[2],
                                                 GrSurfaceOrigin origin) {
    return make_from_yuv_textures_copy(ctx, colorSpace, true, yuvTextureHandles, yuvSizes, origin);
}

static sk_sp<SkImage> create_image_from_maker(GrTextureMaker* maker, SkAlphaType at, uint32_t id) {
    SkAutoTUnref<GrTexture> texture(maker->refTextureForParams(GrTextureParams::ClampNoFilter(),
                                                               SkSourceGammaTreatment::kRespect));
    if (!texture) {
        return nullptr;
    }
    return sk_make_sp<SkImage_Gpu>(texture->width(), texture->height(), id, at, texture,
                                   SkBudgeted::kNo);
}

sk_sp<SkImage> SkImage::makeTextureImage(GrContext *context) const {
    if (!context) {
        return nullptr;
    }
    if (GrTexture* peek = as_IB(this)->peekTexture()) {
        return peek->getContext() == context ? sk_ref_sp(const_cast<SkImage*>(this)) : nullptr;
    }
    // No way to check whether a image is premul or not?
    SkAlphaType at = this->isOpaque() ? kOpaque_SkAlphaType : kPremul_SkAlphaType;

    if (SkImageCacherator* cacher = as_IB(this)->peekCacherator()) {
        GrImageTextureMaker maker(context, cacher, this, kDisallow_CachingHint);
        return create_image_from_maker(&maker, at, this->uniqueID());
    }
    SkBitmap bmp;
    if (!this->asLegacyBitmap(&bmp, kRO_LegacyBitmapMode)) {
        return nullptr;
    }
    GrBitmapTextureMaker maker(context, bmp);
    return create_image_from_maker(&maker, at, this->uniqueID());
}

sk_sp<SkImage> SkImage::MakeTextureFromPixmap(GrContext* ctx, const SkPixmap& pixmap,
                                              SkBudgeted budgeted) {
    if (!ctx) {
        return nullptr;
    }
    SkAutoTUnref<GrTexture> texture(GrUploadPixmapToTexture(ctx, pixmap, budgeted));
    if (!texture) {
        return nullptr;
    }
    return sk_make_sp<SkImage_Gpu>(texture->width(), texture->height(), kNeedNewImageUniqueID,
                                   pixmap.alphaType(), texture, budgeted);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

class DeferredTextureImage {
public:
    SkImage* newImage(GrContext* context, SkBudgeted) const;

private:
    uint32_t fContextUniqueID;
    struct MipMapLevelData {
        void* fPixelData;
        size_t fRowBytes;
    };
    struct Data {
        SkImageInfo fInfo;
        int         fColorTableCnt;
        uint32_t*   fColorTableData;
        int         fMipMapLevelCount;
        // The fMipMapLevelData array may contain more than 1 element.
        // It contains fMipMapLevelCount elements.
        // That means this struct's size is not known at compile-time.
        MipMapLevelData fMipMapLevelData[1];
    };
    Data fData;

    friend class SkImage;
};

size_t SkImage::getDeferredTextureImageData(const GrContextThreadSafeProxy& proxy,
                                            const DeferredTextureImageUsageParams params[],
                                            int paramCnt, void* buffer) const {
    // Extract relevant min/max values from the params array.
    int lowestPreScaleMipLevel = params[0].fPreScaleMipLevel;
    SkFilterQuality highestFilterQuality = params[0].fQuality;
    for (int i = 1; i < paramCnt; ++i) {
        if (lowestPreScaleMipLevel > params[i].fPreScaleMipLevel)
            lowestPreScaleMipLevel = params[i].fPreScaleMipLevel;
        if (highestFilterQuality < params[i].fQuality)
            highestFilterQuality = params[i].fQuality;
    }

    const bool fillMode = SkToBool(buffer);
    if (fillMode && !SkIsAlign8(reinterpret_cast<intptr_t>(buffer))) {
        return 0;
    }

    // Calculate scaling parameters.
    bool isScaled = lowestPreScaleMipLevel != 0;

    SkISize scaledSize;
    if (isScaled) {
        // SkMipMap::ComputeLevelSize takes an index into an SkMipMap. SkMipMaps don't contain the
        // base level, so to get an SkMipMap index we must subtract one from the GL MipMap level.
        scaledSize = SkMipMap::ComputeLevelSize(this->width(), this->height(),
                                                lowestPreScaleMipLevel - 1);
    } else {
        scaledSize = SkISize::Make(this->width(), this->height());
    }

    // We never want to scale at higher than SW medium quality, as SW medium matches GPU high.
    SkFilterQuality scaleFilterQuality = highestFilterQuality;
    if (scaleFilterQuality > kMedium_SkFilterQuality) {
        scaleFilterQuality = kMedium_SkFilterQuality;
    }

    const int maxTextureSize = proxy.fCaps->maxTextureSize();
    if (scaledSize.width() > maxTextureSize || scaledSize.height() > maxTextureSize) {
        return 0;
    }

    SkAutoPixmapStorage pixmap;
    SkImageInfo info;
    size_t pixelSize = 0;
    size_t ctSize = 0;
    int ctCount = 0;
    if (!isScaled && this->peekPixels(&pixmap)) {
        info = pixmap.info();
        pixelSize = SkAlign8(pixmap.getSafeSize());
        if (pixmap.ctable()) {
            ctCount = pixmap.ctable()->count();
            ctSize = SkAlign8(pixmap.ctable()->count() * 4);
        }
    } else {
        // Here we're just using presence of data to know whether there is a codec behind the image.
        // In the future we will access the cacherator and get the exact data that we want to (e.g.
        // yuv planes) upload.
        SkAutoTUnref<SkData> data(this->refEncoded());
        if (!data && !this->peekPixels(nullptr)) {
            return 0;
        }
        SkAlphaType at = this->isOpaque() ? kOpaque_SkAlphaType : kPremul_SkAlphaType;
        info = SkImageInfo::MakeN32(scaledSize.width(), scaledSize.height(), at);
        pixelSize = SkAlign8(SkAutoPixmapStorage::AllocSize(info, nullptr));
        if (fillMode) {
            pixmap.alloc(info);
            if (isScaled) {
                if (!this->scalePixels(pixmap, scaleFilterQuality,
                                       SkImage::kDisallow_CachingHint)) {
                    return 0;
                }
            } else {
                if (!this->readPixels(pixmap, 0, 0, SkImage::kDisallow_CachingHint)) {
                    return 0;
                }
            }
            SkASSERT(!pixmap.ctable());
        }
    }
    int mipMapLevelCount = 1;
    size_t size = 0;
    size_t dtiSize = SkAlign8(sizeof(DeferredTextureImage));
    size += dtiSize;
    size += mipMapLevelCount * sizeof(DeferredTextureImage::MipMapLevelData);
    size_t pixelOffset = size;
    size += pixelSize;
    size_t ctOffset = size;
    size += ctSize;
    if (!fillMode) {
        return size;
    }
    intptr_t bufferAsInt = reinterpret_cast<intptr_t>(buffer);
    void* pixels = reinterpret_cast<void*>(bufferAsInt + pixelOffset);
    SkPMColor* ct = nullptr;
    if (ctSize) {
        ct = reinterpret_cast<SkPMColor*>(bufferAsInt + ctOffset);
    }

    memcpy(pixels, pixmap.addr(), pixmap.getSafeSize());
    if (ctSize) {
        memcpy(ct, pixmap.ctable()->readColors(), ctSize);
    }

    SkASSERT(info == pixmap.info());
    size_t rowBytes = pixmap.rowBytes();
    DeferredTextureImage* dti = new (buffer) DeferredTextureImage();
    dti->fContextUniqueID = proxy.fContextUniqueID;
    dti->fData.fInfo = info;
    dti->fData.fColorTableCnt = ctCount;
    dti->fData.fColorTableData = ct;
    dti->fData.fMipMapLevelCount = mipMapLevelCount;
    dti->fData.fMipMapLevelData[0].fPixelData = pixels;
    dti->fData.fMipMapLevelData[0].fRowBytes = rowBytes;
    return size;
}

sk_sp<SkImage> SkImage::MakeFromDeferredTextureImageData(GrContext* context, const void* data,
                                                         SkBudgeted budgeted) {
    if (!data) {
        return nullptr;
    }
    const DeferredTextureImage* dti = reinterpret_cast<const DeferredTextureImage*>(data);

    if (!context || context->uniqueID() != dti->fContextUniqueID) {
        return nullptr;
    }
    SkAutoTUnref<SkColorTable> colorTable;
    if (dti->fData.fColorTableCnt) {
        SkASSERT(dti->fData.fColorTableData);
        colorTable.reset(new SkColorTable(dti->fData.fColorTableData, dti->fData.fColorTableCnt));
    }
    SkASSERT(dti->fData.fMipMapLevelCount == 1);
    SkPixmap pixmap;
    pixmap.reset(dti->fData.fInfo, dti->fData.fMipMapLevelData[0].fPixelData,
                 dti->fData.fMipMapLevelData[0].fRowBytes, colorTable.get());
    return SkImage::MakeTextureFromPixmap(context, pixmap, budgeted);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

GrTexture* GrDeepCopyTexture(GrTexture* src, SkBudgeted budgeted) {
    GrContext* ctx = src->getContext();

    GrSurfaceDesc desc = src->desc();
    GrTexture* dst = ctx->textureProvider()->createTexture(desc, budgeted, nullptr, 0);
    if (!dst) {
        return nullptr;
    }

    const SkIRect srcR = SkIRect::MakeWH(desc.fWidth, desc.fHeight);
    const SkIPoint dstP = SkIPoint::Make(0, 0);
    ctx->copySurface(dst, src, srcR, dstP);
    ctx->flushSurfaceWrites(dst);
    return dst;
}

sk_sp<SkImage> SkImage::MakeTextureFromMipMap(GrContext* ctx, const SkImageInfo& info,
                                              const GrMipLevel* texels, int mipLevelCount,
                                              SkBudgeted budgeted) {
    if (!ctx) {
        return nullptr;
    }
    SkAutoTUnref<GrTexture> texture(GrUploadMipMapToTexture(ctx, info, texels, mipLevelCount));
    if (!texture) {
        return nullptr;
    }
    return sk_make_sp<SkImage_Gpu>(texture->width(), texture->height(), kNeedNewImageUniqueID,
                                   info.alphaType(), texture, budgeted);
}
