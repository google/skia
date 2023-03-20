/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/image/SkImage_Lazy.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkData.h"
#include "include/core/SkImageGenerator.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRect.h"
#include "include/core/SkSize.h"
#include "include/core/SkYUVAInfo.h"
#include "src/core/SkBitmapCache.h"
#include "src/core/SkCachedData.h"
#include "src/core/SkNextID.h"
#include "src/core/SkResourceCache.h"
#include "src/core/SkYUVPlanesCache.h"

#if defined(SK_GANESH)
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrContextOptions.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/gpu/GrTypes.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/ResourceKey.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/Swizzle.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrColorInfo.h"
#include "src/gpu/ganesh/GrColorSpaceXform.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/GrImageInfo.h"
#include "src/gpu/ganesh/GrProxyProvider.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrSamplerState.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/gpu/ganesh/GrTextureProxy.h"
#include "src/gpu/ganesh/GrYUVATextureProxies.h"
#include "src/gpu/ganesh/SkGr.h"
#include "src/gpu/ganesh/SurfaceContext.h"
#include "src/gpu/ganesh/SurfaceFillContext.h"
#include "src/gpu/ganesh/effects/GrYUVtoRGBEffect.h"
#endif

#if defined(SK_GRAPHITE)
#include "src/gpu/graphite/TextureUtils.h"
#endif

#include <utility>

class SkMatrix;
enum SkColorType : int;
enum class SkTileMode;

// Ref-counted tuple(SkImageGenerator, SkMutex) which allows sharing one generator among N images
class SharedGenerator final : public SkNVRefCnt<SharedGenerator> {
public:
    static sk_sp<SharedGenerator> Make(std::unique_ptr<SkImageGenerator> gen) {
        return gen ? sk_sp<SharedGenerator>(new SharedGenerator(std::move(gen))) : nullptr;
    }

    // This is thread safe.  It is a const field set in the constructor.
    const SkImageInfo& getInfo() { return fGenerator->getInfo(); }

private:
    explicit SharedGenerator(std::unique_ptr<SkImageGenerator> gen)
            : fGenerator(std::move(gen)) {
        SkASSERT(fGenerator);
    }

    friend class ScopedGenerator;
    friend class SkImage_Lazy;

    std::unique_ptr<SkImageGenerator> fGenerator;
    SkMutex                           fMutex;
};

///////////////////////////////////////////////////////////////////////////////

SkImage_Lazy::Validator::Validator(sk_sp<SharedGenerator> gen, const SkColorType* colorType,
                                   sk_sp<SkColorSpace> colorSpace)
        : fSharedGenerator(std::move(gen)) {
    if (!fSharedGenerator) {
        return;
    }

    // The following generator accessors are safe without acquiring the mutex (const getters).
    // TODO: refactor to use a ScopedGenerator instead, for clarity.
    fInfo = fSharedGenerator->fGenerator->getInfo();
    if (fInfo.isEmpty()) {
        fSharedGenerator.reset();
        return;
    }

    fUniqueID = fSharedGenerator->fGenerator->uniqueID();

    if (colorType && (*colorType == fInfo.colorType())) {
        colorType = nullptr;
    }

    if (colorType || colorSpace) {
        if (colorType) {
            fInfo = fInfo.makeColorType(*colorType);
        }
        if (colorSpace) {
            fInfo = fInfo.makeColorSpace(colorSpace);
        }
        fUniqueID = SkNextID::ImageID();
    }
}

///////////////////////////////////////////////////////////////////////////////

// Helper for exclusive access to a shared generator.
class SkImage_Lazy::ScopedGenerator {
public:
    ScopedGenerator(const sk_sp<SharedGenerator>& gen)
      : fSharedGenerator(gen)
      , fAutoAquire(gen->fMutex) {}

    SkImageGenerator* operator->() const {
        fSharedGenerator->fMutex.assertHeld();
        return fSharedGenerator->fGenerator.get();
    }

