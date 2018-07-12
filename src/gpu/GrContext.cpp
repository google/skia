/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrContext.h"
#include "GrBackendSemaphore.h"
#include "GrClip.h"
#include "GrContextOptions.h"
#include "GrContextPriv.h"
#include "GrDrawingManager.h"
#include "GrGpu.h"
#include "GrMemoryPool.h"
#include "GrProxyProvider.h"
#include "GrRenderTargetContext.h"
#include "GrRenderTargetProxy.h"
#include "GrResourceCache.h"
#include "GrResourceProvider.h"
#include "GrSemaphore.h"
#include "GrSoftwarePathRenderer.h"
#include "GrSurfaceContext.h"
#include "GrSurfacePriv.h"
#include "GrSurfaceProxyPriv.h"
#include "GrTexture.h"
#include "GrTextureContext.h"
#include "GrTextureStripAtlas.h"
#include "GrTracing.h"
#include "SkAutoPixmapStorage.h"
#include "SkDeferredDisplayList.h"
#include "SkGr.h"
#include "SkImageInfoPriv.h"
#include "SkJSONWriter.h"
#include "SkMakeUnique.h"
#include "SkSurface_Gpu.h"
#include "SkTaskGroup.h"
#include "SkUnPreMultiplyPriv.h"
#include "effects/GrConfigConversionEffect.h"
#include "text/GrTextBlobCache.h"

#define ASSERT_OWNED_PROXY(P) \
SkASSERT(!(P) || !((P)->priv().peekTexture()) || (P)->priv().peekTexture()->getContext() == this)
#define ASSERT_OWNED_PROXY_PRIV(P) \
SkASSERT(!(P) || !((P)->priv().peekTexture()) || (P)->priv().peekTexture()->getContext() == fContext)

#define ASSERT_OWNED_RESOURCE(R) SkASSERT(!(R) || (R)->getContext() == this)
#define ASSERT_SINGLE_OWNER \
    SkDEBUGCODE(GrSingleOwner::AutoEnforce debug_SingleOwner(&fSingleOwner);)
#define ASSERT_SINGLE_OWNER_PRIV \
    SkDEBUGCODE(GrSingleOwner::AutoEnforce debug_SingleOwner(&fContext->fSingleOwner);)
#define RETURN_IF_ABANDONED if (fDrawingManager->wasAbandoned()) { return; }
#define RETURN_IF_ABANDONED_PRIV if (fContext->fDrawingManager->wasAbandoned()) { return; }
#define RETURN_FALSE_IF_ABANDONED if (fDrawingManager->wasAbandoned()) { return false; }
#define RETURN_FALSE_IF_ABANDONED_PRIV if (fContext->fDrawingManager->wasAbandoned()) { return false; }
#define RETURN_NULL_IF_ABANDONED if (fDrawingManager->wasAbandoned()) { return nullptr; }

////////////////////////////////////////////////////////////////////////////////

static int32_t gNextID = 1;
static int32_t next_id() {
    int32_t id;
    do {
        id = sk_atomic_inc(&gNextID);
    } while (id == SK_InvalidGenID);
    return id;
}

GrContext::GrContext(GrBackend backend, int32_t id)
        : fBackend(backend)
        , fUniqueID(SK_InvalidGenID == id ? next_id() : id) {
    fResourceCache = nullptr;
    fResourceProvider = nullptr;
    fProxyProvider = nullptr;
    fGlyphCache = nullptr;
}

bool GrContext::initCommon(const GrContextOptions& options) {
    ASSERT_SINGLE_OWNER
    SkASSERT(fCaps);  // needs to have been initialized by derived classes
    SkASSERT(fThreadSafeProxy); // needs to have been initialized by derived classes

    if (fGpu) {
        fCaps = fGpu->refCaps();
        fResourceCache = new GrResourceCache(fCaps.get(), fUniqueID);
        fResourceProvider = new GrResourceProvider(fGpu.get(), fResourceCache, &fSingleOwner,
                                                   options.fExplicitlyAllocateGPUResources);
        fProxyProvider =
                new GrProxyProvider(fResourceProvider, fResourceCache, fCaps, &fSingleOwner);
    } else {
        fProxyProvider = new GrProxyProvider(this->uniqueID(), fCaps, &fSingleOwner);
    }

    if (fResourceCache) {
        fResourceCache->setProxyProvider(fProxyProvider);
    }

    fTextureStripAtlasManager.reset(new GrTextureStripAtlasManager);

    fDisableGpuYUVConversion = options.fDisableGpuYUVConversion;
    fSharpenMipmappedTextures = options.fSharpenMipmappedTextures;
    fDidTestPMConversions = false;

    GrPathRendererChain::Options prcOptions;
    prcOptions.fAllowPathMaskCaching = options.fAllowPathMaskCaching;
#if GR_TEST_UTILS
    prcOptions.fGpuPathRenderers = options.fGpuPathRenderers;
#endif
    if (options.fDisableCoverageCountingPaths) {
        prcOptions.fGpuPathRenderers &= ~GpuPathRenderers::kCoverageCounting;
    }
    if (options.fDisableDistanceFieldPaths) {
        prcOptions.fGpuPathRenderers &= ~GpuPathRenderers::kSmall;
    }

    if (!fResourceCache) {
        // DDL TODO: remove this crippling of the path renderer chain
        // Disable the small path renderer bc of the proxies in the atlas. They need to be
        // unified when the opLists are added back to the destination drawing manager.
        prcOptions.fGpuPathRenderers &= ~GpuPathRenderers::kSmall;
        prcOptions.fGpuPathRenderers &= ~GpuPathRenderers::kStencilAndCover;
    }

    GrTextContext::Options textContextOptions;
    textContextOptions.fMaxDistanceFieldFontSize = options.fGlyphsAsPathsFontSize;
    textContextOptions.fMinDistanceFieldFontSize = options.fMinDistanceFieldFontSize;
    textContextOptions.fDistanceFieldVerticesAlwaysHaveW = false;
#if SK_SUPPORT_ATLAS_TEXT
    if (GrContextOptions::Enable::kYes == options.fDistanceFieldGlyphVerticesAlwaysHaveW) {
        textContextOptions.fDistanceFieldVerticesAlwaysHaveW = true;
    }
#endif

    bool explicitlyAllocatingResources = fResourceProvider
                                            ? fResourceProvider->explicitlyAllocateGPUResources()
                                            : false;
    fDrawingManager.reset(new GrDrawingManager(this, prcOptions, textContextOptions,
                                               &fSingleOwner, explicitlyAllocatingResources,
                                               options.fSortRenderTargets));

    fGlyphCache = new GrGlyphCache(fCaps.get(), options.fGlyphCacheTextureMaximumBytes);

    fTextBlobCache.reset(new GrTextBlobCache(TextBlobCacheOverBudgetCB,
                                             this, this->uniqueID()));

    // DDL TODO: we need to think through how the task group & persistent cache
    // get passed on to/shared between all the DDLRecorders created with this context.
    if (options.fExecutor) {
        fTaskGroup = skstd::make_unique<SkTaskGroup>(*options.fExecutor);
    }

    fPersistentCache = options.fPersistentCache;

    return true;
}

