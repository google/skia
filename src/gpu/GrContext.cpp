/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrBackendSemaphore.h"
#include "GrContext.h"
#include "GrClip.h"
#include "GrContextOptions.h"
#include "GrContextPriv.h"
#include "GrDrawingManager.h"
#include "GrGpu.h"
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
#include "GrTracing.h"
#include "SkConvertPixels.h"
#include "SkGr.h"
#include "SkJSONWriter.h"
#include "SkMakeUnique.h"
#include "SkTaskGroup.h"
#include "SkUnPreMultiplyPriv.h"
#include "effects/GrConfigConversionEffect.h"
#include "text/GrTextBlobCache.h"

#include "gl/GrGLGpu.h"
#include "mock/GrMockGpu.h"
#ifdef SK_METAL
#include "mtl/GrMtlTrampoline.h"
#endif
#include "ddl/GrDDLGpu.h"
#ifdef SK_VULKAN
#include "vk/GrVkGpu.h"
#endif

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

GrContext* GrContext::Create(GrBackend backend, GrBackendContext backendContext) {
    GrContextOptions defaultOptions;
    return Create(backend, backendContext, defaultOptions);
}

GrContext* GrContext::Create(GrBackend backend, GrBackendContext backendContext,
                             const GrContextOptions& options) {

    sk_sp<GrContext> context(new GrContext(backend));

    context->fGpu = GrGpu::Make(backend, backendContext, options, context.get());
    if (!context->fGpu) {
        return nullptr;
    }

    if (!context->init(options)) {
        return nullptr;
    }

    return context.release();
}

sk_sp<GrContext> GrContext::MakeGL(sk_sp<const GrGLInterface> interface) {
    GrContextOptions defaultOptions;
    return MakeGL(std::move(interface), defaultOptions);
}

sk_sp<GrContext> GrContext::MakeGL(sk_sp<const GrGLInterface> interface,
                                   const GrContextOptions& options) {
    sk_sp<GrContext> context(new GrContext(kOpenGL_GrBackend));

    context->fGpu = GrGLGpu::Make(std::move(interface), options, context.get());
    if (!context->fGpu) {
        return nullptr;
    }
    if (!context->init(options)) {
        return nullptr;
    }
    return context;
}

sk_sp<GrContext> GrContext::MakeGL(const GrGLInterface* interface) {
    return MakeGL(sk_ref_sp(interface));
}

sk_sp<GrContext> GrContext::MakeGL(const GrGLInterface* interface,
                                   const GrContextOptions& options) {
    return MakeGL(sk_ref_sp(interface), options);
}

sk_sp<GrContext> GrContext::MakeMock(const GrMockOptions* mockOptions) {
    GrContextOptions defaultOptions;
    return MakeMock(mockOptions, defaultOptions);
}

sk_sp<GrContext> GrContext::MakeMock(const GrMockOptions* mockOptions,
                                     const GrContextOptions& options) {
    sk_sp<GrContext> context(new GrContext(kMock_GrBackend));

    context->fGpu = GrMockGpu::Make(mockOptions, options, context.get());
    if (!context->fGpu) {
        return nullptr;
    }
    if (!context->init(options)) {
        return nullptr;
    }
    return context;
}

#ifdef SK_VULKAN
sk_sp<GrContext> GrContext::MakeVulkan(sk_sp<const GrVkBackendContext> backendContext) {
    GrContextOptions defaultOptions;
    return MakeVulkan(std::move(backendContext), defaultOptions);
}

sk_sp<GrContext> GrContext::MakeVulkan(sk_sp<const GrVkBackendContext> backendContext,
                                       const GrContextOptions& options) {
    sk_sp<GrContext> context(new GrContext(kVulkan_GrBackend));

    context->fGpu = GrVkGpu::Make(std::move(backendContext), options, context.get());
    if (!context->fGpu) {
        return nullptr;
    }
    if (!context->init(options)) {
        return nullptr;
    }
    return context;
}
#endif

#ifdef SK_METAL
sk_sp<GrContext> GrContext::MakeMetal(void* device, void* queue) {
    GrContextOptions defaultOptions;
    return MakeMetal(device, queue, defaultOptions);
}

