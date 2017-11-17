
/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrContextFactory.h"
#include "gl/GLTestContext.h"

#if SK_ANGLE
    #include "gl/angle/GLTestContext_angle.h"
#endif
#include "gl/command_buffer/GLTestContext_command_buffer.h"
#include "gl/debug/DebugGLTestContext.h"
#ifdef SK_VULKAN
#include "vk/VkTestContext.h"
#endif
#ifdef SK_METAL
#include "mtl/MtlTestContext.h"
#endif
#include "gl/null/NullGLTestContext.h"
#include "gl/GrGLGpu.h"
#include "mock/MockTestContext.h"
#include "GrCaps.h"

#if defined(SK_BUILD_FOR_WIN32) && defined(SK_ENABLE_DISCRETE_GPU)
extern "C" {
    // NVIDIA documents that the presence and value of this symbol programmatically enable the high
    // performance GPU in laptops with switchable graphics.
    //   https://docs.nvidia.com/gameworks/content/technologies/desktop/optimus.htm
    // From testing, including this symbol, even if it is set to 0, we still get the NVIDIA GPU.
    _declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;

    // AMD has a similar mechanism, although I don't have an AMD laptop, so this is untested.
    //   https://community.amd.com/thread/169965
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

namespace sk_gpu_test {
GrContextFactory::GrContextFactory() { }

GrContextFactory::GrContextFactory(const GrContextOptions& opts)
    : fGlobalOptions(opts) {
}

GrContextFactory::~GrContextFactory() {
    this->destroyContexts();
}

void GrContextFactory::destroyContexts() {
    for (Context& context : fContexts) {
        SkScopeExit restore(nullptr);
        if (context.fTestContext) {
            restore = context.fTestContext->makeCurrentAndAutoRestore();
        }
        if (!context.fGrContext->unique()) {
            context.fGrContext->releaseResourcesAndAbandonContext();
            context.fAbandoned = true;
        }
        context.fGrContext->unref();
        delete context.fTestContext;
    }
    fContexts.reset();
}

void GrContextFactory::abandonContexts() {
    for (Context& context : fContexts) {
        if (!context.fAbandoned) {
            if (context.fTestContext) {
                auto restore = context.fTestContext->makeCurrentAndAutoRestore();
                context.fTestContext->testAbandon();
                delete(context.fTestContext);
                context.fTestContext = nullptr;
            }
            context.fGrContext->abandonContext();
            context.fAbandoned = true;
        }
    }
}

void GrContextFactory::releaseResourcesAndAbandonContexts() {
    for (Context& context : fContexts) {
        SkScopeExit restore(nullptr);
        if (!context.fAbandoned) {
            if (context.fTestContext) {
                restore = context.fTestContext->makeCurrentAndAutoRestore();
            }
            context.fGrContext->releaseResourcesAndAbandonContext();
            context.fAbandoned = true;
            if (context.fTestContext) {
                delete context.fTestContext;
                context.fTestContext = nullptr;
            }
        }
    }
}

GrContext* GrContextFactory::get(ContextType type, ContextOverrides overrides) {
    return this->getContextInfo(type, overrides).grContext();
}

static GrContextOptions modify_context_options(const GrContextOptions& originalOpts,
                                               GrContextFactory::ContextOverrides overrides) {
    GrContextOptions opts = originalOpts;
    if (GrContextFactory::ContextOverrides::kDisableNVPR & overrides) {
        opts.fSuppressPathRendering = true;
    }
    if (GrContextFactory::ContextOverrides::kUseInstanced & overrides) {
        opts.fEnableInstancedRendering = true;
    }
    if (GrContextFactory::ContextOverrides::kAllowSRGBWithoutDecodeControl & overrides) {
        opts.fRequireDecodeDisableForSRGB = false;
    }
    if (GrContextFactory::ContextOverrides::kAvoidStencilBuffers & overrides) {
        opts.fAvoidStencilBuffers = true;
    }
    return opts;
}

static std::unique_ptr<TestContext> make_test_context(GrContextFactory::ContextType type,
                                                      GrContextFactory::ContextOverrides overrides,
                                                      TestContext* sharedContext) {
    GLTestContext* glShareContext = nullptr;
    if (kOpenGL_GrBackend == GrContextFactory::ContextTypeBackend(type)) {
        glShareContext = static_cast<GLTestContext*>(sharedContext);
    }
    using U = std::unique_ptr<TestContext>;
    switch (type) {
        case GrContextFactory::kGL_ContextType:
            return U(CreatePlatformGLTestContext(kGL_GrGLStandard, glShareContext));
        case GrContextFactory::kGLES_ContextType:
            return U(CreatePlatformGLTestContext(kGLES_GrGLStandard, glShareContext));
        #if SK_ANGLE
        case GrContextFactory::kANGLE_D3D9_ES2_ContextType:
            return MakeANGLETestContext(ANGLEBackend::kD3D9, ANGLEContextVersion::kES2,
                                         glShareContext);
        case GrContextFactory::kANGLE_D3D11_ES2_ContextType:
            return MakeANGLETestContext(ANGLEBackend::kD3D11, ANGLEContextVersion::kES2,
                                         glShareContext);
        case GrContextFactory::kANGLE_D3D11_ES3_ContextType:
            return MakeANGLETestContext(ANGLEBackend::kD3D11, ANGLEContextVersion::kES3,
                                         glShareContext);
        case GrContextFactory::kANGLE_GL_ES2_ContextType:
            return MakeANGLETestContext(ANGLEBackend::kOpenGL, ANGLEContextVersion::kES2,
                                         glShareContext);
        case GrContextFactory::kANGLE_GL_ES3_ContextType:
            return MakeANGLETestContext(ANGLEBackend::kOpenGL, ANGLEContextVersion::kES3,
                                         glShareContext);
        #endif
        #ifndef SK_NO_COMMAND_BUFFER
        case GrContextFactory::kCommandBuffer_ContextType:
            return U(CommandBufferGLTestContext::Create(glShareContext));
        #endif
        case GrContextFactory::kNullGL_ContextType:
            return U(CreateNullGLTestContext(
                        GrContextFactory::ContextOverrides::kRequireNVPRSupport & overrides,
                        glShareContext));
        case GrContextFactory::kDebugGL_ContextType:
            return U(CreateDebugGLTestContext(glShareContext));
        #ifdef SK_VULKAN
        case GrContextFactory::kVulkan_ContextType:
            if (ContextOverrides::kRequireNVPRSupport & overrides) {
                return nullptr;
            }
            return U(CreatePlatformVkTestContext(static_cast<VkTestContext*>(sharedContext)));
        #endif
        #ifdef SK_METAL
        case GrContextFactory::kMetal_ContextType:
            SkASSERT(!sharedContext);
            return U(CreatePlatformMtlTestContext(nullptr));
        #endif
        case GrContextFactory::kMock_ContextType:
            if (GrContextFactory::ContextOverrides::kRequireNVPRSupport & overrides) {
                return nullptr;
            }
            return U(CreateMockTestContext(sharedContext));
        default:
            return nullptr;
    }
}

static bool check_context(GrContext* grCtx,
                          GrContextFactory::ContextOverrides overrides) {
    using ContextOverrides = GrContextFactory::ContextOverrides;
    if (SkToBool(ContextOverrides::kRequireNVPRSupport & overrides) &&
        !grCtx->caps()->shaderCaps()->pathRenderingSupport()) {
            return false;
    }
    if (SkToBool(ContextOverrides::kUseInstanced & overrides) &&
        GrCaps::InstancedSupport::kNone == grCtx->caps()->instancedSupport()) {
            return false;
    }
    if (SkToBool(ContextOverrides::kRequireSRGBSupport & overrides) &&
        !grCtx->caps()->srgbSupport()) {
            return false;
    }
    return true;
}

ContextInfo GrContextFactory::getContextInfoInternal(ContextType type, ContextOverrides overrides,
                                                     GrContext* shareContext, uint32_t shareIndex) {
    // (shareIndex != 0) -> (shareContext != nullptr)
    SkASSERT((shareIndex == 0) || (shareContext != nullptr));

    for (int i = 0; i < fContexts.count(); ++i) {
        Context& context = fContexts[i];
        if (context.fType == type &&
            context.fOverrides == overrides &&
            context.fShareContext == shareContext &&
            context.fShareIndex == shareIndex &&
            !context.fAbandoned) {
            context.fTestContext->makeCurrent();
            return ContextInfo(context.fType, context.fTestContext, context.fGrContext,
                               context.fOptions);
        }
    }

    // If we're trying to create a context in a share group, find the master context
    Context* masterContext = nullptr;
    if (shareContext) {
        for (int i = 0; i < fContexts.count(); ++i) {
            if (!fContexts[i].fAbandoned && fContexts[i].fGrContext == shareContext) {
                masterContext = &fContexts[i];
                break;
            }
        }
        SkASSERT(masterContext && masterContext->fType == type);
    }

    TestContext* sharedContext = masterContext ? masterContext->fTestContext : nullptr;
    std::unique_ptr<TestContext> testCtx = make_test_context(type, overrides, sharedContext);
    if (!testCtx) {
        return ContextInfo();
    }
    GrBackend backend = ContextTypeBackend(type);
    SkASSERT(testCtx->backend() == backend);

#ifdef SK_VULKAN
    if (kVulkan_GrBackend == backend) {
        // There is some bug (either in Skia or the NV Vulkan driver) where VkDevice
        // destruction will hang occaisonally. For some reason having an existing GL
        // context fixes this.
        if (!fSentinelGLContext) {
            fSentinelGLContext.reset(CreatePlatformGLTestContext(kGL_GrGLStandard));
            if (!fSentinelGLContext) {
                fSentinelGLContext.reset(CreatePlatformGLTestContext(kGLES_GrGLStandard));
            }
        }
    }
#endif
    GrContextOptions grOptions = modify_context_options(fGlobalOptions, overrides);
    sk_sp<GrContext> grCtx;
    {
        auto restore = testCtx->makeCurrentAndAutoRestore();
        grCtx = testCtx->makeGrContext(grOptions);
    }
    if (!grCtx.get() || !check_context(grCtx.get(), overrides)) {
        return ContextInfo();
    }
    Context& context = fContexts.push_back();
    context.fBackend = backend;
    context.fTestContext = testCtx.release();
    context.fGrContext = SkRef(grCtx.get());
    context.fType = type;
    context.fOverrides = overrides;
    context.fAbandoned = false;
    context.fShareContext = shareContext;
    context.fShareIndex = shareIndex;
    context.fOptions = grOptions;
    context.fTestContext->makeCurrent();
    return ContextInfo(context.fType, context.fTestContext, context.fGrContext, context.fOptions);
}

ContextInfo GrContextFactory::getContextInfo(ContextType type, ContextOverrides overrides) {
    return this->getContextInfoInternal(type, overrides, nullptr, 0);
}

ContextInfo GrContextFactory::getSharedContextInfo(GrContext* shareContext, uint32_t shareIndex) {
    SkASSERT(shareContext);
    for (int i = 0; i < fContexts.count(); ++i) {
        if (!fContexts[i].fAbandoned && fContexts[i].fGrContext == shareContext) {
            return this->getContextInfoInternal(fContexts[i].fType, fContexts[i].fOverrides,
                                                shareContext, shareIndex);
        }
    }

    return ContextInfo();
}

static sk_sp<GrContext> make_grcontext(TestContext* testCtx,
                                       const GrContextOptions& opts,
                                       GrContextFactory::ContextOverrides overrides) {
    if (testCtx) {
        testCtx->makeCurrent();
        sk_sp<GrContext> grCtx = testCtx->makeGrContext(modify_context_options(opts, overrides));
        if (check_context(grCtx.get(), overrides)) {
            return grCtx;
        }
    }
    return nullptr;
}

sk_sp<GrContext> GrMakeContext(const GrContextOptions& opts,
                               GrContextFactory::ContextType type,
                               GrContextFactory::ContextOverrides overrides) {
    std::unique_ptr<TestContext> testCtx = make_test_context(type, overrides, nullptr);
    SkASSERT(!testCtx || testCtx->backend() == GrContextFactory::ContextTypeBackend(type));
    return make_grcontext(testCtx.get(), opts, overrides);
}
}  // namespace sk_gpu_test
