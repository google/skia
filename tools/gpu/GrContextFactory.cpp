
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
#if SK_MESA
    #include "gl/mesa/GLTestContext_mesa.h"
#endif
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
        if (context.fTestContext) {
            context.fTestContext->makeCurrent();
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
                context.fTestContext->makeCurrent();
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
        if (!context.fAbandoned) {
            if (context.fTestContext) {
                context.fTestContext->makeCurrent();
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
            return ContextInfo(context.fType, context.fTestContext, context.fGrContext);
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

    std::unique_ptr<TestContext> testCtx;
    GrBackendContext backendContext = 0;
    sk_sp<const GrGLInterface> glInterface;
    GrBackend backend = ContextTypeBackend(type);
    switch (backend) {
        case kOpenGL_GrBackend: {
            GLTestContext* glShareContext = masterContext
                    ? static_cast<GLTestContext*>(masterContext->fTestContext) : nullptr;
            GLTestContext* glCtx;
            switch (type) {
                case kGL_ContextType:
                    glCtx = CreatePlatformGLTestContext(kGL_GrGLStandard, glShareContext);
                    break;
                case kGLES_ContextType:
                    glCtx = CreatePlatformGLTestContext(kGLES_GrGLStandard, glShareContext);
                    break;
#if SK_ANGLE
                case kANGLE_D3D9_ES2_ContextType:
                    glCtx = MakeANGLETestContext(ANGLEBackend::kD3D9, ANGLEContextVersion::kES2,
                                                 glShareContext).release();
                    break;
                case kANGLE_D3D11_ES2_ContextType:
                    glCtx = MakeANGLETestContext(ANGLEBackend::kD3D11, ANGLEContextVersion::kES2,
                                                 glShareContext).release();
                    break;
                case kANGLE_D3D11_ES3_ContextType:
                    glCtx = MakeANGLETestContext(ANGLEBackend::kD3D11, ANGLEContextVersion::kES3,
                                                 glShareContext).release();
                    break;
                case kANGLE_GL_ES2_ContextType:
                    glCtx = MakeANGLETestContext(ANGLEBackend::kOpenGL, ANGLEContextVersion::kES2,
                                                 glShareContext).release();
                    break;
                case kANGLE_GL_ES3_ContextType:
                    glCtx = MakeANGLETestContext(ANGLEBackend::kOpenGL, ANGLEContextVersion::kES3,
                                                 glShareContext).release();
                    break;
#endif
#ifndef SK_NO_COMMAND_BUFFER
                case kCommandBuffer_ContextType:
                    glCtx = CommandBufferGLTestContext::Create(glShareContext);
                    break;
#endif
#if SK_MESA
                case kMESA_ContextType:
                    glCtx = CreateMesaGLTestContext(glShareContext);
                    break;
#endif
                case kNullGL_ContextType:
                    glCtx = CreateNullGLTestContext(
                            ContextOverrides::kRequireNVPRSupport & overrides, glShareContext);
                    break;
                case kDebugGL_ContextType:
                    glCtx = CreateDebugGLTestContext(glShareContext);
                    break;
                default:
                    return ContextInfo();
            }
            if (!glCtx) {
                return ContextInfo();
            }
            testCtx.reset(glCtx);
            glInterface.reset(SkRef(glCtx->gl()));
            backendContext = reinterpret_cast<GrBackendContext>(glInterface.get());
            break;
        }
#ifdef SK_VULKAN
        case kVulkan_GrBackend: {
            VkTestContext* vkSharedContext = masterContext
                    ? static_cast<VkTestContext*>(masterContext->fTestContext) : nullptr;
            SkASSERT(kVulkan_ContextType == type);
            if (ContextOverrides::kRequireNVPRSupport & overrides) {
                return ContextInfo();
            }
            testCtx.reset(CreatePlatformVkTestContext(vkSharedContext));
            if (!testCtx) {
                return ContextInfo();
            }

            // There is some bug (either in Skia or the NV Vulkan driver) where VkDevice
            // destruction will hang occaisonally. For some reason having an existing GL
            // context fixes this.
            if (!fSentinelGLContext) {
                fSentinelGLContext.reset(CreatePlatformGLTestContext(kGL_GrGLStandard));
                if (!fSentinelGLContext) {
                    fSentinelGLContext.reset(CreatePlatformGLTestContext(kGLES_GrGLStandard));
                }
            }
            backendContext = testCtx->backendContext();
            break;
        }
#endif
#ifdef SK_METAL
        case kMetal_GrBackend: {
            SkASSERT(!masterContext);
            testCtx.reset(CreatePlatformMtlTestContext(nullptr));
            if (!testCtx) {
                return ContextInfo();
            }
            break;
        }
#endif
        case kMock_GrBackend: {
            TestContext* sharedContext = masterContext ? masterContext->fTestContext : nullptr;
            SkASSERT(kMock_ContextType == type);
            if (ContextOverrides::kRequireNVPRSupport & overrides) {
                return ContextInfo();
            }
            testCtx.reset(CreateMockTestContext(sharedContext));
            if (!testCtx) {
                return ContextInfo();
            }
            backendContext = testCtx->backendContext();
            break;
        }
        default:
            return ContextInfo();
    }
    testCtx->makeCurrent();
    SkASSERT(testCtx && testCtx->backend() == backend);
    GrContextOptions grOptions = fGlobalOptions;
    if (ContextOverrides::kDisableNVPR & overrides) {
        grOptions.fSuppressPathRendering = true;
    }
    if (ContextOverrides::kUseInstanced & overrides) {
        grOptions.fEnableInstancedRendering = true;
    }
    if (ContextOverrides::kAllowSRGBWithoutDecodeControl & overrides) {
        grOptions.fRequireDecodeDisableForSRGB = false;
    }
    if (ContextOverrides::kAvoidStencilBuffers & overrides) {
        grOptions.fAvoidStencilBuffers = true;
    }
    sk_sp<GrContext> grCtx = testCtx->makeGrContext(grOptions);
    if (!grCtx.get() && kMetal_GrBackend != backend) {
        grCtx.reset(GrContext::Create(backend, backendContext, grOptions));
    }
    if (!grCtx.get()) {
        return ContextInfo();
    }
    if (ContextOverrides::kRequireNVPRSupport & overrides) {
        if (!grCtx->caps()->shaderCaps()->pathRenderingSupport()) {
            return ContextInfo();
        }
    }
    if (ContextOverrides::kUseInstanced & overrides) {
        if (GrCaps::InstancedSupport::kNone == grCtx->caps()->instancedSupport()) {
            return ContextInfo();
        }
    }
    if (ContextOverrides::kRequireSRGBSupport & overrides) {
        if (!grCtx->caps()->srgbSupport()) {
            return ContextInfo();
        }
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
    return ContextInfo(context.fType, context.fTestContext, context.fGrContext);
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

}  // namespace sk_gpu_test
