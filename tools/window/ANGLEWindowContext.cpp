/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/window/ANGLEWindowContext.h"

#include "include/gpu/gl/GrGLAssembleInterface.h"
#include "src/gpu/ganesh/gl/GrGLDefines.h"

namespace skwindow::internal {

ANGLEWindowContext::~ANGLEWindowContext() { this->destroyContext(); }

sk_sp<const GrGLInterface> ANGLEWindowContext::onInitializeContext() {
    PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT =
            (PFNEGLGETPLATFORMDISPLAYEXTPROC)eglGetProcAddress("eglGetPlatformDisplayEXT");

    // We expect ANGLE to support this extension
    if (!eglGetPlatformDisplayEXT) {
        return nullptr;
    }

    fDisplay = this->onGetEGLDisplay(eglGetPlatformDisplayEXT);
    if (EGL_NO_DISPLAY == fDisplay) {
        return nullptr;
    }

    EGLint majorVersion;
    EGLint minorVersion;
    if (!eglInitialize(fDisplay, &majorVersion, &minorVersion)) {
        SkDebugf("Could not initialize display!\n");
        return nullptr;
    }
    EGLint numConfigs;
    fSampleCount = this->getDisplayParams().fMSAASampleCount;
    const int sampleBuffers = fSampleCount > 1 ? 1 : 0;
    const int eglSampleCnt = fSampleCount > 1 ? fSampleCount : 0;
    const EGLint configAttribs[] = {EGL_RENDERABLE_TYPE,
                                    // We currently only support ES3.
                                    EGL_OPENGL_ES3_BIT,
                                    EGL_RED_SIZE,
                                    8,
                                    EGL_GREEN_SIZE,
                                    8,
                                    EGL_BLUE_SIZE,
                                    8,
                                    EGL_ALPHA_SIZE,
                                    8,
                                    EGL_SAMPLE_BUFFERS,
                                    sampleBuffers,
                                    EGL_SAMPLES,
                                    eglSampleCnt,
                                    EGL_NONE};

    EGLConfig surfaceConfig;
    if (!eglChooseConfig(fDisplay, configAttribs, &surfaceConfig, 1, &numConfigs)) {
        SkDebugf("Could not create choose config!\n");
        return nullptr;
    }
    // We currently only support ES3.
    const EGLint contextAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
    fEGLContext = eglCreateContext(fDisplay, surfaceConfig, nullptr, contextAttribs);
    if (EGL_NO_CONTEXT == fEGLContext) {
        SkDebugf("Could not create context!\n");
        return nullptr;
    }
    fEGLSurface =
            eglCreateWindowSurface(fDisplay, surfaceConfig, this->onGetNativeWindow(), nullptr);
    if (EGL_NO_SURFACE == fEGLSurface) {
        SkDebugf("Could not create surface!\n");
        return nullptr;
    }
    if (!eglMakeCurrent(fDisplay, fEGLSurface, fEGLSurface, fEGLContext)) {
        SkDebugf("Could not make context current!\n");
        return nullptr;
    }

    sk_sp<const GrGLInterface> interface(GrGLMakeAssembledInterface(
            nullptr,
            [](void* ctx, const char name[]) -> GrGLFuncPtr { return eglGetProcAddress(name); }));
    if (interface) {
        interface->fFunctions.fClearStencil(0);
        interface->fFunctions.fClearColor(0, 0, 0, 0);
        interface->fFunctions.fStencilMask(0xffffffff);
        interface->fFunctions.fClear(GR_GL_STENCIL_BUFFER_BIT | GR_GL_COLOR_BUFFER_BIT);

        fStencilBits = this->onGetStencilBits();

        SkISize size = this->onGetSize();
        fWidth = size.width();
        fHeight = size.height();
        interface->fFunctions.fViewport(0, 0, fWidth, fHeight);
    }
    return interface;
}

void ANGLEWindowContext::onDestroyContext() {
    eglMakeCurrent(fDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    if (EGL_NO_CONTEXT != fEGLContext) {
        eglDestroyContext(fDisplay, fEGLContext);
    }
    if (EGL_NO_SURFACE != fEGLSurface) {
        eglDestroySurface(fDisplay, fEGLSurface);
    }
    if (EGL_NO_DISPLAY != fDisplay) {
        eglTerminate(fDisplay);
    }
}

void ANGLEWindowContext::onSwapBuffers() {
    if (!eglSwapBuffers(fDisplay, fEGLSurface)) {
        SkDebugf("Could not complete eglSwapBuffers.\n");
    }
}

}  // namespace skwindow::internal
