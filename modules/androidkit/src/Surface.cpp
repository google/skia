/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <android/bitmap.h>
#include <android/log.h>
#include <jni.h>

#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"

namespace {

class Surface : public SkRefCnt {
public:
    virtual void release(JNIEnv*) = 0;

    SkCanvas* getCanvas() const { return fSurface->getCanvas(); }

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

}  // namespace

/*
 * Takes in a native instance of Bitmap and returns a pointer to the raster surface.
 */
extern "C" JNIEXPORT jlong
JNICALL
Java_org_skia_androidkit_Surface_nCreateBitmap(JNIEnv* env, jobject, jobject bitmap) {
    return reinterpret_cast<jlong>(new BitmapSurface(env, bitmap));
}

extern "C" JNIEXPORT void
JNICALL
Java_org_skia_androidkit_Surface_nRelease(JNIEnv* env, jobject, jlong native_surface) {
    if (auto* surface = reinterpret_cast<Surface*>(native_surface)) {
        surface->release(env);
        delete surface;
    }
}

extern "C" JNIEXPORT jlong
JNICALL
Java_org_skia_androidkit_Surface_nGetNativeCanvas(JNIEnv* env, jobject, jlong native_surface) {
    const auto* surface = reinterpret_cast<Surface*>(native_surface);
    return surface
        ? reinterpret_cast<jlong>(surface->getCanvas())
        : 0;
}
