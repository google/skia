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
#include "GrBackendSurface.h"
#include "GrBackendTextureImageGenerator.h"
#include "GrAHardwareBufferImageGenerator.h"
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
#include "GrTexture.h"
#include "GrTextureProxy.h"
#include "effects/GrNonlinearColorSpaceXformEffect.h"
#include "effects/GrYUVEffect.h"
#include "SkCanvas.h"
#include "SkBitmapCache.h"
#include "SkGr.h"
#include "SkImage_Gpu.h"
#include "SkImageCacherator.h"
#include "SkImageInfoPriv.h"
#include "SkMipMap.h"
#include "SkPixelRef.h"
#include "SkReadPixelsRec.h"

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

SkImageInfo SkImage_Gpu::onImageInfo() const {
    SkColorType ct;
    if (!GrPixelConfigToColorType(fProxy->config(), &ct)) {
        ct = kUnknown_SkColorType;
    }
    return SkImageInfo::Make(fProxy->width(), fProxy->height(), ct, fAlphaType, fColorSpace);
}

bool SkImage_Gpu::getROPixels(SkBitmap* dst, SkColorSpace*, CachingHint chint) const {
    // The SkColorSpace parameter "dstColorSpace" is really just a hint about how/where the bitmap
    // will be used. The client doesn't expect that we convert to that color space, it's intended
    // for codec-backed images, to drive our decoding heuristic. In theory we *could* read directly
    // into that color space (to save the client some effort in whatever they're about to do), but
    // that would make our use of the bitmap cache incorrect (or much less efficient, assuming we
    // rolled the dstColorSpace into the key).
    const auto desc = SkBitmapCacheDesc::Make(this);
    if (SkBitmapCache::Find(desc, dst)) {
        SkASSERT(dst->getGenerationID() == this->uniqueID());
        SkASSERT(dst->isImmutable());
        SkASSERT(dst->getPixels());
        return true;
    }

    SkBitmapCache::RecPtr rec = nullptr;
    SkPixmap pmap;
    if (kAllow_CachingHint == chint) {
        rec = SkBitmapCache::Alloc(desc, this->onImageInfo(), &pmap);
        if (!rec) {
            return false;
        }
    } else {
        if (!dst->tryAllocPixels(this->onImageInfo()) || !dst->peekPixels(&pmap)) {
            return false;
        }
    }

    sk_sp<GrSurfaceContext> sContext = fContext->contextPriv().makeWrappedSurfaceContext(
                                                                                    fProxy,
                                                                                    fColorSpace);
    if (!sContext) {
        return false;
    }

    if (!sContext->readPixels(pmap.info(), pmap.writable_addr(), pmap.rowBytes(), 0, 0)) {
        return false;
    }

    if (rec) {
        SkBitmapCache::Add(std::move(rec), dst);
        fAddedRasterVersionToCache.store(true);
    }
    return true;
}

