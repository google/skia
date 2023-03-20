/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <EGL/egl.h>
#include <GLES/gl.h>
#include "include/gpu/gl/GrGLInterface.h"
#include "tools/window/SkGLWindowContext.h"
#include "tools/window/android/SkWindowContextFactory_android.h"

namespace {
class SkGLWindowContext_android : public SkGLWindowContext {
public:

    SkGLWindowContext_android(ANativeWindow*, const SkDisplayParams&);

    ~SkGLWindowContext_android() override;

    void onSwapBuffers() override;

    sk_sp<const GrGLInterface> onInitializeContext() override;
    void onDestroyContext() override;

private:

    EGLDisplay fDisplay;
    EGLContext fEGLContext;
    EGLSurface fSurfaceAndroid;

    // For setDisplayParams and resize which call onInitializeContext with null platformData
    ANativeWindow* fNativeWindow = nullptr;

    using INHERITED = SkGLWindowContext;
};

SkGLWindowContext_android::SkGLWindowContext_android(ANativeWindow* window,
                                                 const SkDisplayParams& params)
    : INHERITED(params)
    , fDisplay(EGL_NO_DISPLAY)
    , fEGLContext(EGL_NO_CONTEXT)
    , fSurfaceAndroid(EGL_NO_SURFACE)
    , fNativeWindow(window) {

    // any config code here (particularly for msaa)?

    this->initializeContext();
}

SkGLWindowContext_android::~SkGLWindowContext_android() {
    this->destroyContext();
}

sk_sp<const GrGLInterface> SkGLWindowContext_android::onInitializeContext() {
    fWidth = ANativeWindow_getWidth(fNativeWindow);
    fHeight = ANativeWindow_getHeight(fNativeWindow);

    fDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    EGLint majorVersion;
    EGLint minorVersion;
    eglInitialize(fDisplay, &majorVersion, &minorVersion);

    SkAssertResult(eglBindAPI(EGL_OPENGL_ES_API));

    EGLint numConfigs = 0;
    EGLint eglSampleCnt = fDisplayParams.fMSAASampleCount > 1 ? fDisplayParams.fMSAASampleCount > 1
                                                              : 0;
    const EGLint configAttribs[] = {
        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_STENCIL_SIZE, 8,
        EGL_SAMPLE_BUFFERS, eglSampleCnt ? 1 : 0,
        EGL_SAMPLES, eglSampleCnt,
        EGL_NONE
    };

    EGLConfig surfaceConfig;
    SkAssertResult(eglChooseConfig(fDisplay, configAttribs, &surfaceConfig, 1, &numConfigs));
    SkASSERT(numConfigs > 0);

    static const EGLint kEGLContextAttribsForOpenGLES[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };
    fEGLContext = eglCreateContext(
            fDisplay, surfaceConfig, nullptr, kEGLContextAttribsForOpenGLES);
    SkASSERT(EGL_NO_CONTEXT != fEGLContext);

//    SkDebugf("EGL: %d.%d", majorVersion, minorVersion);
//    SkDebugf("Vendor: %s", eglQueryString(fDisplay, EGL_VENDOR));
//    SkDebugf("Extensions: %s", eglQueryString(fDisplay, EGL_EXTENSIONS));

    fSurfaceAndroid = eglCreateWindowSurface(fDisplay, surfaceConfig, fNativeWindow, nullptr);
    SkASSERT(EGL_NO_SURFACE != fSurfaceAndroid);

    SkAssertResult(eglMakeCurrent(fDisplay, fSurfaceAndroid, fSurfaceAndroid, fEGLContext));
    // SkGLWindowContext::initializeContext will call GrGLMakeNativeInterface so we
    // won't call it here.

    glClearStencil(0);
    glClearColor(0, 0, 0, 0);
    glStencilMask(0xffffffff);
    glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    eglGetConfigAttrib(fDisplay, surfaceConfig, EGL_STENCIL_SIZE, &fStencilBits);
    eglGetConfigAttrib(fDisplay, surfaceConfig, EGL_SAMPLES, &fSampleCount);
    fSampleCount = std::max(fSampleCount, 1);

    eglSwapInterval(fDisplay, fDisplayParams.fDisableVsync ? 0 : 1);

    return GrGLMakeNativeInterface();
}

void SkGLWindowContext_android::onDestroyContext() {
    if (!fDisplay || !fEGLContext || !fSurfaceAndroid) {
        return;
    }
    eglMakeCurrent(fDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    SkAssertResult(eglDestroySurface(fDisplay, fSurfaceAndroid));
    SkAssertResult(eglDestroyContext(fDisplay, fEGLContext));
    fEGLContext = EGL_NO_CONTEXT;
    fSurfaceAndroid = EGL_NO_SURFACE;
}

void SkGLWindowContext_android::onSwapBuffers() {
    if (fDisplay && fEGLContext && fSurfaceAndroid) {
        eglSwapBuffers(fDisplay, fSurfaceAndroid);
    }
}

}  // anonymous namespace

namespace window_context_factory {

std::unique_ptr<SkWindowContext> MakeGLForAndroid(ANativeWindow* window,
                                                  const SkDisplayParams& params) {
    std::unique_ptr<SkWindowContext> ctx(new SkGLWindowContext_android(window, params));
    if (!ctx->isValid()) {
        return nullptr;
    }
    return ctx;
}

}  // namespace window_context_factory
