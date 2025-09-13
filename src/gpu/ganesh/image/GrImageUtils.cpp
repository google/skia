/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/image/GrImageUtils.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRect.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkSize.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "include/core/SkYUVAInfo.h"
#include "include/core/SkYUVAPixmaps.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/gpu/ganesh/GrContextOptions.h"
#include "include/gpu/ganesh/GrRecordingContext.h"
#include "include/gpu/ganesh/GrTypes.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/private/SkIDChangeListener.h"
#include "include/private/base/SkMutex.h"
#include "include/private/gpu/ganesh/GrImageContext.h"
#include "include/private/gpu/ganesh/GrTextureGenerator.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/core/SkBlurEngine.h"
#include "src/core/SkCachedData.h"
#include "src/core/SkImageFilterCache.h"
#include "src/core/SkImageFilterTypes.h"
#include "src/core/SkSamplingPriv.h"
#include "src/gpu/ResourceKey.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/Swizzle.h"
#include "src/gpu/ganesh/Device.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrColorSpaceXform.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/GrImageInfo.h"
#include "src/gpu/ganesh/GrProxyProvider.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrSamplerState.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/gpu/ganesh/GrTextureProxy.h"
#include "src/gpu/ganesh/GrThreadSafeCache.h"
#include "src/gpu/ganesh/GrYUVATextureProxies.h"
#include "src/gpu/ganesh/SkGr.h"
#include "src/gpu/ganesh/SurfaceDrawContext.h"
#include "src/gpu/ganesh/SurfaceFillContext.h"
#include "src/gpu/ganesh/effects/GrBicubicEffect.h"
#include "src/gpu/ganesh/effects/GrTextureEffect.h"
#include "src/gpu/ganesh/effects/GrYUVtoRGBEffect.h"
#include "src/gpu/ganesh/image/SkImage_Ganesh.h"
#include "src/gpu/ganesh/image/SkImage_GaneshBase.h"
#include "src/gpu/ganesh/image/SkImage_RasterPinnable.h"
#include "src/gpu/ganesh/image/SkSpecialImage_Ganesh.h"
#include "src/image/SkImage_Base.h"
#include "src/image/SkImage_Lazy.h"
#include "src/image/SkImage_Picture.h"
#include "src/image/SkImage_Raster.h"

#include <string_view>
#include <utility>

class SkDevice;
class SkMatrix;
class SkSurfaceProps;
enum SkColorType : int;

class SkSpecialImage;