GrContext::~GrContext() {
    ASSERT_SINGLE_OWNER

    if (fDrawingManager) {
        fDrawingManager->cleanup();
    }

    fTextureStripAtlasManager = nullptr;
    delete fResourceProvider;
    delete fResourceCache;
    delete fProxyProvider;
    delete fGlyphCache;
}

//////////////////////////////////////////////////////////////////////////////

GrContextThreadSafeProxy::GrContextThreadSafeProxy(sk_sp<const GrCaps> caps, uint32_t uniqueID,
                                                   GrBackend backend,
                                                   const GrContextOptions& options)
        : fCaps(std::move(caps))
        , fContextUniqueID(uniqueID)
        , fBackend(backend)
        , fOptions(options) {}

GrContextThreadSafeProxy::~GrContextThreadSafeProxy() = default;

sk_sp<GrContextThreadSafeProxy> GrContext::threadSafeProxy() {
    return fThreadSafeProxy;
}

SkSurfaceCharacterization GrContextThreadSafeProxy::createCharacterization(
                                     size_t cacheMaxResourceBytes,
                                     const SkImageInfo& ii, const GrBackendFormat& backendFormat,
                                     int sampleCnt, GrSurfaceOrigin origin,
                                     const SkSurfaceProps& surfaceProps,
                                     bool isMipMapped, bool willUseGLFBO0) {
    if (!backendFormat.isValid()) {
        return SkSurfaceCharacterization(); // return an invalid characterization
    }

    if (kOpenGL_GrBackend != backendFormat.backend() && willUseGLFBO0) {
        // The willUseGLFBO0 flags can only be used for a GL backend.
        return SkSurfaceCharacterization(); // return an invalid characterization
    }

    if (!fCaps->mipMapSupport()) {
        isMipMapped = false;
    }

    GrPixelConfig config = kUnknown_GrPixelConfig;
    if (!fCaps->getConfigFromBackendFormat(backendFormat, ii.colorType(), &config)) {
        return SkSurfaceCharacterization(); // return an invalid characterization
    }

    if (!SkSurface_Gpu::Valid(fCaps.get(), config, ii.colorSpace())) {
        return SkSurfaceCharacterization(); // return an invalid characterization
    }

    sampleCnt = fCaps->getRenderTargetSampleCount(sampleCnt, config);
    if (!sampleCnt) {
        return SkSurfaceCharacterization(); // return an invalid characterization
    }

    GrFSAAType FSAAType = GrFSAAType::kNone;
    if (sampleCnt > 1) {
        FSAAType = fCaps->usesMixedSamples() ? GrFSAAType::kMixedSamples : GrFSAAType::kUnifiedMSAA;
    }

    // This surface characterization factory assumes that the resulting characterization is
    // textureable.
    if (!fCaps->isConfigTexturable(config)) {
        return SkSurfaceCharacterization(); // return an invalid characterization
    }

    return SkSurfaceCharacterization(sk_ref_sp<GrContextThreadSafeProxy>(this),
                                     cacheMaxResourceBytes, ii,
                                     origin, config, FSAAType, sampleCnt,
                                     SkSurfaceCharacterization::Textureable(true),
                                     SkSurfaceCharacterization::MipMapped(isMipMapped),
                                     SkSurfaceCharacterization::UsesGLFBO0(willUseGLFBO0),
                                     surfaceProps);
}

void GrContext::abandonContext() {
    ASSERT_SINGLE_OWNER

    fTextureStripAtlasManager->abandon();
    fProxyProvider->abandon();
    fResourceProvider->abandon();

    // Need to abandon the drawing manager first so all the render targets
    // will be released/forgotten before they too are abandoned.
    fDrawingManager->abandon();

    // abandon first to so destructors
    // don't try to free the resources in the API.
    fResourceCache->abandonAll();

    fGpu->disconnect(GrGpu::DisconnectType::kAbandon);

    fGlyphCache->freeAll();
    fTextBlobCache->freeAll();
}

bool GrContext::abandoned() const {
    ASSERT_SINGLE_OWNER
    return fDrawingManager->wasAbandoned();
}

void GrContext::releaseResourcesAndAbandonContext() {
    ASSERT_SINGLE_OWNER

    fTextureStripAtlasManager->abandon();
    fProxyProvider->abandon();
    fResourceProvider->abandon();

    // Need to abandon the drawing manager first so all the render targets
    // will be released/forgotten before they too are abandoned.
    fDrawingManager->abandon();

    // Release all resources in the backend 3D API.
    fResourceCache->releaseAll();

    fGpu->disconnect(GrGpu::DisconnectType::kCleanup);

    fGlyphCache->freeAll();
    fTextBlobCache->freeAll();
}

void GrContext::resetContext(uint32_t state) {
    ASSERT_SINGLE_OWNER
    fGpu->markContextDirty(state);
}

void GrContext::freeGpuResources() {
    ASSERT_SINGLE_OWNER

    fGlyphCache->freeAll();

    fDrawingManager->freeGpuResources();

    fResourceCache->purgeAllUnlocked();
}

void GrContext::purgeUnlockedResources(bool scratchResourcesOnly) {
    ASSERT_SINGLE_OWNER
    fResourceCache->purgeUnlockedResources(scratchResourcesOnly);
    fResourceCache->purgeAsNeeded();
    fTextBlobCache->purgeStaleBlobs();
}

void GrContext::performDeferredCleanup(std::chrono::milliseconds msNotUsed) {
    ASSERT_SINGLE_OWNER
    fResourceCache->purgeAsNeeded();
    fResourceCache->purgeResourcesNotUsedSince(GrStdSteadyClock::now() - msNotUsed);

    fTextBlobCache->purgeStaleBlobs();
}

