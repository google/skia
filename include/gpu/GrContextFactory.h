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
#include "SkTArray.h"

/**
 * This is a simple class that is useful in test apps that use different
 * GrContexts backed by different types of GL contexts. It manages creating the
 * GL context and a GrContext that uses it. The GL/Gr contexts persist until the
 * factory is destroyed (though the caller can always grab a ref on the returned
 * GrContext to make it outlive the factory).
 */
class GrContextFactory : GrNoncopyable {
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

      kLastGLContextType = kDebug_GLContextType
    };

    static const int kGLContextTypeCnt = kLastGLContextType + 1;

    static bool IsRenderingGLContext(GLContextType type) {
        switch (type) {
            case kNull_GLContextType:
            case kDebug_GLContextType:
                return false;
            default:
                return true;
        }
    }

    static const char* GLContextTypeName(GLContextType type) {
        switch (type) {
            case kNative_GLContextType:
                return "native";
            case kNull_GLContextType:
                return "null";
#if SK_ANGLE
            case kANGLE_GLContextType:
                return "angle";
#endif
#if SK_MESA
            case kMESA_GLContextType:
                return "mesa";
#endif
            case kDebug_GLContextType:
                return "debug";
            default:
                GrCrash("Unknown GL Context type.");
        }
    }

    GrContextFactory() {
    }

    ~GrContextFactory() { this->destroyContexts(); }

    void destroyContexts() {
        for (int i = 0; i < fContexts.count(); ++i) {
            fContexts[i].fGrContext->unref();
            fContexts[i].fGLContext->unref();
        }
        fContexts.reset();
    }

    /**
     * Get a GrContext initialized with a type of GL context. It also makes the GL context current.
     */
    GrContext* get(GLContextType type) {

        for (int i = 0; i < fContexts.count(); ++i) {
            if (fContexts[i].fType == type) {
                fContexts[i].fGLContext->makeCurrent();
                return fContexts[i].fGrContext;
            }
        }
        SkAutoTUnref<SkGLContextHelper> glCtx;
        SkAutoTUnref<GrContext> grCtx;
        switch (type) {
            case kNative_GLContextType:
                glCtx.reset(SkNEW(SkNativeGLContext));
                break;
#ifdef SK_ANGLE
            case kANGLE_GLContextType:
                glCtx.reset(SkNEW(SkANGLEGLContext));
                break;
#endif
#ifdef SK_MESA
            case kMESA_GLContextType:
                glCtx.reset(SkNEW(SkMesaGLContext));
                break;
#endif
            case kNull_GLContextType:
                glCtx.reset(SkNEW(SkNullGLContext));
                break;
            case kDebug_GLContextType:
                glCtx.reset(SkNEW(SkDebugGLContext));
                break;
        }
        static const int kBogusSize = 1;
        if (!glCtx.get()) {
            return NULL;
        }
        if (!glCtx.get()->init(kBogusSize, kBogusSize)) {
            return NULL;
        }
        GrBackendContext p3dctx = reinterpret_cast<GrBackendContext>(glCtx.get()->gl());
        grCtx.reset(GrContext::Create(kOpenGL_GrBackend, p3dctx));
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

    // Returns the GLContext of the given type. If it has not been created yet,
    // NULL is returned instead.
    SkGLContextHelper* getGLContext(GLContextType type) {
        for (int i = 0; i < fContexts.count(); ++i) {
            if (fContexts[i].fType == type) {
                return fContexts[i].fGLContext;
            }
        }

        return NULL;
    }

private:
    struct GPUContext {
        GLContextType             fType;
        SkGLContextHelper*        fGLContext;
        GrContext*                fGrContext;
    };
    SkTArray<GPUContext, true> fContexts;
};

#endif
