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
        kNative_GLContextType,
#if SK_ANGLE
        kANGLE_GLContextType,
        kANGLE_GL_GLContextType,
#endif
#if SK_COMMAND_BUFFER
        kCommandBuffer_GLContextType,
#endif
#if SK_MESA
        kMESA_GLContextType,
#endif
        kNull_GLContextType,
        kDebug_GLContextType,
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
            case kNull_GLContextType:
                return "null";
#if SK_ANGLE
            case kANGLE_GLContextType:
                return "angle";
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
            case kDebug_GLContextType:
                return "debug";
            default:
                SkFAIL("Unknown GL Context type.");
        }
    }

    explicit GrContextFactory(const GrContextOptions& opts) : fGlobalOptions(opts) { }
    GrContextFactory() { }

    ~GrContextFactory() { this->destroyContexts(); }

    void destroyContexts() {
        for (int i = 0; i < fContexts.count(); ++i) {
            if (fContexts[i]->fGLContext) {  //  could be abandoned.
                fContexts[i]->fGLContext->makeCurrent();
            }
            fContexts[i]->fGrContext->unref();
            SkSafeUnref(fContexts[i]->fGLContext);
        }
        fContexts.reset();
    }

    void abandonContexts() {
        for (int i = 0; i < fContexts.count(); ++i) {
            if (fContexts[i]->fGLContext) {
                fContexts[i]->fGLContext->testAbandon();
                SkSafeSetNull(fContexts[i]->fGLContext);
            }
            fContexts[i]->fGrContext->abandonContext();
        }
    }

    struct ContextInfo {
        GLContextType             fType;
        GLContextOptions          fOptions;
        SkGLContext*              fGLContext;
        GrContext*                fGrContext;
    };
    /**
     * Get a context initialized with a type of GL context. It also makes the GL context current.
     * Pointer is valid until destroyContexts() is called.
     */
    ContextInfo* getContextInfo(GLContextType type, GrGLStandard forcedGpuAPI = kNone_GrGLStandard, GLContextOptions options = kNone_GLContextOptions);

    /**
     * Get a GrContext initialized with a type of GL context. It also makes the GL context current.
     */
    GrContext* get(GLContextType type, GrGLStandard forcedGpuAPI = kNone_GrGLStandard,
                   GLContextOptions options = kNone_GLContextOptions) {
        if (ContextInfo* info = this->getContextInfo(type, forcedGpuAPI, options)) {
            return info->fGrContext;
        }
        return nullptr;
    }
    const GrContextOptions& getGlobalOptions() const { return fGlobalOptions; }

private:
    SkTArray<SkAutoTDelete<ContextInfo>, true> fContexts;
    const GrContextOptions        fGlobalOptions;
};

#endif