void GrContext::purgeUnlockedResources(size_t bytesToPurge, bool preferScratchResources) {
    ASSERT_SINGLE_OWNER
    fResourceCache->purgeUnlockedResources(bytesToPurge, preferScratchResources);
}

void GrContext::getResourceCacheUsage(int* resourceCount, size_t* resourceBytes) const {
    ASSERT_SINGLE_OWNER

    if (resourceCount) {
        *resourceCount = fResourceCache->getBudgetedResourceCount();
    }
    if (resourceBytes) {
        *resourceBytes = fResourceCache->getBudgetedResourceBytes();
    }
}

size_t GrContext::getResourceCachePurgeableBytes() const {
    ASSERT_SINGLE_OWNER
    return fResourceCache->getPurgeableBytes();
}

////////////////////////////////////////////////////////////////////////////////

int GrContext::maxTextureSize() const { return fCaps->maxTextureSize(); }

int GrContext::maxRenderTargetSize() const { return fCaps->maxRenderTargetSize(); }

bool GrContext::colorTypeSupportedAsImage(SkColorType colorType) const {
    GrPixelConfig config = SkColorType2GrPixelConfig(colorType);
    return fCaps->isConfigTexturable(config);
}

int GrContext::maxSurfaceSampleCountForColorType(SkColorType colorType) const {
    GrPixelConfig config = SkColorType2GrPixelConfig(colorType);
    return fCaps->maxRenderTargetSampleCount(config);
}

////////////////////////////////////////////////////////////////////////////////

void GrContext::TextBlobCacheOverBudgetCB(void* data) {
    SkASSERT(data);
    // TextBlobs are drawn at the SkGpuDevice level, therefore they cannot rely on
    // GrRenderTargetContext to perform a necessary flush.  The solution is to move drawText calls
    // to below the GrContext level, but this is not trivial because they call drawPath on
    // SkGpuDevice.
    GrContext* context = reinterpret_cast<GrContext*>(data);
    context->flush();
}

////////////////////////////////////////////////////////////////////////////////

void GrContext::flush() {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED

    fDrawingManager->flush(nullptr);
}

GrSemaphoresSubmitted GrContext::flushAndSignalSemaphores(int numSemaphores,
                                                          GrBackendSemaphore signalSemaphores[]) {
    ASSERT_SINGLE_OWNER
    if (fDrawingManager->wasAbandoned()) { return GrSemaphoresSubmitted::kNo; }

    return fDrawingManager->flush(nullptr, numSemaphores, signalSemaphores);
}

void GrContextPriv::flush(GrSurfaceProxy* proxy) {
    ASSERT_SINGLE_OWNER_PRIV
    RETURN_IF_ABANDONED_PRIV
    ASSERT_OWNED_PROXY_PRIV(proxy);

    fContext->fDrawingManager->flush(proxy);
}

// TODO: This will be removed when GrSurfaceContexts are aware of their color types.
// (skbug.com/6718)
static bool valid_premul_config(GrPixelConfig config) {
    switch (config) {
        case kUnknown_GrPixelConfig:            return false;
        case kAlpha_8_GrPixelConfig:            return false;
        case kGray_8_GrPixelConfig:             return false;
        case kRGB_565_GrPixelConfig:            return false;
        case kRGBA_4444_GrPixelConfig:          return true;
        case kRGBA_8888_GrPixelConfig:          return true;
        case kRGB_888_GrPixelConfig:            return false;
        case kBGRA_8888_GrPixelConfig:          return true;
        case kSRGBA_8888_GrPixelConfig:         return true;
        case kSBGRA_8888_GrPixelConfig:         return true;
        case kRGBA_1010102_GrPixelConfig:       return true;
        case kRGBA_float_GrPixelConfig:         return true;
        case kRG_float_GrPixelConfig:           return false;
        case kAlpha_half_GrPixelConfig:         return false;
        case kRGBA_half_GrPixelConfig:          return true;
        case kAlpha_8_as_Alpha_GrPixelConfig:   return false;
        case kAlpha_8_as_Red_GrPixelConfig:     return false;
        case kAlpha_half_as_Red_GrPixelConfig:  return false;
        case kGray_8_as_Lum_GrPixelConfig:      return false;
        case kGray_8_as_Red_GrPixelConfig:      return false;
    }
    SK_ABORT("Invalid GrPixelConfig");
    return false;
}

static bool valid_premul_color_type(GrColorType ct) {
    switch (ct) {
        case GrColorType::kUnknown:      return false;
        case GrColorType::kAlpha_8:      return false;
        case GrColorType::kRGB_565:      return false;
        case GrColorType::kABGR_4444:    return true;
        case GrColorType::kRGBA_8888:    return true;
        case GrColorType::kRGB_888x:     return false;
        case GrColorType::kBGRA_8888:    return true;
        case GrColorType::kRGBA_1010102: return true;
        case GrColorType::kGray_8:       return false;
        case GrColorType::kAlpha_F16:    return false;
        case GrColorType::kRGBA_F16:     return true;
        case GrColorType::kRG_F32:       return false;
        case GrColorType::kRGBA_F32:     return true;
    }
    SK_ABORT("Invalid GrColorType");
    return false;
}

static bool valid_pixel_conversion(GrColorType cpuColorType, GrPixelConfig gpuConfig,
                                   bool premulConversion) {
    // We only allow premul <-> unpremul conversions for some formats
    if (premulConversion &&
        (!valid_premul_color_type(cpuColorType) || !valid_premul_config(gpuConfig))) {
        return false;
    }
    return true;
}

