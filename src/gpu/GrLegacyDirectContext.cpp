/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrContext.h"

#include "GrContextPriv.h"
#include "GrContextThreadSafeProxy.h"
#include "GrContextThreadSafeProxyPriv.h"
#include "GrGpu.h"
#include "GrPathRendererChain.h"

#include "effects/GrSkSLFP.h"
#include "gl/GrGLGpu.h"
#include "mock/GrMockGpu.h"
#include "text/GrStrikeCache.h"
#include "text/GrTextBlobCache.h"
#ifdef SK_METAL
#include "mtl/GrMtlTrampoline.h"
#endif
#ifdef SK_VULKAN
#include "vk/GrVkGpu.h"
#endif
//#include "SkInternalAtlasTextContext.h"
#include "SkMakeUnique.h"
#include "SkTaskGroup.h"

class SK_API GrLegacyDirectContext : public GrContext {
public:
    GrLegacyDirectContext(GrBackendApi backend, const GrContextOptions& options)
            : INHERITED(backend, options)
            , fAtlasManager(nullptr) {
    }

    ~GrLegacyDirectContext() override {
        // this if-test protects against the case where the context is being destroyed
        // before having been fully created
        if (this->priv().getGpu()) {
            this->flush();
        }

        delete fAtlasManager;
    }

#if 0
    void abandonContext() override {
        INHERITED::abandonContext();
        fAtlasManager->freeAll();
    }
#else
    void abandon1() override {
        INHERITED::abandon1();
        fAtlasManager->freeAll();
    }
#endif

    void releaseResourcesAndAbandonContext() override {
        INHERITED::releaseResourcesAndAbandonContext();
        fAtlasManager->freeAll();
    }

    void freeGpuResources() override {
        this->flush();
        fAtlasManager->freeAll();

        INHERITED::freeGpuResources();
    }

protected:
    bool init(sk_sp<const GrCaps> caps, sk_sp<GrSkSLFPFactoryCache> FPFactoryCache) override {
        SkASSERT(caps && !FPFactoryCache);
        SkASSERT(!fThreadSafeProxy);

        FPFactoryCache.reset(new GrSkSLFPFactoryCache());
        fThreadSafeProxy = GrContextThreadSafeProxyPriv::Make(this->backend(),
                                                              this->options(),
                                                              this->contextID(),
                                                              caps, FPFactoryCache);

        if (!INHERITED::init(std::move(caps), std::move(FPFactoryCache))) {
            return false;
        }

        SkASSERT(this->caps());

        GrDrawOpAtlas::AllowMultitexturing allowMultitexturing;
        if (GrContextOptions::Enable::kNo == this->options().fAllowMultipleGlyphCacheTextures ||
            // multitexturing supported only if range can represent the index + texcoords fully
            !(this->caps()->shaderCaps()->floatIs32Bits() ||
              this->caps()->shaderCaps()->integerSupport())) {
            allowMultitexturing = GrDrawOpAtlas::AllowMultitexturing::kNo;
        } else {
            allowMultitexturing = GrDrawOpAtlas::AllowMultitexturing::kYes;
        }

        GrStrikeCache* glyphCache = this->priv().getGrStrikeCache();
        GrProxyProvider* proxyProvider = this->priv().proxyProvider();

        fAtlasManager = new GrAtlasManager(proxyProvider, glyphCache,
                                           this->options().fGlyphCacheTextureMaximumBytes,
                                           allowMultitexturing);
        this->priv().addOnFlushCallbackObject(fAtlasManager);

        return true;
    }

    GrAtlasManager* onGetAtlasManager() override { return fAtlasManager; }

private:
    bool initPrivate(const GrContextOptions&);

    GrAtlasManager* fAtlasManager;

    typedef GrContext INHERITED;
};

