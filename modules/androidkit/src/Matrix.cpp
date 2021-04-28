/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <jni.h>

#include "include/core/SkM44.h"

namespace {

static jlong Matrix_Create(JNIEnv* env, jobject) {
    return reinterpret_cast<jlong>(new SkM44);
}
static void Matrix_Release(JNIEnv* env, jobject, jlong native_matrix) {
    delete reinterpret_cast<SkM44*>(native_matrix);
}

static void Matrix_Add(JNIEnv* env, jobject, jlong native_matrixA, jlong native_matrixB) {
    if (auto* mA = reinterpret_cast<SkM44*>(native_matrixA),
            * mB = reinterpret_cast<SkM44*>(native_matrixB); mA && mB) {
            // do math
        }

}

static void Matrix_Multiply(JNIEnv* env, jobject, jlong native_matrixA, jlong native_matrixB) {
   if (auto* mA = reinterpret_cast<SkM44*>(native_matrixA),
            * mB = reinterpret_cast<SkM44*>(native_matrixB); mA && mB) {
            // do math
        }
}

} // namespace

int register_androidkit_Matrix(JNIEnv* env) {
    static const JNINativeMethod methods[] = {
        {"nCreate"   , "(FFFFFFFFFFFFFFFF)J" , reinterpret_cast<void*>(Matrix_Create)},
        {"nRelease"  , "(J)V"                , reinterpret_cast<void*>(Matrix_Release)},
        {"nAdd"      , "(JJ)V"               , reinterpret_cast<void*>(Matrix_Add)},
        {"nMultiply" , "(JJ)V"               , reinterpret_cast<void*>(Matrix_Multiply)},
    };

    const auto clazz = env->FindClass("org/skia/androidkit/Matrix");
    return clazz
        ? env->RegisterNatives(clazz, methods, SK_ARRAY_COUNT(methods))
        : JNI_ERR;
}