sk_sp<GrContext> GrContext::MakeMetal(void* device, void* queue, const GrContextOptions& options) {
    sk_sp<GrContext> context(new GrContext(kMetal_GrBackend));

    context->fGpu = GrMtlTrampoline::MakeGpu(context.get(), options, device, queue);
    if (!context->fGpu) {
        return nullptr;
    }
    if (!context->init(options)) {
        return nullptr;
    }
    return context;
}
#endif

static int32_t gNextID = 1;
static int32_t next_id() {
    int32_t id;
    do {
        id = sk_atomic_inc(&gNextID);
    } while (id == SK_InvalidGenID);
    return id;
}

sk_sp<GrContext> GrContextPriv::MakeDDL(GrContextThreadSafeProxy* proxy) {
    sk_sp<GrContext> context(new GrContext(proxy));

    context->fGpu = GrDDLGpu::Make(context.get(), proxy->fCaps);
    if (!context->fGpu) {
        return nullptr;
    }
    if (!context->init(proxy->fOptions)) {
        return nullptr;
    }
    return context;
}

GrContext::GrContext(GrBackend backend)
        : fUniqueID(next_id())
        , fBackend(backend) {
    fResourceCache = nullptr;
    fResourceProvider = nullptr;
    fProxyProvider = nullptr;
    fAtlasGlyphCache = nullptr;
}

GrContext::GrContext(GrContextThreadSafeProxy* proxy)
        : fUniqueID(proxy->fContextUniqueID)
        , fBackend(proxy->fBackend) {
    fResourceCache = nullptr;
    fResourceProvider = nullptr;
    fProxyProvider = nullptr;
    fAtlasGlyphCache = nullptr;
}

bool GrContext::init(const GrContextOptions& options) {
    ASSERT_SINGLE_OWNER
    fCaps = fGpu->refCaps();
    fResourceCache = new GrResourceCache(fCaps.get(), fUniqueID);
    fResourceProvider = new GrResourceProvider(fGpu.get(), fResourceCache, &fSingleOwner);
    fProxyProvider = new GrProxyProvider(fResourceProvider, fResourceCache, fCaps, &fSingleOwner);
    fResourceCache->setProxyProvider(fProxyProvider);

    // DDL TODO: we need to think through how the task group & persistent cache
    // get passed on to/shared between all the DDLRecorders created with this context.
    fThreadSafeProxy.reset(new GrContextThreadSafeProxy(fCaps, this->uniqueID(), fBackend,
                                                        options));

    fDisableGpuYUVConversion = options.fDisableGpuYUVConversion;
    fDidTestPMConversions = false;

    GrPathRendererChain::Options prcOptions;
    prcOptions.fAllowPathMaskCaching = options.fAllowPathMaskCaching;
#if GR_TEST_UTILS
    prcOptions.fGpuPathRenderers = options.fGpuPathRenderers;
#endif
    if (options.fDisableDistanceFieldPaths) {
        prcOptions.fGpuPathRenderers &= ~GpuPathRenderers::kSmall;
    }

    GrAtlasTextContext::Options atlasTextContextOptions;
    atlasTextContextOptions.fMaxDistanceFieldFontSize = options.fGlyphsAsPathsFontSize;
    atlasTextContextOptions.fMinDistanceFieldFontSize = options.fMinDistanceFieldFontSize;
    atlasTextContextOptions.fDistanceFieldVerticesAlwaysHaveW = false;
#if SK_SUPPORT_ATLAS_TEXT
    if (GrContextOptions::Enable::kYes == options.fDistanceFieldGlyphVerticesAlwaysHaveW) {
        atlasTextContextOptions.fDistanceFieldVerticesAlwaysHaveW = true;
    }
#endif

    fDrawingManager.reset(
            new GrDrawingManager(this, prcOptions, atlasTextContextOptions, &fSingleOwner));

    GrDrawOpAtlas::AllowMultitexturing allowMultitexturing;
    switch (options.fAllowMultipleGlyphCacheTextures) {
        case GrContextOptions::Enable::kDefault:
#ifdef SK_BUILD_FOR_IOS
            allowMultitexturing = GrDrawOpAtlas::AllowMultitexturing::kNo;
#else
            allowMultitexturing = GrDrawOpAtlas::AllowMultitexturing::kYes;
#endif
            break;
        case GrContextOptions::Enable::kNo:
            allowMultitexturing = GrDrawOpAtlas::AllowMultitexturing::kNo;
            break;
        case GrContextOptions::Enable::kYes:
            allowMultitexturing = GrDrawOpAtlas::AllowMultitexturing::kYes;
            break;
    }
    fAtlasGlyphCache = new GrAtlasGlyphCache(this, options.fGlyphCacheTextureMaximumBytes,
                                             allowMultitexturing);
    this->contextPriv().addOnFlushCallbackObject(fAtlasGlyphCache);

    fTextBlobCache.reset(new GrTextBlobCache(TextBlobCacheOverBudgetCB, this, this->uniqueID()));

    if (options.fExecutor) {
        fTaskGroup = skstd::make_unique<SkTaskGroup>(*options.fExecutor);
    }

    fPersistentCache = options.fPersistentCache;

    return true;
}