    operator SkImageGenerator*() const {
        fSharedGenerator->fMutex.assertHeld();
        return fSharedGenerator->fGenerator.get();
    }

private:
    const sk_sp<SharedGenerator>& fSharedGenerator;
    SkAutoMutexExclusive          fAutoAquire;
};

///////////////////////////////////////////////////////////////////////////////

SkImage_Lazy::SkImage_Lazy(Validator* validator)
    : SkImage_Base(validator->fInfo, validator->fUniqueID)
    , fSharedGenerator(std::move(validator->fSharedGenerator))
{
    SkASSERT(fSharedGenerator);
}


//////////////////////////////////////////////////////////////////////////////////////////////////

bool SkImage_Lazy::getROPixels(GrDirectContext* ctx, SkBitmap* bitmap,
                               SkImage::CachingHint chint) const {
    auto check_output_bitmap = [bitmap]() {
        SkASSERT(bitmap->isImmutable());
        SkASSERT(bitmap->getPixels());
        (void)bitmap;
    };

    auto desc = SkBitmapCacheDesc::Make(this);
    if (SkBitmapCache::Find(desc, bitmap)) {
        check_output_bitmap();
        return true;
    }

    if (SkImage::kAllow_CachingHint == chint) {
        SkPixmap pmap;
        SkBitmapCache::RecPtr cacheRec = SkBitmapCache::Alloc(desc, this->imageInfo(), &pmap);
        if (!cacheRec) {
            return false;
        }
        bool success = false;
        {   // make sure ScopedGenerator goes out of scope before we try readPixelsProxy
            success = ScopedGenerator(fSharedGenerator)->getPixels(pmap);
        }
        if (!success && !this->readPixelsProxy(ctx, pmap)) {
            return false;
        }
        SkBitmapCache::Add(std::move(cacheRec), bitmap);
        this->notifyAddedToRasterCache();
    } else {
        if (!bitmap->tryAllocPixels(this->imageInfo())) {
            return false;
        }
        bool success = false;
        {   // make sure ScopedGenerator goes out of scope before we try readPixelsProxy
            success = ScopedGenerator(fSharedGenerator)->getPixels(bitmap->pixmap());
        }
        if (!success && !this->readPixelsProxy(ctx, bitmap->pixmap())) {
            return false;
        }
        bitmap->setImmutable();
    }
    check_output_bitmap();
    return true;
}

bool SkImage_Lazy::readPixelsProxy(GrDirectContext* ctx, const SkPixmap& pixmap) const {
#if defined(SK_GANESH)
    if (!ctx) {
        return false;
    }
    GrSurfaceProxyView view = this->lockTextureProxyView(ctx,
                                                         GrImageTexGenPolicy::kDraw,
                                                         GrMipmapped::kNo);

    if (!view) {
        return false;
    }

    GrColorType ct = this->colorTypeOfLockTextureProxy(ctx->priv().caps());
    GrColorInfo colorInfo(ct, this->alphaType(), this->refColorSpace());
    auto sContext = ctx->priv().makeSC(std::move(view), colorInfo);
    if (!sContext) {
        return false;
    }
    size_t rowBytes = this->imageInfo().minRowBytes();
    return sContext->readPixels(ctx, {this->imageInfo(), pixmap.writable_addr(), rowBytes}, {0, 0});
#else
    return false;
#endif // defined(SK_GANESH)
}

//////////////////////////////////////////////////////////////////////////////////////////////////

bool SkImage_Lazy::onReadPixels(GrDirectContext* dContext,
                                const SkImageInfo& dstInfo,
                                void* dstPixels,
                                size_t dstRB,
                                int srcX,
                                int srcY,
                                CachingHint chint) const {
    SkBitmap bm;
    if (this->getROPixels(dContext, &bm, chint)) {
        return bm.readPixels(dstInfo, dstPixels, dstRB, srcX, srcY);
    }
    return false;
}

