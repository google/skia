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
 * Gr and GL contexts to make them outlive the factory).
 */
class GrContextFactory : public SkNoncopyable {
public:
    /**
     * Types of GL contexts supported. For historical and testing reasons the native GrContext will
     * not use "GL_NV_path_rendering" even when the driver supports it. There is a separate context
     * type that does not remove NVPR support and which will fail when the driver does not support
     * the extension.
     */
    enum GLContextType {
      kNative_GLContextType,
#if SK_ANGLE
      kANGLE_GLContextType,
#endif
#if SK_MESA
      kMESA_GLContextType,
#endif
      /** Similar to kNative but does not filter NVPR. It will fail if the GL driver does not
          support NVPR */
      kNVPR_GLContextType,
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
            case kNVPR_GLContextType:
                return "nvpr";
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
            fContexts[i].fGLContext->makeCurrent();
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
            case kNVPR_GLContextType: // fallthru
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

        // Ensure NVPR is available for the NVPR type and block it from other types.
        SkAutoTUnref<const GrGLInterface> glInterface(SkRef(glCtx.get()->gl()));
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
