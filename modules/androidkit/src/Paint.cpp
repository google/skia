/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <jni.h>

#include "include/core/SkPaint.h"
#include "include/core/SkShader.h"
#include "include/effects/SkImageFilters.h"

namespace {

static jlong Paint_Create(JNIEnv* env, jobject) {
    return reinterpret_cast<jlong>(new SkPaint);
}

static void Paint_Release(JNIEnv* env, jobject, jlong native_paint) {
    delete reinterpret_cast<SkPaint*>(native_paint);
}

static void Paint_SetColor(JNIEnv* env, jobject, jlong native_paint,
                           float r, float g, float b, float a) {
    if (auto* paint = reinterpret_cast<SkPaint*>(native_paint)) {
        paint->setColor4f({r, g, b, a});
    }
}

static void Paint_SetStroke(JNIEnv* env, jobject, jlong native_paint, jboolean stroke) {
    if (auto* paint = reinterpret_cast<SkPaint*>(native_paint)) {
        paint->setStroke(stroke);
    }
}

static void Paint_SetStrokeWidth(JNIEnv* env, jobject, jlong native_paint, jfloat width) {
    if (auto* paint = reinterpret_cast<SkPaint*>(native_paint)) {
        paint->setStrokeWidth(width);
    }
}

static void Paint_SetShader(JNIEnv* env, jobject, jlong native_paint, jlong native_shader) {
    if (auto* paint = reinterpret_cast<SkPaint*>(native_paint)) {
        paint->setShader(sk_ref_sp(reinterpret_cast<SkShader*>(native_shader)));
    }
}

static void Paint_SetImageFilter(JNIEnv* env, jobject, jlong native_paint, jlong native_filter) {
    if (auto* paint = reinterpret_cast<SkPaint*>(native_paint)) {
        paint->setImageFilter(sk_ref_sp(reinterpret_cast<SkImageFilter*>(native_filter)));
    }
}

}  // namespace

int register_androidkit_Paint(JNIEnv* env) {
    static const JNINativeMethod methods[] = {
        {"nCreate"         , "()J"     , reinterpret_cast<void*>(Paint_Create)},
        {"nRelease"        , "(J)V"    , reinterpret_cast<void*>(Paint_Release)},
        {"nSetColor"       , "(JFFFF)V", reinterpret_cast<void*>(Paint_SetColor)},
        {"nSetStroke"      , "(JZ)V"   , reinterpret_cast<void*>(Paint_SetStroke)},
        {"nSetStrokeWidth" , "(JF)V"   , reinterpret_cast<void*>(Paint_SetStrokeWidth)},
        {"nSetShader"      , "(JJ)V"   , reinterpret_cast<void*>(Paint_SetShader)},
        {"nSetImageFilter" , "(JJ)V"   , reinterpret_cast<void*>(Paint_SetImageFilter)},
    };

    const auto clazz = env->FindClass("org/skia/androidkit/Paint");
    return clazz
        ? env->RegisterNatives(clazz, methods, SK_ARRAY_COUNT(methods))
        : JNI_ERR;
}
