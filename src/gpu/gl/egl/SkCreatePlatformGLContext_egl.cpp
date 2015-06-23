
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gl/SkGLContext.h"

#include <GLES2/gl2.h>

#define EGL_EGLEXT_PROTOTYPES
#include <EGL/egl.h>
#include <EGL/eglext.h>

namespace {

// TODO: Share this class with ANGLE if/when it gets support for EGL_KHR_fence_sync.
class SkEGLFenceSync : public SkGpuFenceSync {
public:
    static SkEGLFenceSync* CreateIfSupported(EGLDisplay);

    SkPlatformGpuFence SK_WARN_UNUSED_RESULT insertFence() const override;
    bool flushAndWaitFence(SkPlatformGpuFence fence) const override;
    void deleteFence(SkPlatformGpuFence fence) const override;

private:
    SkEGLFenceSync(EGLDisplay display) : fDisplay(display) {}

    EGLDisplay                    fDisplay;

    typedef SkGpuFenceSync INHERITED;
};

class EGLGLContext : public SkGLContext  {
public:
    EGLGLContext(GrGLStandard forcedGpuAPI);
    ~EGLGLContext() override;

private:
    void destroyGLContext();

    void onPlatformMakeCurrent() const override;
    void onPlatformSwapBuffers() const override;
    GrGLFuncPtr onPlatformGetProcAddress(const char*) const override;

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

    SkAutoTUnref<const GrGLInterface> gl;

    for (; NULL == gl.get() && api < apiLimit; ++api) {
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

        gl.reset(GrGLCreateNativeInterface());
        if (NULL == gl.get()) {
            SkDebugf("Failed to create gl interface.\n");
            this->destroyGLContext();
            continue;
        }

        if (!gl->validate()) {
            SkDebugf("Failed to validate gl interface.\n");
            this->destroyGLContext();
            continue;
        }

        this->init(gl.detach(), SkEGLFenceSync::CreateIfSupported(fDisplay));
        break;
    }
}

EGLGLContext::~EGLGLContext() {
    this->teardown();
    this->destroyGLContext();
}

void EGLGLContext::destroyGLContext() {
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


void EGLGLContext::onPlatformMakeCurrent() const {
    if (!eglMakeCurrent(fDisplay, fSurface, fSurface, fContext)) {
        SkDebugf("Could not set the context.\n");
    }
}

void EGLGLContext::onPlatformSwapBuffers() const {
    if (!eglSwapBuffers(fDisplay, fSurface)) {
        SkDebugf("Could not complete eglSwapBuffers.\n");
    }
}

GrGLFuncPtr EGLGLContext::onPlatformGetProcAddress(const char* procName) const {
    return eglGetProcAddress(procName);
}

static bool supports_egl_extension(EGLDisplay display, const char* extension) {
    int extensionLength = strlen(extension);
    const char* extensionsStr = eglQueryString(display, EGL_EXTENSIONS);
    while (const char* match = strstr(extensionsStr, extension)) {
        // Ensure the string we found is its own extension, not a substring of a larger extension
        // (e.g. GL_ARB_occlusion_query / GL_ARB_occlusion_query2).
        if ((match == extensionsStr || match[-1] == ' ') &&
            (match[extensionLength] == ' ' || match[extensionLength] == '\0')) {
            return true;
        }
        extensionsStr = match + extensionLength;
    }
    return false;
}

SkEGLFenceSync* SkEGLFenceSync::CreateIfSupported(EGLDisplay display) {
    if (!display || !supports_egl_extension(display, "EGL_KHR_fence_sync")) {
        return NULL;
    }
    return SkNEW_ARGS(SkEGLFenceSync, (display));
}

SkPlatformGpuFence SkEGLFenceSync::insertFence() const {
    return eglCreateSyncKHR(fDisplay, EGL_SYNC_FENCE_KHR, NULL);
}

bool SkEGLFenceSync::flushAndWaitFence(SkPlatformGpuFence platformFence) const {
    EGLSyncKHR eglsync = static_cast<EGLSyncKHR>(platformFence);
    return EGL_CONDITION_SATISFIED_KHR == eglClientWaitSyncKHR(fDisplay,
                                                               eglsync,
                                                               EGL_SYNC_FLUSH_COMMANDS_BIT_KHR,
                                                               EGL_FOREVER_KHR);
}

void SkEGLFenceSync::deleteFence(SkPlatformGpuFence platformFence) const {
    EGLSyncKHR eglsync = static_cast<EGLSyncKHR>(platformFence);
    eglDestroySyncKHR(fDisplay, eglsync);
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