sk_sp<SkData> SkImage_Lazy::onRefEncoded() const {
    // check that we aren't a subset or colortype/etc modification of the original
    if (fSharedGenerator->fGenerator->uniqueID() == this->uniqueID()) {
        ScopedGenerator generator(fSharedGenerator);
        return generator->refEncodedData();
    }
    return nullptr;
}

bool SkImage_Lazy::onIsValid(GrRecordingContext* context) const {
    ScopedGenerator generator(fSharedGenerator);
    return generator->isValid(context);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkImage> SkImage_Lazy::onMakeSubset(const SkIRect& subset, GrDirectContext* direct) const {
    // TODO: can we do this more efficiently, by telling the generator we want to
    //       "realize" a subset?

#if defined(SK_GANESH)
    auto pixels = direct ? this->makeTextureImage(direct)
                         : this->makeRasterImage();
#else
    auto pixels = this->makeRasterImage();
#endif
    return pixels ? pixels->makeSubset(subset, direct) : nullptr;
}

#if defined(SK_GRAPHITE)

sk_sp<SkImage> SkImage_Lazy::onMakeSubset(const SkIRect& subset,
                                          skgpu::graphite::Recorder* recorder,
                                          RequiredImageProperties requiredProperties) const {
    // TODO: can we do this more efficiently, by telling the generator we want to
    //       "realize" a subset?

    sk_sp<SkImage> nonLazyImg = recorder ? this->makeTextureImage(recorder, requiredProperties)
                                         : this->makeRasterImage();

    return nonLazyImg ? nonLazyImg->makeSubset(subset, recorder, requiredProperties) : nullptr;
}

#endif // SK_GRAPHITE

sk_sp<SkImage> SkImage_Lazy::onMakeColorTypeAndColorSpace(SkColorType targetCT,
                                                          sk_sp<SkColorSpace> targetCS,
                                                          GrDirectContext*) const {
    SkAutoMutexExclusive autoAquire(fOnMakeColorTypeAndSpaceMutex);
    if (fOnMakeColorTypeAndSpaceResult &&
        targetCT == fOnMakeColorTypeAndSpaceResult->colorType() &&
        SkColorSpace::Equals(targetCS.get(), fOnMakeColorTypeAndSpaceResult->colorSpace())) {
        return fOnMakeColorTypeAndSpaceResult;
    }
    Validator validator(fSharedGenerator, &targetCT, targetCS);
    sk_sp<SkImage> result = validator ? sk_sp<SkImage>(new SkImage_Lazy(&validator)) : nullptr;
    if (result) {
        fOnMakeColorTypeAndSpaceResult = result;
    }
    return result;
}

sk_sp<SkImage> SkImage_Lazy::onReinterpretColorSpace(sk_sp<SkColorSpace> newCS) const {
    // TODO: The correct thing is to clone the generator, and modify its color space. That's hard,
    // because we don't have a clone method, and generator is public (and derived-from by clients).
    // So do the simple/inefficient thing here, and fallback to raster when this is called.

    // We allocate the bitmap with the new color space, then generate the image using the original.
    SkBitmap bitmap;
    if (bitmap.tryAllocPixels(this->imageInfo().makeColorSpace(std::move(newCS)))) {
        SkPixmap pixmap = bitmap.pixmap();
        pixmap.setColorSpace(this->refColorSpace());
        if (ScopedGenerator(fSharedGenerator)->getPixels(pixmap)) {
            bitmap.setImmutable();
            return bitmap.asImage();
        }
    }
    return nullptr;
}

sk_sp<SkImage> SkImage::MakeFromGenerator(std::unique_ptr<SkImageGenerator> generator) {
    SkImage_Lazy::Validator
            validator(SharedGenerator::Make(std::move(generator)), nullptr, nullptr);

    return validator ? sk_make_sp<SkImage_Lazy>(&validator) : nullptr;
}

#if defined(SK_GANESH)

std::tuple<GrSurfaceProxyView, GrColorType> SkImage_Lazy::onAsView(
        GrRecordingContext* context,
        GrMipmapped mipmapped,
        GrImageTexGenPolicy policy) const {
    GrColorType ct = this->colorTypeOfLockTextureProxy(context->priv().caps());
    return {this->lockTextureProxyView(context, policy, mipmapped), ct};
}

std::unique_ptr<GrFragmentProcessor> SkImage_Lazy::onAsFragmentProcessor(
        GrRecordingContext* rContext,
        SkSamplingOptions sampling,
        const SkTileMode tileModes[2],
        const SkMatrix& m,
        const SkRect* subset,
        const SkRect* domain) const {
    // TODO: If the CPU data is extracted as planes return a FP that reconstructs the image from
    // the planes.
    auto mm = sampling.mipmap == SkMipmapMode::kNone ? GrMipmapped::kNo : GrMipmapped::kYes;
    return MakeFragmentProcessorFromView(rContext,
                                         std::get<0>(this->asView(rContext, mm)),
                                         this->alphaType(),
                                         sampling,
                                         tileModes,
                                         m,
                                         subset,
                                         domain);
}

GrSurfaceProxyView SkImage_Lazy::textureProxyViewFromPlanes(GrRecordingContext* ctx,
                                                            skgpu::Budgeted budgeted) const {
    SkYUVAPixmapInfo::SupportedDataTypes supportedDataTypes(*ctx);
    SkYUVAPixmaps yuvaPixmaps;
    sk_sp<SkCachedData> dataStorage = this->getPlanes(supportedDataTypes, &yuvaPixmaps);
    if (!dataStorage) {
        return {};
    }

    GrSurfaceProxyView views[SkYUVAInfo::kMaxPlanes];
    GrColorType pixmapColorTypes[SkYUVAInfo::kMaxPlanes];
    for (int i = 0; i < yuvaPixmaps.numPlanes(); ++i) {
        // If the sizes of the components are not all the same we choose to create exact-match
        // textures for the smaller ones rather than add a texture domain to the draw.
        // TODO: revisit this decision to improve texture reuse?
        SkBackingFit fit = yuvaPixmaps.plane(i).dimensions() == this->dimensions()
                                   ? SkBackingFit::kApprox
                                   : SkBackingFit::kExact;

        // We grab a ref to cached yuv data. When the SkBitmap we create below goes away it will
        // call releaseProc which will release this ref.
        // DDL TODO: Currently we end up creating a lazy proxy that will hold onto a ref to the
        // SkImage in its lambda. This means that we'll keep the ref on the YUV data around for the
        // life time of the proxy and not just upload. For non-DDL draws we should look into
        // releasing this SkImage after uploads (by deleting the lambda after instantiation).
        auto releaseProc = [](void*, void* data) {
            auto cachedData = static_cast<SkCachedData*>(data);
            SkASSERT(cachedData);
            cachedData->unref();
        };
        SkBitmap bitmap;
        bitmap.installPixels(yuvaPixmaps.plane(i).info(),
                             yuvaPixmaps.plane(i).writable_addr(),
                             yuvaPixmaps.plane(i).rowBytes(),
                             releaseProc,
                             SkRef(dataStorage.get()));
        bitmap.setImmutable();

        std::tie(views[i], std::ignore) = GrMakeUncachedBitmapProxyView(ctx,
                                                                        bitmap,
                                                                        GrMipmapped::kNo,
                                                                        fit);
        if (!views[i]) {
            return {};
        }
        pixmapColorTypes[i] = SkColorTypeToGrColorType(bitmap.colorType());
    }

    // TODO: investigate preallocating mip maps here
    GrImageInfo info(SkColorTypeToGrColorType(this->colorType()),
                     kPremul_SkAlphaType,
                     /*color space*/ nullptr,
                     this->dimensions());

    auto sfc = ctx->priv().makeSFC(info,
                                   "ImageLazy_TextureProxyViewFromPlanes",
                                   SkBackingFit::kExact,
                                   1,
                                   GrMipmapped::kNo,
                                   GrProtected::kNo,
                                   kTopLeft_GrSurfaceOrigin,
                                   budgeted);
    if (!sfc) {
        return {};
    }

    GrYUVATextureProxies yuvaProxies(yuvaPixmaps.yuvaInfo(), views, pixmapColorTypes);
    SkAssertResult(yuvaProxies.isValid());

    std::unique_ptr<GrFragmentProcessor> fp = GrYUVtoRGBEffect::Make(
            yuvaProxies,
            GrSamplerState::Filter::kNearest,
            *ctx->priv().caps());

    // The pixels after yuv->rgb will be in the generator's color space.
    // If onMakeColorTypeAndColorSpace has been called then this will not match this image's
    // color space. To correct this, apply a color space conversion from the generator's color
    // space to this image's color space.
    SkColorSpace* srcColorSpace;
    {
        ScopedGenerator generator(fSharedGenerator);
        srcColorSpace = generator->getInfo().colorSpace();
    }
    SkColorSpace* dstColorSpace = this->colorSpace();

    // If the caller expects the pixels in a different color space than the one from the image,
    // apply a color conversion to do this.
    fp = GrColorSpaceXformEffect::Make(std::move(fp),
                                       srcColorSpace, kOpaque_SkAlphaType,
                                       dstColorSpace, kOpaque_SkAlphaType);
    sfc->fillWithFP(std::move(fp));

    return sfc->readSurfaceView();
}

sk_sp<SkCachedData> SkImage_Lazy::getPlanes(
        const SkYUVAPixmapInfo::SupportedDataTypes& supportedDataTypes,
        SkYUVAPixmaps* yuvaPixmaps) const {
    ScopedGenerator generator(fSharedGenerator);

    sk_sp<SkCachedData> data(SkYUVPlanesCache::FindAndRef(generator->uniqueID(), yuvaPixmaps));

    if (data) {
        SkASSERT(yuvaPixmaps->isValid());
        SkASSERT(yuvaPixmaps->yuvaInfo().dimensions() == this->dimensions());
        return data;
    }
    SkYUVAPixmapInfo yuvaPixmapInfo;
    if (!generator->queryYUVAInfo(supportedDataTypes, &yuvaPixmapInfo) ||
        yuvaPixmapInfo.yuvaInfo().dimensions() != this->dimensions()) {
        return nullptr;
    }
    data.reset(SkResourceCache::NewCachedData(yuvaPixmapInfo.computeTotalBytes()));
    SkYUVAPixmaps tempPixmaps = SkYUVAPixmaps::FromExternalMemory(yuvaPixmapInfo,
                                                                  data->writable_data());
    SkASSERT(tempPixmaps.isValid());
    if (!generator->getYUVAPlanes(tempPixmaps)) {
        return nullptr;
    }
    // Decoding is done, cache the resulting YUV planes
    *yuvaPixmaps = tempPixmaps;
    SkYUVPlanesCache::Add(this->uniqueID(), data.get(), *yuvaPixmaps);
    return data;
}

/*
 *  We have 4 ways to try to return a texture (in sorted order)
 *
 *  1. Check the cache for a pre-existing one
 *  2. Ask the generator to natively create one
 *  3. Ask the generator to return YUV planes, which the GPU can convert
 *  4. Ask the generator to return RGB(A) data, which the GPU can convert
 */
GrSurfaceProxyView SkImage_Lazy::lockTextureProxyView(GrRecordingContext* rContext,
                                                      GrImageTexGenPolicy texGenPolicy,
                                                      GrMipmapped mipmapped) const {
    // Values representing the various texture lock paths we can take. Used for logging the path
    // taken to a histogram.
    enum LockTexturePath {
        kFailure_LockTexturePath,
        kPreExisting_LockTexturePath,
        kNative_LockTexturePath,
        kCompressed_LockTexturePath, // Deprecated
        kYUV_LockTexturePath,
        kRGBA_LockTexturePath,
    };

    enum { kLockTexturePathCount = kRGBA_LockTexturePath + 1 };

    skgpu::UniqueKey key;
    if (texGenPolicy == GrImageTexGenPolicy::kDraw) {
        GrMakeKeyFromImageID(&key, this->uniqueID(), SkIRect::MakeSize(this->dimensions()));
    }

    const GrCaps* caps = rContext->priv().caps();
    GrProxyProvider* proxyProvider = rContext->priv().proxyProvider();

    auto installKey = [&](const GrSurfaceProxyView& view) {
        SkASSERT(view && view.asTextureProxy());
        if (key.isValid()) {
            auto listener = GrMakeUniqueKeyInvalidationListener(&key, rContext->priv().contextID());
            this->addUniqueIDListener(std::move(listener));
            proxyProvider->assignUniqueKeyToProxy(key, view.asTextureProxy());
        }
    };

    auto ct = this->colorTypeOfLockTextureProxy(caps);

    // 1. Check the cache for a pre-existing one.
    if (key.isValid()) {
        auto proxy = proxyProvider->findOrCreateProxyByUniqueKey(key);
        if (proxy) {
            skgpu::Swizzle swizzle = caps->getReadSwizzle(proxy->backendFormat(), ct);
            GrSurfaceOrigin origin = ScopedGenerator(fSharedGenerator)->origin();
            GrSurfaceProxyView view(std::move(proxy), origin, swizzle);
            if (mipmapped == GrMipmapped::kNo ||
                view.asTextureProxy()->mipmapped() == GrMipmapped::kYes) {
                return view;
            } else {
                // We need a mipped proxy, but we found a cached proxy that wasn't mipped. Thus we
                // generate a new mipped surface and copy the original proxy into the base layer. We
                // will then let the gpu generate the rest of the mips.
                auto mippedView = GrCopyBaseMipMapToView(rContext, view);
                if (!mippedView) {
                    // We failed to make a mipped proxy with the base copied into it. This could
                    // have been from failure to make the proxy or failure to do the copy. Thus we
                    // will fall back to just using the non mipped proxy; See skbug.com/7094.
                    return view;
                }
                proxyProvider->removeUniqueKeyFromProxy(view.asTextureProxy());
                installKey(mippedView);
                return mippedView;
            }
        }
    }

    // 2. Ask the generator to natively create one.
    {
        ScopedGenerator generator(fSharedGenerator);
        if (auto view = generator->generateTexture(rContext,
                                                   this->imageInfo(),
                                                   mipmapped,
                                                   texGenPolicy)) {
            installKey(view);
            return view;
        }
    }

    // 3. Ask the generator to return YUV planes, which the GPU can convert. If we will be mipping
    //    the texture we skip this step so the CPU generate non-planar MIP maps for us.
    if (mipmapped == GrMipmapped::kNo && !rContext->priv().options().fDisableGpuYUVConversion) {
        // TODO: Update to create the mipped surface in the textureProxyViewFromPlanes generator and
        //  draw the base layer directly into the mipped surface.
        skgpu::Budgeted budgeted = texGenPolicy == GrImageTexGenPolicy::kNew_Uncached_Unbudgeted
                                           ? skgpu::Budgeted::kNo
                                           : skgpu::Budgeted::kYes;
        auto view = this->textureProxyViewFromPlanes(rContext, budgeted);
        if (view) {
            installKey(view);
            return view;
        }
    }

    // 4. Ask the generator to return a bitmap, which the GPU can convert.
    auto hint = texGenPolicy == GrImageTexGenPolicy::kDraw ? CachingHint::kAllow_CachingHint
                                                           : CachingHint::kDisallow_CachingHint;
    if (SkBitmap bitmap; this->getROPixels(nullptr, &bitmap, hint)) {
        // We always make an uncached bitmap here because we will cache it based on passed in policy
        // with *our* key, not a key derived from bitmap. We're just making the proxy here.
        auto budgeted = texGenPolicy == GrImageTexGenPolicy::kNew_Uncached_Unbudgeted
                                ? skgpu::Budgeted::kNo
                                : skgpu::Budgeted::kYes;
        auto view = std::get<0>(GrMakeUncachedBitmapProxyView(rContext,
                                                              bitmap,
                                                              mipmapped,
                                                              SkBackingFit::kExact,
                                                              budgeted));
        if (view) {
            installKey(view);
            return view;
        }
    }

    return {};
}

GrColorType SkImage_Lazy::colorTypeOfLockTextureProxy(const GrCaps* caps) const {
    GrColorType ct = SkColorTypeToGrColorType(this->colorType());
    GrBackendFormat format = caps->getDefaultBackendFormat(ct, GrRenderable::kNo);
    if (!format.isValid()) {
        ct = GrColorType::kRGBA_8888;
    }
    return ct;
}

void SkImage_Lazy::addUniqueIDListener(sk_sp<SkIDChangeListener> listener) const {
    fUniqueIDListeners.add(std::move(listener));
}
#endif // defined(SK_GANESH)

#if defined(SK_GRAPHITE)

/*
 *  We only have 2 ways to create a Graphite-backed image.
 *
 *  1. Ask the generator to natively create one
 *  2. Ask the generator to return RGB(A) data, which the GPU can convert
 */
sk_sp<SkImage> SkImage_Lazy::onMakeTextureImage(skgpu::graphite::Recorder* recorder,
                                                RequiredImageProperties requiredProps) const {
    using namespace skgpu::graphite;

    // 1. Ask the generator to natively create one.
    {
        // Disable mipmaps here bc Graphite doesn't currently support mipmap regeneration
        // In this case, we would allocate the mipmaps and fill in the base layer but the mipmap
        // levels would never be filled out - yielding incorrect draws. Please see: b/238754357.
        requiredProps.fMipmapped = skgpu::Mipmapped::kNo;

        ScopedGenerator generator(fSharedGenerator);
        sk_sp<SkImage> newImage = generator->makeTextureImage(recorder,
                                                              this->imageInfo(),
                                                              requiredProps.fMipmapped);
        if (newImage) {
            SkASSERT(as_IB(newImage)->isGraphiteBacked());
            return newImage;
        }
    }

    // 2. Ask the generator to return a bitmap, which the GPU can convert.
    if (SkBitmap bitmap; this->getROPixels(nullptr, &bitmap, CachingHint::kDisallow_CachingHint)) {
        return skgpu::graphite::MakeFromBitmap(recorder,
                                               this->imageInfo().colorInfo(),
                                               bitmap,
                                               nullptr,
                                               skgpu::Budgeted::kNo,
                                               requiredProps);
    }

    return nullptr;
}

sk_sp<SkImage> SkImage_Lazy::onMakeColorTypeAndColorSpace(
        SkColorType targetCT,
        sk_sp<SkColorSpace> targetCS,
        skgpu::graphite::Recorder* recorder,
        RequiredImageProperties requiredProps) const {
    SkAutoMutexExclusive autoAquire(fOnMakeColorTypeAndSpaceMutex);
    if (fOnMakeColorTypeAndSpaceResult &&
        targetCT == fOnMakeColorTypeAndSpaceResult->colorType() &&
        SkColorSpace::Equals(targetCS.get(), fOnMakeColorTypeAndSpaceResult->colorSpace())) {
        return fOnMakeColorTypeAndSpaceResult;
    }
    Validator validator(fSharedGenerator, &targetCT, targetCS);
    sk_sp<SkImage> result = validator ? sk_sp<SkImage>(new SkImage_Lazy(&validator)) : nullptr;
    if (result) {
        fOnMakeColorTypeAndSpaceResult = result;
    }

    if (recorder) {
        return result->makeTextureImage(recorder, requiredProps);
    } else {
        return result;
    }
}

#endif
