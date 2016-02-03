/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrContextFactory_DEFINED
#define GrContextFactory_DEFINED

#include "GrContext.h"
#include "GrContextOptions.h"

#include "gl/SkGLContext.h"
#include "SkTArray.h"

/**
 * This is a simple class that is useful in test apps that use different
 * GrContexts backed by different types of GL contexts. It manages creating the
 * GL context and a GrContext that uses it. The GL/Gr contexts persist until the
 * factory is destroyed (though the caller can always grab a ref on the returned
 * Gr and GL contexts to make them outlive the factory).
 */
class GrContextFactory : SkNoncopyable {
public:
    enum GLContextType {
        kNative_GLContextType,  //! OpenGL or OpenGL ES context.
        kGL_GLContextType,      //! OpenGL context.
        kGLES_GLContextType,    //! OpenGL ES context.
#if SK_ANGLE
#ifdef SK_BUILD_FOR_WIN
        kANGLE_GLContextType,    //! ANGLE on DirectX OpenGL ES context.
#endif
        kANGLE_GL_GLContextType, //! ANGLE on OpenGL OpenGL ES context.
#endif
#if SK_COMMAND_BUFFER
        kCommandBuffer_GLContextType, //! Chromium command buffer OpenGL ES context.
#endif
#if SK_MESA
        kMESA_GLContextType,  //! MESA OpenGL context
#endif
        kNull_GLContextType,  //! Non-rendering OpenGL mock context.
        kDebug_GLContextType, //! Non-rendering, state verifying OpenGL context.
        kLastGLContextType = kDebug_GLContextType
    };

    static const int kGLContextTypeCnt = kLastGLContextType + 1;

    /**
     * Options for GL context creation. For historical and testing reasons the options will default
     * to not using GL_NV_path_rendering extension  even when the driver supports it.
     */
    enum GLContextOptions {
        kNone_GLContextOptions = 0,
        kEnableNVPR_GLContextOptions = 0x1,
    };

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
            case kGL_GLContextType:
                return "gl";
            case kGLES_GLContextType:
                return "gles";
#if SK_ANGLE
#ifdef SK_BUILD_FOR_WIN
            case kANGLE_GLContextType:
                return "angle";
#endif
            case kANGLE_GL_GLContextType:
                return "angle-gl";
#endif
#if SK_COMMAND_BUFFER
            case kCommandBuffer_GLContextType:
                return "commandbuffer";
#endif
#if SK_MESA
            case kMESA_GLContextType:
                return "mesa";
#endif
            case kNull_GLContextType:
                return "null";
            case kDebug_GLContextType:
                return "debug";
            default:
                SkFAIL("Unknown GL Context type.");
        }
    }

    explicit GrContextFactory(const GrContextOptions& opts);
    GrContextFactory();

    ~GrContextFactory();

    void destroyContexts();
    void abandonContexts();

    struct ContextInfo {
        ContextInfo()
            : fGrContext(nullptr), fGLContext(nullptr) { }
        ContextInfo(GrContext* grContext, SkGLContext* glContext)
            : fGrContext(grContext), fGLContext(glContext) { }
        GrContext* fGrContext;
        SkGLContext* fGLContext; //! Valid until the factory destroys it via abandonContexts() or
                                 //! destroyContexts().
    };

    /**
     * Get a context initialized with a type of GL context. It also makes the GL context current.
     */
    ContextInfo getContextInfo(GLContextType type,
                               GLContextOptions options = kNone_GLContextOptions);
    /**
     * Get a GrContext initialized with a type of GL context. It also makes the GL context current.
     */
    GrContext* get(GLContextType type,
                   GLContextOptions options = kNone_GLContextOptions) {
        return this->getContextInfo(type, options).fGrContext;
    }
    const GrContextOptions& getGlobalOptions() const { return fGlobalOptions; }

private:
    struct Context {
        GLContextType fType;
        GLContextOptions fOptions;
        SkGLContext*  fGLContext;
        GrContext*    fGrContext;
    };
    SkTArray<Context, true> fContexts;
    const GrContextOptions  fGlobalOptions;
};

#endif
