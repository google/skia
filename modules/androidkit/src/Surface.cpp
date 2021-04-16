/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <android/bitmap.h>
#include <android/native_window_jni.h>
#include <android/log.h>
#include <EGL/egl.h>
#include <GLES/gl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES3/gl3.h>
#include <jni.h>

#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrContextOptions.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/gl/GrGLInterface.h"
#include "include/gpu/gl/GrGLTypes.h"

namespace {

class Surface : public SkRefCnt {
public:
    virtual void release(JNIEnv*) = 0;

    SkCanvas* getCanvas() const { return fSurface ? fSurface->getCanvas() : nullptr; }

protected:
    sk_sp<SkSurface> fSurface;
};

class BitmapSurface final : public Surface {
public:
    BitmapSurface(JNIEnv* env, jobject bitmap) {
        AndroidBitmapInfo bm_info;
        if (AndroidBitmap_getInfo(env, bitmap, &bm_info) != ANDROID_BITMAP_RESULT_SUCCESS) {
            return;
        }

        const auto info = SkImageInfo::Make(bm_info.width, bm_info.height,
                                            color_type(bm_info.format), alpha_type(bm_info.flags));

        void* pixels;
        if (AndroidBitmap_lockPixels(env, bitmap, &pixels) != ANDROID_BITMAP_RESULT_SUCCESS) {
            return;
        }

        fSurface = SkSurface::MakeRasterDirect(info, pixels, bm_info.stride);
        if (!fSurface) {
            AndroidBitmap_unlockPixels(env, bitmap);
            return;
        }

        fBitmap = env->NewGlobalRef(bitmap);
    }

private:
    void release(JNIEnv* env) override {
        if (fSurface) {
            AndroidBitmap_unlockPixels(env, fBitmap);
            fSurface.reset();
            env->DeleteGlobalRef(fBitmap);
        }
    }

    static SkColorType color_type(int32_t format) {
        switch (format) {
            case ANDROID_BITMAP_FORMAT_RGBA_8888: return kRGBA_8888_SkColorType;
            case ANDROID_BITMAP_FORMAT_RGB_565:   return kRGB_565_SkColorType;
            case ANDROID_BITMAP_FORMAT_RGBA_4444: return kARGB_4444_SkColorType;
            case ANDROID_BITMAP_FORMAT_RGBA_F16:  return kRGBA_F16_SkColorType;
            case ANDROID_BITMAP_FORMAT_A_8:       return kAlpha_8_SkColorType;
            default: break;
        }

        return kUnknown_SkColorType;
    }

    static SkAlphaType alpha_type(int32_t flags) {
        switch ((flags >> ANDROID_BITMAP_FLAGS_ALPHA_SHIFT) & ANDROID_BITMAP_FLAGS_ALPHA_MASK) {
            case ANDROID_BITMAP_FLAGS_ALPHA_OPAQUE:   return kOpaque_SkAlphaType;
            case ANDROID_BITMAP_FLAGS_ALPHA_PREMUL:   return kPremul_SkAlphaType;
            case ANDROID_BITMAP_FLAGS_ALPHA_UNPREMUL: return kUnpremul_SkAlphaType;
            default: break;
        }

        return kUnknown_SkAlphaType;
    }

    jobject fBitmap;
};

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
        glClearColor(0, 0, 1, 1);
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
        eglSwapBuffers(fEGLDisplay, fEGLSurface);
    }

private:
    void release(JNIEnv* env) override {
        eglSwapBuffers(fEGLDisplay, fEGLSurface);
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

static jlong Surface_CreateBitmap(JNIEnv* env, jobject, jobject bitmap) {
    return reinterpret_cast<jlong>(new BitmapSurface(env, bitmap));
}

static jlong Surface_CreateSurface(JNIEnv* env, jobject, jobject surface) {
    __android_log_print(ANDROID_LOG_ERROR, "*** AK", "Surface_CreateSurface");

    auto* win = ANativeWindow_fromSurface(env, surface);
    if (!win) {
        return 0;
    }

    return reinterpret_cast<jlong>(new GLSurface(win));
}

static void Surface_Release(JNIEnv* env, jobject, jlong native_surface) {
    if (auto* surface = reinterpret_cast<Surface*>(native_surface)) {
        surface->release(env);
        delete surface;
    }
}

static jlong Surface_GetNativeCanvas(JNIEnv* env, jobject, jlong native_surface) {
    const auto* surface = reinterpret_cast<Surface*>(native_surface);
    return surface
        ? reinterpret_cast<jlong>(surface->getCanvas())
        : 0;
}

}  // namespace

int register_androidkit_Surface(JNIEnv* env) {
    static const JNINativeMethod methods[] = {
        {"nCreateBitmap"   , "(Landroid/graphics/Bitmap;)J",
            reinterpret_cast<void*>(Surface_CreateBitmap)},
        {"nCreateSurface"  , "(Landroid/view/Surface;)J",
            reinterpret_cast<void*>(Surface_CreateSurface)},
        {"nRelease"        , "(J)V", reinterpret_cast<void*>(Surface_Release)},
        {"nGetNativeCanvas", "(J)J", reinterpret_cast<void*>(Surface_GetNativeCanvas)},
    };

    const auto clazz = env->FindClass("org/skia/androidkit/Surface");
    return clazz
        ? env->RegisterNatives(clazz, methods, SK_ARRAY_COUNT(methods))
        : JNI_ERR;
}
