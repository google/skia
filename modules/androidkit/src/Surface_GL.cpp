/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <android/native_window_jni.h>
#include <android/log.h>
#include <EGL/egl.h>
#include <GLES/gl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES3/gl3.h>

#include "modules/androidkit/src/Surface.h"

#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrContextOptions.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/gl/GrGLInterface.h"
#include "include/gpu/gl/GrGLTypes.h"

namespace {

class GLSurface final : public Surface {
public:
    explicit GLSurface(ANativeWindow* win)
        : fWindow(win)
    {
        SkASSERT(fWindow);

        fEGLDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);

        EGLint majVer, minVer;
        eglInitialize(fEGLDisplay, &majVer, &minVer);
        SkAssertResult(eglBindAPI(EGL_OPENGL_ES_API));

        EGLint eglNumConfigs = 0,
               eglSampleCnt  = 0; // TODO: MSAA?

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
        SkAssertResult(
            eglChooseConfig(fEGLDisplay, configAttribs, &surfaceConfig, 1, &eglNumConfigs));
        SkASSERT(eglNumConfigs > 0);

        static const EGLint kEGLContextAttribsForOpenGLES[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2,
            EGL_NONE
        };
        fEGLContext =
            eglCreateContext(fEGLDisplay, surfaceConfig, nullptr, kEGLContextAttribsForOpenGLES);
        SkASSERT(EGL_NO_CONTEXT != fEGLContext);

        fEGLSurface = eglCreateWindowSurface(fEGLDisplay, surfaceConfig, fWindow, nullptr);
        SkASSERT(EGL_NO_SURFACE != fEGLSurface);

        SkAssertResult(eglMakeCurrent(fEGLDisplay, fEGLSurface, fEGLSurface, fEGLContext));

        glClearStencil(0);
        glClearColor(0, 0, 0, 0);
        glStencilMask(0xffffffff);
        glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        eglSwapInterval(fEGLDisplay, 1); // TODO: disable vsync?

        __android_log_print(ANDROID_LOG_ERROR, "*** AK", "EGL %d.%d", majVer, minVer);
        __android_log_print(ANDROID_LOG_ERROR, "*** AK", "extensions: %s",
                            eglQueryString(fEGLDisplay, EGL_EXTENSIONS));

        auto glInterface = GrGLMakeNativeInterface();
        if (!glInterface) {
            __android_log_print(ANDROID_LOG_ERROR, "*** AK", "BOO #1");
            return;
        }

        GrContextOptions options;
        options.fDisableDistanceFieldPaths = true;

        fGrContext = GrDirectContext::MakeGL(std::move(glInterface), options);
        if (!fGrContext) {
            __android_log_print(ANDROID_LOG_ERROR, "*** AK", "BOO #2");
            return;
        }

        GrGLFramebufferInfo fboInfo;
        fboInfo.fFBOID = 0;
        fboInfo.fFormat = GL_RGBA8;

        GrBackendRenderTarget backendRT(ANativeWindow_getWidth(fWindow),
                                        ANativeWindow_getWidth(fWindow),
                                        0, 8, fboInfo);
        SkSurfaceProps props(0, kUnknown_SkPixelGeometry);

        fSurface = SkSurface::MakeFromBackendRenderTarget(fGrContext.get(), backendRT,
                                                          kBottomLeft_GrSurfaceOrigin,
                                                          kN32_SkColorType,
                                                          nullptr, &props);
        __android_log_print(ANDROID_LOG_ERROR, "*** AK", "surface: %p\n", fSurface.get());
    }

private:
    void swapBuffers() override {
        if (fSurface) {
            fSurface->flushAndSubmit();
            eglSwapBuffers(fEGLDisplay, fEGLSurface);
        }
    }

    void release(JNIEnv* env) override {
        fSurface.reset();
        fGrContext.reset();

        eglMakeCurrent(fEGLDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (fEGLSurface != EGL_NO_SURFACE) {
            SkAssertResult(eglDestroySurface(fEGLDisplay, fEGLSurface));
            fEGLSurface = EGL_NO_SURFACE;
        }
        if (fEGLContext != EGL_NO_CONTEXT) {
            SkAssertResult(eglDestroyContext(fEGLDisplay, fEGLContext));
            fEGLContext = EGL_NO_CONTEXT;
        }
        if (fWindow) {
            ANativeWindow_release(fWindow);
            fWindow = nullptr;
        }
    }

    ANativeWindow*         fWindow;
    sk_sp<GrDirectContext> fGrContext;
    EGLDisplay             fEGLDisplay = EGL_NO_DISPLAY;
    EGLContext             fEGLContext = EGL_NO_CONTEXT;
    EGLSurface             fEGLSurface = EGL_NO_SURFACE;
};

}  // namespace

jlong Surface_CreateGL(JNIEnv* env, jobject, jobject surface) {
    __android_log_print(ANDROID_LOG_ERROR, "*** AK", "Surface_CreateSurface");

    auto* win = ANativeWindow_fromSurface(env, surface);
    if (!win) {
        return 0;
    }

    return reinterpret_cast<jlong>(new GLSurface(win));
}