bool GrContextPriv::writeSurfacePixels(GrSurfaceContext* dst, int left, int top, int width,
                                       int height, GrColorType srcColorType,
                                       SkColorSpace* srcColorSpace, const void* buffer,
                                       size_t rowBytes, uint32_t pixelOpsFlags) {
    ASSERT_SINGLE_OWNER_PRIV
    RETURN_FALSE_IF_ABANDONED_PRIV
    SkASSERT(dst);
    SkASSERT(buffer);
    ASSERT_OWNED_PROXY_PRIV(dst->asSurfaceProxy());
    GR_CREATE_TRACE_MARKER_CONTEXT("GrContextPriv", "writeSurfacePixels", fContext);

    if (GrColorType::kUnknown == srcColorType) {
        return false;
    }

    if (!dst->asSurfaceProxy()->instantiate(this->resourceProvider())) {
        return false;
    }

    GrSurfaceProxy* dstProxy = dst->asSurfaceProxy();
    GrSurface* dstSurface = dstProxy->priv().peekSurface();

    if (!GrSurfacePriv::AdjustWritePixelParams(dstSurface->width(), dstSurface->height(),
                                               GrColorTypeBytesPerPixel(srcColorType), &left, &top,
                                               &width, &height, &buffer, &rowBytes)) {
        return false;
    }

    // TODO: Make GrSurfaceContext know its alpha type and pass src buffer's alpha type.
    bool premul = SkToBool(kUnpremul_PixelOpsFlag & pixelOpsFlags);

    // For canvas2D putImageData performance we have a special code path for unpremul RGBA_8888 srcs
    // that are premultiplied on the GPU. This is kept as narrow as possible for now.
    bool canvas2DFastPath =
            premul &&
            !dst->colorSpaceInfo().colorSpace() &&
            (srcColorType == GrColorType::kRGBA_8888 || srcColorType == GrColorType::kBGRA_8888) &&
            SkToBool(dst->asRenderTargetContext()) &&
            (dstProxy->config() == kRGBA_8888_GrPixelConfig ||
             dstProxy->config() == kBGRA_8888_GrPixelConfig) &&
            !(pixelOpsFlags & kDontFlush_PixelOpsFlag) &&
            fContext->contextPriv().caps()->isConfigTexturable(kRGBA_8888_GrPixelConfig) &&
            fContext->validPMUPMConversionExists();

    if (!fContext->contextPriv().caps()->surfaceSupportsWritePixels(dstSurface) ||
        canvas2DFastPath) {
        // We don't expect callers that are skipping flushes to require an intermediate draw.
        SkASSERT(!(pixelOpsFlags & kDontFlush_PixelOpsFlag));
        if (pixelOpsFlags & kDontFlush_PixelOpsFlag) {
            return false;
        }

        GrSurfaceDesc desc;
        desc.fConfig = canvas2DFastPath ? kRGBA_8888_GrPixelConfig : dstProxy->config();
        desc.fWidth = width;
        desc.fHeight = height;
        desc.fSampleCnt = 1;
        auto tempProxy = this->proxyProvider()->createProxy(
                desc, kTopLeft_GrSurfaceOrigin, SkBackingFit::kApprox, SkBudgeted::kYes);
        if (!tempProxy) {
            return false;
        }
        auto tempCtx = this->drawingManager()->makeTextureContext(
                tempProxy, dst->colorSpaceInfo().refColorSpace());
        if (!tempCtx) {
            return false;
        }
        uint32_t flags = canvas2DFastPath ? 0 : pixelOpsFlags;
        // In the fast path we always write the srcData to the temp context as though it were RGBA.
        // When the data is really BGRA the write will cause the R and B channels to be swapped in
        // the intermediate surface which gets corrected by a swizzle effect when drawing to the
        // dst.
        auto tmpColorType = canvas2DFastPath ? GrColorType::kRGBA_8888 : srcColorType;
        if (!this->writeSurfacePixels(tempCtx.get(), 0, 0, width, height, tmpColorType,
                                      srcColorSpace, buffer, rowBytes, flags)) {
            return false;
        }
        if (canvas2DFastPath) {
            GrPaint paint;
            paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
            auto fp = fContext->createUPMToPMEffect(
                    GrSimpleTextureEffect::Make(std::move(tempProxy), SkMatrix::I()), true);
            if (srcColorType == GrColorType::kBGRA_8888) {
                fp = GrFragmentProcessor::SwizzleOutput(std::move(fp), GrSwizzle::BGRA());
            }
            if (!fp) {
                return false;
            }
            paint.addColorFragmentProcessor(std::move(fp));
            dst->asRenderTargetContext()->fillRectToRect(
                    GrNoClip(), std::move(paint), GrAA::kNo, SkMatrix::I(),
                    SkRect::MakeXYWH(left, top, width, height), SkRect::MakeWH(width, height));
            return true;
        } else {
            return dst->copy(tempProxy.get(), SkIRect::MakeWH(width, height), {left, top});
        }
    }

    bool convert = premul;

    if (!valid_pixel_conversion(srcColorType, dstProxy->config(), premul)) {
        return false;
    }

    GrColorType allowedColorType = fContext->contextPriv().caps()->supportedWritePixelsColorType(
            dstProxy->config(), srcColorType);
    convert = convert || (srcColorType != allowedColorType);

    if (!dst->colorSpaceInfo().colorSpace()) {
        // "Legacy" mode - no color space conversions.
        srcColorSpace = nullptr;
    }
    convert = convert || !SkColorSpace::Equals(srcColorSpace, dst->colorSpaceInfo().colorSpace());

    std::unique_ptr<char[]> tempBuffer;
    if (convert) {
        auto srcSkColorType = GrColorTypeToSkColorType(srcColorType);
        auto dstSkColorType = GrColorTypeToSkColorType(allowedColorType);
        if (kUnknown_SkColorType == srcSkColorType || kUnknown_SkColorType == dstSkColorType) {
            return false;
        }
        auto srcAlphaType = premul ? kUnpremul_SkAlphaType : kPremul_SkAlphaType;
        SkPixmap src(SkImageInfo::Make(width, height, srcSkColorType, srcAlphaType,
                                       sk_ref_sp(srcColorSpace)),
                     buffer, rowBytes);
        auto tempSrcII = SkImageInfo::Make(width, height, dstSkColorType, kPremul_SkAlphaType,
                                           dst->colorSpaceInfo().refColorSpace());
        auto size = tempSrcII.computeMinByteSize();
        if (!size) {
            return false;
        }
        tempBuffer.reset(new char[size]);
        SkPixmap tempSrc(tempSrcII, tempBuffer.get(), tempSrcII.minRowBytes());
        if (!src.readPixels(tempSrc)) {
            return false;
        }
        srcColorType = allowedColorType;
        buffer = tempSrc.addr();
        rowBytes = tempSrc.rowBytes();
        if (dstProxy->origin() == kBottomLeft_GrSurfaceOrigin) {
            std::unique_ptr<char[]> row(new char[rowBytes]);
            for (int y = 0; y < height / 2; ++y) {
                memcpy(row.get(), tempSrc.addr(0, y), rowBytes);
                memcpy(tempSrc.writable_addr(0, y), tempSrc.addr(0, height - 1 - y), rowBytes);
                memcpy(tempSrc.writable_addr(0, height - 1 - y), row.get(), rowBytes);
            }
            top = dstSurface->height() - top - height;
        }
    } else if (dstProxy->origin() == kBottomLeft_GrSurfaceOrigin) {
        size_t trimRowBytes = GrColorTypeBytesPerPixel(srcColorType) * width;
        tempBuffer.reset(new char[trimRowBytes * height]);
        char* dst = reinterpret_cast<char*>(tempBuffer.get()) + trimRowBytes * (height - 1);
        const char* src = reinterpret_cast<const char*>(buffer);
        for (int i = 0; i < height; ++i, src += rowBytes, dst -= trimRowBytes) {
            memcpy(dst, src, trimRowBytes);
        }
        buffer = tempBuffer.get();
        rowBytes = trimRowBytes;
        top = dstSurface->height() - top - height;
    }

    if (!(kDontFlush_PixelOpsFlag & pixelOpsFlags) && dstSurface->surfacePriv().hasPendingIO()) {
        this->flush(nullptr);  // MDB TODO: tighten this
    }

    return this->getGpu()->writePixels(dstSurface, left, top, width, height, srcColorType, buffer,
                                       rowBytes);
}

