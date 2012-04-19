/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrContextFactory_DEFINED
#define GrContextFactory_DEFINED

#if SK_ANGLE
    #include "gl/SkANGLEGLContext.h"
#endif
#include "gl/SkDebugGLContext.h"
#if SK_MESA
    #include "gl/SkMesaGLContext.h"
#endif
#include "gl/SkNativeGLContext.h"
#include "gl/SkNullGLContext.h"

#include "GrContext.h"

/**
 * This is a simple class that is useful in test apps that use different 
 * GrContexts backed by different types of GL contexts. It manages creating the
 * GL context and a GrContext that uses it. The GL/Gr contexts persist until the
 * factory is destroyed (though the caller can always grab a ref on the returned
 * GrContext to make it outlive the factory).
 */
class GrContextFactory  : GrNoncopyable {
public:
    /**
     * Types of GL contexts supported.
     */
    enum GLContextType {
      kNative_GLContextType,
#if SK_ANGLE
      kANGLE_GLContextType,
#endif
#if SK_MESA
      kMESA_GLContextType,
#endif
      kNull_GLContextType,
      kDebug_GLContextType,
    };

    GrContextFactory() {
    }

    ~GrContextFactory() {
        for (int i = 0; i < fContexts.count(); ++i) {
            fContexts[i].fGrContext->unref();
            fContexts[i].fGLContext->unref();
        }
    }

    /**
     * Get a GrContext initalized with a type of GL context.
     */
    GrContext* get(GLContextType type) {

        for (int i = 0; i < fContexts.count(); ++i) {
            if (fContexts[i].fType == type) {
                return fContexts[i].fGrContext;
            }
        }
        SkAutoTUnref<SkGLContext> glCtx;
        SkAutoTUnref<GrContext> grCtx;
        switch (type) {
            case kNative_GLContextType:
                glCtx.reset(new SkNativeGLContext());
                break;
#ifdef SK_ANGLE
            case kANGLE_GLContextType:
                glCtx.reset(new SkANGLEGLContext());
                break;
#endif
#ifdef SK_MESA
            case kMESA_GLContextType:
                glCtx.reset(new SkMesaGLContext());
                break;
#endif
            case kNull_GLContextType:
                glCtx.reset(new SkNullGLContext());
                break;
            case kDebug_GLContextType:
                glCtx.reset(new SkDebugGLContext());
                break;
        }
        static const int kBogusSize = 1;
        if (!glCtx.get()) {
            return NULL;
        }
        if (!glCtx.get()->init(kBogusSize, kBogusSize)) {
            return NULL;
        }
        GrPlatform3DContext p3dctx =
            reinterpret_cast<GrPlatform3DContext>(glCtx.get()->gl());
        grCtx.reset(GrContext::Create(kOpenGL_Shaders_GrEngine, p3dctx));
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
private:
    struct GPUContext {
        GLContextType             fType;
        SkGLContext*              fGLContext;
        GrContext*                fGrContext;
    };
    SkTArray<GPUContext, true> fContexts;
};

#endif
