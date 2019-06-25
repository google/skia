
/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrContextPriv.h"
#include "tools/gpu/GrContextFactory.h"
#include "tools/gpu/gl/GLTestContext.h"

#if SK_ANGLE
    #include "tools/gpu/gl/angle/GLTestContext_angle.h"
#endif
#include "tools/gpu/gl/command_buffer/GLTestContext_command_buffer.h"
#ifdef SK_VULKAN
#include "tools/gpu/vk/VkTestContext.h"
#endif
#ifdef SK_METAL
#include "tools/gpu/mtl/MtlTestContext.h"
#endif
#include "src/gpu/GrCaps.h"
#include "src/gpu/gl/GrGLGpu.h"
#include "tools/gpu/mock/MockTestContext.h"

#if defined(SK_BUILD_FOR_WIN) && defined(SK_ENABLE_DISCRETE_GPU)
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
    // We must delete the test contexts in reverse order so that any child context is finished and
    // deleted before a parent context. This relies on the fact that when we make a new context we
    // append it to the end of fContexts array.
    // TODO: Look into keeping a dependency dag for contexts and deletion order
    for (int i = fContexts.count() - 1; i >= 0; --i) {
        Context& context = fContexts[i];
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
    // We must abandon the test contexts in reverse order so that any child context is finished and
    // abandoned before a parent context. This relies on the fact that when we make a new context we
    // append it to the end of fContexts array.
    // TODO: Look into keeping a dependency dag for contexts and deletion order
    for (int i = fContexts.count() - 1; i >= 0; --i) {
        Context& context = fContexts[i];
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
    // We must abandon the test contexts in reverse order so that any child context is finished and
    // abandoned before a parent context. This relies on the fact that when we make a new context we
    // append it to the end of fContexts array.
    // TODO: Look into keeping a dependency dag for contexts and deletion order
    for (int i = fContexts.count() - 1; i >= 0; --i) {
        Context& context = fContexts[i];
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

    std::unique_ptr<TestContext> testCtx;
    GrBackendApi backend = ContextTypeBackend(type);
    switch (backend) {
        case GrBackendApi::kOpenGL: {
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
                default:
                    return ContextInfo();
            }
            if (!glCtx) {
                return ContextInfo();
            }
            testCtx.reset(glCtx);
            break;
        }
#ifdef SK_VULKAN
        case GrBackendApi::kVulkan: {
            VkTestContext* vkSharedContext = masterContext
                    ? static_cast<VkTestContext*>(masterContext->fTestContext) : nullptr;
            SkASSERT(kVulkan_ContextType == type);
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
            break;
        }
#endif
#ifdef SK_METAL
        case GrBackendApi::kMetal: {
            MtlTestContext* mtlSharedContext = masterContext
                    ? static_cast<MtlTestContext*>(masterContext->fTestContext) : nullptr;
            SkASSERT(kMetal_ContextType == type);
            testCtx.reset(CreatePlatformMtlTestContext(mtlSharedContext));
            if (!testCtx) {
                return ContextInfo();
            }
            break;
        }
#endif
        case GrBackendApi::kMock: {
            TestContext* sharedContext = masterContext ? masterContext->fTestContext : nullptr;
            SkASSERT(kMock_ContextType == type);
            testCtx.reset(CreateMockTestContext(sharedContext));
            if (!testCtx) {
                return ContextInfo();
            }
            break;
        }
        default:
            return ContextInfo();
    }

    SkASSERT(testCtx && testCtx->backend() == backend);
    GrContextOptions grOptions = fGlobalOptions;
    if (ContextOverrides::kAvoidStencilBuffers & overrides) {
        grOptions.fAvoidStencilBuffers = true;
    }
    sk_sp<GrContext> grCtx;
    {
        auto restore = testCtx->makeCurrentAndAutoRestore();
        grCtx = testCtx->makeGrContext(grOptions);
    }
    if (!grCtx.get()) {
        return ContextInfo();
    }

    // We must always add new contexts by pushing to the back so that when we delete them we delete
    // them in reverse order in which they were made.
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

}  // namespace sk_gpu_test