GrContext::~GrContext() {
    ASSERT_SINGLE_OWNER

    if (!fGpu) {
        SkASSERT(!fCaps);
        return;
    }

    this->flush();

    fDrawingManager->cleanup();

    for (int i = 0; i < fCleanUpData.count(); ++i) {
        (*fCleanUpData[i].fFunc)(this, fCleanUpData[i].fInfo);
    }

    delete fResourceProvider;
    delete fResourceCache;
    delete fProxyProvider;
    delete fAtlasGlyphCache;
}

sk_sp<GrContextThreadSafeProxy> GrContext::threadSafeProxy() {
    return fThreadSafeProxy;
}

void GrContext::abandonContext() {
    ASSERT_SINGLE_OWNER

    fProxyProvider->abandon();
    fResourceProvider->abandon();

    // Need to abandon the drawing manager first so all the render targets
    // will be released/forgotten before they too are abandoned.
    fDrawingManager->abandon();

    // abandon first to so destructors
    // don't try to free the resources in the API.
    fResourceCache->abandonAll();

    fGpu->disconnect(GrGpu::DisconnectType::kAbandon);

    fAtlasGlyphCache->freeAll();
    fTextBlobCache->freeAll();
}

void GrContext::releaseResourcesAndAbandonContext() {
    ASSERT_SINGLE_OWNER

    fProxyProvider->abandon();
    fResourceProvider->abandon();

    // Need to abandon the drawing manager first so all the render targets
    // will be released/forgotten before they too are abandoned.
    fDrawingManager->abandon();

    // Release all resources in the backend 3D API.
    fResourceCache->releaseAll();

    fGpu->disconnect(GrGpu::DisconnectType::kCleanup);

    fAtlasGlyphCache->freeAll();
    fTextBlobCache->freeAll();
}

void GrContext::resetContext(uint32_t state) {
    ASSERT_SINGLE_OWNER
    fGpu->markContextDirty(state);
}