bool GrContextPriv::readSurfacePixels(GrSurfaceContext* src, int left, int top, int width,
                                      int height, GrColorType dstColorType,
                                      SkColorSpace* dstColorSpace, void* buffer, size_t rowBytes,
                                      uint32_t pixelOpsFlags) {
    ASSERT_SINGLE_OWNER_PRIV
    RETURN_FALSE_IF_ABANDONED_PRIV
    SkASSERT(src);
    SkASSERT(buffer);
    ASSERT_OWNED_PROXY_PRIV(src->asSurfaceProxy());
    GR_CREATE_TRACE_MARKER_CONTEXT("GrContextPriv", "readSurfacePixels", fContext);

    SkASSERT(!(pixelOpsFlags & kDontFlush_PixelOpsFlag));
    if (pixelOpsFlags & kDontFlush_PixelOpsFlag) {
        return false;
    }

    // MDB TODO: delay this instantiation until later in the method
    if (!src->asSurfaceProxy()->instantiate(this->resourceProvider())) {
        return false;
    }

    GrSurfaceProxy* srcProxy = src->asSurfaceProxy();
    GrSurface* srcSurface = srcProxy->priv().peekSurface();

    if (!GrSurfacePriv::AdjustReadPixelParams(srcSurface->width(), srcSurface->height(),
                                              GrColorTypeBytesPerPixel(dstColorType), &left, &top,
                                              &width, &height, &buffer, &rowBytes)) {
        return false;
    }

    // TODO: Make GrSurfaceContext know its alpha type and pass dst buffer's alpha type.
    bool unpremul = SkToBool(kUnpremul_PixelOpsFlag & pixelOpsFlags);

    if (!valid_pixel_conversion(dstColorType, srcProxy->config(), unpremul)) {
        return false;
    }

    // This is the getImageData equivalent to the canvas2D putImageData fast path. We probably don't
    // care so much about getImageData performance. However, in order to ensure putImageData/
    // getImageData in "legacy" mode are round-trippable we use the GPU to do the complementary
    // unpremul step to writeSurfacePixels's premul step (which is determined empirically in
    // fContext->vaildaPMUPMConversionExists()).
    bool canvas2DFastPath =
            unpremul &&
            !src->colorSpaceInfo().colorSpace() &&
            (GrColorType::kRGBA_8888 == dstColorType || GrColorType::kBGRA_8888 == dstColorType) &&
            SkToBool(srcProxy->asTextureProxy()) &&
            (srcProxy->config() == kRGBA_8888_GrPixelConfig ||
             srcProxy->config() == kBGRA_8888_GrPixelConfig) &&
            fContext->contextPriv().caps()->isConfigRenderable(kRGBA_8888_GrPixelConfig) &&
            fContext->validPMUPMConversionExists();

    if (!fContext->contextPriv().caps()->surfaceSupportsReadPixels(srcSurface) ||
        canvas2DFastPath) {
        GrSurfaceDesc desc;
        desc.fFlags = canvas2DFastPath ? kRenderTarget_GrSurfaceFlag : kNone_GrSurfaceFlags;
        desc.fConfig = canvas2DFastPath ? kRGBA_8888_GrPixelConfig : srcProxy->config();
        desc.fWidth = width;
        desc.fHeight = height;
        desc.fSampleCnt = 1;
        auto tempProxy = this->proxyProvider()->createProxy(
                desc, kTopLeft_GrSurfaceOrigin, SkBackingFit::kApprox, SkBudgeted::kYes);
        if (!tempProxy) {
            return false;
        }
        sk_sp<GrSurfaceContext> tempCtx;
        if (canvas2DFastPath) {
            tempCtx = this->drawingManager()->makeRenderTargetContext(std::move(tempProxy), nullptr,
                                                                      nullptr);
            SkASSERT(tempCtx->asRenderTargetContext());
            tempCtx->asRenderTargetContext()->discard();
        } else {
            tempCtx = this->drawingManager()->makeTextureContext(
                    std::move(tempProxy), src->colorSpaceInfo().refColorSpace());
        }
        if (!tempCtx) {
            return false;
        }
        if (canvas2DFastPath) {
            GrPaint paint;
            paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
            auto fp = fContext->createPMToUPMEffect(
                    GrSimpleTextureEffect::Make(sk_ref_sp(srcProxy->asTextureProxy()),
                                                SkMatrix::I()),
                    true);
            if (dstColorType == GrColorType::kBGRA_8888) {
                fp = GrFragmentProcessor::SwizzleOutput(std::move(fp), GrSwizzle::BGRA());
                dstColorType = GrColorType::kRGBA_8888;
            }
            if (!fp) {
                return false;
            }
            paint.addColorFragmentProcessor(std::move(fp));
            tempCtx->asRenderTargetContext()->fillRectToRect(
                    GrNoClip(), std::move(paint), GrAA::kNo, SkMatrix::I(),
                    SkRect::MakeWH(width, height), SkRect::MakeXYWH(left, top, width, height));
        } else if (!tempCtx->copy(srcProxy, SkIRect::MakeXYWH(left, top, width, height), {0, 0})) {
            return false;
        }
        uint32_t flags = canvas2DFastPath ? 0 : pixelOpsFlags;
        return this->readSurfacePixels(tempCtx.get(), 0, 0, width, height, dstColorType,
                                       dstColorSpace, buffer, rowBytes, flags);
    }

    bool convert = unpremul;

    bool flip = srcProxy->origin() == kBottomLeft_GrSurfaceOrigin;
    if (flip) {
        top = srcSurface->height() - top - height;
    }

    GrColorType allowedColorType = fContext->contextPriv().caps()->supportedReadPixelsColorType(
            srcProxy->config(), dstColorType);
    convert = convert || (dstColorType != allowedColorType);

    if (!src->colorSpaceInfo().colorSpace()) {
        // "Legacy" mode - no color space conversions.
        dstColorSpace = nullptr;
    }
    convert = convert || !SkColorSpace::Equals(dstColorSpace, src->colorSpaceInfo().colorSpace());

    SkAutoPixmapStorage tempPixmap;
    SkPixmap finalPixmap;
    if (convert) {
        SkColorType srcSkColorType = GrColorTypeToSkColorType(allowedColorType);
        SkColorType dstSkColorType = GrColorTypeToSkColorType(dstColorType);
        if (kUnknown_SkColorType == srcSkColorType || kUnknown_SkColorType == dstSkColorType) {
            return false;
        }
        auto tempAT = SkColorTypeIsAlwaysOpaque(srcSkColorType) ? kOpaque_SkAlphaType
                                                                : kPremul_SkAlphaType;
        auto tempII = SkImageInfo::Make(width, height, srcSkColorType, tempAT,
                                        src->colorSpaceInfo().refColorSpace());
        SkASSERT(!unpremul || !SkColorTypeIsAlwaysOpaque(dstSkColorType));
        auto finalAT = SkColorTypeIsAlwaysOpaque(srcSkColorType)
                               ? kOpaque_SkAlphaType
                               : unpremul ? kUnpremul_SkAlphaType : kPremul_SkAlphaType;
        auto finalII =
                SkImageInfo::Make(width, height, dstSkColorType, finalAT, sk_ref_sp(dstColorSpace));
        if (!SkImageInfoValidConversion(finalII, tempII)) {
            return false;
        }
        if (!tempPixmap.tryAlloc(tempII)) {
            return false;
        }
        finalPixmap.reset(finalII, buffer, rowBytes);
        buffer = tempPixmap.writable_addr();
        rowBytes = tempPixmap.rowBytes();
        // Chrome msan bots require this.
        sk_bzero(buffer, tempPixmap.computeByteSize());
    }

    if (srcSurface->surfacePriv().hasPendingWrite()) {
        this->flush(nullptr);  // MDB TODO: tighten this
    }

    if (!fContext->fGpu->readPixels(srcSurface, left, top, width, height, allowedColorType, buffer,
                                    rowBytes)) {
        return false;
    }

    if (flip) {
        size_t trimRowBytes = GrColorTypeBytesPerPixel(allowedColorType) * width;
        std::unique_ptr<char[]> row(new char[trimRowBytes]);
        char* upper = reinterpret_cast<char*>(buffer);
        char* lower = reinterpret_cast<char*>(buffer) + (height - 1) * rowBytes;
        for (int y = 0; y < height / 2; ++y, upper += rowBytes, lower -= rowBytes) {
            memcpy(row.get(), upper, trimRowBytes);
            memcpy(upper, lower, trimRowBytes);
            memcpy(lower, row.get(), trimRowBytes);
        }
    }
    if (convert) {
        if (!tempPixmap.readPixels(finalPixmap)) {
            return false;
        }
    }
    return true;
}

