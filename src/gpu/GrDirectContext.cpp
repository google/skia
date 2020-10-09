/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "include/gpu/GrDirectContext.h"

#include "include/gpu/GrContextThreadSafeProxy.h"
#include "src/core/SkTaskGroup.h"
#include "src/gpu/GrClientMappedBufferManager.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrContextThreadSafeProxyPriv.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrShaderUtils.h"

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

#define ASSERT_SINGLE_OWNER GR_ASSERT_SINGLE_OWNER(this->singleOwner())

GrDirectContext::GrDirectContext(GrBackendApi backend, const GrContextOptions& options)
        : INHERITED(GrContextThreadSafeProxyPriv::Make(backend, options)) {
}

GrDirectContext::~GrDirectContext() {
    ASSERT_SINGLE_OWNER
    // this if-test protects against the case where the context is being destroyed
    // before having been fully created
    if (fGpu) {
        this->flushAndSubmit();
    }

    this->destroyDrawingManager();
    fMappedBufferManager.reset();

    // Ideally we could just let the ptr drop, but resource cache queries this ptr in releaseAll.
    if (fResourceCache) {
        fResourceCache->releaseAll();
    }
}

void GrDirectContext::resetGLTextureBindings() {
    if (this->abandoned() || this->backend() != GrBackendApi::kOpenGL) {
        return;
    }
    fGpu->resetTextureBindings();
}

void GrDirectContext::resetContext(uint32_t state) {
    ASSERT_SINGLE_OWNER
    fGpu->markContextDirty(state);
}

void GrDirectContext::abandonContext() {
    if (INHERITED::abandoned()) {
        return;
    }

    INHERITED::abandonContext();

    fStrikeCache->freeAll();

    fMappedBufferManager->abandon();

    fResourceProvider->abandon();

    // abandon first to so destructors
    // don't try to free the resources in the API.
    fResourceCache->abandonAll();

    fGpu->disconnect(GrGpu::DisconnectType::kAbandon);

    fMappedBufferManager.reset();
    if (fSmallPathAtlasMgr) {
        fSmallPathAtlasMgr->reset();
    }
    fAtlasManager->freeAll();
}

bool GrDirectContext::abandoned() {
    if (INHERITED::abandoned()) {
        return true;
    }

    if (fGpu && fGpu->isDeviceLost()) {
        this->abandonContext();
        return true;
    }
    return false;
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
    ASSERT_SINGLE_OWNER
    if (!fGpu) {
        return false;
    }

    fThreadSafeProxy->priv().init(fGpu->refCaps());
    if (!INHERITED::init()) {
        return false;
    }

    SkASSERT(this->getTextBlobCache());
    SkASSERT(this->threadSafeCache());

    fStrikeCache = std::make_unique<GrStrikeCache>();
    fResourceCache = std::make_unique<GrResourceCache>(this->caps(), this->singleOwner(),
                                                       this->contextID());
    fResourceCache->setProxyProvider(this->proxyProvider());
    fResourceCache->setThreadSafeCache(this->threadSafeCache());
    fResourceProvider = std::make_unique<GrResourceProvider>(fGpu.get(), fResourceCache.get(),
                                                             this->singleOwner());
    fMappedBufferManager = std::make_unique<GrClientMappedBufferManager>(this->contextID());

    fDidTestPMConversions = false;

    // DDL TODO: we need to think through how the task group & persistent cache
    // get passed on to/shared between all the DDLRecorders created with this context.
    if (this->options().fExecutor) {
        fTaskGroup = std::make_unique<SkTaskGroup>(*this->options().fExecutor);
    }

    fPersistentCache = this->options().fPersistentCache;
    fShaderErrorHandler = this->options().fShaderErrorHandler;
    if (!fShaderErrorHandler) {
        fShaderErrorHandler = GrShaderUtils::DefaultShaderErrorHandler();
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
