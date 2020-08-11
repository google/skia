/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "include/gpu/GrDirectContext.h"

#include "include/gpu/GrContextThreadSafeProxy.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrContextThreadSafeProxyPriv.h"
#include "src/gpu/GrGpu.h"

#include "src/gpu/effects/GrSkSLFP.h"
#include "src/gpu/gl/GrGLGpu.h"
#include "src/gpu/mock/GrMockGpu.h"
#include "src/gpu/ops/GrSmallPathAtlasMgr.h"
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

GrDirectContext::GrDirectContext(GrBackendApi backend, const GrContextOptions& options)
        : INHERITED(GrContextThreadSafeProxyPriv::Make(backend, options)) {
}

GrDirectContext::~GrDirectContext() {
    // this if-test protects against the case where the context is being destroyed
    // before having been fully created
    if (this->priv().getGpu()) {
        this->flushAndSubmit();
    }
}

void GrDirectContext::abandonContext() {
    INHERITED::abandonContext();
    if (fSmallPathAtlasMgr) {
        fSmallPathAtlasMgr->reset();
    }
    fAtlasManager->freeAll();
}

void GrDirectContext::releaseResourcesAndAbandonContext() {
    INHERITED::releaseResourcesAndAbandonContext();
    if (fSmallPathAtlasMgr) {
        fSmallPathAtlasMgr->reset();
    }
    fAtlasManager->freeAll();
}

void GrDirectContext::freeGpuResources() {
    this->flushAndSubmit();
    if (fSmallPathAtlasMgr) {
        fSmallPathAtlasMgr->reset();
    }
    fAtlasManager->freeAll();

    INHERITED::freeGpuResources();
}

bool GrDirectContext::init() {
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

    fAtlasManager = std::make_unique<GrAtlasManager>(proxyProvider,
                                                     this->options().fGlyphCacheTextureMaximumBytes,
                                                     allowMultitexturing);
    this->priv().addOnFlushCallbackObject(fAtlasManager.get());

    return true;
}

GrSmallPathAtlasMgr* GrDirectContext::onGetSmallPathAtlasMgr() {
    if (!fSmallPathAtlasMgr) {
        fSmallPathAtlasMgr = std::make_unique<GrSmallPathAtlasMgr>();

        this->priv().addOnFlushCallbackObject(fSmallPathAtlasMgr.get());
    }

    if (!fSmallPathAtlasMgr->initAtlas(this->proxyProvider(), this->caps())) {
        return nullptr;
    }

    return fSmallPathAtlasMgr.get();
}

#ifdef SK_GL
/*************************************************************************************************/
#ifndef SK_DISABLE_LEGACY_CONTEXT_FACTORIES

sk_sp<GrContext> GrContext::MakeGL(sk_sp<const GrGLInterface> glInterface) {
    return GrDirectContext::MakeGL(std::move(glInterface));
}

sk_sp<GrContext> GrContext::MakeGL(const GrContextOptions& options) {
    return GrDirectContext::MakeGL(options);
}

sk_sp<GrContext> GrContext::MakeGL() {
    return GrDirectContext::MakeGL();
}

sk_sp<GrContext> GrContext::MakeGL(sk_sp<const GrGLInterface> glInterface,
                                   const GrContextOptions& options) {
    return GrDirectContext::MakeGL(std::move(glInterface), options);
}

#endif

/*************************************************************************************************/
sk_sp<GrDirectContext> GrDirectContext::MakeGL(sk_sp<const GrGLInterface> glInterface) {
    GrContextOptions defaultOptions;
    return MakeGL(std::move(glInterface), defaultOptions);
}

sk_sp<GrDirectContext> GrDirectContext::MakeGL(const GrContextOptions& options) {
    return MakeGL(nullptr, options);
}

