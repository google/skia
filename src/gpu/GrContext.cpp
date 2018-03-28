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
#include "SkDeferredDisplayList.h"
#include "SkGr.h"
#include "SkImageInfoPriv.h"
#include "SkJSONWriter.h"
#include "SkMakeUnique.h"
#include "SkTaskGroup.h"
#include "SkUnPreMultiplyPriv.h"
#include "effects/GrConfigConversionEffect.h"
#include "gl/GrGLGpu.h"
#include "mock/GrMockGpu.h"
#include "text/GrTextBlobCache.h"
#ifdef SK_METAL
#include "mtl/GrMtlTrampoline.h"
#endif
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

class SK_API GrDirectContext : public GrContext {
public:
    GrDirectContext(GrBackend backend)
            : INHERITED(backend)
            , fFullAtlasManager(nullptr) {
    }

    ~GrDirectContext() override {
        // this if-test protects against the case where the context is being destroyed
        // before having been fully created
        if (this->contextPriv().getGpu()) {
            this->flush();
        }

        delete fFullAtlasManager;
    }

    void abandonContext() override {
        INHERITED::abandonContext();
        fFullAtlasManager->freeAll();
    }

    void releaseResourcesAndAbandonContext() override {
        INHERITED::releaseResourcesAndAbandonContext();
        fFullAtlasManager->freeAll();
    }

    void freeGpuResources() override {
        this->flush();
        fFullAtlasManager->freeAll();

        INHERITED::freeGpuResources();
    }

protected:
    bool init(const GrContextOptions& options) override {
        SkASSERT(fCaps);  // should've been set in ctor
        SkASSERT(!fThreadSafeProxy);

        fThreadSafeProxy.reset(new GrContextThreadSafeProxy(fCaps, this->uniqueID(),
                                                            fBackend, options));

        if (!INHERITED::initCommon(options)) {
            return false;
        }

        GrDrawOpAtlas::AllowMultitexturing allowMultitexturing;
        if (GrContextOptions::Enable::kNo == options.fAllowMultipleGlyphCacheTextures ||
            // multitexturing supported only if range can represent the index + texcoords fully
            !(fCaps->shaderCaps()->floatIs32Bits() || fCaps->shaderCaps()->integerSupport())) {
            allowMultitexturing = GrDrawOpAtlas::AllowMultitexturing::kNo;
        } else {
            allowMultitexturing = GrDrawOpAtlas::AllowMultitexturing::kYes;
        }

        GrGlyphCache* glyphCache = this->contextPriv().getGlyphCache();
        GrProxyProvider* proxyProvider = this->contextPriv().proxyProvider();

        fFullAtlasManager = new GrAtlasManager(proxyProvider, glyphCache,
                                               options.fGlyphCacheTextureMaximumBytes,
                                               allowMultitexturing);
        this->contextPriv().addOnFlushCallbackObject(fFullAtlasManager);

        glyphCache->setGlyphSizeLimit(fFullAtlasManager->getGlyphSizeLimit());
        return true;
    }

    GrRestrictedAtlasManager* onGetRestrictedAtlasManager() override { return fFullAtlasManager; }
    GrAtlasManager* onGetFullAtlasManager() override { return fFullAtlasManager; }

private:
    GrAtlasManager* fFullAtlasManager;

    typedef GrContext INHERITED;
};

/**
 * The DDL Context is the one in effect during DDL Recording. It isn't backed by a GrGPU and
 * cannot allocate any GPU resources.
 */
class SK_API GrDDLContext : public GrContext {
public:
    GrDDLContext(sk_sp<GrContextThreadSafeProxy> proxy)
            : INHERITED(proxy->fBackend, proxy->fContextUniqueID)
            , fRestrictedAtlasManager(nullptr) {
        fCaps = proxy->fCaps;
        fThreadSafeProxy = std::move(proxy);
    }

