/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSurface.h"

#include <android/bitmap.h>
#include <android/log.h>
#include <jni.h>

namespace {

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

SkPaint skpaint(JNIEnv* env, jobject jpaint) {
    SkPaint paint;

    // TODO: reflect jpaint
    paint.setColor(0xff00ff00);

    return paint;
}

class CanvasWrapper {
 public:
    CanvasWrapper(JNIEnv* env, jobject bitmap) {
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
        }
    }

    void unlockPixels(JNIEnv* env, jobject bitmap) {
        if (fSurface) {
            AndroidBitmap_unlockPixels(env, bitmap);
        }
    }

    SkCanvas* canvas() const { return fSurface ? fSurface->getCanvas() : nullptr; }

 private:
    sk_sp<SkSurface> fSurface;
};

}  // namespace

/*
 * Takes in a native instance of bitmap and returns a pointer to the raster canvas.
 */
extern "C" JNIEXPORT jlong
JNICALL
Java_org_skia_androidkit_Canvas_nCreateRaster(JNIEnv* env, jobject, jobject bitmap) {
    return (jlong) new CanvasWrapper(env, bitmap);
}

extern "C" JNIEXPORT void
JNICALL
Java_org_skia_androidkit_Canvas_nFinalize(JNIEnv* env, jobject,
                                          jlong canvas_wrapper, jobject bitmap) {
    auto* wrapper = reinterpret_cast<CanvasWrapper*>(canvas_wrapper);
    wrapper->unlockPixels(env, bitmap);
    delete wrapper;
}

extern "C" JNIEXPORT void
JNICALL
Java_org_skia_androidkit_Canvas_nDrawRect(JNIEnv* env, jobject, jlong canvas_wrapper,
                                          jfloat left, jfloat top, jfloat right, jfloat bottom,
                                          jobject paint) {
    auto* canvas = reinterpret_cast<CanvasWrapper*>(canvas_wrapper)->canvas();
    if (!canvas) {
        return;
    }

    canvas->drawRect(SkRect::MakeLTRB(left, top, right, bottom), skpaint(env, paint));
}