void GrContextPriv::prepareSurfaceForExternalIO(GrSurfaceProxy* proxy) {
    ASSERT_SINGLE_OWNER_PRIV
    RETURN_IF_ABANDONED_PRIV
    SkASSERT(proxy);
    ASSERT_OWNED_PROXY_PRIV(proxy);
    fContext->fDrawingManager->prepareSurfaceForExternalIO(proxy, 0, nullptr);
}

void GrContextPriv::flushSurfaceWrites(GrSurfaceProxy* proxy) {
    ASSERT_SINGLE_OWNER_PRIV
    RETURN_IF_ABANDONED_PRIV
    SkASSERT(proxy);
    ASSERT_OWNED_PROXY_PRIV(proxy);
    if (proxy->priv().hasPendingWrite()) {
        this->flush(proxy);
    }
}

void GrContextPriv::flushSurfaceIO(GrSurfaceProxy* proxy) {
    ASSERT_SINGLE_OWNER_PRIV
    RETURN_IF_ABANDONED_PRIV
    SkASSERT(proxy);
    ASSERT_OWNED_PROXY_PRIV(proxy);
    if (proxy->priv().hasPendingIO()) {
        this->flush(proxy);
    }
}

////////////////////////////////////////////////////////////////////////////////

sk_sp<GrOpMemoryPool> GrContextPriv::refOpMemoryPool() {
    if (!fContext->fOpMemoryPool) {
        // DDL TODO: should the size of the memory pool be decreased in DDL mode? CPU-side memory
        // consumed in DDL mode vs. normal mode for a single skp might be a good metric of wasted
        // memory.
        fContext->fOpMemoryPool = sk_sp<GrOpMemoryPool>(new GrOpMemoryPool(16384, 16384));
    }

    SkASSERT(fContext->fOpMemoryPool);
    return fContext->fOpMemoryPool;
}

GrOpMemoryPool* GrContextPriv::opMemoryPool() {
    return this->refOpMemoryPool().get();
}

sk_sp<GrSurfaceContext> GrContextPriv::makeWrappedSurfaceContext(sk_sp<GrSurfaceProxy> proxy,
                                                                 sk_sp<SkColorSpace> colorSpace,
                                                                 const SkSurfaceProps* props) {
    ASSERT_SINGLE_OWNER_PRIV

    if (proxy->asRenderTargetProxy()) {
        return this->drawingManager()->makeRenderTargetContext(std::move(proxy),
                                                               std::move(colorSpace), props);
    } else {
        SkASSERT(proxy->asTextureProxy());
        SkASSERT(!props);
        return this->drawingManager()->makeTextureContext(std::move(proxy), std::move(colorSpace));
    }
}