    ~GrDDLContext() override {
        // The GrDDLContext doesn't actually own the fRestrictedAtlasManager so don't delete it
    }

    void abandonContext() override {
        SkASSERT(0); // abandoning in a DDL Recorder doesn't make a whole lot of sense
        INHERITED::abandonContext();
    }

    void releaseResourcesAndAbandonContext() override {
        SkASSERT(0); // abandoning in a DDL Recorder doesn't make a whole lot of sense
        INHERITED::releaseResourcesAndAbandonContext();
    }

    void freeGpuResources() override {
        SkASSERT(0); // freeing resources in a DDL Recorder doesn't make a whole lot of sense
        INHERITED::freeGpuResources();
    }

protected:
    bool init(const GrContextOptions& options) override {
        SkASSERT(fCaps);  // should've been set in ctor
        SkASSERT(fThreadSafeProxy); // should've been set in the ctor

        if (!INHERITED::initCommon(options)) {
            return false;
        }

        // DDL TODO: in DDL-mode grab a GrRestrictedAtlasManager from the thread-proxy and
        // do not add an onFlushCB
        return true;
    }

    GrRestrictedAtlasManager* onGetRestrictedAtlasManager() override {
        return fRestrictedAtlasManager;
    }

    GrAtlasManager* onGetFullAtlasManager() override {
        SkASSERT(0);   // the DDL Recorders should never invoke this
        return nullptr;
    }

private:
    GrRestrictedAtlasManager* fRestrictedAtlasManager;

    typedef GrContext INHERITED;
};

GrContext* GrContext::Create(GrBackend backend, GrBackendContext backendContext) {
    GrContextOptions defaultOptions;
    return Create(backend, backendContext, defaultOptions);
}

