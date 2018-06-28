/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrContext.h"

#include "GrContextPriv.h"
#include "GrGpu.h"

#include "gl/GrGLGpu.h"
#include "mock/GrMockGpu.h"
#include "text/GrGlyphCache.h"
#ifdef SK_METAL
#include "mtl/GrMtlTrampoline.h"
#endif
#ifdef SK_VULKAN
#include "vk/GrVkGpu.h"
#endif

class SK_API GrDirectContext : public GrContext {
public:
    GrDirectContext(GrBackend backend)
            : INHERITED(backend)
            , fAtlasManager(nullptr) {
    }

    ~GrDirectContext() override {
        // this if-test protects against the case where the context is being destroyed
        // before having been fully created
        if (this->contextPriv().getGpu()) {
            this->flush();
        }

        delete fAtlasManager;
    }

    void abandonContext() override {
        INHERITED::abandonContext();
        fAtlasManager->freeAll();
    }

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

        fAtlasManager = new GrAtlasManager(proxyProvider, glyphCache,
                                           options.fGlyphCacheTextureMaximumBytes,
                                           allowMultitexturing);
        this->contextPriv().addOnFlushCallbackObject(fAtlasManager);

        SkASSERT(glyphCache->getGlyphSizeLimit() == fAtlasManager->getGlyphSizeLimit());
        return true;
    }

    GrAtlasManager* onGetAtlasManager() override { return fAtlasManager; }

private:
    GrAtlasManager* fAtlasManager;

    typedef GrContext INHERITED;
};

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
sk_sp<GrContext> GrContext::MakeVulkan(const GrVkBackendContext& backendContext) {
    GrContextOptions defaultOptions;
    return MakeVulkan(backendContext, defaultOptions);
}

sk_sp<GrContext> GrContext::MakeVulkan(const GrVkBackendContext& backendContext,
                                       const GrContextOptions& options) {
    sk_sp<GrContext> context(new GrDirectContext(kVulkan_GrBackend));

    context->fGpu = GrVkGpu::Make(backendContext, options, context.get());
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

    context->fCaps = context->fGpu->refCaps();
    if (!context->init(options)) {
        return nullptr;
    }
    return context;
}
#endif