sk_sp<GrSurfaceContext> GrContextPriv::makeDeferredSurfaceContext(const GrSurfaceDesc& dstDesc,
                                                                  GrSurfaceOrigin origin,
                                                                  GrMipMapped mipMapped,
                                                                  SkBackingFit fit,
                                                                  SkBudgeted isDstBudgeted,
                                                                  sk_sp<SkColorSpace> colorSpace,
                                                                  const SkSurfaceProps* props) {
    sk_sp<GrTextureProxy> proxy;
    if (GrMipMapped::kNo == mipMapped) {
        proxy = this->proxyProvider()->createProxy(dstDesc, origin, fit, isDstBudgeted);
    } else {
        SkASSERT(SkBackingFit::kExact == fit);
        proxy = this->proxyProvider()->createMipMapProxy(dstDesc, origin, isDstBudgeted);
    }
    if (!proxy) {
        return nullptr;
    }

    sk_sp<GrSurfaceContext> sContext = this->makeWrappedSurfaceContext(std::move(proxy),
                                                                       std::move(colorSpace),
                                                                       props);
    if (sContext && sContext->asRenderTargetContext()) {
        sContext->asRenderTargetContext()->discard();
    }

    return sContext;
}

sk_sp<GrTextureContext> GrContextPriv::makeBackendTextureContext(const GrBackendTexture& tex,
                                                                 GrSurfaceOrigin origin,
                                                                 sk_sp<SkColorSpace> colorSpace) {
    ASSERT_SINGLE_OWNER_PRIV

    sk_sp<GrSurfaceProxy> proxy = this->proxyProvider()->wrapBackendTexture(tex, origin);
    if (!proxy) {
        return nullptr;
    }

    return this->drawingManager()->makeTextureContext(std::move(proxy), std::move(colorSpace));
}

sk_sp<GrRenderTargetContext> GrContextPriv::makeBackendTextureRenderTargetContext(
                                                                   const GrBackendTexture& tex,
                                                                   GrSurfaceOrigin origin,
                                                                   int sampleCnt,
                                                                   sk_sp<SkColorSpace> colorSpace,
                                                                   const SkSurfaceProps* props) {
    ASSERT_SINGLE_OWNER_PRIV
    SkASSERT(sampleCnt > 0);

    sk_sp<GrTextureProxy> proxy(
            this->proxyProvider()->wrapRenderableBackendTexture(tex, origin, sampleCnt));
    if (!proxy) {
        return nullptr;
    }

    return this->drawingManager()->makeRenderTargetContext(std::move(proxy),
                                                           std::move(colorSpace), props);
}

sk_sp<GrRenderTargetContext> GrContextPriv::makeBackendRenderTargetRenderTargetContext(
                                                const GrBackendRenderTarget& backendRT,
                                                GrSurfaceOrigin origin,
                                                sk_sp<SkColorSpace> colorSpace,
                                                const SkSurfaceProps* surfaceProps) {
    ASSERT_SINGLE_OWNER_PRIV

    sk_sp<GrSurfaceProxy> proxy = this->proxyProvider()->wrapBackendRenderTarget(backendRT, origin);
    if (!proxy) {
        return nullptr;
    }

    return this->drawingManager()->makeRenderTargetContext(std::move(proxy),
                                                           std::move(colorSpace),
                                                           surfaceProps);
}

sk_sp<GrRenderTargetContext> GrContextPriv::makeBackendTextureAsRenderTargetRenderTargetContext(
                                                     const GrBackendTexture& tex,
                                                     GrSurfaceOrigin origin,
                                                     int sampleCnt,
                                                     sk_sp<SkColorSpace> colorSpace,
                                                     const SkSurfaceProps* props) {
    ASSERT_SINGLE_OWNER_PRIV
    SkASSERT(sampleCnt > 0);
    sk_sp<GrSurfaceProxy> proxy(
            this->proxyProvider()->wrapBackendTextureAsRenderTarget(tex, origin, sampleCnt));
    if (!proxy) {
        return nullptr;
    }

    return this->drawingManager()->makeRenderTargetContext(std::move(proxy),
                                                           std::move(colorSpace),
                                                           props);
}

void GrContextPriv::addOnFlushCallbackObject(GrOnFlushCallbackObject* onFlushCBObject) {
    fContext->fDrawingManager->addOnFlushCallbackObject(onFlushCBObject);
}

void GrContextPriv::moveOpListsToDDL(SkDeferredDisplayList* ddl) {
    fContext->fDrawingManager->moveOpListsToDDL(ddl);
}

void GrContextPriv::copyOpListsFromDDL(const SkDeferredDisplayList* ddl,
                                       GrRenderTargetProxy* newDest) {
    fContext->fDrawingManager->copyOpListsFromDDL(ddl, newDest);
}

static inline GrPixelConfig GrPixelConfigFallback(GrPixelConfig config) {
    switch (config) {
        case kAlpha_8_GrPixelConfig:
        case kAlpha_8_as_Alpha_GrPixelConfig:
        case kAlpha_8_as_Red_GrPixelConfig:
        case kRGB_565_GrPixelConfig:
        case kRGBA_4444_GrPixelConfig:
        case kBGRA_8888_GrPixelConfig:
        case kRGBA_1010102_GrPixelConfig:
            return kRGBA_8888_GrPixelConfig;
        case kSBGRA_8888_GrPixelConfig:
            return kSRGBA_8888_GrPixelConfig;
        case kAlpha_half_GrPixelConfig:
        case kAlpha_half_as_Red_GrPixelConfig:
            return kRGBA_half_GrPixelConfig;
        case kGray_8_GrPixelConfig:
        case kGray_8_as_Lum_GrPixelConfig:
        case kGray_8_as_Red_GrPixelConfig:
            return kRGB_888_GrPixelConfig;
        default:
            return kUnknown_GrPixelConfig;
    }
}

sk_sp<GrRenderTargetContext> GrContextPriv::makeDeferredRenderTargetContextWithFallback(
                                                                 SkBackingFit fit,
                                                                 int width, int height,
                                                                 GrPixelConfig config,
                                                                 sk_sp<SkColorSpace> colorSpace,
                                                                 int sampleCnt,
                                                                 GrMipMapped mipMapped,
                                                                 GrSurfaceOrigin origin,
                                                                 const SkSurfaceProps* surfaceProps,
                                                                 SkBudgeted budgeted) {
    SkASSERT(sampleCnt > 0);
    if (0 == fContext->contextPriv().caps()->getRenderTargetSampleCount(sampleCnt, config)) {
        config = GrPixelConfigFallback(config);
    }

    return this->makeDeferredRenderTargetContext(fit, width, height, config, std::move(colorSpace),
                                                 sampleCnt, mipMapped, origin, surfaceProps,
                                                 budgeted);
}

