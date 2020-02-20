/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "include/gpu/GrContext.h"

#include "include/gpu/GrContextThreadSafeProxy.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrContextThreadSafeProxyPriv.h"
#include "src/gpu/GrGpu.h"

#include "src/gpu/effects/GrSkSLFP.h"
#include "src/gpu/gl/GrGLGpu.h"
#include "src/gpu/mock/GrMockGpu.h"
#include "src/gpu/text/GrStrikeCache.h"
#ifdef SK_METAL
#include "src/gpu/mtl/GrMtlTrampoline.h"
#endif
#ifdef SK_VULKAN
#include "src/gpu/vk/GrVkGpu.h"
#endif
#ifdef SK_DIRECT3D
#include "src/gpu/d3d/GrD3DGpu.h"
#endif
#ifdef SK_DAWN
#include "src/gpu/dawn/GrDawnGpu.h"
#endif

#ifdef SK_DISABLE_REDUCE_OPLIST_SPLITTING
static const bool kDefaultReduceOpsTaskSplitting = false;
#else
static const bool kDefaultReduceOpsTaskSplitting = false;
#endif

class GrLegacyDirectContext : public GrContext {
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
    bool init(sk_sp<const GrCaps> caps) override {
        SkASSERT(caps);
        SkASSERT(!fThreadSafeProxy);

        fThreadSafeProxy = GrContextThreadSafeProxyPriv::Make(this->backend(),
                                                              this->options(),
                                                              this->contextID(),
                                                              caps);

        if (!INHERITED::init(std::move(caps))) {
            return false;
        }

        bool reduceOpsTaskSplitting = kDefaultReduceOpsTaskSplitting;
        if (GrContextOptions::Enable::kNo == this->options().fReduceOpsTaskSplitting) {
            reduceOpsTaskSplitting = false;
        } else if (GrContextOptions::Enable::kYes == this->options().fReduceOpsTaskSplitting) {
            reduceOpsTaskSplitting = true;
        }

        this->setupDrawingManager(true, reduceOpsTaskSplitting);

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
    GrAtlasManager* fAtlasManager;

    typedef GrContext INHERITED;
};

#ifdef SK_GL
sk_sp<GrContext> GrContext::MakeGL(sk_sp<const GrGLInterface> glInterface) {
    GrContextOptions defaultOptions;
    return MakeGL(std::move(glInterface), defaultOptions);
}

sk_sp<GrContext> GrContext::MakeGL(const GrContextOptions& options) {
    return MakeGL(nullptr, options);
}

sk_sp<GrContext> GrContext::MakeGL() {
    GrContextOptions defaultOptions;
    return MakeGL(nullptr, defaultOptions);
}

sk_sp<GrContext> GrContext::MakeGL(sk_sp<const GrGLInterface> glInterface,
                                   const GrContextOptions& options) {
    sk_sp<GrContext> context(new GrLegacyDirectContext(GrBackendApi::kOpenGL, options));

    context->fGpu = GrGLGpu::Make(std::move(glInterface), options, context.get());
    if (!context->fGpu) {
        return nullptr;
    }

    if (!context->init(context->fGpu->refCaps())) {
        return nullptr;
    }
    return context;
}
#endif

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

    if (!context->init(context->fGpu->refCaps())) {
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

    if (!context->init(context->fGpu->refCaps())) {
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

    if (!context->init(context->fGpu->refCaps())) {
        return nullptr;
    }
    return context;
}
#endif

#ifdef SK_DIRECT3D
sk_sp<GrContext> GrContext::MakeDirect3D(const GrD3DBackendContext& backendContext) {
    GrContextOptions defaultOptions;
    return MakeDirect3D(backendContext, defaultOptions);
}

sk_sp<GrContext> GrContext::MakeDirect3D(const GrD3DBackendContext& backendContext,
                                         const GrContextOptions& options) {
    sk_sp<GrContext> context(new GrLegacyDirectContext(GrBackendApi::kDirect3D, options));

    context->fGpu = GrD3DGpu::Make(backendContext, options, context.get());
    if (!context->fGpu) {
        return nullptr;
    }

    if (!context->init(context->fGpu->refCaps())) {
        return nullptr;
    }
    return context;
}
#endif

#ifdef SK_DAWN
sk_sp<GrContext> GrContext::MakeDawn(const wgpu::Device& device) {
    GrContextOptions defaultOptions;
    return MakeDawn(device, defaultOptions);
}

sk_sp<GrContext> GrContext::MakeDawn(const wgpu::Device& device, const GrContextOptions& options) {
    sk_sp<GrContext> context(new GrLegacyDirectContext(GrBackendApi::kDawn, options));

    context->fGpu = GrDawnGpu::Make(device, options, context.get());
    if (!context->fGpu) {
        return nullptr;
    }

    if (!context->init(context->fGpu->refCaps())) {
        return nullptr;
    }
    return context;
}
#endif
