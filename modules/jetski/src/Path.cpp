/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <jni.h>

#include "include/core/SkPath.h"

namespace {
static void Path_Release(JNIEnv* env, jobject, jlong native_path) {
    delete reinterpret_cast<SkPath*>(native_path);
}

}  // namespace

int register_jetski_Path(JNIEnv* env) {
    static const JNINativeMethod methods[] = {
        {"nRelease" , "(J)V"    , reinterpret_cast<void*>(Path_Release)},
    };

    const auto clazz = env->FindClass("org/skia/jetski/Path");
    return clazz
        ? env->RegisterNatives(clazz, methods, std::size(methods))
        : JNI_ERR;
}
