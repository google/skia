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
#include "src/gpu/text/GrAtlasManager.h"
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

#if GR_TEST_UTILS
#   include "include/utils/SkRandom.h"
#   if defined(SK_ENABLE_SCOPED_LSAN_SUPPRESSIONS)
#       include <sanitizer/lsan_interface.h>
#   endif
#endif

#ifdef SK_DISABLE_REDUCE_OPLIST_SPLITTING
static const bool kDefaultReduceOpsTaskSplitting = false;
#else
static const bool kDefaultReduceOpsTaskSplitting = false;
#endif

class GrLegacyDirectContext : public GrContext {
public:
    GrLegacyDirectContext(GrBackendApi backend, const GrContextOptions& options)
            : INHERITED(GrContextThreadSafeProxyPriv::Make(backend, options))
            , fAtlasManager(nullptr) {
    }

    ~GrLegacyDirectContext() override {
        // this if-test protects against the case where the context is being destroyed
        // before having been fully created
        if (this->priv().getGpu()) {
            this->flushAndSubmit();
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
        this->flushAndSubmit();
        fAtlasManager->freeAll();

        INHERITED::freeGpuResources();
    }

protected:
    bool init() override {
        const GrGpu* gpu = this->priv().getGpu();
        if (!gpu) {
            return false;
        }

        fThreadSafeProxy->priv().init(gpu->refCaps());
        if (!INHERITED::init()) {
            return false;
        }

        bool reduceOpsTaskSplitting = kDefaultReduceOpsTaskSplitting;
        if (GrContextOptions::Enable::kNo == this->options().fReduceOpsTaskSplitting) {
            reduceOpsTaskSplitting = false;
        } else if (GrContextOptions::Enable::kYes == this->options().fReduceOpsTaskSplitting) {
            reduceOpsTaskSplitting = true;
        }

        this->setupDrawingManager(true, reduceOpsTaskSplitting);

        GrDrawOpAtlas::AllowMultitexturing allowMultitexturing;
        if (GrContextOptions::Enable::kNo == this->options().fAllowMultipleGlyphCacheTextures ||
            // multitexturing supported only if range can represent the index + texcoords fully
            !(this->caps()->shaderCaps()->floatIs32Bits() ||
              this->caps()->shaderCaps()->integerSupport())) {
            allowMultitexturing = GrDrawOpAtlas::AllowMultitexturing::kNo;
        } else {
            allowMultitexturing = GrDrawOpAtlas::AllowMultitexturing::kYes;
        }

        GrProxyProvider* proxyProvider = this->priv().proxyProvider();

        fAtlasManager = new GrAtlasManager(proxyProvider,
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

#if GR_TEST_UTILS
GrGLFunction<GrGLGetErrorFn> make_get_error_with_random_oom(GrGLFunction<GrGLGetErrorFn> original) {
    // A SkRandom and a GrGLFunction<GrGLGetErrorFn> are too big to be captured by a
    // GrGLFunction<GrGLGetError> (surprise, surprise). So we make a context object and
    // capture that by pointer. However, GrGLFunction doesn't support calling a destructor
    // on the thing it captures. So we leak the context.
    struct GetErrorContext {
        SkRandom fRandom;
        GrGLFunction<GrGLGetErrorFn> fGetError;
    };

    auto errorContext = new GetErrorContext;

#if defined(SK_ENABLE_SCOPED_LSAN_SUPPRESSIONS)
    __lsan_ignore_object(errorContext);
#endif

    errorContext->fGetError = original;

    return GrGLFunction<GrGLGetErrorFn>([errorContext]() {
        GrGLenum error = errorContext->fGetError();
        if (error == GR_GL_NO_ERROR && (errorContext->fRandom.nextU() % 300) == 0) {
            error = GR_GL_OUT_OF_MEMORY;
        }
        return error;
    });
}
#endif

sk_sp<GrContext> GrContext::MakeGL(sk_sp<const GrGLInterface> glInterface,
                                   const GrContextOptions& options) {
    sk_sp<GrContext> context(new GrLegacyDirectContext(GrBackendApi::kOpenGL, options));
#if GR_TEST_UTILS
    if (options.fRandomGLOOM) {
        auto copy = sk_make_sp<GrGLInterface>(*glInterface);
        copy->fFunctions.fGetError =
                make_get_error_with_random_oom(glInterface->fFunctions.fGetError);
#if GR_GL_CHECK_ERROR
        // Suppress logging GL errors since we'll be synthetically generating them.
        copy->suppressErrorLogging();
#endif
        glInterface = std::move(copy);
    }
#endif
    context->fGpu = GrGLGpu::Make(std::move(glInterface), options, context.get());
    if (!context->init()) {
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
    if (!context->init()) {
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
    if (!context->init()) {
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
    if (!context->init()) {
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
    if (!context->init()) {
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
    if (!context->init()) {
        return nullptr;
    }

    return context;
}
#endif
