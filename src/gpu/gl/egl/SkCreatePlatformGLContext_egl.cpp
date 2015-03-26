
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gl/SkGLContext.h"

#include <GLES2/gl2.h>
#include <EGL/egl.h>

namespace {

class EGLGLContext : public SkGLContext  {
public:
    EGLGLContext(GrGLStandard forcedGpuAPI);
    ~EGLGLContext() override;
    void makeCurrent() const override;
    void swapBuffers() const override;

private:
    void destroyGLContext();

    EGLContext fContext;
    EGLDisplay fDisplay;
    EGLSurface fSurface;
};

EGLGLContext::EGLGLContext(GrGLStandard forcedGpuAPI)
    : fContext(EGL_NO_CONTEXT)
    , fDisplay(EGL_NO_DISPLAY)
    , fSurface(EGL_NO_SURFACE) {
    static const EGLint kEGLContextAttribsForOpenGL[] = {
        EGL_NONE
    };

    static const EGLint kEGLContextAttribsForOpenGLES[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };

    static const struct {
        const EGLint* fContextAttribs;
        EGLenum fAPI;
        EGLint  fRenderableTypeBit;
        GrGLStandard fStandard;
    } kAPIs[] = {
        {   // OpenGL
            kEGLContextAttribsForOpenGL,
            EGL_OPENGL_API,
            EGL_OPENGL_BIT,
            kGL_GrGLStandard
        },
        {   // OpenGL ES. This seems to work for both ES2 and 3 (when available).
            kEGLContextAttribsForOpenGLES,
            EGL_OPENGL_ES_API,
            EGL_OPENGL_ES2_BIT,
            kGLES_GrGLStandard
        },
    };

    size_t apiLimit = SK_ARRAY_COUNT(kAPIs);
    size_t api = 0;
    if (forcedGpuAPI == kGL_GrGLStandard) {
        apiLimit = 1;
    } else if (forcedGpuAPI == kGLES_GrGLStandard) {
        api = 1;
    }
    SkASSERT(forcedGpuAPI == kNone_GrGLStandard || kAPIs[api].fStandard == forcedGpuAPI);

    for (; NULL == fGL.get() && api < apiLimit; ++api) {
        fDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);

        EGLint majorVersion;
        EGLint minorVersion;
        eglInitialize(fDisplay, &majorVersion, &minorVersion);

#if 0
        SkDebugf("VENDOR: %s\n", eglQueryString(fDisplay, EGL_VENDOR));
        SkDebugf("APIS: %s\n", eglQueryString(fDisplay, EGL_CLIENT_APIS));
        SkDebugf("VERSION: %s\n", eglQueryString(fDisplay, EGL_VERSION));
        SkDebugf("EXTENSIONS %s\n", eglQueryString(fDisplay, EGL_EXTENSIONS));
#endif

        if (!eglBindAPI(kAPIs[api].fAPI)) {
            continue;
        }

        EGLint numConfigs = 0;
        const EGLint configAttribs[] = {
            EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
            EGL_RENDERABLE_TYPE, kAPIs[api].fRenderableTypeBit,
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_ALPHA_SIZE, 8,
            EGL_NONE
        };

        EGLConfig surfaceConfig;
        if (!eglChooseConfig(fDisplay, configAttribs, &surfaceConfig, 1, &numConfigs)) {
            SkDebugf("eglChooseConfig failed. EGL Error: 0x%08x\n", eglGetError());
            continue;
        }

        if (0 == numConfigs) {
            SkDebugf("No suitable EGL config found.\n");
            continue;
        }

        fContext = eglCreateContext(fDisplay, surfaceConfig, NULL, kAPIs[api].fContextAttribs);
        if (EGL_NO_CONTEXT == fContext) {
            SkDebugf("eglCreateContext failed.  EGL Error: 0x%08x\n", eglGetError());
            continue;
        }

        static const EGLint kSurfaceAttribs[] = {
            EGL_WIDTH, 1,
            EGL_HEIGHT, 1,
            EGL_NONE
        };

        fSurface = eglCreatePbufferSurface(fDisplay, surfaceConfig, kSurfaceAttribs);
        if (EGL_NO_SURFACE == fSurface) {
            SkDebugf("eglCreatePbufferSurface failed. EGL Error: 0x%08x\n", eglGetError());
            this->destroyGLContext();
            continue;
        }

        if (!eglMakeCurrent(fDisplay, fSurface, fSurface, fContext)) {
            SkDebugf("eglMakeCurrent failed.  EGL Error: 0x%08x\n", eglGetError());
            this->destroyGLContext();
            continue;
        }

        fGL.reset(GrGLCreateNativeInterface());
        if (NULL == fGL.get()) {
            SkDebugf("Failed to create gl interface.\n");
            this->destroyGLContext();
            continue;
        }

        if (!fGL->validate()) {
            SkDebugf("Failed to validate gl interface.\n");
            this->destroyGLContext();
            continue;
        }
    }
}

EGLGLContext::~EGLGLContext() {
    this->destroyGLContext();
}

void EGLGLContext::destroyGLContext() {
    fGL.reset(NULL);
    if (fDisplay) {
        eglMakeCurrent(fDisplay, 0, 0, 0);

        if (fContext) {
            eglDestroyContext(fDisplay, fContext);
            fContext = EGL_NO_CONTEXT;
        }

        if (fSurface) {
            eglDestroySurface(fDisplay, fSurface);
            fSurface = EGL_NO_SURFACE;
        }

        //TODO should we close the display?
        fDisplay = EGL_NO_DISPLAY;
    }
}


void EGLGLContext::makeCurrent() const {
    if (!eglMakeCurrent(fDisplay, fSurface, fSurface, fContext)) {
        SkDebugf("Could not set the context.\n");
    }
}

void EGLGLContext::swapBuffers() const {
    if (!eglSwapBuffers(fDisplay, fSurface)) {
        SkDebugf("Could not complete eglSwapBuffers.\n");
    }
}

} // anonymous namespace

SkGLContext* SkCreatePlatformGLContext(GrGLStandard forcedGpuAPI) {
    EGLGLContext* ctx = SkNEW_ARGS(EGLGLContext, (forcedGpuAPI));
    if (!ctx->isValid()) {
        SkDELETE(ctx);
        return NULL;
    }
    return ctx;
}

