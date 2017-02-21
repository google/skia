
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
#include "gl/null/NullGLTestContext.h"
#include "gl/GrGLGpu.h"
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
    // In this factory, instanced rendering is specified with ContextOptions::kUseInstanced.
    SkASSERT(!fGlobalOptions.fEnableInstancedRendering);
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

#if defined(SK_BUILD_FOR_UNIX) || defined(SK_BUILD_FOR_WIN) || defined(SK_BUILD_FOR_MAC)
const GrContextFactory::ContextType GrContextFactory::kNativeGL_ContextType =
    GrContextFactory::kGL_ContextType;
#else
const GrContextFactory::ContextType GrContextFactory::kNativeGL_ContextType =
    GrContextFactory::kGLES_ContextType;
#endif

ContextInfo GrContextFactory::getContextInfo(ContextType type, ContextOptions options,
                                             GrContext* shareContext, uint32_t shareIndex) {
    // (shareIndex != 0) -> (shareContext != nullptr)
    SkASSERT((shareIndex == 0) || (shareContext != nullptr));

    for (int i = 0; i < fContexts.count(); ++i) {
        Context& context = fContexts[i];
        if (context.fType == type &&
            context.fOptions == options &&
            context.fShareContext == shareContext &&
            context.fShareIndex == shareIndex &&
            !context.fAbandoned) {
            context.fTestContext->makeCurrent();
            return ContextInfo(context.fBackend, context.fTestContext, context.fGrContext);
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

        if (!masterContext) {
            return ContextInfo();
        }

        SkASSERT(masterContext->fType == type);
    }

    std::unique_ptr<TestContext> testCtx;
    GrBackendContext backendContext = 0;
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
                    SkASSERT(!masterContext);   // Shared contexts not supported yet
                    glCtx = MakeANGLETestContext(ANGLEBackend::kD3D9, ANGLEContextVersion::kES2).release();
                    break;
                case kANGLE_D3D11_ES2_ContextType:
                    SkASSERT(!masterContext);   // Shared contexts not supported yet
                    glCtx = MakeANGLETestContext(ANGLEBackend::kD3D11, ANGLEContextVersion::kES2).release();
                    break;
                case kANGLE_D3D11_ES3_ContextType:
                    SkASSERT(!masterContext);   // Shared contexts not supported yet
                    glCtx = MakeANGLETestContext(ANGLEBackend::kD3D11, ANGLEContextVersion::kES3).release();
                    break;
                case kANGLE_GL_ES2_ContextType:
                    SkASSERT(!masterContext);   // Shared contexts not supported yet
                    glCtx = MakeANGLETestContext(ANGLEBackend::kOpenGL, ANGLEContextVersion::kES2).release();
                    break;
                case kANGLE_GL_ES3_ContextType:
                    SkASSERT(!masterContext);   // Shared contexts not supported yet
                    glCtx = MakeANGLETestContext(ANGLEBackend::kOpenGL, ANGLEContextVersion::kES3).release();
                    break;
#endif
                case kCommandBuffer_ContextType:
                    SkASSERT(!masterContext);   // Shared contexts not supported yet
                    glCtx = CommandBufferGLTestContext::Create();
                    break;
#if SK_MESA
                case kMESA_ContextType:
                    SkASSERT(!masterContext);   // Shared contexts not supported yet
                    glCtx = CreateMesaGLTestContext();
                    break;
#endif
                case kNullGL_ContextType:
                    SkASSERT(!masterContext);   // Shared contexts not supported yet
                    glCtx = CreateNullGLTestContext(ContextOptions::kEnableNVPR & options);
                    break;
                case kDebugGL_ContextType:
                    SkASSERT(!masterContext);   // Shared contexts not supported yet
                    glCtx = CreateDebugGLTestContext();
                    break;
                default:
                    return ContextInfo();
            }
            if (!glCtx) {
                return ContextInfo();
            }
            testCtx.reset(glCtx);
            sk_sp<const GrGLInterface> glInterface(SkRef(glCtx->gl()));
            // Block NVPR from non-NVPR types. We don't block NVPR from contexts that will use
            // instanced rendering because that would prevent us from testing mixed samples.
            if (!((ContextOptions::kEnableNVPR | ContextOptions::kUseInstanced) & options)) {
                glInterface.reset(GrGLInterfaceRemoveNVPR(glInterface.get()));
                if (!glInterface) {
                    return ContextInfo();
                }
            }
            backendContext = reinterpret_cast<GrBackendContext>(glInterface.get());
            break;
        }
#ifdef SK_VULKAN
        case kVulkan_GrBackend:
            SkASSERT(!masterContext);   // Shared contexts not supported yet
            SkASSERT(kVulkan_ContextType == type);
            if (ContextOptions::kEnableNVPR & options) {
                return ContextInfo();
            }
            testCtx.reset(CreatePlatformVkTestContext());
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
#endif
        default:
            return ContextInfo();
    }
    testCtx->makeCurrent();
    SkASSERT(testCtx && testCtx->backend() == backend);
    GrContextOptions grOptions = fGlobalOptions;
    if (ContextOptions::kUseInstanced & options) {
        grOptions.fEnableInstancedRendering = true;
    }
    grOptions.fRequireDecodeDisableForSRGB =
        SkToBool(ContextOptions::kRequireSRGBDecodeDisableSupport & options);
    sk_sp<GrContext> grCtx(GrContext::Create(backend, backendContext, grOptions));
    if (!grCtx.get()) {
        return ContextInfo();
    }
    if (ContextOptions::kEnableNVPR & options) {
        if (!grCtx->caps()->shaderCaps()->pathRenderingSupport()) {
            return ContextInfo();
        }
    }
    if (ContextOptions::kUseInstanced & options) {
        if (GrCaps::InstancedSupport::kNone == grCtx->caps()->instancedSupport()) {
            return ContextInfo();
        }
    }
    if (ContextOptions::kRequireSRGBSupport & options) {
        if (!grCtx->caps()->srgbSupport()) {
            return ContextInfo();
        }
    }

    Context& context = fContexts.push_back();
    context.fBackend = backend;
    context.fTestContext = testCtx.release();
    context.fGrContext = SkRef(grCtx.get());
    context.fType = type;
    context.fOptions = options;
    context.fAbandoned = false;
    context.fShareContext = shareContext;
    context.fShareIndex = shareIndex;
    return ContextInfo(context.fBackend, context.fTestContext, context.fGrContext);
}

}  // namespace sk_gpu_test
