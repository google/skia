/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <jni.h>

#include "include/core/SkM44.h"

namespace {

static jlong Matrix_Create(JNIEnv* env, jobject, jfloat m0, jfloat m4, jfloat m8,  jfloat m12,
                                                 jfloat m1, jfloat m5, jfloat m9,  jfloat m13,
                                                 jfloat m2, jfloat m6, jfloat m10, jfloat m14,
                                                 jfloat m3, jfloat m7, jfloat m11, jfloat m15) {
    return reinterpret_cast<jlong>(new SkM44(m0, m4, m8, m12,
                                             m1, m5, m9, m13,
                                             m2, m6, m10, m14,
                                             m3, m7, m11, m15));
}
static void Matrix_Release(JNIEnv* env, jobject, jlong native_matrix) {
    delete reinterpret_cast<SkM44*>(native_matrix);
}

static void Matrix_PostConcat(JNIEnv* env, jobject, jlong native_matrixA, jlong native_matrixB) {
    if (auto* mA = reinterpret_cast<SkM44*>(native_matrixA),
            * mB = reinterpret_cast<SkM44*>(native_matrixB); mA && mB) {
        mA->postConcat(*mB);
    }

}

static void Matrix_PreConcat(JNIEnv* env, jobject, jlong native_matrixA, jlong native_matrixB) {
    if (auto* mA = reinterpret_cast<SkM44*>(native_matrixA),
           * mB = reinterpret_cast<SkM44*>(native_matrixB); mA && mB) {
        mA->preConcat(*mB);
    }
}

static long Matrix_Concat(JNIEnv* env, jobject, jlong native_matrixA, jlong native_matrixB) {
    if (auto* mA = reinterpret_cast<SkM44*>(native_matrixA),
           * mB = reinterpret_cast<SkM44*>(native_matrixB); mA && mB) {
        return reinterpret_cast<jlong>(new SkM44(*mA, *mB));
    }
    return 0;
}
} // namespace

int register_androidkit_Matrix(JNIEnv* env) {
    static const JNINativeMethod methods[] = {
        {"nCreate"     , "(FFFFFFFFFFFFFFFF)J" , reinterpret_cast<void*>(Matrix_Create)},
        {"nRelease"    , "(J)V"                , reinterpret_cast<void*>(Matrix_Release)},
        {"nPostConcat" , "(JJ)V"               , reinterpret_cast<void*>(Matrix_PostConcat)},
        {"nPreConcat"  , "(JJ)V"               , reinterpret_cast<void*>(Matrix_PreConcat)},
        {"nConcat"     , "(JJ)J"               , reinterpret_cast<void*>(Matrix_Concat)},
    };

    const auto clazz = env->FindClass("org/skia/androidkit/Matrix");
    return clazz
        ? env->RegisterNatives(clazz, methods, SK_ARRAY_COUNT(methods))
        : JNI_ERR;
}