sk_sp<GrDirectContext> GrDirectContext::MakeGL() {
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

sk_sp<GrDirectContext> GrDirectContext::MakeGL(sk_sp<const GrGLInterface> glInterface,
                                               const GrContextOptions& options) {
    sk_sp<GrDirectContext> direct(new GrDirectContext(GrBackendApi::kOpenGL, options));
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
    direct->fGpu = GrGLGpu::Make(std::move(glInterface), options, direct.get());
    if (!direct->init()) {
        return nullptr;
    }
    return direct;
}
#endif

/*************************************************************************************************/
#ifndef SK_DISABLE_LEGACY_CONTEXT_FACTORIES

sk_sp<GrContext> GrContext::MakeMock(const GrMockOptions* mockOptions) {
    return GrDirectContext::MakeMock(mockOptions);
}

sk_sp<GrContext> GrContext::MakeMock(const GrMockOptions* mockOptions,
                                     const GrContextOptions& options) {
    return GrDirectContext::MakeMock(mockOptions, options);
}

#endif

/*************************************************************************************************/
sk_sp<GrDirectContext> GrDirectContext::MakeMock(const GrMockOptions* mockOptions) {
    GrContextOptions defaultOptions;
    return MakeMock(mockOptions, defaultOptions);
}

sk_sp<GrDirectContext> GrDirectContext::MakeMock(const GrMockOptions* mockOptions,
                                                 const GrContextOptions& options) {
    sk_sp<GrDirectContext> direct(new GrDirectContext(GrBackendApi::kMock, options));

    direct->fGpu = GrMockGpu::Make(mockOptions, options, direct.get());
    if (!direct->init()) {
        return nullptr;
    }

    return direct;
}

#ifdef SK_VULKAN
/*************************************************************************************************/
#ifndef SK_DISABLE_LEGACY_CONTEXT_FACTORIES

sk_sp<GrContext> GrContext::MakeVulkan(const GrVkBackendContext& backendContext) {
    return GrDirectContext::MakeVulkan(backendContext);
}

sk_sp<GrContext> GrContext::MakeVulkan(const GrVkBackendContext& backendContext,
                                       const GrContextOptions& options) {
    return GrDirectContext::MakeVulkan(backendContext, options);
}

#endif

/*************************************************************************************************/
sk_sp<GrDirectContext> GrDirectContext::MakeVulkan(const GrVkBackendContext& backendContext) {
    GrContextOptions defaultOptions;
    return MakeVulkan(backendContext, defaultOptions);
}

sk_sp<GrDirectContext> GrDirectContext::MakeVulkan(const GrVkBackendContext& backendContext,
                                                   const GrContextOptions& options) {
    sk_sp<GrDirectContext> direct(new GrDirectContext(GrBackendApi::kVulkan, options));

    direct->fGpu = GrVkGpu::Make(backendContext, options, direct.get());
    if (!direct->init()) {
        return nullptr;
    }

    return direct;
}
#endif

#ifdef SK_METAL
/*************************************************************************************************/
#ifndef SK_DISABLE_LEGACY_CONTEXT_FACTORIES

sk_sp<GrContext> GrContext::MakeMetal(void* device, void* queue) {
    return GrDirectContext::MakeMetal(device, queue);
}

sk_sp<GrContext> GrContext::MakeMetal(void* device, void* queue, const GrContextOptions& options) {
    return GrDirectContext::MakeMetal(device, queue, options);
}

#endif

/*************************************************************************************************/
sk_sp<GrDirectContext> GrDirectContext::MakeMetal(void* device, void* queue) {
    GrContextOptions defaultOptions;
    return MakeMetal(device, queue, defaultOptions);
}

sk_sp<GrDirectContext> GrDirectContext::MakeMetal(void* device, void* queue,
                                                  const GrContextOptions& options) {
    sk_sp<GrDirectContext> direct(new GrDirectContext(GrBackendApi::kMetal, options));

    direct->fGpu = GrMtlTrampoline::MakeGpu(direct.get(), options, device, queue);
    if (!direct->init()) {
        return nullptr;
    }

    return direct;
}
#endif

#ifdef SK_DIRECT3D
/*************************************************************************************************/
#ifndef SK_DISABLE_LEGACY_CONTEXT_FACTORIES

sk_sp<GrContext> GrContext::MakeDirect3D(const GrD3DBackendContext& backendContext) {
    return GrDirectContext::MakeDirect3D(backendContext);
}

sk_sp<GrContext> GrContext::MakeDirect3D(const GrD3DBackendContext& backendContext,
                                         const GrContextOptions& options) {
    return GrDirectContext::MakeDirect3D(backendContext, options);
}

#endif

/*************************************************************************************************/
sk_sp<GrDirectContext> GrDirectContext::MakeDirect3D(const GrD3DBackendContext& backendContext) {
    GrContextOptions defaultOptions;
    return MakeDirect3D(backendContext, defaultOptions);
}

sk_sp<GrDirectContext> GrDirectContext::MakeDirect3D(const GrD3DBackendContext& backendContext,
                                                     const GrContextOptions& options) {
    sk_sp<GrDirectContext> direct(new GrDirectContext(GrBackendApi::kDirect3D, options));

    direct->fGpu = GrD3DGpu::Make(backendContext, options, direct.get());
    if (!direct->init()) {
        return nullptr;
    }

    return direct;
}
#endif

#ifdef SK_DAWN
/*************************************************************************************************/
#ifndef SK_DISABLE_LEGACY_CONTEXT_FACTORIES

sk_sp<GrContext> GrContext::MakeDawn(const wgpu::Device& device) {
    return GrDirectContext::MakeDawn(device);
}

sk_sp<GrContext> GrContext::MakeDawn(const wgpu::Device& device, const GrContextOptions& options) {
    return GrDirectContext::MakeDawn(device, options);
}

#endif

/*************************************************************************************************/
sk_sp<GrDirectContext> GrDirectContext::MakeDawn(const wgpu::Device& device) {
    GrContextOptions defaultOptions;
    return MakeDawn(device, defaultOptions);
}

sk_sp<GrDirectContext> GrDirectContext::MakeDawn(const wgpu::Device& device,
                                                 const GrContextOptions& options) {
    sk_sp<GrDirectContext> direct(new GrDirectContext(GrBackendApi::kDawn, options));

    direct->fGpu = GrDawnGpu::Make(device, options, direct.get());
    if (!direct->init()) {
        return nullptr;
    }

    return direct;
}

#endif