void GrContext::freeGpuResources() {
    ASSERT_SINGLE_OWNER

    this->flush();

    fAtlasGlyphCache->freeAll();

    fDrawingManager->freeGpuResources();

    fResourceCache->purgeAllUnlocked();
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

bool sw_convert_to_premul(GrPixelConfig srcConfig, int width, int height, size_t inRowBytes,
                          const void* inPixels, size_t outRowBytes, void* outPixels) {
    SkColorType colorType;
    if (!GrPixelConfigToColorType(srcConfig, &colorType) ||
        4 != SkColorTypeBytesPerPixel(colorType))
    {
        return false;
    }

    for (int y = 0; y < height; y++) {
        SkOpts::RGBA_to_rgbA((uint32_t*) outPixels, inPixels, width);
        outPixels = SkTAddOffset<void>(outPixels, outRowBytes);
        inPixels = SkTAddOffset<const void>(inPixels, inRowBytes);
    }

    return true;
}

static bool valid_premul_config(GrPixelConfig config) {
    return GrPixelConfigIs8888Unorm(config) || kRGBA_half_GrPixelConfig == config;
}

static bool valid_pixel_conversion(GrPixelConfig srcConfig, GrPixelConfig dstConfig,
                                   bool premulConversion) {
    // We don't allow conversion between integer configs and float/fixed configs.
    if (GrPixelConfigIsSint(srcConfig) != GrPixelConfigIsSint(dstConfig)) {
        return false;
    }

    // We only allow premul <-> unpremul conversions for some formats
    if (premulConversion && (!valid_premul_config(srcConfig) || !valid_premul_config(dstConfig))) {
        return false;
    }

    return true;
}

static bool pm_upm_must_round_trip(GrPixelConfig config, SkColorSpace* colorSpace) {
    return !colorSpace &&
           (kRGBA_8888_GrPixelConfig == config || kBGRA_8888_GrPixelConfig == config);
}

bool GrContextPriv::writeSurfacePixels(GrSurfaceContext* dst,
                                       int left, int top, int width, int height,
                                       GrPixelConfig srcConfig, SkColorSpace* srcColorSpace,
                                       const void* buffer, size_t rowBytes,
                                       uint32_t pixelOpsFlags) {
    // TODO: Color space conversion

    ASSERT_SINGLE_OWNER_PRIV
    RETURN_FALSE_IF_ABANDONED_PRIV
    SkASSERT(dst);
    ASSERT_OWNED_PROXY_PRIV(dst->asSurfaceProxy());
    GR_CREATE_TRACE_MARKER_CONTEXT("GrContextPriv", "writeSurfacePixels", fContext);

    if (!dst->asSurfaceProxy()->instantiate(this->resourceProvider())) {
        return false;
    }

    GrSurfaceProxy* dstProxy = dst->asSurfaceProxy();
    GrSurface* dstSurface = dstProxy->priv().peekSurface();

    // The src is unpremul but the dst is premul -> premul the src before or as part of the write
    const bool premul = SkToBool(kUnpremul_PixelOpsFlag & pixelOpsFlags);
    if (!valid_pixel_conversion(srcConfig, dstProxy->config(), premul)) {
        return false;
    }

    // We need to guarantee round-trip conversion if we are reading and writing 8888 non-sRGB data,
    // without any color spaces attached, and the caller wants us to premul.
    bool useConfigConversionEffect =
            premul && pm_upm_must_round_trip(srcConfig, srcColorSpace) &&
            pm_upm_must_round_trip(dstProxy->config(), dst->colorSpaceInfo().colorSpace());

    // Are we going to try to premul as part of a draw? For the non-legacy case, we always allow
    // this. GrConfigConversionEffect fails on some GPUs, so only allow this if it works perfectly.
    bool premulOnGpu = premul &&
                       (!useConfigConversionEffect || fContext->validPMUPMConversionExists());

    // Trim the params here so that if we wind up making a temporary surface it can be as small as
    // necessary and because GrGpu::getWritePixelsInfo requires it.
    if (!GrSurfacePriv::AdjustWritePixelParams(dstSurface->width(), dstSurface->height(),
                                               GrBytesPerPixel(srcConfig), &left, &top, &width,
                                               &height, &buffer, &rowBytes)) {
        return false;
    }

    GrGpu::DrawPreference drawPreference = premulOnGpu ? GrGpu::kCallerPrefersDraw_DrawPreference
                                                       : GrGpu::kNoDraw_DrawPreference;
    GrGpu::WritePixelTempDrawInfo tempDrawInfo;
    if (!fContext->fGpu->getWritePixelsInfo(dstSurface, dstProxy->origin(), width, height,
                                            srcConfig, &drawPreference, &tempDrawInfo)) {
        return false;
    }

    if (!(kDontFlush_PixelOpsFlag & pixelOpsFlags) && dstSurface->surfacePriv().hasPendingIO()) {
        this->flush(nullptr); // MDB TODO: tighten this
    }

    sk_sp<GrTextureProxy> tempProxy;
    if (GrGpu::kNoDraw_DrawPreference != drawPreference) {
        tempProxy = this->proxyProvider()->createProxy(tempDrawInfo.fTempSurfaceDesc,
                                                       SkBackingFit::kApprox,
                                                       SkBudgeted::kYes);
        if (!tempProxy && GrGpu::kRequireDraw_DrawPreference == drawPreference) {
            return false;
        }
    }

    // temp buffer for doing sw premul conversion, if needed.
    SkAutoSTMalloc<128 * 128, uint32_t> tmpPixels(0);
    // We need to do sw premul if we were unable to create a RT for drawing, or if we can't do the
    // premul on the GPU
    if (premul && (!tempProxy || !premulOnGpu)) {
        size_t tmpRowBytes = 4 * width;
        tmpPixels.reset(width * height);
        if (!sw_convert_to_premul(srcConfig, width, height, rowBytes, buffer, tmpRowBytes,
                                  tmpPixels.get())) {
            return false;
        }
        rowBytes = tmpRowBytes;
        buffer = tmpPixels.get();
    }

    if (tempProxy) {
        auto fp = GrSimpleTextureEffect::Make(tempProxy, SkMatrix::I());
        if (premulOnGpu) {
            fp = fContext->createUPMToPMEffect(std::move(fp), useConfigConversionEffect);
        }
        fp = GrFragmentProcessor::SwizzleOutput(std::move(fp), tempDrawInfo.fSwizzle);
        if (!fp) {
            return false;
        }

        if (!tempProxy->instantiate(this->resourceProvider())) {
            return false;
        }
        GrTexture* texture = tempProxy->priv().peekTexture();

        if (tempProxy->priv().hasPendingIO()) {
            this->flush(tempProxy.get());
        }

        if (!fContext->fGpu->writePixels(texture, tempProxy->origin(), 0, 0, width, height,
                                         tempDrawInfo.fWriteConfig, buffer, rowBytes)) {
            return false;
        }
        tempProxy = nullptr;

        SkMatrix matrix;
        matrix.setTranslate(SkIntToScalar(left), SkIntToScalar(top));
        GrRenderTargetContext* renderTargetContext = dst->asRenderTargetContext();
        if (!renderTargetContext) {
            return false;
        }
        GrPaint paint;
        paint.addColorFragmentProcessor(std::move(fp));
        paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
        paint.setAllowSRGBInputs(dst->colorSpaceInfo().isGammaCorrect() ||
                                 GrPixelConfigIsSRGB(dst->colorSpaceInfo().config()));
        SkRect rect = SkRect::MakeWH(SkIntToScalar(width), SkIntToScalar(height));
        renderTargetContext->drawRect(GrNoClip(), std::move(paint), GrAA::kNo, matrix, rect,
                                        nullptr);

        if (kFlushWrites_PixelOp & pixelOpsFlags) {
            this->flushSurfaceWrites(renderTargetContext->asRenderTargetProxy());
        }
    } else {
        return fContext->fGpu->writePixels(dstSurface, dstProxy->origin(), left, top, width,
                                           height, srcConfig, buffer, rowBytes);
    }
    return true;
}

bool GrContextPriv::readSurfacePixels(GrSurfaceContext* src,
                                      int left, int top, int width, int height,
                                      GrPixelConfig dstConfig, SkColorSpace* dstColorSpace,
                                      void* buffer, size_t rowBytes, uint32_t flags) {
    // TODO: Color space conversion

    ASSERT_SINGLE_OWNER_PRIV
    RETURN_FALSE_IF_ABANDONED_PRIV
    SkASSERT(src);
    ASSERT_OWNED_PROXY_PRIV(src->asSurfaceProxy());
    GR_CREATE_TRACE_MARKER_CONTEXT("GrContextPriv", "readSurfacePixels", fContext);

    // MDB TODO: delay this instantiation until later in the method
    if (!src->asSurfaceProxy()->instantiate(this->resourceProvider())) {
        return false;
    }

    GrSurfaceProxy* srcProxy = src->asSurfaceProxy();
    GrSurface* srcSurface = srcProxy->priv().peekSurface();

    // The src is premul but the dst is unpremul -> unpremul the src after or as part of the read
    bool unpremul = SkToBool(kUnpremul_PixelOpsFlag & flags);
    if (!valid_pixel_conversion(srcProxy->config(), dstConfig, unpremul)) {
        return false;
    }

    // We need to guarantee round-trip conversion if we are reading and writing 8888 non-sRGB data,
    // without any color spaces attached, and the caller wants us to unpremul.
    bool useConfigConversionEffect =
            unpremul &&
            pm_upm_must_round_trip(srcProxy->config(), src->colorSpaceInfo().colorSpace()) &&
            pm_upm_must_round_trip(dstConfig, dstColorSpace);

    // Are we going to try to unpremul as part of a draw? For the non-legacy case, we always allow
    // this. GrConfigConversionEffect fails on some GPUs, so only allow this if it works perfectly.
    bool unpremulOnGpu = unpremul &&
                         (!useConfigConversionEffect || fContext->validPMUPMConversionExists());

    // Adjust the params so that if we wind up using an intermediate surface we've already done
    // all the trimming and the temporary can be the min size required.
    if (!GrSurfacePriv::AdjustReadPixelParams(srcSurface->width(), srcSurface->height(),
                                              GrBytesPerPixel(dstConfig), &left,
                                              &top, &width, &height, &buffer, &rowBytes)) {
        return false;
    }

    GrGpu::DrawPreference drawPreference = unpremulOnGpu ? GrGpu::kCallerPrefersDraw_DrawPreference
                                                         : GrGpu::kNoDraw_DrawPreference;
    GrGpu::ReadPixelTempDrawInfo tempDrawInfo;
    if (!fContext->fGpu->getReadPixelsInfo(srcSurface, srcProxy->origin(), width, height, rowBytes,
                                           dstConfig, &drawPreference, &tempDrawInfo)) {
        return false;
    }

    if (!(kDontFlush_PixelOpsFlag & flags) && srcSurface->surfacePriv().hasPendingWrite()) {
        this->flush(nullptr); // MDB TODO: tighten this
    }

    sk_sp<GrSurfaceProxy> proxyToRead = src->asSurfaceProxyRef();
    bool didTempDraw = false;
    if (GrGpu::kNoDraw_DrawPreference != drawPreference) {
        if (SkBackingFit::kExact == tempDrawInfo.fTempSurfaceFit) {
            // We only respect this when the entire src is being read. Otherwise we can trigger too
            // many odd ball texture sizes and trash the cache.
            if (width != srcSurface->width() || height != srcSurface->height()) {
                tempDrawInfo.fTempSurfaceFit= SkBackingFit::kApprox;
            }
        }
        // TODO: Need to decide the semantics of this function for color spaces. Do we support
        // conversion to a passed-in color space? For now, specifying nullptr means that this
        // path will do no conversion, so it will match the behavior of the non-draw path.
        sk_sp<GrRenderTargetContext> tempRTC = fContext->makeDeferredRenderTargetContext(
                                                           tempDrawInfo.fTempSurfaceFit,
                                                           tempDrawInfo.fTempSurfaceDesc.fWidth,
                                                           tempDrawInfo.fTempSurfaceDesc.fHeight,
                                                           tempDrawInfo.fTempSurfaceDesc.fConfig,
                                                           nullptr,
                                                           tempDrawInfo.fTempSurfaceDesc.fSampleCnt,
                                                           GrMipMapped::kNo,
                                                           tempDrawInfo.fTempSurfaceDesc.fOrigin);
        if (tempRTC) {
            SkMatrix textureMatrix = SkMatrix::MakeTrans(SkIntToScalar(left), SkIntToScalar(top));
            sk_sp<GrTextureProxy> proxy = src->asTextureProxyRef();
            auto fp = GrSimpleTextureEffect::Make(std::move(proxy), textureMatrix);
            if (unpremulOnGpu) {
                fp = fContext->createPMToUPMEffect(std::move(fp), useConfigConversionEffect);
                // We no longer need to do this on CPU after the read back.
                unpremul = false;
            }
            fp = GrFragmentProcessor::SwizzleOutput(std::move(fp), tempDrawInfo.fSwizzle);
            if (!fp) {
                return false;
            }

            GrPaint paint;
            paint.addColorFragmentProcessor(std::move(fp));
            paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
            paint.setAllowSRGBInputs(true);
            SkRect rect = SkRect::MakeWH(SkIntToScalar(width), SkIntToScalar(height));
            tempRTC->drawRect(GrNoClip(), std::move(paint), GrAA::kNo, SkMatrix::I(), rect,
                                nullptr);
            proxyToRead = tempRTC->asTextureProxyRef();
            left = 0;
            top = 0;
            didTempDraw = true;
        }
    }

    if (!proxyToRead) {
        return false;
    }

    if (GrGpu::kRequireDraw_DrawPreference == drawPreference && !didTempDraw) {
        return false;
    }
    GrPixelConfig configToRead = dstConfig;
    if (didTempDraw) {
        this->flushSurfaceWrites(proxyToRead.get());
        configToRead = tempDrawInfo.fReadConfig;
    }

    if (!proxyToRead->instantiate(this->resourceProvider())) {
        return false;
    }

    GrSurface* surfaceToRead = proxyToRead->priv().peekSurface();

    if (!fContext->fGpu->readPixels(surfaceToRead, proxyToRead->origin(),
                                    left, top, width, height, configToRead, buffer, rowBytes)) {
        return false;
    }

    // Perform umpremul conversion if we weren't able to perform it as a draw.
    if (unpremul) {
        SkColorType colorType;
        if (!GrPixelConfigToColorType(dstConfig, &colorType) ||
            4 != SkColorTypeBytesPerPixel(colorType))
        {
            return false;
        }

        for (int y = 0; y < height; y++) {
            SkUnpremultiplyRow<false>((uint32_t*) buffer, (const uint32_t*) buffer, width);
            buffer = SkTAddOffset<void>(buffer, rowBytes);
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
int GrContext::getRecommendedSampleCount(GrPixelConfig config,
                                         SkScalar dpi) const {
    ASSERT_SINGLE_OWNER

    if (!this->caps()->isConfigRenderable(config, true)) {
        return 0;
    }
    int chosenSampleCount = 0;
    if (fGpu->caps()->shaderCaps()->pathRenderingSupport()) {
        if (dpi >= 250.0f) {
            chosenSampleCount = 4;
        } else {
            chosenSampleCount = 16;
        }
    }
    int supportedSampleCount = fGpu->caps()->getSampleCount(chosenSampleCount, config);
    return chosenSampleCount <= supportedSampleCount ? supportedSampleCount : 0;
}

sk_sp<GrSurfaceContext> GrContextPriv::makeWrappedSurfaceContext(sk_sp<GrSurfaceProxy> proxy,
                                                                 sk_sp<SkColorSpace> colorSpace) {
    ASSERT_SINGLE_OWNER_PRIV

    if (proxy->asRenderTargetProxy()) {
        return this->drawingManager()->makeRenderTargetContext(std::move(proxy),
                                                               std::move(colorSpace), nullptr);
    } else {
        SkASSERT(proxy->asTextureProxy());
        return this->drawingManager()->makeTextureContext(std::move(proxy), std::move(colorSpace));
    }
}

sk_sp<GrSurfaceContext> GrContextPriv::makeDeferredSurfaceContext(const GrSurfaceDesc& dstDesc,
                                                                  GrMipMapped mipMapped,
                                                                  SkBackingFit fit,
                                                                  SkBudgeted isDstBudgeted) {

    sk_sp<GrTextureProxy> proxy;
    if (GrMipMapped::kNo == mipMapped) {
        proxy = this->proxyProvider()->createProxy(dstDesc, fit, isDstBudgeted);
    } else {
        SkASSERT(SkBackingFit::kExact == fit);
        proxy = this->proxyProvider()->createMipMapProxy(dstDesc, isDstBudgeted);
    }
    if (!proxy) {
        return nullptr;
    }

    return this->makeWrappedSurfaceContext(std::move(proxy), nullptr);
}

sk_sp<GrTextureContext> GrContextPriv::makeBackendTextureContext(const GrBackendTexture& tex,
                                                                 GrSurfaceOrigin origin,
                                                                 sk_sp<SkColorSpace> colorSpace) {
    ASSERT_SINGLE_OWNER_PRIV

    sk_sp<GrSurfaceProxy> proxy = this->proxyProvider()->createWrappedTextureProxy(tex, origin);
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

    sk_sp<GrTextureProxy> proxy(this->proxyProvider()->createWrappedTextureProxy(tex, origin,
                                                                                 sampleCnt));
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

    sk_sp<GrSurfaceProxy> proxy = this->proxyProvider()->createWrappedRenderTargetProxy(backendRT,
                                                                                        origin);
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

    sk_sp<GrSurfaceProxy> proxy(this->proxyProvider()->createWrappedRenderTargetProxy(tex, origin,
                                                                                      sampleCnt));
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


static inline GrPixelConfig GrPixelConfigFallback(GrPixelConfig config) {
    switch (config) {
        case kAlpha_8_GrPixelConfig:
        case kRGB_565_GrPixelConfig:
        case kRGBA_4444_GrPixelConfig:
        case kBGRA_8888_GrPixelConfig:
            return kRGBA_8888_GrPixelConfig;
        case kSBGRA_8888_GrPixelConfig:
            return kSRGBA_8888_GrPixelConfig;
        case kAlpha_half_GrPixelConfig:
            return kRGBA_half_GrPixelConfig;
        default:
            return kUnknown_GrPixelConfig;
    }
}

sk_sp<GrRenderTargetContext> GrContext::makeDeferredRenderTargetContextWithFallback(
                                                                 SkBackingFit fit,
                                                                 int width, int height,
                                                                 GrPixelConfig config,
                                                                 sk_sp<SkColorSpace> colorSpace,
                                                                 int sampleCnt,
                                                                 GrMipMapped mipMapped,
                                                                 GrSurfaceOrigin origin,
                                                                 const SkSurfaceProps* surfaceProps,
                                                                 SkBudgeted budgeted) {
    if (!this->caps()->isConfigRenderable(config, sampleCnt > 0)) {
        config = GrPixelConfigFallback(config);
    }

    return this->makeDeferredRenderTargetContext(fit, width, height, config, std::move(colorSpace),
                                                 sampleCnt, mipMapped, origin, surfaceProps,
                                                 budgeted);
}

sk_sp<GrRenderTargetContext> GrContext::makeDeferredRenderTargetContext(
                                                        SkBackingFit fit,
                                                        int width, int height,
                                                        GrPixelConfig config,
                                                        sk_sp<SkColorSpace> colorSpace,
                                                        int sampleCnt,
                                                        GrMipMapped mipMapped,
                                                        GrSurfaceOrigin origin,
                                                        const SkSurfaceProps* surfaceProps,
                                                        SkBudgeted budgeted) {
    if (this->abandoned()) {
        return nullptr;
    }

    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fOrigin = origin;
    desc.fWidth = width;
    desc.fHeight = height;
    desc.fConfig = config;
    desc.fSampleCnt = sampleCnt;

    sk_sp<GrTextureProxy> rtp;
    if (GrMipMapped::kNo == mipMapped) {
        rtp = fProxyProvider->createProxy(desc, fit, budgeted);
    } else {
        rtp = fProxyProvider->createMipMapProxy(desc, budgeted);
    }
    if (!rtp) {
        return nullptr;
    }

    sk_sp<GrRenderTargetContext> renderTargetContext(
        fDrawingManager->makeRenderTargetContext(std::move(rtp),
                                                 std::move(colorSpace),
                                                 surfaceProps));
    if (!renderTargetContext) {
        return nullptr;
    }

    renderTargetContext->discard();

    return renderTargetContext;
}

bool GrContext::abandoned() const {
    ASSERT_SINGLE_OWNER
    return fDrawingManager->wasAbandoned();
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

//////////////////////////////////////////////////////////////////////////////

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

SkString GrContext::dump() const {
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
    writer.appendString("backend", kBackendStr[fBackend]);

    writer.appendName("caps");
    fCaps->dumpJSON(&writer);

    writer.appendName("gpu");
    fGpu->dumpJSON(&writer);

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