bool GrDirectContext::initPrivate(const GrContextOptions& options) {
//    ASSERT_SINGLE_OWNER
    SkASSERT(this->caps());  // needs to have been initialized by derived classes
    SkASSERT(this->threadSafeProxy()); // needs to have been initialized by derived classes
    SkASSERT(this->proxyProvider());

    if (fGpu) {
        fResourceCache = new GrResourceCache(this->caps(), this->singleOwner(), this->uniqueID());
        fResourceProvider = new GrResourceProvider(fGpu.get(), fResourceCache, this->singleOwner(),
                                                   options.fExplicitlyAllocateGPUResources);
//        this->proxyProvider()->bam1(fResourceProvider, fResourceCache);
    }

    if (fResourceCache) {
        fResourceCache->setProxyProvider(this->proxyProvider());
    }

//    fDisableGpuYUVConversion = options.fDisableGpuYUVConversion;
//    fSharpenMipmappedTextures = options.fSharpenMipmappedTextures;
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
                                               this->singleOwner(), explicitlyAllocatingResources,
                                               options.fSortRenderTargets,
                                               options.fReduceOpListSplitting));

//    fGlyphCache = new GrGlyphCache(this->caps(), options.fGlyphCacheTextureMaximumBytes);

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

sk_sp<GrContext> GrContext::MakeGL(sk_sp<const GrGLInterface> interface) {
    GrContextOptions defaultOptions;
    return MakeGL(std::move(interface), defaultOptions);
}

sk_sp<GrContext> GrContext::MakeGL(const GrContextOptions& options) {
    return MakeGL(nullptr, options);
}

sk_sp<GrContext> GrContext::MakeGL() {
    GrContextOptions defaultOptions;
    return MakeGL(nullptr, defaultOptions);
}

sk_sp<GrContext> GrContext::MakeGL(sk_sp<const GrGLInterface> interface,
                                   const GrContextOptions& options) {
    sk_sp<GrContext> context(new GrLegacyDirectContext(GrBackendApi::kOpenGL, options));

    context->fGpu = GrGLGpu::Make(std::move(interface), options, context.get());
    if (!context->fGpu) {
        return nullptr;
    }

    if (!context->init(context->fGpu->refCaps(), nullptr)) {
        return nullptr;
    }

    return context;
}

sk_sp<GrContext> GrContext::MakeMock(const GrMockOptions* mockOptions) {
    GrContextOptions defaultOptions;
    return MakeMock(mockOptions, defaultOptions);
}

sk_sp<GrContext> GrContext::MakeMock(const GrMockOptions* mockOptions,
                                     const GrContextOptions& options) {
    sk_sp<GrContext> context(new GrLegacyDirectContext(GrBackendApi::kMock, options));

    context->fGpu = GrMockGpu::Make(mockOptions, options, context.get());
    if (!context->fGpu) {
        return nullptr;
    }

    if (!context->init(context->fGpu->refCaps(), nullptr)) {
        return nullptr;
    }

    return context;
}

sk_sp<GrContext> GrContext::MakeVulkan(const GrVkBackendContext& backendContext) {
#ifdef SK_VULKAN
    GrContextOptions defaultOptions;
    return MakeVulkan(backendContext, defaultOptions);
#else
    return nullptr;
#endif
}

sk_sp<GrContext> GrContext::MakeVulkan(const GrVkBackendContext& backendContext,
                                       const GrContextOptions& options) {
#ifdef SK_VULKAN
    sk_sp<GrContext> context(new GrLegacyDirectContext(GrBackendApi::kVulkan, options));

    context->fGpu = GrVkGpu::Make(backendContext, options, context.get());
    if (!context->fGpu) {
        return nullptr;
    }

    if (!context->init(context->fGpu->refCaps(), nullptr)) {
        return nullptr;
    }

    return context;
#else
    return nullptr;
#endif
}

#ifdef SK_METAL
sk_sp<GrContext> GrContext::MakeMetal(void* device, void* queue) {
    GrContextOptions defaultOptions;
    return MakeMetal(device, queue, defaultOptions);
}

sk_sp<GrContext> GrContext::MakeMetal(void* device, void* queue, const GrContextOptions& options) {
    sk_sp<GrContext> context(new GrLegacyDirectContext(GrBackendApi::kMetal, options));

    context->fGpu = GrMtlTrampoline::MakeGpu(context.get(), options, device, queue);
    if (!context->fGpu) {
        return nullptr;
    }

    if (!context->init(context->fGpu->refCaps(), nullptr)) {
        return nullptr;
    }
    return context;
}
#endif

