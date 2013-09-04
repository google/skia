
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gl/SkNativeGLContext.h"

SkNativeGLContext::AutoContextRestore::AutoContextRestore() {
    fOldEGLContext = eglGetCurrentContext();
    fOldDisplay = eglGetCurrentDisplay();
    fOldSurface = eglGetCurrentSurface(EGL_DRAW);

}

SkNativeGLContext::AutoContextRestore::~AutoContextRestore() {
    if (NULL != fOldDisplay) {
        eglMakeCurrent(fOldDisplay, fOldSurface, fOldSurface, fOldEGLContext);
    }
}

///////////////////////////////////////////////////////////////////////////////

SkNativeGLContext::SkNativeGLContext()
    : fContext(EGL_NO_CONTEXT)
    , fDisplay(EGL_NO_DISPLAY)
    , fSurface(EGL_NO_SURFACE) {
}

SkNativeGLContext::~SkNativeGLContext() {
    this->destroyGLContext();
}

void SkNativeGLContext::destroyGLContext() {
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

const GrGLInterface* SkNativeGLContext::createGLContext() {
    static const EGLint kEGLContextAttribsForOpenGL[] = {
        EGL_NONE
    };

    static const EGLint kEGLContextAttribsForOpenGLES[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };

    // Try first for OpenGL, then fall back to OpenGL ES.
    EGLint renderableTypeBit = EGL_OPENGL_BIT;
    const EGLint* contextAttribs = kEGLContextAttribsForOpenGL;
    EGLBoolean apiBound = eglBindAPI(EGL_OPENGL_API);

    if (!apiBound) {
        apiBound = eglBindAPI(EGL_OPENGL_ES_API);
        renderableTypeBit = EGL_OPENGL_ES2_BIT;
        contextAttribs = kEGLContextAttribsForOpenGLES;
    }

    if (!apiBound) {
        return NULL;
    }

    fDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    EGLint majorVersion;
    EGLint minorVersion;
    eglInitialize(fDisplay, &majorVersion, &minorVersion);

    EGLint numConfigs;
    const EGLint configAttribs[] = {
        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
        EGL_RENDERABLE_TYPE, renderableTypeBit,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_NONE
    };

    EGLConfig surfaceConfig;
    if (!eglChooseConfig(fDisplay, configAttribs, &surfaceConfig, 1, &numConfigs)) {
        SkDebugf("eglChooseConfig failed.\n");
        return NULL;
    }

    fContext = eglCreateContext(fDisplay, surfaceConfig, NULL, contextAttribs);
    if (EGL_NO_CONTEXT == fContext) {
        SkDebugf("eglCreateContext failed.\n");
        return NULL;
    }

    static const EGLint kSurfaceAttribs[] = {
        EGL_WIDTH, 1,
        EGL_HEIGHT, 1,
        EGL_NONE
    };

    fSurface = eglCreatePbufferSurface(fDisplay, surfaceConfig, kSurfaceAttribs);
    if (EGL_NO_SURFACE == fSurface) {
        SkDebugf("eglCreatePbufferSurface failed.\n");
        this->destroyGLContext();
        return NULL;
    }

    if (!eglMakeCurrent(fDisplay, fSurface, fSurface, fContext)) {
        SkDebugf("eglMakeCurrent failed.\n");
        this->destroyGLContext();
        return NULL;
    }

    const GrGLInterface* interface = GrGLCreateNativeInterface();
    if (!interface) {
        SkDebugf("Failed to create gl interface.\n");
        this->destroyGLContext();
        return NULL;
    }

    return interface;
}

void SkNativeGLContext::makeCurrent() const {
    if (!eglMakeCurrent(fDisplay, fSurface, fSurface, fContext)) {
        SkDebugf("Could not set the context.\n");
    }
}
