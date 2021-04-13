/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <jni.h>

#include "include/core/SkPaint.h"

namespace {

static jlong Paint_Create(JNIEnv* env, jobject) {
    return reinterpret_cast<jlong>(new SkPaint);
}

static void Paint_Release(JNIEnv* env, jobject, jlong native_paint) {
    delete reinterpret_cast<SkPaint*>(native_paint);
}

static void Paint_SetColor(JNIEnv* env, jobject, jlong native_paint,
                           float r, float g, float b, float a) {
    reinterpret_cast<SkPaint*>(native_paint)->setColor4f({r, g, b, a});
}

}  // namespace

int register_androidkit_Paint(JNIEnv* env) {
    static const JNINativeMethod methods[] = {
        {"nCreate"  , "()J"     , reinterpret_cast<void*>(Paint_Create)},
        {"nRelease" , "(J)V"    , reinterpret_cast<void*>(Paint_Release)},
        {"nSetColor", "(JFFFF)V", reinterpret_cast<void*>(Paint_SetColor)},
    };

    const auto clazz = env->FindClass("org/skia/androidkit/Paint");
    return clazz
        ? env->RegisterNatives(clazz, methods, SK_ARRAY_COUNT(methods))
        : JNI_ERR;
}
