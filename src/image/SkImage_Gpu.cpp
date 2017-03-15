/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <cstddef>
#include <cstring>
#include <type_traits>

#include "SkAutoPixmapStorage.h"
#include "GrBitmapTextureMaker.h"
#include "GrCaps.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrGpu.h"
#include "GrImageTextureMaker.h"
#include "GrRenderTargetContext.h"
#include "GrResourceProvider.h"
#include "GrSemaphore.h"
#include "GrTextureAdjuster.h"
#include "GrTexturePriv.h"
#include "GrTextureProxy.h"
#include "GrTextureToYUVPlanes.h"
#include "effects/GrNonlinearColorSpaceXformEffect.h"
#include "effects/GrYUVEffect.h"
#include "SkCanvas.h"
#include "SkCrossContextImageData.h"
#include "SkBitmapCache.h"
#include "SkGr.h"
#include "SkImage_Gpu.h"
#include "SkImageCacherator.h"
#include "SkImageInfoPriv.h"
#include "SkMipMap.h"
#include "SkPixelRef.h"
#include "SkReadPixelsRec.h"

SkImage_Gpu::SkImage_Gpu(uint32_t uniqueID, SkAlphaType at, sk_sp<GrTexture> tex,
                         sk_sp<SkColorSpace> colorSpace, SkBudgeted budgeted)
    : INHERITED(tex->width(), tex->height(), uniqueID)
    , fContext(tex->getContext())
    , fProxy(GrSurfaceProxy::MakeWrapped(std::move(tex)))
    , fAlphaType(at)
    , fBudgeted(budgeted)
    , fColorSpace(std::move(colorSpace))
    , fAddedRasterVersionToCache(false) {
}

SkImage_Gpu::SkImage_Gpu(GrContext* context, uint32_t uniqueID, SkAlphaType at,
                         sk_sp<GrTextureProxy> proxy,
                         sk_sp<SkColorSpace> colorSpace, SkBudgeted budgeted)
    : INHERITED(proxy->width(), proxy->height(), uniqueID)
    , fContext(context)
    , fProxy(std::move(proxy))
    , fAlphaType(at)
    , fBudgeted(budgeted)
    , fColorSpace(std::move(colorSpace))
    , fAddedRasterVersionToCache(false) {
}

SkImage_Gpu::~SkImage_Gpu() {
    if (fAddedRasterVersionToCache.load()) {
        SkNotifyBitmapGenIDIsStale(this->uniqueID());
    }
}

extern void SkTextureImageApplyBudgetedDecision(SkImage* image) {
    if (image->isTextureBacked()) {
        ((SkImage_Gpu*)image)->applyBudgetDecision();
    }
}

SkImageInfo SkImage_Gpu::onImageInfo() const {
    SkColorType ct;
    if (!GrPixelConfigToColorType(fProxy->config(), &ct)) {
        ct = kUnknown_SkColorType;
    }
    return SkImageInfo::Make(fProxy->width(), fProxy->height(), ct, fAlphaType, fColorSpace);
}

static SkImageInfo make_info(int w, int h, SkAlphaType at, sk_sp<SkColorSpace> colorSpace) {
    return SkImageInfo::MakeN32(w, h, at, std::move(colorSpace));
}

bool SkImage_Gpu::getROPixels(SkBitmap* dst, SkColorSpace* dstColorSpace,
                              CachingHint chint) const {
    if (SkBitmapCache::Find(this->uniqueID(), dst)) {
        SkASSERT(dst->getGenerationID() == this->uniqueID());
        SkASSERT(dst->isImmutable());
        SkASSERT(dst->getPixels());
        return true;
    }

    SkImageInfo ii = make_info(this->width(), this->height(), this->alphaType(),
                               sk_ref_sp(dstColorSpace));
    if (!dst->tryAllocPixels(ii)) {
        return false;
    }

    sk_sp<GrSurfaceContext> sContext = fContext->contextPriv().makeWrappedSurfaceContext(
                                                                                    fProxy,
                                                                                    fColorSpace);
    if (!sContext) {
        return false;
    }

    if (!sContext->readPixels(dst->info(), dst->getPixels(), dst->rowBytes(), 0, 0)) {
        return false;
    }

    dst->pixelRef()->setImmutableWithID(this->uniqueID());
    if (kAllow_CachingHint == chint) {
        SkBitmapCache::Add(this->uniqueID(), *dst);
        fAddedRasterVersionToCache.store(true);
    }
    return true;
}