namespace skgpu::ganesh {

GrSurfaceProxyView CopyView(GrRecordingContext* context,
                            GrSurfaceProxyView src,
                            skgpu::Mipmapped mipmapped,
                            GrImageTexGenPolicy policy,
                            std::string_view label) {
    skgpu::Budgeted budgeted = policy == GrImageTexGenPolicy::kNew_Uncached_Budgeted
                                       ? skgpu::Budgeted::kYes
                                       : skgpu::Budgeted::kNo;
    return GrSurfaceProxyView::Copy(context,
                                    std::move(src),
                                    mipmapped,
                                    SkBackingFit::kExact,
                                    budgeted,
                                    /*label=*/label);
}

std::tuple<GrSurfaceProxyView, GrColorType> RasterAsView(GrRecordingContext* rContext,
                                                         const SkImage_Raster* raster,
                                                         skgpu::Mipmapped mipmapped,
                                                         GrImageTexGenPolicy policy) {
    if (policy == GrImageTexGenPolicy::kDraw) {
        // If the draw doesn't require mipmaps but this SkImage has them go ahead and make a
        // mipmapped texture. There are three reasons for this:
        // 1) Avoiding another texture creation if a later draw requires mipmaps.
        // 2) Ensuring we upload the bitmap's levels instead of generating on the GPU from the base.
        if (raster->hasMipmaps()) {
            mipmapped = skgpu::Mipmapped::kYes;
        }
        return GrMakeCachedBitmapProxyView(rContext,
                                           raster->bitmap(),
                                           /*label=*/"TextureForImageRasterWithPolicyEqualKDraw",
                                           mipmapped);
    }
    auto budgeted = (policy == GrImageTexGenPolicy::kNew_Uncached_Unbudgeted)
                            ? skgpu::Budgeted::kNo
                            : skgpu::Budgeted::kYes;
    return GrMakeUncachedBitmapProxyView(
            rContext, raster->bitmap(), mipmapped, SkBackingFit::kExact, budgeted);
}

// Returns the GrColorType to use with the GrTextureProxy returned from lockTextureProxy. This
// may be different from the color type on the image in the case where we need up upload CPU
// data to a texture but the GPU doesn't support the format of CPU data. In this case we convert
// the data to RGBA_8888 unorm on the CPU then upload that.
GrColorType ColorTypeOfLockTextureProxy(const GrCaps* caps, SkColorType sct) {
    GrColorType ct = SkColorTypeToGrColorType(sct);
    GrBackendFormat format = caps->getDefaultBackendFormat(ct, GrRenderable::kNo);
    if (!format.isValid()) {
        ct = GrColorType::kRGBA_8888;
    }
    return ct;
}

static GrSurfaceOrigin get_origin(const SkImage_Lazy* img) {
    SkASSERT(img->generator());
    if (!img->generator()->isTextureGenerator()) {
        return kTopLeft_GrSurfaceOrigin;
    }
    // origin should be thread safe
    return static_cast<const GrTextureGenerator*>(img->generator()->fGenerator.get())->origin();
}


static GrSurfaceProxyView texture_proxy_view_from_planes(GrRecordingContext* ctx,
                                                         const SkImage_Lazy* img,
                                                         skgpu::Budgeted budgeted) {
    auto supportedDataTypes = SupportedTextureFormats(*ctx);
    SkYUVAPixmaps yuvaPixmaps;
    sk_sp<SkCachedData> dataStorage = img->getPlanes(supportedDataTypes, &yuvaPixmaps);
    if (!dataStorage) {
        return {};
    }

    GrSurfaceProxyView views[SkYUVAInfo::kMaxPlanes];
    GrColorType pixmapColorTypes[SkYUVAInfo::kMaxPlanes];
    for (int i = 0; i < yuvaPixmaps.numPlanes(); ++i) {
        // If the sizes of the components are not all the same we choose to create exact-match
        // textures for the smaller ones rather than add a texture domain to the draw.
        // TODO: revisit this decision to improve texture reuse?
        SkBackingFit fit = yuvaPixmaps.plane(i).dimensions() == img->dimensions()
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

        std::tie(views[i], std::ignore) =
                GrMakeUncachedBitmapProxyView(ctx, bitmap, skgpu::Mipmapped::kNo, fit);
        if (!views[i]) {
            return {};
        }
        pixmapColorTypes[i] = SkColorTypeToGrColorType(bitmap.colorType());
    }

    // TODO: investigate preallocating mip maps here
    GrImageInfo info(SkColorTypeToGrColorType(img->colorType()),
                     kPremul_SkAlphaType,
                     /*color space*/ nullptr,
                     img->dimensions());

    auto sfc = ctx->priv().makeSFC(info,
                                   "ImageLazy_TextureProxyViewFromPlanes",
                                   SkBackingFit::kExact,
                                   1,
                                   skgpu::Mipmapped::kNo,
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
    SkColorSpace* srcColorSpace = img->generator()->getInfo().colorSpace();
    SkColorSpace* dstColorSpace = img->colorSpace();

    // If the caller expects the pixels in a different color space than the one from the image,
    // apply a color conversion to do this.
    fp = GrColorSpaceXformEffect::Make(std::move(fp),
                                       srcColorSpace, kOpaque_SkAlphaType,
                                       dstColorSpace, kOpaque_SkAlphaType);
    sfc->fillWithFP(std::move(fp));

    return sfc->readSurfaceView();
}

static GrSurfaceProxyView generate_picture_texture(GrRecordingContext* ctx,
                                                   const SkImage_Picture* img,
                                                   skgpu::Mipmapped mipmapped,
                                                   GrImageTexGenPolicy texGenPolicy) {
    SkASSERT(ctx);
    SkASSERT(img);

    skgpu::Budgeted budgeted = texGenPolicy == GrImageTexGenPolicy::kNew_Uncached_Unbudgeted
                                       ? skgpu::Budgeted::kNo
                                       : skgpu::Budgeted::kYes;
    auto surface = SkSurfaces::RenderTarget(ctx,
                                            budgeted,
                                            img->imageInfo(),
                                            0,
                                            kTopLeft_GrSurfaceOrigin,
                                            img->props(),
                                            mipmapped == skgpu::Mipmapped::kYes);
    if (!surface) {
        return {};
    }

    img->replay(surface->getCanvas());

    sk_sp<SkImage> image(surface->makeImageSnapshot());
    if (!image) {
        return {};
    }

    auto [view, ct] = AsView(ctx, image, mipmapped);
    SkASSERT(view);
    SkASSERT(mipmapped == skgpu::Mipmapped::kNo ||
             view.asTextureProxy()->mipmapped() == skgpu::Mipmapped::kYes);
    return view;
}

// Returns the texture proxy. We will always cache the generated texture on success.
// We have 4 ways to try to return a texture (in sorted order)
//
// 1. Check the cache for a pre-existing one
// 2. Ask the generator to natively create one
// 3. Ask the generator to return YUV planes, which the GPU can convert
// 4. Ask the generator to return RGB(A) data, which the GPU can convert
GrSurfaceProxyView LockTextureProxyView(GrRecordingContext* rContext,
                                        const SkImage_Lazy* img,
                                        GrImageTexGenPolicy texGenPolicy,
                                        skgpu::Mipmapped mipmapped) {
    skgpu::UniqueKey key;
    if (texGenPolicy == GrImageTexGenPolicy::kDraw) {
        GrMakeKeyFromImageID(&key, img->uniqueID(), SkIRect::MakeSize(img->dimensions()));
    }

    const GrCaps* caps = rContext->priv().caps();
    GrProxyProvider* proxyProvider = rContext->priv().proxyProvider();

    auto installKey = [&](const GrSurfaceProxyView& view) {
        SkASSERT(view && view.asTextureProxy());
        if (key.isValid()) {
            auto listener = GrMakeUniqueKeyInvalidationListener(&key, rContext->priv().contextID());
            img->addUniqueIDListener(std::move(listener));
            proxyProvider->assignUniqueKeyToProxy(key, view.asTextureProxy());
        }
    };

    auto ct = ColorTypeOfLockTextureProxy(caps, img->colorType());

    // 1. Check the cache for a pre-existing one.
    if (key.isValid()) {
        auto proxy = proxyProvider->findOrCreateProxyByUniqueKey(key);
        if (proxy) {
            skgpu::Swizzle swizzle = caps->getReadSwizzle(proxy->backendFormat(), ct);
            GrSurfaceOrigin origin = get_origin(img);
            GrSurfaceProxyView view(std::move(proxy), origin, swizzle);
            if (mipmapped == skgpu::Mipmapped::kNo ||
                view.asTextureProxy()->mipmapped() == skgpu::Mipmapped::kYes) {
                return view;
            } else {
                // We need a mipped proxy, but we found a cached proxy that wasn't mipped. Thus we
                // generate a new mipped surface and copy the original proxy into the base layer. We
                // will then let the gpu generate the rest of the mips.
                auto mippedView = GrCopyBaseMipMapToView(rContext, view);
                if (!mippedView) {
                    // We failed to make a mipped proxy with the base copied into it. This could
                    // have been from failure to make the proxy or failure to do the copy. Thus we
                    // will fall back to just using the non mipped proxy; See skbug.com/40038328.
                    return view;
                }
                proxyProvider->removeUniqueKeyFromProxy(view.asTextureProxy());
                installKey(mippedView);
                return mippedView;
            }
        }
    }

    // 2. Ask the generator to natively create one (if it knows how)
    {
        if (img->type() == SkImage_Base::Type::kLazyPicture) {
            if (auto view = generate_picture_texture(rContext,
                                                     static_cast<const SkImage_Picture*>(img),
                                                     mipmapped,
                                                     texGenPolicy)) {
                installKey(view);
                return view;
            }
            // The fallback for this would be to generate a bitmap, but some picture-backed
            // images can only be played back on the GPU.
            return {};
        } else if (img->generator()->isTextureGenerator()) {
            auto sharedGenerator = img->generator();
            SkAutoMutexExclusive mutex(sharedGenerator->fMutex);
            auto textureGen = static_cast<GrTextureGenerator*>(sharedGenerator->fGenerator.get());
            if (auto view = textureGen->generateTexture(rContext,
                                                        img->imageInfo(),
                                                        mipmapped,
                                                        texGenPolicy)) {
                installKey(view);
                return view;
            }
        }
    }

    // 3. Ask the generator to return YUV planes, which the GPU can convert. If we will be mipping
    //    the texture we skip this step so the CPU generate non-planar MIP maps for us.
    if (mipmapped == skgpu::Mipmapped::kNo &&
        !rContext->priv().options().fDisableGpuYUVConversion) {
        // TODO: Update to create the mipped surface in the textureProxyViewFromPlanes generator and
        //  draw the base layer directly into the mipped surface.
        skgpu::Budgeted budgeted = texGenPolicy == GrImageTexGenPolicy::kNew_Uncached_Unbudgeted
                                           ? skgpu::Budgeted::kNo
                                           : skgpu::Budgeted::kYes;
        auto view = texture_proxy_view_from_planes(rContext, img, budgeted);
        if (view) {
            installKey(view);
            return view;
        }
    }

    // 4. Ask the generator to return a bitmap, which the GPU can convert.
    auto hint = texGenPolicy == GrImageTexGenPolicy::kDraw ? SkImage::CachingHint::kAllow_CachingHint
                                                           : SkImage::CachingHint::kDisallow_CachingHint;
    if (SkBitmap bitmap; img->getROPixels(nullptr, &bitmap, hint)) {
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

static std::tuple<GrSurfaceProxyView, GrColorType> lazy_as_view(GrRecordingContext* context,
                                                                const SkImage_Lazy* img,
                                                                skgpu::Mipmapped mipmapped,
                                                                GrImageTexGenPolicy policy) {
    GrColorType ct = ColorTypeOfLockTextureProxy(context->priv().caps(), img->colorType());
    return {LockTextureProxyView(context, img, policy, mipmapped), ct};
}

std::tuple<GrSurfaceProxyView, GrColorType> AsView(GrRecordingContext* rContext,
                                                   const SkImage* img,
                                                   skgpu::Mipmapped mipmapped,
                                                   GrImageTexGenPolicy policy) {
    SkASSERT(img);
    if (!rContext) {
        return {};
    }
    if (!rContext->priv().caps()->mipmapSupport() || img->dimensions().area() <= 1) {
        mipmapped = skgpu::Mipmapped::kNo;
    }

    auto ib = static_cast<const SkImage_Base*>(img);
    if (ib->type() == SkImage_Base::Type::kRaster) {
        return skgpu::ganesh::RasterAsView(
                    rContext, static_cast<const SkImage_Raster*>(ib), mipmapped, policy);
    } else if (ib->type() == SkImage_Base::Type::kRasterPinnable) {
        auto rp = static_cast<const SkImage_RasterPinnable*>(img);
        return rp->asView(rContext, mipmapped, policy);
    } else if (ib->isGaneshBacked()) {
        auto gb = static_cast<const SkImage_GaneshBase*>(img);
        return gb->asView(rContext, mipmapped, policy);
    } else if (ib->isLazyGenerated()) {
        return lazy_as_view(rContext, static_cast<const SkImage_Lazy*>(ib), mipmapped, policy);
    }

    SkDEBUGFAIL("Unsupported image type to make a View");
    return {};
}

static std::unique_ptr<GrFragmentProcessor> make_fp_from_view(GrRecordingContext* rContext,
                                                              GrSurfaceProxyView view,
                                                              SkAlphaType at,
                                                              SkSamplingOptions sampling,
                                                              const SkTileMode tileModes[2],
                                                              const SkMatrix& m,
                                                              const SkRect* subset,
                                                              const SkRect* domain) {
    if (!view) {
        return nullptr;
    }
    const GrCaps& caps = *rContext->priv().caps();
    auto wmx = SkTileModeToWrapMode(tileModes[0]);
    auto wmy = SkTileModeToWrapMode(tileModes[1]);
    if (sampling.useCubic) {
        if (subset) {
            if (domain) {
                return GrBicubicEffect::MakeSubset(std::move(view),
                                                   at,
                                                   m,
                                                   wmx,
                                                   wmy,
                                                   *subset,
                                                   *domain,
                                                   sampling.cubic,
                                                   GrBicubicEffect::Direction::kXY,
                                                   *rContext->priv().caps());
            }
            return GrBicubicEffect::MakeSubset(std::move(view),
                                               at,
                                               m,
                                               wmx,
                                               wmy,
                                               *subset,
                                               sampling.cubic,
                                               GrBicubicEffect::Direction::kXY,
                                               *rContext->priv().caps());
        }
        return GrBicubicEffect::Make(std::move(view),
                                     at,
                                     m,
                                     wmx,
                                     wmy,
                                     sampling.cubic,
                                     GrBicubicEffect::Direction::kXY,
                                     *rContext->priv().caps());
    }
    if (sampling.isAniso()) {
        if (!rContext->priv().caps()->anisoSupport()) {
            // Fallback to linear
            sampling = SkSamplingPriv::AnisoFallback(view.mipmapped() == skgpu::Mipmapped::kYes);
        }
    } else if (view.mipmapped() == skgpu::Mipmapped::kNo) {
        sampling = SkSamplingOptions(sampling.filter);
    }
    GrSamplerState sampler;
    if (sampling.isAniso()) {
        sampler = GrSamplerState::Aniso(wmx, wmy, sampling.maxAniso, view.mipmapped());
    } else {
        sampler = GrSamplerState(wmx, wmy, sampling.filter, sampling.mipmap);
    }
    if (subset) {
        if (domain) {
            return GrTextureEffect::MakeSubset(
                    std::move(view), at, m, sampler, *subset, *domain, caps);
        }
        return GrTextureEffect::MakeSubset(std::move(view), at, m, sampler, *subset, caps);
    } else {
        return GrTextureEffect::Make(std::move(view), at, m, sampler, caps);
    }
}

std::unique_ptr<GrFragmentProcessor> raster_as_fp(GrRecordingContext* rContext,
                                                  const SkImage_Raster* img,
                                                  SkSamplingOptions sampling,
                                                  const SkTileMode tileModes[2],
                                                  const SkMatrix& m,
                                                  const SkRect* subset,
                                                  const SkRect* domain) {
    auto mm =
            sampling.mipmap == SkMipmapMode::kNone ? skgpu::Mipmapped::kNo : skgpu::Mipmapped::kYes;
    return make_fp_from_view(rContext,
                             std::get<0>(AsView(rContext, img, mm)),
                             img->alphaType(),
                             sampling,
                             tileModes,
                             m,
                             subset,
                             domain);
}

std::unique_ptr<GrFragmentProcessor> AsFragmentProcessor(SurfaceDrawContext* sdc,
                                                         const SkImage* img,
                                                         SkSamplingOptions sampling,
                                                         const SkTileMode tileModes[2],
                                                         const SkMatrix& m,
                                                         const SkRect* subset,
                                                         const SkRect* domain) {
    if (!sdc) {
        return {};
    }
    GrRecordingContext* rContext = sdc->recordingContext();
    if (!rContext) {
        return {};
    }
    if (sampling.useCubic && !GrValidCubicResampler(sampling.cubic)) {
        return {};
    }
    if (sampling.mipmap != SkMipmapMode::kNone &&
        (!rContext->priv().caps()->mipmapSupport() || img->dimensions().area() <= 1)) {
        sampling = SkSamplingOptions(sampling.filter);
    }

    auto ib = static_cast<const SkImage_Base*>(img);
    if (ib->isRasterBacked()) {
        return raster_as_fp(rContext,
                            static_cast<const SkImage_Raster*>(ib),
                            sampling,
                            tileModes,
                            m,
                            subset,
                            domain);
    } else if (ib->isGaneshBacked()) {
        auto gb = static_cast<const SkImage_GaneshBase*>(img);
        return gb->asFragmentProcessor(sdc, sampling, tileModes, m, subset, domain);
    } else if (ib->isLazyGenerated()) {
        // TODO: If the CPU data is extracted as planes return a FP that reconstructs the image from
        // the planes.
        auto mm = sampling.mipmap == SkMipmapMode::kNone ? skgpu::Mipmapped::kNo : skgpu::Mipmapped::kYes;
        return MakeFragmentProcessorFromView(rContext,
                                             std::get<0>(AsView(rContext, img, mm)),
                                             img->alphaType(),
                                             sampling,
                                             tileModes,
                                             m,
                                             subset,
                                             domain);
    }

    SkDEBUGFAIL("Unsupported image type to make a FragmentProcessor");
    return {};
}

std::unique_ptr<GrFragmentProcessor> MakeFragmentProcessorFromView(
        GrRecordingContext* rContext,
        GrSurfaceProxyView view,
        SkAlphaType at,
        SkSamplingOptions sampling,
        const SkTileMode tileModes[2],
        const SkMatrix& m,
        const SkRect* subset,
        const SkRect* domain) {
    if (!view) {
        return nullptr;
    }
    const GrCaps& caps = *rContext->priv().caps();
    auto wmx = SkTileModeToWrapMode(tileModes[0]);
    auto wmy = SkTileModeToWrapMode(tileModes[1]);
    if (sampling.useCubic) {
        if (subset) {
            if (domain) {
                return GrBicubicEffect::MakeSubset(std::move(view),
                                                   at,
                                                   m,
                                                   wmx,
                                                   wmy,
                                                   *subset,
                                                   *domain,
                                                   sampling.cubic,
                                                   GrBicubicEffect::Direction::kXY,
                                                   *rContext->priv().caps());
            }
            return GrBicubicEffect::MakeSubset(std::move(view),
                                               at,
                                               m,
                                               wmx,
                                               wmy,
                                               *subset,
                                               sampling.cubic,
                                               GrBicubicEffect::Direction::kXY,
                                               *rContext->priv().caps());
        }
        return GrBicubicEffect::Make(std::move(view),
                                     at,
                                     m,
                                     wmx,
                                     wmy,
                                     sampling.cubic,
                                     GrBicubicEffect::Direction::kXY,
                                     *rContext->priv().caps());
    }
    if (sampling.isAniso()) {
        if (!rContext->priv().caps()->anisoSupport()) {
            // Fallback to linear
            sampling = SkSamplingPriv::AnisoFallback(view.mipmapped() == skgpu::Mipmapped::kYes);
        }
    } else if (view.mipmapped() == skgpu::Mipmapped::kNo) {
        sampling = SkSamplingOptions(sampling.filter);
    }
    GrSamplerState sampler;
    if (sampling.isAniso()) {
        sampler = GrSamplerState::Aniso(wmx, wmy, sampling.maxAniso, view.mipmapped());
    } else {
        sampler = GrSamplerState(wmx, wmy, sampling.filter, sampling.mipmap);
    }
    if (subset) {
        if (domain) {
            return GrTextureEffect::MakeSubset(std::move(view),
                                               at,
                                               m,
                                               sampler,
                                               *subset,
                                               *domain,
                                               caps);
        }
        return GrTextureEffect::MakeSubset(std::move(view),
                                           at,
                                           m,
                                           sampler,
                                           *subset,
                                           caps);
    } else {
        return GrTextureEffect::Make(std::move(view), at, m, sampler, caps);
    }
}

GrSurfaceProxyView FindOrMakeCachedMipmappedView(GrRecordingContext* rContext,
                                                 GrSurfaceProxyView view,
                                                 uint32_t imageUniqueID) {
    SkASSERT(rContext);
    SkASSERT(imageUniqueID != SK_InvalidUniqueID);

    if (!view || view.proxy()->asTextureProxy()->mipmapped() == skgpu::Mipmapped::kYes) {
        return view;
    }
    GrProxyProvider* proxyProvider = rContext->priv().proxyProvider();

    skgpu::UniqueKey baseKey;
    GrMakeKeyFromImageID(&baseKey, imageUniqueID, SkIRect::MakeSize(view.dimensions()));
    SkASSERT(baseKey.isValid());
    skgpu::UniqueKey mipmappedKey;
    static const skgpu::UniqueKey::Domain kMipmappedDomain = skgpu::UniqueKey::GenerateDomain();
    {  // No extra values beyond the domain are required. Must name the var to please
       // clang-tidy.
        skgpu::UniqueKey::Builder b(&mipmappedKey, baseKey, kMipmappedDomain, 0);
    }
    SkASSERT(mipmappedKey.isValid());
    if (sk_sp<GrTextureProxy> cachedMippedView =
                proxyProvider->findOrCreateProxyByUniqueKey(mipmappedKey)) {
        return {std::move(cachedMippedView), view.origin(), view.swizzle()};
    }

    auto copy = GrCopyBaseMipMapToView(rContext, view);
    if (!copy) {
        return view;
    }
    // TODO: If we move listeners up from SkImage_Lazy to SkImage_Base then add one here.
    proxyProvider->assignUniqueKeyToProxy(mipmappedKey, copy.asTextureProxy());
    return copy;
}

using DataType = SkYUVAPixmapInfo::DataType;

SkYUVAPixmapInfo::SupportedDataTypes SupportedTextureFormats(const GrImageContext& context) {
    SkYUVAPixmapInfo::SupportedDataTypes dataTypes;
    const auto isValid = [&context](DataType dt, int n) {
        return context.defaultBackendFormat(SkYUVAPixmapInfo::DefaultColorTypeForDataType(dt, n),
                                            GrRenderable::kNo).isValid();
    };
     for (int n = 1; n <= 4; ++n) {
        if (isValid(DataType::kUnorm8, n)) {
            dataTypes.enableDataType(DataType::kUnorm8, n);
        }
        if (isValid(DataType::kUnorm16, n)) {
            dataTypes.enableDataType(DataType::kUnorm16, n);
        }
        if (isValid(DataType::kFloat16, n)) {
            dataTypes.enableDataType(DataType::kFloat16, n);
        }
        if (isValid(DataType::kUnorm10_Unorm2, n)) {
            dataTypes.enableDataType(DataType::kUnorm10_Unorm2, n);
        }
    }
     return dataTypes;
}

}  // namespace skgpu::ganesh

namespace skif {

namespace {

class GaneshBackend :
        public Backend,
        private SkShaderBlurAlgorithm,
        private SkBlurEngine {
public:

    GaneshBackend(sk_sp<GrRecordingContext> context,
                  GrSurfaceOrigin origin,
                  const SkSurfaceProps& surfaceProps,
                  SkColorType colorType)
            : Backend(SkImageFilterCache::Create(SkImageFilterCache::kDefaultTransientSize),
                      surfaceProps, colorType)
            , fContext(std::move(context))
            , fOrigin(origin) {}

    // Backend
    sk_sp<SkDevice> makeDevice(SkISize size,
                               sk_sp<SkColorSpace> colorSpace,
                               const SkSurfaceProps* props) const override {
        SkImageInfo imageInfo = SkImageInfo::Make(size,
                                                  this->colorType(),
                                                  kPremul_SkAlphaType,
                                                  std::move(colorSpace));

        return fContext->priv().createDevice(skgpu::Budgeted::kYes,
                                             imageInfo,
                                             SkBackingFit::kApprox,
                                             1,
                                             skgpu::Mipmapped::kNo,
                                             GrProtected::kNo,
                                             fOrigin,
                                             props ? *props : this->surfaceProps(),
                                             skgpu::ganesh::Device::InitContents::kUninit);
    }

    sk_sp<SkSpecialImage> makeImage(const SkIRect& subset, sk_sp<SkImage> image) const override {
        return SkSpecialImages::MakeFromTextureImage(
                fContext.get(), subset, image, this->surfaceProps());
    }

    sk_sp<SkImage> getCachedBitmap(const SkBitmap& data) const override {
        // This uses the thread safe cache (instead of GrMakeCachedBitmapProxyView) so that image
        // filters can be evaluated on other threads with DDLs.
        auto threadSafeCache = fContext->priv().threadSafeCache();

        skgpu::UniqueKey key;
        SkIRect subset = SkIRect::MakePtSize(data.pixelRefOrigin(), data.dimensions());
        GrMakeKeyFromImageID(&key, data.getGenerationID(), subset);

        auto view = threadSafeCache->find(key);
        if (!view) {
            view = std::get<0>(GrMakeUncachedBitmapProxyView(fContext.get(), data));
            if (!view) {
                return nullptr;
            }
            threadSafeCache->add(key, view);
        }

        return sk_make_sp<SkImage_Ganesh>(fContext,
                                          data.getGenerationID(),
                                          std::move(view),
                                          data.info().colorInfo());
    }

    const SkBlurEngine* getBlurEngine() const override { return this; }

    // SkBlurEngine
    const SkBlurEngine::Algorithm* findAlgorithm(SkSize sigma,
                                                 SkColorType colorType) const override {
        // GrBlurUtils supports all tile modes and color types
        return this;
    }

    // SkShaderBlurAlgorithm
    sk_sp<SkDevice> makeDevice(const SkImageInfo& imageInfo) const override {
        return fContext->priv().createDevice(skgpu::Budgeted::kYes,
                                             imageInfo,
                                             SkBackingFit::kApprox,
                                             1,
                                             skgpu::Mipmapped::kNo,
                                             GrProtected::kNo,
                                             fOrigin,
                                             this->surfaceProps(),
                                             skgpu::ganesh::Device::InitContents::kUninit);
    }

private:
    sk_sp<GrRecordingContext> fContext;
    GrSurfaceOrigin fOrigin;
};

} // anonymous namespace

sk_sp<Backend> MakeGaneshBackend(sk_sp<GrRecordingContext> context,
                                 GrSurfaceOrigin origin,
                                 const SkSurfaceProps& surfaceProps,
                                 SkColorType colorType) {
    SkASSERT(context);
    return sk_make_sp<GaneshBackend>(std::move(context), origin, surfaceProps, colorType);
}

}  // namespace skif