sk_sp<GrTextureProxy> SkImage_Gpu::asTextureProxyRef(GrContext* context,
                                                     const GrSamplerParams& params,
                                                     SkColorSpace* dstColorSpace,
                                                     sk_sp<SkColorSpace>* texColorSpace,
                                                     SkScalar scaleAdjust[2]) const {
    if (context != fContext) {
        SkASSERT(0);
        return nullptr;
    }

    if (texColorSpace) {
        *texColorSpace = this->fColorSpace;
    }

    GrTextureAdjuster adjuster(fContext, fProxy, this->alphaType(), this->bounds(),
                               this->uniqueID(), this->fColorSpace.get());
    return adjuster.refTextureProxySafeForParams(params, nullptr, scaleAdjust);
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

GrBackendObject SkImage_Gpu::onGetTextureHandle(bool flushPendingGrContextIO,
                                                GrSurfaceOrigin* origin) const {
    SkASSERT(fProxy);

    if (!fProxy->instantiate(fContext->resourceProvider())) {
        return 0;
    }

    GrTexture* texture = fProxy->priv().peekTexture();

    if (texture) {
        if (flushPendingGrContextIO) {
            fContext->contextPriv().prepareSurfaceForExternalIO(fProxy.get());
        }
        if (origin) {
            *origin = fProxy->origin();
        }
        return texture->getTextureHandle();
    }
    return 0;
}

GrTexture* SkImage_Gpu::onGetTexture() const {
    GrTextureProxy* proxy = this->peekProxy();
    if (!proxy) {
        return nullptr;
    }

    if (!proxy->instantiate(fContext->resourceProvider())) {
        return nullptr;
    }

    return proxy->priv().peekTexture();
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
        flags = GrContextPriv::kUnpremul_PixelOpsFlag;
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
    GrSurfaceDesc desc;
    desc.fConfig = fProxy->config();
    desc.fWidth = subset.width();
    desc.fHeight = subset.height();
    desc.fOrigin = fProxy->origin();

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

static sk_sp<SkImage> new_wrapped_texture_common(GrContext* ctx,
                                                 const GrBackendTexture& backendTex,
                                                 GrSurfaceOrigin origin,
                                                 SkAlphaType at, sk_sp<SkColorSpace> colorSpace,
                                                 GrWrapOwnership ownership,
                                                 SkImage::TextureReleaseProc releaseProc,
                                                 SkImage::ReleaseContext releaseCtx) {
    if (backendTex.width() <= 0 || backendTex.height() <= 0) {
        return nullptr;
    }

    sk_sp<GrTexture> tex = ctx->resourceProvider()->wrapBackendTexture(backendTex,
                                                                       origin,
                                                                       ownership);
    if (!tex) {
        return nullptr;
    }
    if (releaseProc) {
        tex->setRelease(releaseProc, releaseCtx);
    }

    const SkBudgeted budgeted = SkBudgeted::kNo;
    sk_sp<GrTextureProxy> proxy(GrSurfaceProxy::MakeWrapped(std::move(tex)));
    return sk_make_sp<SkImage_Gpu>(ctx, kNeedNewImageUniqueID,
                                   at, std::move(proxy), std::move(colorSpace), budgeted);
}

sk_sp<SkImage> SkImage::MakeFromTexture(GrContext* ctx,
                                        const GrBackendTexture& tex, GrSurfaceOrigin origin,
                                        SkAlphaType at, sk_sp<SkColorSpace> cs,
                                        TextureReleaseProc releaseP, ReleaseContext releaseC) {
    return new_wrapped_texture_common(ctx, tex, origin, at, std::move(cs), kBorrow_GrWrapOwnership,
                                      releaseP, releaseC);
}

sk_sp<SkImage> SkImage::MakeFromAdoptedTexture(GrContext* ctx,
                                               const GrBackendTexture& tex, GrSurfaceOrigin origin,
                                               SkAlphaType at, sk_sp<SkColorSpace> cs) {
    return new_wrapped_texture_common(ctx, tex, origin, at, std::move(cs), kAdopt_GrWrapOwnership,
                                      nullptr, nullptr);
}

static GrBackendTexture make_backend_texture_from_handle(GrBackend backend,
                                                         int width, int height,
                                                         GrPixelConfig config,
                                                         GrBackendObject handle) {
    switch (backend) {
        case kOpenGL_GrBackend: {
            const GrGLTextureInfo* glInfo = (const GrGLTextureInfo*)(handle);
            return GrBackendTexture(width, height, config, *glInfo);
        }
#ifdef SK_VULKAN
        case kVulkan_GrBackend: {
            const GrVkImageInfo* vkInfo = (const GrVkImageInfo*)(handle);
            return GrBackendTexture(width, height, *vkInfo);
        }
#endif
        case kMock_GrBackend: {
            const GrMockTextureInfo* mockInfo = (const GrMockTextureInfo*)(handle);
            return GrBackendTexture(width, height, config, *mockInfo);
        }
        default:
            return GrBackendTexture();
    }
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

    GrBackend backend = ctx->contextPriv().getBackend();
    GrBackendTexture yTex = make_backend_texture_from_handle(backend,
                                                             yuvSizes[0].fWidth,
                                                             yuvSizes[0].fHeight,
                                                             kConfig,
                                                             yuvTextureHandles[0]);
    GrBackendTexture uTex = make_backend_texture_from_handle(backend,
                                                             yuvSizes[1].fWidth,
                                                             yuvSizes[1].fHeight,
                                                             kConfig,
                                                             yuvTextureHandles[1]);

    sk_sp<GrTextureProxy> yProxy = GrSurfaceProxy::MakeWrappedBackend(ctx, yTex, origin);
    sk_sp<GrTextureProxy> uProxy = GrSurfaceProxy::MakeWrappedBackend(ctx, uTex, origin);
    sk_sp<GrTextureProxy> vProxy;

    if (nv12) {
        vProxy = uProxy;
    } else {
        GrBackendTexture vTex = make_backend_texture_from_handle(backend,
                                                                 yuvSizes[2].fWidth,
                                                                 yuvSizes[2].fHeight,
                                                                 kConfig,
                                                                 yuvTextureHandles[2]);
        vProxy = GrSurfaceProxy::MakeWrappedBackend(ctx, vTex, origin);
    }
    if (!yProxy || !uProxy || !vProxy) {
        return nullptr;
    }

    const int width = yuvSizes[0].fWidth;
    const int height = yuvSizes[0].fHeight;

    // Needs to be a render target in order to draw to it for the yuv->rgb conversion.
    sk_sp<GrRenderTargetContext> renderTargetContext(ctx->makeDeferredRenderTargetContext(
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
    paint.addColorFragmentProcessor(GrYUVEffect::MakeYUVToRGB(yProxy, uProxy, vProxy,
                                                              yuvSizes, colorSpace, nv12));

    const SkRect rect = SkRect::MakeIWH(width, height);

    renderTargetContext->drawRect(GrNoClip(), std::move(paint), GrAA::kNo, SkMatrix::I(), rect);

    if (!renderTargetContext->asSurfaceProxy()) {
        return nullptr;
    }
    ctx->contextPriv().flushSurfaceWrites(renderTargetContext->asSurfaceProxy());

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

static sk_sp<SkImage> create_image_from_maker(GrContext* context, GrTextureMaker* maker,
                                              SkAlphaType at, uint32_t id,
                                              SkColorSpace* dstColorSpace) {
    sk_sp<SkColorSpace> texColorSpace;
    sk_sp<GrTextureProxy> proxy(maker->refTextureProxyForParams(GrSamplerParams::ClampNoFilter(),
                                                                dstColorSpace,
                                                                &texColorSpace, nullptr));
    if (!proxy) {
        return nullptr;
    }
    return sk_make_sp<SkImage_Gpu>(context, id, at,
                                   std::move(proxy), std::move(texColorSpace), SkBudgeted::kNo);
}

sk_sp<SkImage> SkImage::makeTextureImage(GrContext* context, SkColorSpace* dstColorSpace) const {
    if (!context) {
        return nullptr;
    }
    if (GrContext* incumbent = as_IB(this)->context()) {
        return incumbent == context ? sk_ref_sp(const_cast<SkImage*>(this)) : nullptr;
    }

    if (this->isLazyGenerated()) {
        GrImageTextureMaker maker(context, this, kDisallow_CachingHint);
        return create_image_from_maker(context, &maker, this->alphaType(),
                                       this->uniqueID(), dstColorSpace);
    }

    if (const SkBitmap* bmp = as_IB(this)->onPeekBitmap()) {
        GrBitmapTextureMaker maker(context, *bmp);
        return create_image_from_maker(context, &maker, this->alphaType(),
                                       this->uniqueID(), dstColorSpace);
    }
    return nullptr;
}

sk_sp<SkImage> SkImage::MakeCrossContextFromEncoded(GrContext* context, sk_sp<SkData> encoded,
                                                    bool buildMips, SkColorSpace* dstColorSpace) {
    sk_sp<SkImage> codecImage = SkImage::MakeFromEncoded(std::move(encoded));
    if (!codecImage) {
        return nullptr;
    }

    // Some backends or drivers don't support (safely) moving resources between contexts
    if (!context || !context->caps()->crossContextTextureSupport()) {
        return codecImage;
    }

    // Turn the codec image into a GrTextureProxy
    GrImageTextureMaker maker(context, codecImage.get(), kDisallow_CachingHint);
    sk_sp<SkColorSpace> texColorSpace;
    GrSamplerParams params(SkShader::kClamp_TileMode,
                           buildMips ? GrSamplerParams::kMipMap_FilterMode
                                     : GrSamplerParams::kBilerp_FilterMode);
    sk_sp<GrTextureProxy> proxy(maker.refTextureProxyForParams(params, dstColorSpace,
                                                               &texColorSpace, nullptr));
    if (!proxy) {
        return codecImage;
    }

    if (!proxy->instantiate(context->resourceProvider())) {
        return codecImage;
    }
    sk_sp<GrTexture> texture = sk_ref_sp(proxy->priv().peekTexture());

    // Flush any writes or uploads
    context->contextPriv().prepareSurfaceForExternalIO(proxy.get());

    sk_sp<GrSemaphore> sema = context->getGpu()->prepareTextureForCrossContextUsage(texture.get());

    auto gen = GrBackendTextureImageGenerator::Make(std::move(texture), std::move(sema),
                                                    codecImage->alphaType(),
                                                    std::move(texColorSpace));
    return SkImage::MakeFromGenerator(std::move(gen));
}

#if defined(SK_BUILD_FOR_ANDROID) && __ANDROID_API__ >= 26
sk_sp<SkImage> SkImage::MakeFromAHardwareBuffer(AHardwareBuffer* graphicBuffer, SkAlphaType at,
                                               sk_sp<SkColorSpace> cs) {
    auto gen = GrAHardwareBufferImageGenerator::Make(graphicBuffer, at, cs);
    return SkImage::MakeFromGenerator(std::move(gen));
}
#endif

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

static bool SupportsColorSpace(SkColorType colorType) {
    switch (colorType) {
        case kRGBA_8888_SkColorType:
        case kBGRA_8888_SkColorType:
        case kRGBA_F16_SkColorType:
            return true;
        default:
            return false;
    }
}

size_t SkImage::getDeferredTextureImageData(const GrContextThreadSafeProxy& proxy,
                                            const DeferredTextureImageUsageParams params[],
                                            int paramCnt, void* buffer,
                                            SkColorSpace* dstColorSpace,
                                            SkColorType dstColorType) const {
    // Some quick-rejects where is makes no sense to return CPU data
    //  e.g.
    //      - texture backed
    //      - picture backed
    //
    if (this->isTextureBacked()) {
        return 0;
    }
    if (as_IB(this)->onCanLazyGenerateOnGPU()) {
        return 0;
    }

    bool supportsColorSpace = SupportsColorSpace(dstColorType);
    // Quick reject if the caller requests a color space with an unsupported color type.
    if (SkToBool(dstColorSpace) && !supportsColorSpace) {
        return 0;
    }

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
    if (!isScaled && this->peekPixels(&pixmap) && pixmap.info().colorType() == dstColorType) {
        info = pixmap.info();
        pixelSize = SkAlign8(pixmap.getSafeSize());
        if (!dstColorSpace) {
            pixmap.setColorSpace(nullptr);
            info = info.makeColorSpace(nullptr);
        }
    } else {
        if (!this->isLazyGenerated() && !this->peekPixels(nullptr)) {
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
            if (!dstColorSpace) {
                info = info.makeColorSpace(nullptr);
            }
        }
        // Force color type to be the requested type.
        info = info.makeColorType(dstColorType);
        pixelSize = SkAlign8(SkAutoPixmapStorage::AllocSize(info, nullptr));
        if (fillMode) {
            // Always decode to N32 and convert to the requested type if necessary.
            SkImageInfo decodeInfo = info.makeColorType(kN32_SkColorType);
            SkAutoPixmapStorage decodePixmap;
            decodePixmap.alloc(decodeInfo);

            if (isScaled) {
                if (!this->scalePixels(decodePixmap, scaleFilterQuality,
                                       SkImage::kDisallow_CachingHint)) {
                    return 0;
                }
            } else {
                if (!this->readPixels(decodePixmap, 0, 0, SkImage::kDisallow_CachingHint)) {
                    return 0;
                }
            }

            if (decodeInfo.colorType() != info.colorType()) {
                pixmap.alloc(info);
                // Convert and copy the decoded pixmap to the target pixmap.
                decodePixmap.readPixels(pixmap.info(), pixmap.writable_addr(), pixmap.rowBytes(), 0,
                                        0);
            } else {
                pixmap = std::move(decodePixmap);
            }
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
    SkColorSpaceTransferFn fn;
    if (info.colorSpace()) {
        SkASSERT(dstColorSpace);
        SkASSERT(supportsColorSpace);
        colorSpaceOffset = size;
        colorSpaceSize = info.colorSpace()->writeToMemory(nullptr);
        size += colorSpaceSize;
    } else if (supportsColorSpace && this->colorSpace() && this->colorSpace()->isNumericalTransferFn(&fn)) {
        // In legacy mode, preserve the color space tag on the SkImage.  This is only
        // supported if the color space has a parametric transfer function.
        SkASSERT(!dstColorSpace);
        colorSpaceOffset = size;
        colorSpaceSize = this->colorSpace()->writeToMemory(nullptr);
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
        SkASSERT(supportsColorSpace);
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
        if (info.colorSpace()) {
            info.colorSpace()->writeToMemory(bufferAsCharPtr + colorSpaceOffset);
        } else {
            SkASSERT(this->colorSpace() && this->colorSpace()->isNumericalTransferFn(&fn));
            SkASSERT(!dstColorSpace);
            this->colorSpace()->writeToMemory(bufferAsCharPtr + colorSpaceOffset);
        }
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

    if (!context || context->uniqueID() != dti->fContextUniqueID || context->abandoned()) {
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

        // Pass nullptr for the |dstColorSpace|.  This opts in to more lenient color space
        // verification.  This is ok because we've already verified the color space in
        // getDeferredTextureImageData().
        sk_sp<GrTextureProxy> proxy(GrUploadPixmapToTextureProxy(
                context->resourceProvider(), pixmap, budgeted, nullptr));
        if (!proxy) {
            return nullptr;
        }
        return sk_make_sp<SkImage_Gpu>(context, kNeedNewImageUniqueID, pixmap.alphaType(),
                                       std::move(proxy), std::move(colorSpace), budgeted);
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
                                              const GrMipLevel texels[], int mipLevelCount,
                                              SkBudgeted budgeted,
                                              SkDestinationSurfaceColorMode colorMode) {
    SkASSERT(mipLevelCount >= 1);
    if (!ctx) {
        return nullptr;
    }
    sk_sp<GrTextureProxy> proxy(GrUploadMipMapToTextureProxy(ctx, info, texels, mipLevelCount,
                                                             colorMode));
    if (!proxy) {
        return nullptr;
    }

    SkASSERT(proxy->priv().isExact());
    return sk_make_sp<SkImage_Gpu>(ctx, kNeedNewImageUniqueID,
                                   info.alphaType(), std::move(proxy),
                                   info.refColorSpace(), budgeted);
}

sk_sp<SkImage> SkImage_Gpu::onMakeColorSpace(sk_sp<SkColorSpace> target, SkColorType,
                                             SkTransferFunctionBehavior premulBehavior) const {
    if (SkTransferFunctionBehavior::kRespect == premulBehavior) {
        // TODO: Implement this.
        return nullptr;
    }

    sk_sp<SkColorSpace> srcSpace = fColorSpace;
    if (!fColorSpace) {
        if (target->isSRGB()) {
            return sk_ref_sp(const_cast<SkImage*>((SkImage*)this));
        }

        srcSpace = SkColorSpace::MakeSRGB();
    }

    auto xform = GrNonlinearColorSpaceXformEffect::Make(srcSpace.get(), target.get());
    if (!xform) {
        return sk_ref_sp(const_cast<SkImage_Gpu*>(this));
    }

    sk_sp<GrRenderTargetContext> renderTargetContext(fContext->makeDeferredRenderTargetContext(
        SkBackingFit::kExact, this->width(), this->height(), kRGBA_8888_GrPixelConfig, nullptr));
    if (!renderTargetContext) {
        return nullptr;
    }

    GrPaint paint;
    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
    paint.addColorTextureProcessor(fProxy, nullptr, SkMatrix::I());
    paint.addColorFragmentProcessor(std::move(xform));

    const SkRect rect = SkRect::MakeIWH(this->width(), this->height());

    renderTargetContext->drawRect(GrNoClip(), std::move(paint), GrAA::kNo, SkMatrix::I(), rect);

    if (!renderTargetContext->asTextureProxy()) {
        return nullptr;
    }

    // MDB: this call is okay bc we know 'renderTargetContext' was exact
    return sk_make_sp<SkImage_Gpu>(fContext, kNeedNewImageUniqueID,
                                   fAlphaType, renderTargetContext->asTextureProxyRef(),
                                   std::move(target), fBudgeted);

}

bool SkImage_Gpu::onIsValid(GrContext* context) const {
    // The base class has already checked that context isn't abandoned (if it's not nullptr)
    if (fContext->abandoned()) {
        return false;
    }

    if (context && context != fContext) {
        return false;
    }

    return true;
}
