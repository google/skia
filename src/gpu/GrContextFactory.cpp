
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
#include "gl/debug/SkDebugGLContext.h"
#if SK_MESA
    #include "gl/mesa/SkMesaGLContext.h"
#endif
#include "gl/SkGLContext.h"
#include "gl/SkNullGLContext.h"


GrContext* GrContextFactory::get(GLContextType type, GrGLStandard forcedGpuAPI) {
    for (int i = 0; i < fContexts.count(); ++i) {
        if (forcedGpuAPI != kNone_GrGLStandard &&
            forcedGpuAPI != fContexts[i].fGLContext->gl()->fStandard)
            continue;

        if (fContexts[i].fType == type) {
            fContexts[i].fGLContext->makeCurrent();
            return fContexts[i].fGrContext;
        }
    }
    SkAutoTUnref<SkGLContext> glCtx;
    SkAutoTUnref<GrContext> grCtx;
    switch (type) {
        case kNVPR_GLContextType: // fallthru
        case kNative_GLContextType:
            glCtx.reset(SkCreatePlatformGLContext(forcedGpuAPI));
            break;
#ifdef SK_ANGLE
        case kANGLE_GLContextType:
            glCtx.reset(SkANGLEGLContext::Create(forcedGpuAPI));
            break;
#endif
#ifdef SK_MESA
        case kMESA_GLContextType:
            glCtx.reset(SkMesaGLContext::Create(forcedGpuAPI));
            break;
#endif
        case kNull_GLContextType:
            glCtx.reset(SkNullGLContext::Create(forcedGpuAPI));
            break;
        case kDebug_GLContextType:
            glCtx.reset(SkDebugGLContext::Create(forcedGpuAPI));
            break;
    }
    if (NULL == glCtx.get()) {
        return NULL;
    }

    SkASSERT(glCtx->isValid());

    // Ensure NVPR is available for the NVPR type and block it from other types.
    SkAutoTUnref<const GrGLInterface> glInterface(SkRef(glCtx->gl()));
    if (kNVPR_GLContextType == type) {
        if (!glInterface->hasExtension("GL_NV_path_rendering")) {
            return NULL;
        }
    } else {
        glInterface.reset(GrGLInterfaceRemoveNVPR(glInterface));
        if (!glInterface) {
            return NULL;
        }
    }

    glCtx->makeCurrent();
    GrBackendContext p3dctx = reinterpret_cast<GrBackendContext>(glInterface.get());
    grCtx.reset(GrContext::Create(kOpenGL_GrBackend, p3dctx, &fGlobalOptions));
    if (!grCtx.get()) {
        return NULL;
    }
    GPUContext& ctx = fContexts.push_back();
    ctx.fGLContext = glCtx.get();
    ctx.fGLContext->ref();
    ctx.fGrContext = grCtx.get();
    ctx.fGrContext->ref();
    ctx.fType = type;
    return ctx.fGrContext;
}
