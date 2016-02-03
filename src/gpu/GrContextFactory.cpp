
/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrContextFactory.h"

#if SK_ANGLE
    #include "gl/angle/SkANGLEGLContext.h"
#endif
#if SK_COMMAND_BUFFER
    #include "gl/command_buffer/SkCommandBufferGLContext.h"
#endif
#include "gl/debug/SkDebugGLContext.h"
#if SK_MESA
    #include "gl/mesa/SkMesaGLContext.h"
#endif
#include "gl/SkGLContext.h"
#include "gl/SkNullGLContext.h"
#include "gl/GrGLGpu.h"
#include "GrCaps.h"

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
    SkAutoTDelete<SkGLContext> glCtx;
    SkAutoTUnref<GrContext> grCtx;
    switch (type) {
        case kNative_GLContextType:
            glCtx.reset(SkCreatePlatformGLContext(kNone_GrGLStandard));
            break;
        case kGL_GLContextType:
            glCtx.reset(SkCreatePlatformGLContext(kGL_GrGLStandard));
            break;
        case kGLES_GLContextType:
            glCtx.reset(SkCreatePlatformGLContext(kGLES_GrGLStandard));
            break;
#if SK_ANGLE
#ifdef SK_BUILD_FOR_WIN
        case kANGLE_GLContextType:
            glCtx.reset(SkANGLEGLContext::CreateDirectX());
            break;
#endif
        case kANGLE_GL_GLContextType:
            glCtx.reset(SkANGLEGLContext::CreateOpenGL());
            break;
#endif
#if SK_COMMAND_BUFFER
        case kCommandBuffer_GLContextType:
            glCtx.reset(SkCommandBufferGLContext::Create());
            break;
#endif
#if SK_MESA
        case kMESA_GLContextType:
            glCtx.reset(SkMesaGLContext::Create());
            break;
#endif
        case kNull_GLContextType:
            glCtx.reset(SkNullGLContext::Create());
            break;
        case kDebug_GLContextType:
            glCtx.reset(SkDebugGLContext::Create());
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
    GrBackendContext p3dctx = reinterpret_cast<GrBackendContext>(glInterface.get());
#ifdef SK_VULKAN
    grCtx.reset(GrContext::Create(kVulkan_GrBackend, p3dctx, fGlobalOptions));
#else
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

    Context& context = fContexts.push_back();
    context.fGLContext = glCtx.detach();
    context.fGrContext = SkRef(grCtx.get());
    context.fType = type;
    context.fOptions = options;
    return ContextInfo(context.fGrContext, context.fGLContext);
}
