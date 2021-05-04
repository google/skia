/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <android/bitmap.h>
#include <android/log.h>
#include <android/native_window_jni.h>
#include <android/native_window.h>
#include <jni.h>

#include "include/core/SkPictureRecorder.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "tools/sk_app/Application.h"
#include "tools/sk_app/DisplayParams.h"
#include "tools/sk_app/WindowContext.h"
#include "tools/sk_app/android/WindowContextFactory_android.h"

namespace sk_app {
// Required to appease the dynamic linker.
// TODO: split WindowContext from sk_app.
Application* Application::Create(int argc, char** argv, void* platformData) {
    return nullptr;
}
}

#include "modules/androidkit/src/SurfaceThread.h"

namespace {

class Surface : public SkRefCnt {
public:
    virtual void release(JNIEnv*) = 0;
    virtual void flushAndSubmit() = 0;
    virtual SkCanvas* getCanvas() = 0;

    int width()  const { return fSurface ? fSurface->width()  : 0; }
    int height() const { return fSurface ? fSurface->height() : 0; }

protected:
    sk_sp<SkSurface> fSurface;
};

class WindowSurface final : public Surface {
public:
    WindowSurface(ANativeWindow* win, std::unique_ptr<sk_app::WindowContext> wctx)
        : fWindow(win)
        , fWindowContext(std::move(wctx))
    {
        SkASSERT(fWindow);
        SkASSERT(fWindowContext);

        fSurface = fWindowContext->getBackbufferSurface();
    }

private:
    void release(JNIEnv* env) override {
        fWindowContext.reset();
        ANativeWindow_release(fWindow);
    }

    SkCanvas* getCanvas() override {
        if (fSurface) {
            return fSurface->getCanvas();
        }
        return nullptr;
    }

    void flushAndSubmit() override {
        fSurface->flushAndSubmit();
        fWindowContext->swapBuffers();
        fSurface = fWindowContext->getBackbufferSurface();
    }

    ANativeWindow*                         fWindow;
    std::unique_ptr<sk_app::WindowContext> fWindowContext;
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

    SkCanvas* getCanvas() override {
        if (fSurface) {
            return fSurface->getCanvas();
        }
        return nullptr;
    }