GrContext* GrContext::Create(GrBackend backend, GrBackendContext backendContext,
                             const GrContextOptions& options) {

    sk_sp<GrContext> context(new GrDirectContext(backend));

    context->fGpu = GrGpu::Make(backend, backendContext, options, context.get());
    if (!context->fGpu) {
        return nullptr;
    }

    context->fCaps = context->fGpu->refCaps();
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
    sk_sp<GrContext> context(new GrDirectContext(kOpenGL_GrBackend));

    context->fGpu = GrGLGpu::Make(std::move(interface), options, context.get());
    if (!context->fGpu) {
        return nullptr;
    }

    context->fCaps = context->fGpu->refCaps();
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
    sk_sp<GrContext> context(new GrDirectContext(kMock_GrBackend));

    context->fGpu = GrMockGpu::Make(mockOptions, options, context.get());
    if (!context->fGpu) {
        return nullptr;
    }

    context->fCaps = context->fGpu->refCaps();
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
    sk_sp<GrContext> context(new GrDirectContext(kVulkan_GrBackend));

    context->fGpu = GrVkGpu::Make(std::move(backendContext), options, context.get());
    if (!context->fGpu) {
        return nullptr;
    }

    context->fCaps = context->fGpu->refCaps();
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
    sk_sp<GrContext> context(new GrDirectContext(kMetal_GrBackend));

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

sk_sp<GrContext> GrContextPriv::MakeDDL(sk_sp<GrContextThreadSafeProxy> proxy) {
    sk_sp<GrContext> context(new GrDDLContext(proxy));

    // Note: we aren't creating a Gpu here. This causes the resource provider & cache to
    // also not be created
    if (!context->init(proxy->fOptions)) {
        return nullptr;
    }
    return context;
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
    }

    fProxyProvider = new GrProxyProvider(fResourceProvider, fResourceCache, fCaps, &fSingleOwner);

    if (fResourceCache) {
        fResourceCache->setProxyProvider(fProxyProvider);
    }

    fDisableGpuYUVConversion = options.fDisableGpuYUVConversion;
    fSharpenMipmappedTextures = options.fSharpenMipmappedTextures;
    fDidTestPMConversions = false;

    GrPathRendererChain::Options prcOptions;
    prcOptions.fAllowPathMaskCaching = options.fAllowPathMaskCaching;
#if GR_TEST_UTILS
    prcOptions.fGpuPathRenderers = options.fGpuPathRenderers;
#endif
    if (options.fDisableDistanceFieldPaths) {
        prcOptions.fGpuPathRenderers &= ~GpuPathRenderers::kSmall;
    }

    if (!fResourceCache) {
        // DDL TODO: remove this crippling of the path renderer chain
        // Disable the small path renderer bc of the proxies in the atlas. They need to be
        // unified when the opLists are added back to the destination drawing manager.
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

    fDrawingManager.reset(new GrDrawingManager(this, prcOptions, atlasTextContextOptions,
                                               &fSingleOwner, options.fSortRenderTargets));

    fGlyphCache = new GrGlyphCache;

    fTextBlobCache.reset(new GrTextBlobCache(TextBlobCacheOverBudgetCB,
                                             this, this->uniqueID(), SkToBool(fGpu)));

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

    for (int i = 0; i < fCleanUpData.count(); ++i) {
        (*fCleanUpData[i].fFunc)(this, fCleanUpData[i].fInfo);
    }

    delete fResourceProvider;
    delete fResourceCache;
    delete fProxyProvider;
    delete fGlyphCache;
}

sk_sp<GrContextThreadSafeProxy> GrContext::threadSafeProxy() {
    return fThreadSafeProxy;
}

SkSurfaceCharacterization GrContextThreadSafeProxy::createCharacterization(
                                     size_t cacheMaxResourceBytes,
                                     const SkImageInfo& ii, const GrBackendFormat& backendFormat,
                                     int sampleCnt, GrSurfaceOrigin origin,
                                     const SkSurfaceProps& surfaceProps,
                                     bool isMipMapped) {
    if (!backendFormat.isValid()) {
        return SkSurfaceCharacterization(); // return an invalid characterization
    }

    // We're assuming GrFSAAType::kMixedSamples will never be specified via this code path
    GrFSAAType FSAAType = sampleCnt > 1 ? GrFSAAType::kUnifiedMSAA : GrFSAAType::kNone;

    if (!fCaps->mipMapSupport()) {
        isMipMapped = false;
    }

    GrPixelConfig config = kUnknown_GrPixelConfig;
    if (!fCaps->getConfigFromBackendFormat(backendFormat, ii.colorType(), &config)) {
        return SkSurfaceCharacterization(); // return an invalid characterization
    }

    // This surface characterization factory assumes that the resulting characterization is
    // textureable.
    if (!fCaps->isConfigTexturable(config)) {
        return SkSurfaceCharacterization(); // return an invalid characterization
    }

    return SkSurfaceCharacterization(sk_ref_sp<GrContextThreadSafeProxy>(this),
                                     cacheMaxResourceBytes,
                                     origin, ii.width(), ii.height(), config, FSAAType, sampleCnt,
                                     SkSurfaceCharacterization::Textureable(true),
                                     SkSurfaceCharacterization::MipMapped(isMipMapped),
                                     ii.refColorSpace(), surfaceProps);
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

    fGlyphCache->freeAll();
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

bool GrContext::colorTypeSupportedAsImage(SkColorType colorType) const {
    GrPixelConfig config = SkImageInfo2GrPixelConfig(colorType, nullptr, *this->caps());
    return this->caps()->isConfigTexturable(config);
}

int GrContext::maxSurfaceSampleCountForColorType(SkColorType colorType) const {
    GrPixelConfig config = SkImageInfo2GrPixelConfig(colorType, nullptr, *this->caps());
    return this->caps()->maxRenderTargetSampleCount(config);
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

bool sw_convert_to_premul(GrColorType srcColorType, int width, int height, size_t inRowBytes,
                          const void* inPixels, size_t outRowBytes, void* outPixels) {
    SkColorType colorType = GrColorTypeToSkColorType(srcColorType);
    if (kUnknown_SkColorType == colorType || 4 != SkColorTypeBytesPerPixel(colorType)) {
        return false;
    }

    for (int y = 0; y < height; y++) {
        SkOpts::RGBA_to_rgbA((uint32_t*) outPixels, inPixels, width);
        outPixels = SkTAddOffset<void>(outPixels, outRowBytes);
        inPixels = SkTAddOffset<const void>(inPixels, inRowBytes);
    }

    return true;
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

static bool pm_upm_must_round_trip(GrColorType cpuColorType, const SkColorSpace* cpuColorSpace) {
    return !cpuColorSpace &&
           (GrColorType::kRGBA_8888 == cpuColorType || GrColorType::kBGRA_8888 == cpuColorType);
}

// TODO: This will be removed when GrSurfaceContexts are aware of their color types.
// (skbug.com/6718)
static bool pm_upm_must_round_trip(GrPixelConfig surfaceConfig,
                                   const SkColorSpace* surfaceColorSpace) {
    return !surfaceColorSpace &&
           (kRGBA_8888_GrPixelConfig == surfaceConfig || kBGRA_8888_GrPixelConfig == surfaceConfig);
}

static GrSRGBConversion determine_write_pixels_srgb_conversion(GrColorType srcColorType,
                                                               const SkColorSpace* srcColorSpace,
                                                               GrSRGBEncoded dstSRGBEncoded,
                                                               const SkColorSpace* dstColorSpace,
                                                               const GrCaps& caps) {
    // No support for sRGB-encoded alpha.
    if (GrColorTypeIsAlphaOnly(srcColorType)) {
        return GrSRGBConversion::kNone;
    }
    // No conversions without GPU support for sRGB. (Legacy mode)
    if (!caps.srgbSupport()) {
        return GrSRGBConversion::kNone;
    }
    // If the GrSurfaceContext has no color space then it is in legacy mode.
    if (!dstColorSpace) {
        return GrSRGBConversion::kNone;
    }

    bool srcColorSpaceIsSRGB = srcColorSpace && srcColorSpace->gammaCloseToSRGB();
    bool dstColorSpaceIsSRGB = dstColorSpace->gammaCloseToSRGB();

    // For now we are assuming that if color space of the dst does not have sRGB gamma then the
    // texture format is not sRGB encoded and vice versa. Note that we already checked for "legacy"
    // mode being forced on by caps above. This may change in the future. We will then have to
    // perform shader based conversions.
    SkASSERT(dstColorSpaceIsSRGB == (GrSRGBEncoded::kYes == dstSRGBEncoded));

    if (srcColorSpaceIsSRGB == dstColorSpaceIsSRGB) {
        return GrSRGBConversion::kNone;
    }
    return srcColorSpaceIsSRGB ? GrSRGBConversion::kSRGBToLinear : GrSRGBConversion::kLinearToSRGB;
}

static GrSRGBConversion determine_read_pixels_srgb_conversion(GrSRGBEncoded srcSRGBEncoded,
                                                              const SkColorSpace* srcColorSpace,
                                                              GrColorType dstColorType,
                                                              const SkColorSpace* dstColorSpace,
                                                              const GrCaps& caps) {
    // This is symmetrical with the write version.
    switch (determine_write_pixels_srgb_conversion(dstColorType, dstColorSpace, srcSRGBEncoded,
                                                   srcColorSpace, caps)) {
        case GrSRGBConversion::kNone:         return GrSRGBConversion::kNone;
        case GrSRGBConversion::kLinearToSRGB: return GrSRGBConversion::kSRGBToLinear;
        case GrSRGBConversion::kSRGBToLinear: return GrSRGBConversion::kLinearToSRGB;
    }
    return GrSRGBConversion::kNone;
}

bool GrContextPriv::writeSurfacePixels(GrSurfaceContext* dst, int left, int top, int width,
                                       int height, GrColorType srcColorType,
                                       SkColorSpace* srcColorSpace, const void* buffer,
                                       size_t rowBytes, uint32_t pixelOpsFlags) {
#ifndef SK_LEGACY_GPU_PIXEL_OPS
    return this->writeSurfacePixels2(dst, left, top, width, height, srcColorType, srcColorSpace,
                                     buffer, rowBytes, pixelOpsFlags);
#endif

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

    if (!valid_pixel_conversion(srcColorType, dstProxy->config(), premul)) {
        return false;
    }

    // We need to guarantee round-trip conversion if we are reading and writing 8888 non-sRGB data,
    // without any color spaces attached, and the caller wants us to premul.
    bool useConfigConversionEffect =
            premul && pm_upm_must_round_trip(srcColorType, srcColorSpace) &&
            pm_upm_must_round_trip(dstProxy->config(), dst->colorSpaceInfo().colorSpace());

    // Are we going to try to premul as part of a draw? For the non-legacy case, we always allow
    // this. GrConfigConversionEffect fails on some GPUs, so only allow this if it works perfectly.
    bool premulOnGpu = premul &&
                       (!useConfigConversionEffect || fContext->validPMUPMConversionExists());

    // Trim the params here so that if we wind up making a temporary surface it can be as small as
    // necessary and because GrGpu::getWritePixelsInfo requires it.
    if (!GrSurfacePriv::AdjustWritePixelParams(dstSurface->width(), dstSurface->height(),
                                               GrColorTypeBytesPerPixel(srcColorType), &left, &top,
                                               &width, &height, &buffer, &rowBytes)) {
        return false;
    }

    GrGpu::DrawPreference drawPreference = premulOnGpu ? GrGpu::kCallerPrefersDraw_DrawPreference
                                                       : GrGpu::kNoDraw_DrawPreference;
    GrGpu::WritePixelTempDrawInfo tempDrawInfo;
    GrSRGBConversion srgbConversion = determine_write_pixels_srgb_conversion(
            srcColorType, srcColorSpace, GrPixelConfigIsSRGBEncoded(dstProxy->config()),
            dst->colorSpaceInfo().colorSpace(), *fContext->caps());
    if (!fContext->fGpu->getWritePixelsInfo(dstSurface, dstProxy->origin(), width, height,
                                            srcColorType, srgbConversion, &drawPreference,
                                            &tempDrawInfo)) {
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
        if (!sw_convert_to_premul(srcColorType, width, height, rowBytes, buffer, tmpRowBytes,
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
                                         tempDrawInfo.fWriteColorType, buffer, rowBytes)) {
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
        return fContext->fGpu->writePixels(dstSurface, dstProxy->origin(), left, top, width, height,
                                           srcColorType, buffer, rowBytes);
    }
    return true;
}

bool GrContextPriv::readSurfacePixels(GrSurfaceContext* src, int left, int top, int width,
                                      int height, GrColorType dstColorType,
                                      SkColorSpace* dstColorSpace, void* buffer, size_t rowBytes,
                                      uint32_t flags) {
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

    if (!valid_pixel_conversion(dstColorType, srcProxy->config(), unpremul)) {
        return false;
    }

    // We need to guarantee round-trip conversion if we are reading and writing 8888 non-sRGB data,
    // without any color spaces attached, and the caller wants us to unpremul.
    bool useConfigConversionEffect =
            unpremul &&
            pm_upm_must_round_trip(srcProxy->config(), src->colorSpaceInfo().colorSpace()) &&
            pm_upm_must_round_trip(dstColorType, dstColorSpace);

    // Are we going to try to unpremul as part of a draw? For the non-legacy case, we always allow
    // this. GrConfigConversionEffect fails on some GPUs, so only allow this if it works perfectly.
    bool unpremulOnGpu = unpremul &&
                         (!useConfigConversionEffect || fContext->validPMUPMConversionExists());

    // Adjust the params so that if we wind up using an intermediate surface we've already done
    // all the trimming and the temporary can be the min size required.
    if (!GrSurfacePriv::AdjustReadPixelParams(srcSurface->width(), srcSurface->height(),
                                              GrColorTypeBytesPerPixel(dstColorType), &left, &top,
                                              &width, &height, &buffer, &rowBytes)) {
        return false;
    }

    GrGpu::DrawPreference drawPreference = unpremulOnGpu ? GrGpu::kCallerPrefersDraw_DrawPreference
                                                         : GrGpu::kNoDraw_DrawPreference;
    GrGpu::ReadPixelTempDrawInfo tempDrawInfo;
    GrSRGBConversion srgbConversion = determine_read_pixels_srgb_conversion(
            GrPixelConfigIsSRGBEncoded(srcProxy->config()), src->colorSpaceInfo().colorSpace(),
            dstColorType, dstColorSpace, *fContext->caps());

    if (!fContext->fGpu->getReadPixelsInfo(srcSurface, srcProxy->origin(), width, height, rowBytes,
                                           dstColorType, srgbConversion, &drawPreference,
                                           &tempDrawInfo)) {
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
        // path will do no conversion, so it will match the behavior of the non-draw path. For
        // now we simply infer an sRGB color space if the config is sRGB in order to avoid an
        // illegal combination.
        sk_sp<SkColorSpace> colorSpace;
        if (GrPixelConfigIsSRGB(tempDrawInfo.fTempSurfaceDesc.fConfig)) {
            colorSpace = SkColorSpace::MakeSRGB();
        }
        sk_sp<GrRenderTargetContext> tempRTC =
                fContext->makeDeferredRenderTargetContext(tempDrawInfo.fTempSurfaceFit,
                                                          tempDrawInfo.fTempSurfaceDesc.fWidth,
                                                          tempDrawInfo.fTempSurfaceDesc.fHeight,
                                                          tempDrawInfo.fTempSurfaceDesc.fConfig,
                                                          std::move(colorSpace),
                                                          tempDrawInfo.fTempSurfaceDesc.fSampleCnt,
                                                          GrMipMapped::kNo,
                                                          tempDrawInfo.fTempSurfaceDesc.fOrigin);
        if (tempRTC) {
            // Adding discard to appease vulkan validation warning about loading uninitialized data
            // on draw
            tempRTC->discard();
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
    GrColorType colorTypeToRead = dstColorType;
    if (didTempDraw) {
        this->flushSurfaceWrites(proxyToRead.get());
        colorTypeToRead = tempDrawInfo.fReadColorType;
    }

    if (!proxyToRead->instantiate(this->resourceProvider())) {
        return false;
    }

    GrSurface* surfaceToRead = proxyToRead->priv().peekSurface();

    if (!fContext->fGpu->readPixels(surfaceToRead, proxyToRead->origin(), left, top, width, height,
                                    colorTypeToRead, buffer, rowBytes)) {
        return false;
    }

    // Perform umpremul conversion if we weren't able to perform it as a draw.
    if (unpremul) {
        SkColorType colorType = GrColorTypeToSkColorType(dstColorType);
        if (kUnknown_SkColorType == colorType || 4 != SkColorTypeBytesPerPixel(colorType)) {
            return false;
        }

        for (int y = 0; y < height; y++) {
            SkUnpremultiplyRow<false>((uint32_t*) buffer, (const uint32_t*) buffer, width);
            buffer = SkTAddOffset<void>(buffer, rowBytes);
        }
    }
    return true;
}

bool GrContextPriv::writeSurfacePixels2(GrSurfaceContext* dst, int left, int top, int width,
                                        int height, GrColorType srcColorType,
                                        SkColorSpace* srcColorSpace, const void* buffer,
                                        size_t rowBytes, uint32_t pixelOpsFlags) {
    ASSERT_SINGLE_OWNER_PRIV
    RETURN_FALSE_IF_ABANDONED_PRIV
    SkASSERT(dst);
    SkASSERT(buffer);
    ASSERT_OWNED_PROXY_PRIV(dst->asSurfaceProxy());
    GR_CREATE_TRACE_MARKER_CONTEXT("GrContextPriv", "writeSurfacePixels2", fContext);

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

    if (!fContext->caps()->surfaceSupportsWritePixels(dstSurface)) {
        GrSurfaceDesc desc;
        desc.fConfig = dstProxy->config();
        desc.fWidth = width;
        desc.fHeight = height;
        desc.fSampleCnt = 1;
        desc.fOrigin = kTopLeft_GrSurfaceOrigin;
        auto tempProxy =
                this->proxyProvider()->createProxy(desc, SkBackingFit::kApprox, SkBudgeted::kYes);
        if (!tempProxy) {
            return false;
        }
        auto tempCtx = this->drawingManager()->makeTextureContext(
                tempProxy, dst->colorSpaceInfo().refColorSpace());
        if (!tempCtx) {
            return false;
        }
        if (!this->writeSurfacePixels2(tempCtx.get(), 0, 0, width, height, srcColorType,
                                       srcColorSpace, buffer, rowBytes, pixelOpsFlags)) {
            return false;
        }
        return dst->copy(tempProxy.get(), SkIRect::MakeWH(width, height), {left, top});
    }

    // TODO: Make GrSurfaceContext know its alpha type and pass src buffer's alpha type.
    bool premul = SkToBool(kUnpremul_PixelOpsFlag & pixelOpsFlags);
    bool convert = premul;

    if (!valid_pixel_conversion(srcColorType, dstProxy->config(), premul)) {
        return false;
    }

    GrColorType allowedColorType =
            fContext->caps()->supportedWritePixelsColorType(dstProxy->config(), srcColorType);
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
            top = dstProxy->height() - top - height;
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
        top = dstProxy->height() - top - height;
    }

    if (!(kDontFlush_PixelOpsFlag & pixelOpsFlags) && dstSurface->surfacePriv().hasPendingIO()) {
        this->flush(nullptr);  // MDB TODO: tighten this
    }

    return this->getGpu()->writePixels(dstSurface, left, top, width, height, srcColorType, buffer,
                                       rowBytes);
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

sk_sp<GrSurfaceContext> GrContextPriv::makeWrappedSurfaceContext(sk_sp<GrSurfaceProxy> proxy,
                                                                 sk_sp<SkColorSpace> colorSpace,
                                                                 const SkSurfaceProps* props) {
    ASSERT_SINGLE_OWNER_PRIV

    // sRGB pixel configs may only be used with near-sRGB gamma color spaces.
    if (GrPixelConfigIsSRGB(proxy->config())) {
        if (!colorSpace || !colorSpace->gammaCloseToSRGB()) {
            return nullptr;
        }
    }
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
                                                                  GrMipMapped mipMapped,
                                                                  SkBackingFit fit,
                                                                  SkBudgeted isDstBudgeted,
                                                                  sk_sp<SkColorSpace> colorSpace,
                                                                  const SkSurfaceProps* props) {
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

    return this->makeWrappedSurfaceContext(std::move(proxy), std::move(colorSpace), props);
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
    SkASSERT(sampleCnt > 0);

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
    SkASSERT(sampleCnt > 0);
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
        case kRGB_565_GrPixelConfig:
        case kRGBA_4444_GrPixelConfig:
        case kBGRA_8888_GrPixelConfig:
        case kRGBA_1010102_GrPixelConfig:
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
    SkASSERT(sampleCnt > 0);
    if (0 == this->caps()->getRenderTargetSampleCount(sampleCnt, config)) {
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
    SkASSERT(sampleCnt > 0);
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
