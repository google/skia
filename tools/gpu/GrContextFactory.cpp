
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
#if SK_COMMAND_BUFFER
    #include "gl/command_buffer/GLTestContext_command_buffer.h"
#endif
#include "gl/debug/DebugGLTestContext.h"
#if SK_MESA
    #include "gl/mesa/GLTestContext_mesa.h"
#endif
#if SK_VULKAN
#include "vk/GrVkBackendContext.h"
#endif
#include "gl/null/NullGLTestContext.h"
#include "gl/GrGLGpu.h"
#include "GrCaps.h"

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
        if (context.fGLContext) {
            context.fGLContext->makeCurrent();
        }
        if (!context.fGrContext->unique()) {
            context.fGrContext->abandonContext();
        }
        context.fGrContext->unref();
        delete(context.fGLContext);
    }
    fContexts.reset();
}

void GrContextFactory::abandonContexts() {
    for (Context& context : fContexts) {
        if (context.fGLContext) {
            context.fGLContext->makeCurrent();
            context.fGLContext->testAbandon();
            delete(context.fGLContext);
            context.fGLContext = nullptr;
        }
        context.fGrContext->abandonContext();
    }
}

GrContextFactory::ContextInfo GrContextFactory::getContextInfo(GLContextType type,
                                                               GLContextOptions options) {
    for (int i = 0; i < fContexts.count(); ++i) {
        Context& context = fContexts[i];
        if (!context.fGLContext) {
            continue;
        }
        if (context.fType == type &&
            context.fOptions == options) {
            context.fGLContext->makeCurrent();
            return ContextInfo(context.fGrContext, context.fGLContext);
        }
    }
    SkAutoTDelete<GLTestContext> glCtx;
    SkAutoTUnref<GrContext> grCtx;
    switch (type) {
        case kNative_GLContextType:
            glCtx.reset(CreatePlatformGLTestContext(kNone_GrGLStandard));
            break;
        case kGL_GLContextType:
            glCtx.reset(CreatePlatformGLTestContext(kGL_GrGLStandard));
            break;
        case kGLES_GLContextType:
            glCtx.reset(CreatePlatformGLTestContext(kGLES_GrGLStandard));
            break;
#if SK_ANGLE
#ifdef SK_BUILD_FOR_WIN
        case kANGLE_GLContextType:
            glCtx.reset(CreateANGLEDirect3DGLTestContext());
            break;
#endif
        case kANGLE_GL_GLContextType:
            glCtx.reset(CreateANGLEOpenGLGLTestContext());
            break;
#endif
#if SK_COMMAND_BUFFER
        case kCommandBuffer_GLContextType:
            glCtx.reset(CommandBufferGLTestContext::Create());
            break;
#endif
#if SK_MESA
        case kMESA_GLContextType:
            glCtx.reset(CreateMesaGLTestContext());
            break;
#endif
        case kNull_GLContextType:
            glCtx.reset(CreateNullGLTestContext());
            break;
        case kDebug_GLContextType:
            glCtx.reset(CreateDebugGLTestContext());
            break;
    }
    if (nullptr == glCtx.get()) {
        return ContextInfo();
    }

    SkASSERT(glCtx->isValid());

    // Block NVPR from non-NVPR types.
    SkAutoTUnref<const GrGLInterface> glInterface(SkRef(glCtx->gl()));
    if (!(kEnableNVPR_GLContextOptions & options)) {
        glInterface.reset(GrGLInterfaceRemoveNVPR(glInterface));
        if (!glInterface) {
            return ContextInfo();
        }
    }

    glCtx->makeCurrent();
#ifdef SK_VULKAN
    if (kEnableNVPR_GLContextOptions & options) {
        return ContextInfo();
    } else {
        GrBackendContext p3dctx = reinterpret_cast<GrBackendContext>(GrVkBackendContext::Create());
        grCtx.reset(GrContext::Create(kVulkan_GrBackend, p3dctx, fGlobalOptions));
    }
#else
    GrBackendContext p3dctx = reinterpret_cast<GrBackendContext>(glInterface.get());
    grCtx.reset(GrContext::Create(kOpenGL_GrBackend, p3dctx, fGlobalOptions));
#endif
    if (!grCtx.get()) {
        return ContextInfo();
    }
    if (kEnableNVPR_GLContextOptions & options) {
        if (!grCtx->caps()->shaderCaps()->pathRenderingSupport()) {
            return ContextInfo();
        }
    }
    if (kRequireSRGBSupport_GLContextOptions & options) {
        if (!grCtx->caps()->srgbSupport()) {
            return ContextInfo();
        }
    }

    Context& context = fContexts.push_back();
    context.fGLContext = glCtx.release();
    context.fGrContext = SkRef(grCtx.get());
    context.fType = type;
    context.fOptions = options;
    return ContextInfo(context.fGrContext, context.fGLContext);
}
}  // namespace sk_gpu_test
