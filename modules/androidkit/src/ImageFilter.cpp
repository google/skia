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

namespace {

static void ImageFilter_Release(JNIEnv* env, jobject, jlong native_imageFilter) {
    SkSafeUnref(reinterpret_cast<SkImageFilter*>(native_imageFilter));
}

static long ImageFilter_DistantLitDiffuse(JNIEnv* env, jobject, jfloat x, jfloat y, jfloat z,
                                                                jfloat r, jfloat g, jfloat b,
                                                                jfloat surfaceScale, jfloat kd,
                                                                jlong native_input) {
    SkPoint3 direction = SkPoint3::Make(x, y, z);
    SkColor color = SkColorSetARGB(255, SkScalarRoundToInt(r * 255),
                                        SkScalarRoundToInt(g * 255),
                                        SkScalarRoundToInt(b * 255));
    auto input = sk_ref_sp(reinterpret_cast<SkImageFilter*>(native_input));
    auto filter = SkImageFilters::DistantLitDiffuse(direction, color, surfaceScale, kd, std::move(input));
    return reinterpret_cast<jlong>(filter.release());
}

} // namespace

int register_androidkit_ImageFilter(JNIEnv* env) {
    static const JNINativeMethod methods[] = {
        {"nRelease"          , "(J)V"        , reinterpret_cast<void*>(ImageFilter_Release)},
        {"nDistantLitDiffuse", "(FFFFFFFFJ)J", reinterpret_cast<void*>(ImageFilter_DistantLitDiffuse)},
    };

    const auto clazz = env->FindClass("org/skia/androidkit/ImageFilter");
    return clazz
        ? env->RegisterNatives(clazz, methods, SK_ARRAY_COUNT(methods))
        : JNI_ERR;
}
