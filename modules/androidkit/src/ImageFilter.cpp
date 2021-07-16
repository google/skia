/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <jni.h>

#include "include/core/SkImageFilter.h"
#include "include/core/SkPoint3.h"
#include "include/effects/SkImageFilters.h"
#include "modules/androidkit/src/Utils.h"

namespace {

static void ImageFilter_Release(JNIEnv* env, jobject, jlong native_imageFilter) {
    SkSafeUnref(reinterpret_cast<SkImageFilter*>(native_imageFilter));
}

static long ImageFilter_DistantLitDiffuse(JNIEnv* env, jobject, jfloat x, jfloat y, jfloat z,
                                                                jfloat r, jfloat g, jfloat b,
                                                                jfloat surfaceScale, jfloat kd,
                                                                jlong native_input) {
    SkPoint3 direction = SkPoint3::Make(x, y, z);
      auto color = SkColor4f{r, g, b, 1}.toSkColor();
    auto input = sk_ref_sp(reinterpret_cast<SkImageFilter*>(native_input));
    auto filter = SkImageFilters::DistantLitDiffuse(direction, color, surfaceScale, kd, std::move(input));
    return reinterpret_cast<jlong>(filter.release());
}

static long ImageFilter_Blur(JNIEnv* env, jobject, jfloat sigmaX, jfloat sigmaY,
                                                   jint jTileMode, jlong native_input) {
    auto input = sk_ref_sp(reinterpret_cast<SkImageFilter*>(native_input));
    auto filter = SkImageFilters::Blur(sigmaX, sigmaY,
                                       androidkit::utils::TileMode(jTileMode),
                                       std::move(input));
    return reinterpret_cast<jlong>(filter.release());
}

static long ImageFilter_DropShadow(JNIEnv* env, jobject, jfloat dx, jfloat dy,
                                                         jfloat sigmaX, jfloat sigmaY,
                                                         jfloat r, jfloat g, jfloat b,
                                                         jlong native_input) {
    auto color = SkColor4f{r, g, b, 1}.toSkColor();
    auto input = sk_ref_sp(reinterpret_cast<SkImageFilter*>(native_input));
    auto filter = SkImageFilters::DropShadow(dx, dy, sigmaX, sigmaY, color, std::move(input));
    return reinterpret_cast<jlong>(filter.release());
}

static long ImageFilter_Blend(JNIEnv* env, jobject, jint bm, jlong background, jlong foreground) {
    auto bg = sk_ref_sp(reinterpret_cast<SkImageFilter*>(background));
    auto fg = sk_ref_sp(reinterpret_cast<SkImageFilter*>(foreground));
    auto filter = SkImageFilters::Blend(androidkit::utils::BlendMode(bm), std::move(bg), std::move(fg));
    return reinterpret_cast<jlong>(filter.release());
}

static long ImageFilter_Image(JNIEnv* env, jobject, jlong native_image) {
    auto image = sk_ref_sp(reinterpret_cast<SkImage*>(native_image));
    auto filter = SkImageFilters::Image(std::move(image));
    return reinterpret_cast<jlong>(filter.release());
}

} // namespace

int register_androidkit_ImageFilter(JNIEnv* env) {
    static const JNINativeMethod methods[] = {
        {"nRelease"          , "(J)V"        , reinterpret_cast<void*>(ImageFilter_Release)},
        {"nDistantLitDiffuse", "(FFFFFFFFJ)J", reinterpret_cast<void*>(ImageFilter_DistantLitDiffuse)},
        {"nBlur"             , "(FFIJ)J"     , reinterpret_cast<void*>(ImageFilter_Blur)},
        {"nDropShadow"       , "(FFFFFFFJ)J" , reinterpret_cast<void*>(ImageFilter_DropShadow)},
        {"nBlend"            , "(IJJ)J"      , reinterpret_cast<void*>(ImageFilter_Blend)},
        {"nImage"            , "(J)J"        , reinterpret_cast<void*>(ImageFilter_Image)},
    };

    const auto clazz = env->FindClass("org/skia/androidkit/ImageFilter");
    return clazz
        ? env->RegisterNatives(clazz, methods, SK_ARRAY_COUNT(methods))
        : JNI_ERR;
}