    void flushAndSubmit() override {
        // Nothing to do.
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

// SkSurface created from being passed an android.view.Surface
// For now, assume we are always rendering with OpenGL
// TODO: add option of choose backing
class ThreadedSurface final : public Surface {
public:
    ThreadedSurface(JNIEnv* env, jobject surface) {
        fWindow = ANativeWindow_fromSurface(env, surface);
        Message message(kInitialize);
        message.fNativeWindow = fWindow;
    }

private:
    void release(JNIEnv* env) override {
        fThread.postMessage(Message(kDestroy));
        if (fWindow) {
            ANativeWindow_release(fWindow);
        }
       fSurface.reset();
    }

    SkCanvas* getCanvas() override {
        return fRecorder.beginRecording(ANativeWindow_getWidth(fWindow),
                                        ANativeWindow_getHeight(fWindow));
    }

    void flushAndSubmit() override{
        Message message(kRenderPicture);
        message.fNativeWindow = fWindow;
        message.fPicture = fRecorder.finishRecordingAsPicture().release();
        fThread.postMessage(message);
    }

    ANativeWindow* fWindow;
    SkPictureRecorder fRecorder;
    SurfaceThread fThread;
};

// *** JNI methods ***

static jlong Surface_CreateBitmap(JNIEnv* env, jobject, jobject bitmap) {
    return reinterpret_cast<jlong>(new BitmapSurface(env, bitmap));
}

static jlong Surface_CreateThreadedSurface(JNIEnv* env, jobject, jobject surface) {
    return reinterpret_cast<jlong>(new ThreadedSurface(env, surface));
}

static jlong Surface_CreateVK(JNIEnv* env, jobject, jobject jsurface) {
#ifdef SK_VULKAN
    auto* win = ANativeWindow_fromSurface(env, jsurface);
    if (!win) {
        return 0;
    }

    // TODO: match window params?
    sk_app::DisplayParams params;
    auto winctx = sk_app::window_context_factory::MakeVulkanForAndroid(win, params);
    if (!winctx) {
        return 0;
    }

    return reinterpret_cast<jlong>(sk_make_sp<WindowSurface>(win, std::move(winctx)).release());
#endif // SK_VULKAN
    return 0;
}

static jlong Surface_CreateGL(JNIEnv* env, jobject, jobject jsurface) {
#ifdef SK_GL
    auto* win = ANativeWindow_fromSurface(env, jsurface);
    if (!win) {
        return 0;
    }

    // TODO: match window params?
    sk_app::DisplayParams params;
    auto winctx = sk_app::window_context_factory::MakeGLForAndroid(win, params);
    if (!winctx) {
        return 0;
    }

    return reinterpret_cast<jlong>(sk_make_sp<WindowSurface>(win, std::move(winctx)).release());
#endif // SK_GL
    return 0;
}

static void Surface_Release(JNIEnv* env, jobject, jlong native_surface) {
    if (auto* surface = reinterpret_cast<Surface*>(native_surface)) {
        surface->release(env);
        SkSafeUnref(surface);
    }
}

static jlong Surface_GetNativeCanvas(JNIEnv* env, jobject, jlong native_surface) {
    auto* surface = reinterpret_cast<Surface*>(native_surface);
    return surface
        ? reinterpret_cast<jlong>(surface->getCanvas())
        : 0;
}

static void Surface_FlushAndSubmit(JNIEnv* env, jobject, jlong native_surface) {
    if (auto* surface = reinterpret_cast<Surface*>(native_surface)) {
        surface->flushAndSubmit();
    }
}

static int Surface_GetWidth(JNIEnv* env, jobject, jlong native_surface) {
    const auto* surface = reinterpret_cast<Surface*>(native_surface);
    return surface ? surface->width() : 0;
}

static int Surface_GetHeight(JNIEnv* env, jobject, jlong native_surface) {
    const auto* surface = reinterpret_cast<Surface*>(native_surface);
    return surface ? surface->height() : 0;
}

// *** End of JNI methods ***

}  // namespace

int register_androidkit_Surface(JNIEnv* env) {
    static const JNINativeMethod methods[] = {
        {"nCreateBitmap"   , "(Landroid/graphics/Bitmap;)J",
            reinterpret_cast<void*>(Surface_CreateBitmap)                            },
        {"nCreateThreadedSurface"  , "(Landroid/view/Surface;)J",
            reinterpret_cast<void*>(Surface_CreateThreadedSurface)                   },
        {"nCreateVKSurface", "(Landroid/view/Surface;)J",
            reinterpret_cast<void*>(Surface_CreateVK)                                },
        {"nCreateGLSurface", "(Landroid/view/Surface;)J",
            reinterpret_cast<void*>(Surface_CreateGL)                                },
        {"nRelease"        , "(J)V", reinterpret_cast<void*>(Surface_Release)        },
        {"nGetNativeCanvas", "(J)J", reinterpret_cast<void*>(Surface_GetNativeCanvas)},
        {"nFlushAndSubmit" , "(J)V", reinterpret_cast<void*>(Surface_FlushAndSubmit) },
        {"nGetWidth"       , "(J)I", reinterpret_cast<void*>(Surface_GetWidth)       },
        {"nGetHeight"      , "(J)I", reinterpret_cast<void*>(Surface_GetHeight)      },
    };

    const auto clazz = env->FindClass("org/skia/androidkit/Surface");
    return clazz
        ? env->RegisterNatives(clazz, methods, SK_ARRAY_COUNT(methods))
        : JNI_ERR;
}
