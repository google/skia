/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <jni.h>

#include "include/core/SkShader.h"

namespace {
static void Shader_Release(JNIEnv* env, jobject, jlong native_shader) {
    SkSafeUnref(reinterpret_cast<SkShader*>(native_shader));
}

}  // namespace

int register_androidkit_Shader(JNIEnv* env) {
    static const JNINativeMethod methods[] = {
        {"nRelease" , "(J)V"    , reinterpret_cast<void*>(Shader_Release)},
    };

    const auto clazz = env->FindClass("org/skia/androidkit/Shader");
    return clazz
        ? env->RegisterNatives(clazz, methods, SK_ARRAY_COUNT(methods))
        : JNI_ERR;
}