sk_sp<GrRenderTargetContext> GrContextPriv::makeDeferredRenderTargetContext(
                                                        SkBackingFit fit,
                                                        int width, int height,
                                                        GrPixelConfig config,
                                                        sk_sp<SkColorSpace> colorSpace,
                                                        int sampleCnt,
                                                        GrMipMapped mipMapped,
                                                        GrSurfaceOrigin origin,
                                                        const SkSurfaceProps* surfaceProps,
                                                        SkBudgeted budgeted) {
    SkASSERT(sampleCnt > 0);
    if (fContext->abandoned()) {
        return nullptr;
    }

    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fWidth = width;
    desc.fHeight = height;
    desc.fConfig = config;
    desc.fSampleCnt = sampleCnt;

    sk_sp<GrTextureProxy> rtp;
    if (GrMipMapped::kNo == mipMapped) {
        rtp = fContext->fProxyProvider->createProxy(desc, origin, fit, budgeted);
    } else {
        rtp = fContext->fProxyProvider->createMipMapProxy(desc, origin, budgeted);
    }
    if (!rtp) {
        return nullptr;
    }

    sk_sp<GrRenderTargetContext> renderTargetContext(
        fContext->fDrawingManager->makeRenderTargetContext(std::move(rtp),
                                                           std::move(colorSpace),
                                                           surfaceProps));
    if (!renderTargetContext) {
        return nullptr;
    }

    renderTargetContext->discard();

    return renderTargetContext;
}

std::unique_ptr<GrFragmentProcessor> GrContext::createPMToUPMEffect(
        std::unique_ptr<GrFragmentProcessor> fp, bool useConfigConversionEffect) {
    ASSERT_SINGLE_OWNER
    // We have specialized effects that guarantee round-trip conversion for some formats
    if (useConfigConversionEffect) {
        // We should have already called this->validPMUPMConversionExists() in this case
        SkASSERT(fDidTestPMConversions);
        // ...and it should have succeeded
        SkASSERT(this->validPMUPMConversionExists());

        return GrConfigConversionEffect::Make(std::move(fp), PMConversion::kToUnpremul);
    } else {
        // For everything else (sRGB, half-float, etc...), it doesn't make sense to try and
        // explicitly round the results. Just do the obvious, naive thing in the shader.
        return GrFragmentProcessor::UnpremulOutput(std::move(fp));
    }
}

std::unique_ptr<GrFragmentProcessor> GrContext::createUPMToPMEffect(
        std::unique_ptr<GrFragmentProcessor> fp, bool useConfigConversionEffect) {
    ASSERT_SINGLE_OWNER
    // We have specialized effects that guarantee round-trip conversion for these formats
    if (useConfigConversionEffect) {
        // We should have already called this->validPMUPMConversionExists() in this case
        SkASSERT(fDidTestPMConversions);
        // ...and it should have succeeded
        SkASSERT(this->validPMUPMConversionExists());

        return GrConfigConversionEffect::Make(std::move(fp), PMConversion::kToPremul);
    } else {
        // For everything else (sRGB, half-float, etc...), it doesn't make sense to try and
        // explicitly round the results. Just do the obvious, naive thing in the shader.
        return GrFragmentProcessor::PremulOutput(std::move(fp));
    }
}

bool GrContext::validPMUPMConversionExists() {
    ASSERT_SINGLE_OWNER
    if (!fDidTestPMConversions) {
        fPMUPMConversionsRoundTrip = GrConfigConversionEffect::TestForPreservingPMConversions(this);
        fDidTestPMConversions = true;
    }

    // The PM<->UPM tests fail or succeed together so we only need to check one.
    return fPMUPMConversionsRoundTrip;
}

bool GrContext::supportsDistanceFieldText() const {
    return fCaps->shaderCaps()->supportsDistanceFieldText();
}

//////////////////////////////////////////////////////////////////////////////

// DDL TODO: remove 'maxResources'
void GrContext::getResourceCacheLimits(int* maxResources, size_t* maxResourceBytes) const {
    ASSERT_SINGLE_OWNER
    if (maxResources) {
        *maxResources = fResourceCache->getMaxResourceCount();
    }
    if (maxResourceBytes) {
        *maxResourceBytes = fResourceCache->getMaxResourceBytes();
    }
}

void GrContext::setResourceCacheLimits(int maxResources, size_t maxResourceBytes) {
    ASSERT_SINGLE_OWNER
    fResourceCache->setLimits(maxResources, maxResourceBytes);
}

//////////////////////////////////////////////////////////////////////////////
void GrContext::dumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump) const {
    ASSERT_SINGLE_OWNER
    fResourceCache->dumpMemoryStatistics(traceMemoryDump);
}

//////////////////////////////////////////////////////////////////////////////

SkString GrContextPriv::dump() const {
    SkDynamicMemoryWStream stream;
    SkJSONWriter writer(&stream, SkJSONWriter::Mode::kPretty);
    writer.beginObject();

    static const char* kBackendStr[] = {
        "Metal",
        "OpenGL",
        "Vulkan",
        "Mock",
    };
    GR_STATIC_ASSERT(0 == kMetal_GrBackend);
    GR_STATIC_ASSERT(1 == kOpenGL_GrBackend);
    GR_STATIC_ASSERT(2 == kVulkan_GrBackend);
    GR_STATIC_ASSERT(3 == kMock_GrBackend);
    writer.appendString("backend", kBackendStr[fContext->fBackend]);

    writer.appendName("caps");
    fContext->fCaps->dumpJSON(&writer);

    writer.appendName("gpu");
    fContext->fGpu->dumpJSON(&writer);

    // Flush JSON to the memory stream
    writer.endObject();
    writer.flush();

    // Null terminate the JSON data in the memory stream
    stream.write8(0);

    // Allocate a string big enough to hold all the data, then copy out of the stream
    SkString result(stream.bytesWritten());
    stream.copyToAndReset(result.writable_str());
    return result;
}