sk_sp<GrTextureProxy> SkImage_Gpu::asTextureProxyRef(GrContext* context,
                                                     const GrSamplerParams& params,
                                                     SkColorSpace* dstColorSpace,
                                                     sk_sp<SkColorSpace>* texColorSpace,
                                                     SkScalar scaleAdjust[2]) const {
    sk_sp<GrTexture> tex(this->asTextureRef(context, params, dstColorSpace,
                                            texColorSpace, scaleAdjust));

    return GrSurfaceProxy::MakeWrapped(std::move(tex));
}

GrTexture* SkImage_Gpu::asTextureRef(GrContext* ctx, const GrSamplerParams& params,
                                     SkColorSpace* dstColorSpace,
                                     sk_sp<SkColorSpace>* texColorSpace,
                                     SkScalar scaleAdjust[2]) const {
    if (ctx != fContext) {
        SkASSERT(0);
        return nullptr;
    }

    if (texColorSpace) {
        *texColorSpace = this->fColorSpace;
    }
    GrTexture* texture = fProxy->instantiate(fContext->resourceProvider());
    if (!texture) {
        return nullptr;
    }

    GrTextureAdjuster adjuster(texture, this->alphaType(), this->bounds(),
                               this->uniqueID(), this->fColorSpace.get());
    return adjuster.refTextureSafeForParams(params, nullptr, scaleAdjust);
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

bool SkImage_Gpu::onReadYUV8Planes(const SkISize sizes[3], void* const planes[3],
                                   const size_t rowBytes[3], SkYUVColorSpace colorSpace) const {
    if (GrTextureToYUVPlanes(fContext, fProxy, sizes, planes, rowBytes, colorSpace)) {
        return true;
    }

    return INHERITED::onReadYUV8Planes(sizes, planes, rowBytes, colorSpace);
}

bool SkImage_Gpu::onReadPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRB,
                               int srcX, int srcY, CachingHint) const {
    if (!SkImageInfoValidConversion(dstInfo, this->onImageInfo())) {
        return false;
    }

    SkReadPixelsRec rec(dstInfo, dstPixels, dstRB, srcX, srcY);
    if (!rec.trim(this->width(), this->height())) {
        return false;
    }

    // TODO: this seems to duplicate code in GrTextureContext::onReadPixels and
    // GrRenderTargetContext::onReadPixels
    uint32_t flags = 0;
    if (kUnpremul_SkAlphaType == rec.fInfo.alphaType() && kPremul_SkAlphaType == fAlphaType) {
        // let the GPU perform this transformation for us
        flags = GrContext::kUnpremul_PixelOpsFlag;
    }

    sk_sp<GrSurfaceContext> sContext = fContext->contextPriv().makeWrappedSurfaceContext(
                                                                                    fProxy,
                                                                                    fColorSpace);
    if (!sContext) {
        return false;
    }

    if (!sContext->readPixels(rec.fInfo, rec.fPixels, rec.fRowBytes, rec.fX, rec.fY, flags)) {
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
    if (kPremul_SkAlphaType == rec.fInfo.alphaType() && kUnpremul_SkAlphaType == fAlphaType) {
        apply_premul(rec.fInfo, rec.fPixels, rec.fRowBytes);
    }
    return true;
}

sk_sp<SkImage> SkImage_Gpu::onMakeSubset(const SkIRect& subset) const {
    GrSurfaceDesc desc = fProxy->desc();
    desc.fWidth = subset.width();
    desc.fHeight = subset.height();

    sk_sp<GrSurfaceContext> sContext(fContext->contextPriv().makeDeferredSurfaceContext(
                                                                        desc,
                                                                        SkBackingFit::kExact,
                                                                        fBudgeted));
    if (!sContext) {
        return nullptr;
    }

    if (!sContext->copy(fProxy.get(), subset, SkIPoint::Make(0, 0))) {
        return nullptr;
    }

    // MDB: this call is okay bc we know 'sContext' was kExact
    return sk_make_sp<SkImage_Gpu>(fContext, kNeedNewImageUniqueID,
                                   fAlphaType, sContext->asTextureProxyRef(),
                                   fColorSpace, fBudgeted);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static sk_sp<SkImage> new_wrapped_texture_common(GrContext* ctx, const GrBackendTextureDesc& desc,
                                                 SkAlphaType at, sk_sp<SkColorSpace> colorSpace,
                                                 GrWrapOwnership ownership,
                                                 SkImage::TextureReleaseProc releaseProc,
                                                 SkImage::ReleaseContext releaseCtx) {
    if (desc.fWidth <= 0 || desc.fHeight <= 0) {
        return nullptr;
    }
    sk_sp<GrTexture> tex = ctx->resourceProvider()->wrapBackendTexture(desc, ownership);
    if (!tex) {
        return nullptr;
    }
    if (releaseProc) {
        tex->setRelease(releaseProc, releaseCtx);
    }

    const SkBudgeted budgeted = (kAdoptAndCache_GrWrapOwnership == ownership)
            ? SkBudgeted::kYes : SkBudgeted::kNo;
    return sk_make_sp<SkImage_Gpu>(kNeedNewImageUniqueID,
                                   at, std::move(tex), std::move(colorSpace), budgeted);
}

sk_sp<SkImage> SkImage::MakeFromTexture(GrContext* ctx, const GrBackendTextureDesc& desc,
                                        SkAlphaType at, sk_sp<SkColorSpace> cs,
                                        TextureReleaseProc releaseP, ReleaseContext releaseC) {
    return new_wrapped_texture_common(ctx, desc, at, std::move(cs), kBorrow_GrWrapOwnership,
                                      releaseP, releaseC);
}

sk_sp<SkImage> SkImage::MakeFromAdoptedTexture(GrContext* ctx, const GrBackendTextureDesc& desc,
                                               SkAlphaType at, sk_sp<SkColorSpace> cs) {
    return new_wrapped_texture_common(ctx, desc, at, std::move(cs), kAdopt_GrWrapOwnership,
                                      nullptr, nullptr);
}

static sk_sp<SkImage> make_from_yuv_textures_copy(GrContext* ctx, SkYUVColorSpace colorSpace,
                                                  bool nv12,
                                                  const GrBackendObject yuvTextureHandles[],
                                                  const SkISize yuvSizes[],
                                                  GrSurfaceOrigin origin,
                                                  sk_sp<SkColorSpace> imageColorSpace) {
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

    sk_sp<GrSurfaceProxy> yProxy = GrSurfaceProxy::MakeWrappedBackend(ctx, yDesc);
    sk_sp<GrSurfaceProxy> uProxy = GrSurfaceProxy::MakeWrappedBackend(ctx, uDesc);
    sk_sp<GrSurfaceProxy> vProxy;

    if (nv12) {
        vProxy = uProxy;
    } else {
        GrBackendTextureDesc vDesc;
        vDesc.fConfig = kConfig;
        vDesc.fOrigin = origin;
        vDesc.fSampleCnt = 0;
        vDesc.fTextureHandle = yuvTextureHandles[2];
        vDesc.fWidth = yuvSizes[2].fWidth;
        vDesc.fHeight = yuvSizes[2].fHeight;

        vProxy = GrSurfaceProxy::MakeWrappedBackend(ctx, vDesc);
    }
    if (!yProxy || !uProxy || !vProxy) {
        return nullptr;
    }

    const int width = yuvSizes[0].fWidth;
    const int height = yuvSizes[0].fHeight;

    // Needs to be a render target in order to draw to it for the yuv->rgb conversion.
    sk_sp<GrRenderTargetContext> renderTargetContext(ctx->makeRenderTargetContext(
                                                                         SkBackingFit::kExact,
                                                                         width, height,
                                                                         kRGBA_8888_GrPixelConfig,
                                                                         std::move(imageColorSpace),
                                                                         0,
                                                                         origin));
    if (!renderTargetContext) {
        return nullptr;
    }

    GrPaint paint;
    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
    paint.addColorFragmentProcessor(
        GrYUVEffect::MakeYUVToRGB(ctx->resourceProvider(),
                                  sk_ref_sp(yProxy->asTextureProxy()),
                                  sk_ref_sp(uProxy->asTextureProxy()),
                                  sk_ref_sp(vProxy->asTextureProxy()), yuvSizes, colorSpace, nv12));

    const SkRect rect = SkRect::MakeIWH(width, height);

    renderTargetContext->drawRect(GrNoClip(), std::move(paint), GrAA::kNo, SkMatrix::I(), rect);

    if (!renderTargetContext->accessRenderTarget()) {
        return nullptr;
    }
    ctx->flushSurfaceWrites(renderTargetContext->accessRenderTarget());

    // MDB: this call is okay bc we know 'renderTargetContext' was exact
    return sk_make_sp<SkImage_Gpu>(ctx, kNeedNewImageUniqueID,
                                   kOpaque_SkAlphaType, renderTargetContext->asTextureProxyRef(),
                                   renderTargetContext->refColorSpace(), budgeted);
}

sk_sp<SkImage> SkImage::MakeFromYUVTexturesCopy(GrContext* ctx, SkYUVColorSpace colorSpace,
                                                const GrBackendObject yuvTextureHandles[3],
                                                const SkISize yuvSizes[3], GrSurfaceOrigin origin,
                                                sk_sp<SkColorSpace> imageColorSpace) {
    return make_from_yuv_textures_copy(ctx, colorSpace, false, yuvTextureHandles, yuvSizes, origin,
                                       std::move(imageColorSpace));
}

sk_sp<SkImage> SkImage::MakeFromNV12TexturesCopy(GrContext* ctx, SkYUVColorSpace colorSpace,
                                                 const GrBackendObject yuvTextureHandles[2],
                                                 const SkISize yuvSizes[2],
                                                 GrSurfaceOrigin origin,
                                                 sk_sp<SkColorSpace> imageColorSpace) {
    return make_from_yuv_textures_copy(ctx, colorSpace, true, yuvTextureHandles, yuvSizes, origin,
                                       std::move(imageColorSpace));
}

static sk_sp<SkImage> create_image_from_maker(GrTextureMaker* maker, SkAlphaType at, uint32_t id,
                                              SkColorSpace* dstColorSpace) {
    sk_sp<SkColorSpace> texColorSpace;
    sk_sp<GrTexture> texture(maker->refTextureForParams(GrSamplerParams::ClampNoFilter(),
                                                        dstColorSpace, &texColorSpace, nullptr));
    if (!texture) {
        return nullptr;
    }
    return sk_make_sp<SkImage_Gpu>(id, at, std::move(texture),
                                   std::move(texColorSpace), SkBudgeted::kNo);
}

sk_sp<SkImage> SkImage::makeTextureImage(GrContext* context, SkColorSpace* dstColorSpace) const {
    if (!context) {
        return nullptr;
    }
    if (GrTexture* peek = as_IB(this)->peekTexture()) {
        return peek->getContext() == context ? sk_ref_sp(const_cast<SkImage*>(this)) : nullptr;
    }

    if (SkImageCacherator* cacher = as_IB(this)->peekCacherator()) {
        GrImageTextureMaker maker(context, cacher, this, kDisallow_CachingHint);
        return create_image_from_maker(&maker, this->alphaType(), this->uniqueID(), dstColorSpace);
    }

    if (const SkBitmap* bmp = as_IB(this)->onPeekBitmap()) {
        GrBitmapTextureMaker maker(context, *bmp);
        return create_image_from_maker(&maker, this->alphaType(), this->uniqueID(), dstColorSpace);
    }
    return nullptr;
}

std::unique_ptr<SkCrossContextImageData> SkCrossContextImageData::MakeFromEncoded(
        GrContext* context, sk_sp<SkData> encoded, SkColorSpace* dstColorSpace) {
    sk_sp<SkImage> codecImage = SkImage::MakeFromEncoded(std::move(encoded));
    if (!codecImage) {
        return nullptr;
    }

    // Some backends or drivers don't support (safely) moving resources between contexts
    if (!context->caps()->crossContextTextureSupport()) {
        return std::unique_ptr<SkCrossContextImageData>(
            new SkCrossContextImageData(std::move(codecImage)));
    }

    sk_sp<SkImage> textureImage = codecImage->makeTextureImage(context, dstColorSpace);
    if (!textureImage) {
        // TODO: Force decode to raster here? Do mip-mapping, like getDeferredTextureImageData?
        return std::unique_ptr<SkCrossContextImageData>(
            new SkCrossContextImageData(std::move(codecImage)));
    }

    // Crack open the gpu image, extract the backend data, stick it in the SkCCID
    GrTexture* texture = as_IB(textureImage)->peekTexture();
    SkASSERT(texture);

    GrBackendTextureDesc desc;
    desc.fFlags = kNone_GrBackendTextureFlag;
    desc.fOrigin = texture->origin();
    desc.fWidth = texture->width();
    desc.fHeight = texture->height();
    desc.fConfig = texture->config();
    desc.fSampleCnt = 0;

    auto textureData = texture->texturePriv().detachBackendTexture();
    SkASSERT(textureData);

    SkImageInfo info = as_IB(textureImage)->onImageInfo();
    return std::unique_ptr<SkCrossContextImageData>(new SkCrossContextImageData(
        desc, std::move(textureData), info.alphaType(), info.refColorSpace()));
}

sk_sp<SkImage> SkImage::MakeFromCrossContextImageData(
        GrContext* context, std::unique_ptr<SkCrossContextImageData> ccid) {
    if (ccid->fImage) {
        // No pre-existing GPU resource. We could upload it now (with makeTextureImage),
        // but we'd need a dstColorSpace.
        return ccid->fImage;
    }

    if (ccid->fTextureData) {
        ccid->fTextureData->attachToContext(context);
    }

    // This texture was created by Ganesh on another thread (see MakeFromEncoded, above).
    // Thus, we can import it back into our cache and treat it as our own (again).
    GrWrapOwnership ownership = kAdoptAndCache_GrWrapOwnership;
    return new_wrapped_texture_common(context, ccid->fDesc, ccid->fAlphaType,
                                      std::move(ccid->fColorSpace), ownership, nullptr, nullptr);
}

sk_sp<SkImage> SkImage::makeNonTextureImage() const {
    if (!this->isTextureBacked()) {
        return sk_ref_sp(const_cast<SkImage*>(this));
    }
    SkImageInfo info = as_IB(this)->onImageInfo();
    size_t rowBytes = info.minRowBytes();
    size_t size = info.getSafeSize(rowBytes);
    auto data = SkData::MakeUninitialized(size);
    if (!data) {
        return nullptr;
    }
    SkPixmap pm(info, data->writable_data(), rowBytes);
    if (!this->readPixels(pm, 0, 0, kDisallow_CachingHint)) {
        return nullptr;
    }
    return MakeRasterData(info, data, rowBytes);
}

sk_sp<SkImage> SkImage::MakeTextureFromPixmap(GrContext* ctx, const SkPixmap& pixmap,
                                              SkBudgeted budgeted) {
    if (!ctx) {
        return nullptr;
    }
    sk_sp<GrTexture> texture(GrUploadPixmapToTexture(ctx, pixmap, budgeted));
    if (!texture) {
        return nullptr;
    }
    return sk_make_sp<SkImage_Gpu>(kNeedNewImageUniqueID,
                                   pixmap.alphaType(), std::move(texture),
                                   sk_ref_sp(pixmap.info().colorSpace()), budgeted);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

namespace {
struct MipMapLevelData {
    void* fPixelData;
    size_t fRowBytes;
};

struct DeferredTextureImage {
    uint32_t                      fContextUniqueID;
    // Right now, the destination color mode is only considered when generating mipmaps
    SkDestinationSurfaceColorMode fColorMode;
    // We don't store a SkImageInfo because it contains a ref-counted SkColorSpace.
    int                           fWidth;
    int                           fHeight;
    SkColorType                   fColorType;
    SkAlphaType                   fAlphaType;
    void*                         fColorSpace;
    size_t                        fColorSpaceSize;
    int                           fMipMapLevelCount;
    // The fMipMapLevelData array may contain more than 1 element.
    // It contains fMipMapLevelCount elements.
    // That means this struct's size is not known at compile-time.
    MipMapLevelData               fMipMapLevelData[1];
};
}  // anonymous namespace

static bool should_use_mip_maps(const SkImage::DeferredTextureImageUsageParams & param) {
    // There is a bug in the mipmap pre-generation logic in use in getDeferredTextureImageData.
    // This can cause runaway memory leaks, so we are disabling this path until we can
    // investigate further. crbug.com/669775
    return false;
}

namespace {

class DTIBufferFiller
{
public:
    explicit DTIBufferFiller(char* bufferAsCharPtr)
        : bufferAsCharPtr_(bufferAsCharPtr) {}

    void fillMember(const void* source, size_t memberOffset, size_t size) {
        memcpy(bufferAsCharPtr_ + memberOffset, source, size);
    }

private:

    char* bufferAsCharPtr_;
};
}

#define FILL_MEMBER(bufferFiller, member, source) \
    bufferFiller.fillMember(source, \
               offsetof(DeferredTextureImage, member), \
               sizeof(DeferredTextureImage::member));

size_t SkImage::getDeferredTextureImageData(const GrContextThreadSafeProxy& proxy,
                                            const DeferredTextureImageUsageParams params[],
                                            int paramCnt, void* buffer,
                                            SkColorSpace* dstColorSpace) const {
    // Extract relevant min/max values from the params array.
    int lowestPreScaleMipLevel = params[0].fPreScaleMipLevel;
    SkFilterQuality highestFilterQuality = params[0].fQuality;
    bool useMipMaps = should_use_mip_maps(params[0]);
    for (int i = 1; i < paramCnt; ++i) {
        if (lowestPreScaleMipLevel > params[i].fPreScaleMipLevel)
            lowestPreScaleMipLevel = params[i].fPreScaleMipLevel;
        if (highestFilterQuality < params[i].fQuality)
            highestFilterQuality = params[i].fQuality;
        useMipMaps |= should_use_mip_maps(params[i]);
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
    if (!isScaled && this->peekPixels(&pixmap) && !pixmap.ctable()) {
        info = pixmap.info();
        pixelSize = SkAlign8(pixmap.getSafeSize());
    } else {
        // Here we're just using presence of data to know whether there is a codec behind the image.
        // In the future we will access the cacherator and get the exact data that we want to (e.g.
        // yuv planes) upload.
        sk_sp<SkData> data(this->refEncoded());
        if (!data && !this->peekPixels(nullptr)) {
            return 0;
        }
        if (SkImageCacherator* cacher = as_IB(this)->peekCacherator()) {
            // Generator backed image. Tweak info to trigger correct kind of decode.
            SkImageCacherator::CachedFormat cacheFormat = cacher->chooseCacheFormat(
                dstColorSpace, proxy.fCaps.get());
            info = cacher->buildCacheInfo(cacheFormat).makeWH(scaledSize.width(),
                                                              scaledSize.height());
        } else {
            info = as_IB(this)->onImageInfo().makeWH(scaledSize.width(), scaledSize.height());
        }
        if (kIndex_8_SkColorType == info.colorType()) {
            // Force Index8 to be N32 instead. Index8 is unsupported in Ganesh.
            info = info.makeColorType(kN32_SkColorType);
        }
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
    if (useMipMaps) {
        // SkMipMap only deals with the mipmap levels it generates, which does
        // not include the base level.
        // That means it generates and holds levels 1-x instead of 0-x.
        // So the total mipmap level count is 1 more than what
        // SkMipMap::ComputeLevelCount returns.
        mipMapLevelCount = SkMipMap::ComputeLevelCount(scaledSize.width(), scaledSize.height()) + 1;

        // We already initialized pixelSize to the size of the base level.
        // SkMipMap will generate the extra mipmap levels. Their sizes need to
        // be added to the total.
        // Index 0 here does not refer to the base mipmap level -- it is
        // SkMipMap's first generated mipmap level (level 1).
        for (int currentMipMapLevelIndex = mipMapLevelCount - 2; currentMipMapLevelIndex >= 0;
             currentMipMapLevelIndex--) {
            SkISize mipSize = SkMipMap::ComputeLevelSize(scaledSize.width(), scaledSize.height(),
                                                         currentMipMapLevelIndex);
            SkImageInfo mipInfo = info.makeWH(mipSize.fWidth, mipSize.fHeight);
            pixelSize += SkAlign8(SkAutoPixmapStorage::AllocSize(mipInfo, nullptr));
        }
    }
    size_t size = 0;
    size_t dtiSize = SkAlign8(sizeof(DeferredTextureImage));
    size += dtiSize;
    size += (mipMapLevelCount - 1) * sizeof(MipMapLevelData);
    // We subtract 1 because DeferredTextureImage already includes the base
    // level in its size
    size_t pixelOffset = size;
    size += pixelSize;
    size_t colorSpaceOffset = 0;
    size_t colorSpaceSize = 0;
    if (info.colorSpace()) {
        colorSpaceOffset = size;
        colorSpaceSize = info.colorSpace()->writeToMemory(nullptr);
        size += colorSpaceSize;
    }
    if (!fillMode) {
        return size;
    }
    char* bufferAsCharPtr = reinterpret_cast<char*>(buffer);
    char* pixelsAsCharPtr = bufferAsCharPtr + pixelOffset;
    void* pixels = pixelsAsCharPtr;

    memcpy(reinterpret_cast<void*>(SkAlign8(reinterpret_cast<uintptr_t>(pixelsAsCharPtr))),
                                   pixmap.addr(), pixmap.getSafeSize());

    // If the context has sRGB support, and we're intending to render to a surface with an attached
    // color space, and the image has an sRGB-like color space attached, then use our gamma (sRGB)
    // aware mip-mapping.
    SkDestinationSurfaceColorMode colorMode = SkDestinationSurfaceColorMode::kLegacy;
    if (proxy.fCaps->srgbSupport() && SkToBool(dstColorSpace) &&
        info.colorSpace() && info.colorSpace()->gammaCloseToSRGB()) {
        colorMode = SkDestinationSurfaceColorMode::kGammaAndColorSpaceAware;
    }

    SkASSERT(info == pixmap.info());
    size_t rowBytes = pixmap.rowBytes();
    static_assert(std::is_standard_layout<DeferredTextureImage>::value,
                  "offsetof, which we use below, requires the type have standard layout");
    auto dtiBufferFiller = DTIBufferFiller{bufferAsCharPtr};
    FILL_MEMBER(dtiBufferFiller, fColorMode, &colorMode);
    FILL_MEMBER(dtiBufferFiller, fContextUniqueID, &proxy.fContextUniqueID);
    int width = info.width();
    FILL_MEMBER(dtiBufferFiller, fWidth, &width);
    int height = info.height();
    FILL_MEMBER(dtiBufferFiller, fHeight, &height);
    SkColorType colorType = info.colorType();
    FILL_MEMBER(dtiBufferFiller, fColorType, &colorType);
    SkAlphaType alphaType = info.alphaType();
    FILL_MEMBER(dtiBufferFiller, fAlphaType, &alphaType);
    FILL_MEMBER(dtiBufferFiller, fMipMapLevelCount, &mipMapLevelCount);
    memcpy(bufferAsCharPtr + offsetof(DeferredTextureImage, fMipMapLevelData[0].fPixelData),
           &pixels, sizeof(pixels));
    memcpy(bufferAsCharPtr + offsetof(DeferredTextureImage, fMipMapLevelData[0].fRowBytes),
           &rowBytes, sizeof(rowBytes));
    if (colorSpaceSize) {
        void* colorSpace = bufferAsCharPtr + colorSpaceOffset;
        FILL_MEMBER(dtiBufferFiller, fColorSpace, &colorSpace);
        FILL_MEMBER(dtiBufferFiller, fColorSpaceSize, &colorSpaceSize);
        info.colorSpace()->writeToMemory(bufferAsCharPtr + colorSpaceOffset);
    } else {
        memset(bufferAsCharPtr + offsetof(DeferredTextureImage, fColorSpace),
               0, sizeof(DeferredTextureImage::fColorSpace));
        memset(bufferAsCharPtr + offsetof(DeferredTextureImage, fColorSpaceSize),
               0, sizeof(DeferredTextureImage::fColorSpaceSize));
    }

    // Fill in the mipmap levels if they exist
    char* mipLevelPtr = pixelsAsCharPtr + SkAlign8(pixmap.getSafeSize());

    if (useMipMaps) {
        static_assert(std::is_standard_layout<MipMapLevelData>::value,
                      "offsetof, which we use below, requires the type have a standard layout");

        std::unique_ptr<SkMipMap> mipmaps(SkMipMap::Build(pixmap, colorMode, nullptr));
        // SkMipMap holds only the mipmap levels it generates.
        // A programmer can use the data they provided to SkMipMap::Build as level 0.
        // So the SkMipMap provides levels 1-x but it stores them in its own
        // range 0-(x-1).
        for (int generatedMipLevelIndex = 0; generatedMipLevelIndex < mipMapLevelCount - 1;
             generatedMipLevelIndex++) {
            SkMipMap::Level mipLevel;
            mipmaps->getLevel(generatedMipLevelIndex, &mipLevel);

            // Make sure the mipmap data is after the start of the buffer
            SkASSERT(mipLevelPtr > bufferAsCharPtr);
            // Make sure the mipmap data starts before the end of the buffer
            SkASSERT(mipLevelPtr < bufferAsCharPtr + pixelOffset + pixelSize);
            // Make sure the mipmap data ends before the end of the buffer
            SkASSERT(mipLevelPtr + mipLevel.fPixmap.getSafeSize() <=
                     bufferAsCharPtr + pixelOffset + pixelSize);

            // getSafeSize includes rowbyte padding except for the last row,
            // right?

            memcpy(mipLevelPtr, mipLevel.fPixmap.addr(), mipLevel.fPixmap.getSafeSize());

            memcpy(bufferAsCharPtr + offsetof(DeferredTextureImage, fMipMapLevelData) +
                   sizeof(MipMapLevelData) * (generatedMipLevelIndex + 1) +
                   offsetof(MipMapLevelData, fPixelData), &mipLevelPtr, sizeof(void*));
            size_t rowBytes = mipLevel.fPixmap.rowBytes();
            memcpy(bufferAsCharPtr + offsetof(DeferredTextureImage, fMipMapLevelData) +
                   sizeof(MipMapLevelData) * (generatedMipLevelIndex + 1) +
                   offsetof(MipMapLevelData, fRowBytes), &rowBytes, sizeof(rowBytes));

            mipLevelPtr += SkAlign8(mipLevel.fPixmap.getSafeSize());
        }
    }
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
    int mipLevelCount = dti->fMipMapLevelCount;
    SkASSERT(mipLevelCount >= 1);
    sk_sp<SkColorSpace> colorSpace;
    if (dti->fColorSpaceSize) {
        colorSpace = SkColorSpace::Deserialize(dti->fColorSpace, dti->fColorSpaceSize);
    }
    SkImageInfo info = SkImageInfo::Make(dti->fWidth, dti->fHeight,
                                         dti->fColorType, dti->fAlphaType, colorSpace);
    if (mipLevelCount == 1) {
        SkPixmap pixmap;
        pixmap.reset(info, dti->fMipMapLevelData[0].fPixelData, dti->fMipMapLevelData[0].fRowBytes);
        return SkImage::MakeTextureFromPixmap(context, pixmap, budgeted);
    } else {
        std::unique_ptr<GrMipLevel[]> texels(new GrMipLevel[mipLevelCount]);
        for (int i = 0; i < mipLevelCount; i++) {
            texels[i].fPixels = dti->fMipMapLevelData[i].fPixelData;
            texels[i].fRowBytes = dti->fMipMapLevelData[i].fRowBytes;
        }

        return SkImage::MakeTextureFromMipMap(context, info, texels.get(),
                                              mipLevelCount, SkBudgeted::kYes,
                                              dti->fColorMode);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkImage> SkImage::MakeTextureFromMipMap(GrContext* ctx, const SkImageInfo& info,
                                              const GrMipLevel* texels, int mipLevelCount,
                                              SkBudgeted budgeted,
                                              SkDestinationSurfaceColorMode colorMode) {
    if (!ctx) {
        return nullptr;
    }
    sk_sp<GrTexture> texture(GrUploadMipMapToTexture(ctx, info, texels, mipLevelCount, colorMode));
    if (!texture) {
        return nullptr;
    }
    return sk_make_sp<SkImage_Gpu>(kNeedNewImageUniqueID,
                                   info.alphaType(), std::move(texture),
                                   sk_ref_sp(info.colorSpace()), budgeted);
}

sk_sp<SkImage> SkImage_Gpu::onMakeColorSpace(sk_sp<SkColorSpace> colorSpace) const {
    sk_sp<SkColorSpace> srcSpace = fColorSpace ? fColorSpace : SkColorSpace::MakeSRGB();
    auto xform = GrNonlinearColorSpaceXformEffect::Make(srcSpace.get(), colorSpace.get());
    if (!xform) {
        return sk_ref_sp(const_cast<SkImage_Gpu*>(this));
    }

    sk_sp<GrRenderTargetContext> renderTargetContext(fContext->makeRenderTargetContext(
        SkBackingFit::kExact, this->width(), this->height(), kRGBA_8888_GrPixelConfig, nullptr));
    if (!renderTargetContext) {
        return nullptr;
    }

    GrPaint paint;
    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
    paint.addColorTextureProcessor(fContext->resourceProvider(), fProxy, nullptr, SkMatrix::I());
    paint.addColorFragmentProcessor(std::move(xform));

    const SkRect rect = SkRect::MakeIWH(this->width(), this->height());

    renderTargetContext->drawRect(GrNoClip(), std::move(paint), GrAA::kNo, SkMatrix::I(), rect);

    if (!renderTargetContext->accessRenderTarget()) {
        return nullptr;
    }

    // MDB: this call is okay bc we know 'renderTargetContext' was exact
    return sk_make_sp<SkImage_Gpu>(fContext, kNeedNewImageUniqueID,
                                   fAlphaType, renderTargetContext->asTextureProxyRef(),
                                   std::move(colorSpace), fBudgeted);

}
