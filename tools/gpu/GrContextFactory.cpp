
/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrDirectContext.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "tools/gpu/GrContextFactory.h"
#ifdef SK_GL
#include "tools/gpu/gl/GLTestContext.h"
#endif

#if SK_ANGLE
#include "tools/gpu/gl/angle/GLTestContext_angle.h"
#endif
#ifdef SK_VULKAN
#include "tools/gpu/vk/VkTestContext.h"
#endif
#ifdef SK_METAL
#include "tools/gpu/mtl/MtlTestContext.h"
#endif
#ifdef SK_DIRECT3D
#include "tools/gpu/d3d/D3DTestContext.h"
#endif
#include "src/gpu/ganesh/GrCaps.h"
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

bool gCreateProtectedContext = false;

namespace sk_gpu_test {
GrContextFactory::GrContextFactory() {}

GrContextFactory::GrContextFactory(const GrContextOptions& opts)
    : fGlobalOptions(opts) {}

GrContextFactory::~GrContextFactory() {
    this->destroyContexts();
}

void GrContextFactory::destroyContexts() {
    // We must delete the test contexts in reverse order so that any child context is finished and
    // deleted before a parent context. This relies on the fact that when we make a new context we
    // append it to the end of fContexts array.
    // TODO: Look into keeping a dependency dag for contexts and deletion order
    for (int i = fContexts.size() - 1; i >= 0; --i) {
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
    fContexts.clear();
}

void GrContextFactory::abandonContexts() {
    // We must abandon the test contexts in reverse order so that any child context is finished and
    // abandoned before a parent context. This relies on the fact that when we make a new context we
    // append it to the end of fContexts array.
    // TODO: Look into keeping a dependency dag for contexts and deletion order
    for (int i = fContexts.size() - 1; i >= 0; --i) {
        Context& context = fContexts[i];
        if (!context.fAbandoned) {
            if (context.fTestContext) {
                auto restore = context.fTestContext->makeCurrentAndAutoRestore();
                context.fTestContext->testAbandon();
            }
            GrBackendApi api = context.fGrContext->backend();
            bool requiresEarlyAbandon = api == GrBackendApi::kVulkan;
            if (requiresEarlyAbandon) {
                context.fGrContext->abandonContext();
            }
            if (context.fTestContext) {
                delete(context.fTestContext);
                context.fTestContext = nullptr;
            }
            if (!requiresEarlyAbandon) {
                context.fGrContext->abandonContext();
            }
            context.fAbandoned = true;
        }
    }
}

void GrContextFactory::releaseResourcesAndAbandonContexts() {
    // We must abandon the test contexts in reverse order so that any child context is finished and
    // abandoned before a parent context. This relies on the fact that when we make a new context we
    // append it to the end of fContexts array.
    // TODO: Look into keeping a dependency dag for contexts and deletion order
    for (int i = fContexts.size() - 1; i >= 0; --i) {
        Context& context = fContexts[i];
        SkScopeExit restore(nullptr);
        if (!context.fAbandoned) {
            if (context.fTestContext) {
                restore = context.fTestContext->makeCurrentAndAutoRestore();
            }
            context.fGrContext->releaseResourcesAndAbandonContext();
            if (context.fTestContext) {
                delete context.fTestContext;
                context.fTestContext = nullptr;
            }
            context.fAbandoned = true;
        }
    }
}

GrDirectContext* GrContextFactory::get(ContextType type, ContextOverrides overrides) {
    return this->getContextInfo(type, overrides).directContext();
}

ContextInfo GrContextFactory::getContextInfoInternal(ContextType type, ContextOverrides overrides,
                                                     GrDirectContext* shareContext,
                                                     uint32_t shareIndex) {
    // (shareIndex != 0) -> (shareContext != nullptr)
    SkASSERT((shareIndex == 0) || (shareContext != nullptr));

    for (int i = 0; i < fContexts.size(); ++i) {
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

    // If we're trying to create a context in a share group, find the primary context
    Context* primaryContext = nullptr;
    if (shareContext) {
        for (int i = 0; i < fContexts.size(); ++i) {
            if (!fContexts[i].fAbandoned && fContexts[i].fGrContext == shareContext) {
                primaryContext = &fContexts[i];
                break;
            }
        }
        SkASSERT(primaryContext && primaryContext->fType == type);
    }

    std::unique_ptr<TestContext> testCtx;
    GrBackendApi backend = skgpu::ganesh::ContextTypeBackend(type);
    switch (backend) {
#ifdef SK_GL
        case GrBackendApi::kOpenGL: {
            GLTestContext* glShareContext = primaryContext
                    ? static_cast<GLTestContext*>(primaryContext->fTestContext) : nullptr;
            GLTestContext* glCtx;
            switch (type) {
                case ContextType::kGL:
                    glCtx = CreatePlatformGLTestContext(kGL_GrGLStandard, glShareContext);
                    break;
                case ContextType::kGLES:
                    glCtx = CreatePlatformGLTestContext(kGLES_GrGLStandard, glShareContext);
                    break;
#if SK_ANGLE
                case ContextType::kANGLE_D3D9_ES2:
                    glCtx = MakeANGLETestContext(ANGLEBackend::kD3D9, ANGLEContextVersion::kES2,
                                                 glShareContext).release();
                    // Chrome will only run on D3D9 with NVIDIA for 2012 and earlier drivers.
                    // (<= 269.73). We get shader link failures when testing on recent drivers
                    // using this backend.
                    if (glCtx) {
                        GrGLDriverInfo info = GrGLGetDriverInfo(glCtx->gl());
                        if (info.fANGLEVendor == GrGLVendor::kNVIDIA) {
                            delete glCtx;
                            return ContextInfo();
                        }
                    }
                    break;
                case ContextType::kANGLE_D3D11_ES2:
                    glCtx = MakeANGLETestContext(ANGLEBackend::kD3D11, ANGLEContextVersion::kES2,
                                                 glShareContext).release();
                    break;
                case ContextType::kANGLE_D3D11_ES3:
                    glCtx = MakeANGLETestContext(ANGLEBackend::kD3D11, ANGLEContextVersion::kES3,
                                                 glShareContext).release();
                    break;
                case ContextType::kANGLE_GL_ES2:
                    glCtx = MakeANGLETestContext(ANGLEBackend::kOpenGL, ANGLEContextVersion::kES2,
                                                 glShareContext).release();
                    break;
                case ContextType::kANGLE_GL_ES3:
                    glCtx = MakeANGLETestContext(ANGLEBackend::kOpenGL, ANGLEContextVersion::kES3,
                                                 glShareContext).release();
                    break;
                case ContextType::kANGLE_Metal_ES2:
                    glCtx = MakeANGLETestContext(ANGLEBackend::kMetal, ANGLEContextVersion::kES2,
                                                 glShareContext).release();
                    break;
                case ContextType::kANGLE_Metal_ES3:
                    glCtx = MakeANGLETestContext(ANGLEBackend::kMetal, ANGLEContextVersion::kES3,
                                                 glShareContext).release();
                    break;
#endif
                default:
                    return ContextInfo();
            }
            if (!glCtx) {
                return ContextInfo();
            }
            if (glCtx->gl()->fStandard == kGLES_GrGLStandard &&
                (overrides & ContextOverrides::kFakeGLESVersionAs2)) {
                glCtx->overrideVersion("OpenGL ES 2.0", "OpenGL ES GLSL ES 1.00");
            }
            testCtx.reset(glCtx);
            break;
        }
#endif  // SK_GL
#ifdef SK_VULKAN
        case GrBackendApi::kVulkan: {
            VkTestContext* vkSharedContext = primaryContext
                    ? static_cast<VkTestContext*>(primaryContext->fTestContext) : nullptr;
            SkASSERT(ContextType::kVulkan == type);
            testCtx.reset(CreatePlatformVkTestContext(vkSharedContext));
            if (!testCtx) {
                return ContextInfo();
            }
#ifdef SK_GL
            // We previously had an issue where the VkDevice destruction would occasionally hang
            // on systems with NVIDIA GPUs and having an existing GL context fixed it. Now (Feb
            // 2022) we still need the GL context to keep Vulkan/TSAN bots from running incredibly
            // slow. Perhaps this prevents repeated driver loading/unloading? Note that keeping
            // a persistent VkTestContext around instead was tried and did not work.
            if (!fSentinelGLContext) {
                fSentinelGLContext.reset(CreatePlatformGLTestContext(kGL_GrGLStandard));
                if (!fSentinelGLContext) {
                    fSentinelGLContext.reset(CreatePlatformGLTestContext(kGLES_GrGLStandard));
                }
            }
#endif
            break;
        }
#endif
#ifdef SK_METAL
        case GrBackendApi::kMetal: {
            MtlTestContext* mtlSharedContext = primaryContext
                    ? static_cast<MtlTestContext*>(primaryContext->fTestContext) : nullptr;
            SkASSERT(ContextType::kMetal == type);
            testCtx.reset(CreatePlatformMtlTestContext(mtlSharedContext));
            if (!testCtx) {
                return ContextInfo();
            }
            break;
        }
#endif
#ifdef SK_DIRECT3D
        case GrBackendApi::kDirect3D: {
            D3DTestContext* d3dSharedContext = primaryContext
                    ? static_cast<D3DTestContext*>(primaryContext->fTestContext) : nullptr;
            SkASSERT(ContextType::kDirect3D == type);
            testCtx.reset(CreatePlatformD3DTestContext(d3dSharedContext));
            if (!testCtx) {
                return ContextInfo();
            }
            break;
        }
#endif
        case GrBackendApi::kMock: {
            TestContext* sharedContext = primaryContext ? primaryContext->fTestContext : nullptr;
            SkASSERT(ContextType::kMock == type);
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
    if (ContextOverrides::kReducedShaders & overrides) {
        grOptions.fReducedShaderVariations = true;
    }
    sk_sp<GrDirectContext> grCtx;
    {
        auto restore = testCtx->makeCurrentAndAutoRestore();
        grCtx = testCtx->makeContext(grOptions);
    }
    if (!grCtx) {
        return ContextInfo();
    }

    if (shareContext) {
        SkASSERT(grCtx->directContextID() != shareContext->directContextID());
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

ContextInfo GrContextFactory::getSharedContextInfo(GrDirectContext* shareContext,
                                                   uint32_t shareIndex) {
    SkASSERT(shareContext);
    for (int i = 0; i < fContexts.size(); ++i) {
        if (!fContexts[i].fAbandoned && fContexts[i].fGrContext == shareContext) {
            return this->getContextInfoInternal(fContexts[i].fType, fContexts[i].fOverrides,
                                                shareContext, shareIndex);
        }
    }

    return ContextInfo();
}

}  // namespace sk_gpu_test
