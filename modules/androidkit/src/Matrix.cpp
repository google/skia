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

static jlong Matrix_CreateLookAt(JNIEnv* env, jobject, float eyeX, float eyeY, float eyeZ,
                                                       float coaX, float coaY, float coaZ,
                                                       float upX, float upY, float upZ) {
    return reinterpret_cast<jlong>(new SkM44(SkM44::LookAt({eyeX, eyeY, eyeZ},
                                                           {coaX, coaY, coaZ},
                                                           {upX, upY, upZ})));
}

static jlong Matrix_CreatePerspective(JNIEnv* env, jobject, float near, float far, float angle) {
    return reinterpret_cast<jlong>(new SkM44(SkM44::Perspective(near, far, angle)));
}

static jfloatArray Matrix_GetRowMajor(JNIEnv* env, jobject, jlong native_matrix) {
    jfloatArray result = nullptr;
    if (auto* m = reinterpret_cast<SkM44*>(native_matrix)) {
        SkScalar temp[16];
        m->getRowMajor(temp);

        result = env->NewFloatArray(16);
        if (result) {
            env->SetFloatArrayRegion(result, 0, 16, temp);
        }
    }
    return result;
}

static void Matrix_Release(JNIEnv* env, jobject, jlong native_matrix) {
    delete reinterpret_cast<SkM44*>(native_matrix);
}

static void Matrix_PreConcat(JNIEnv* env, jobject, jlong native_matrixA, jlong native_matrixB) {
    if (auto* mA = reinterpret_cast<SkM44*>(native_matrixA),
            * mB = reinterpret_cast<SkM44*>(native_matrixB); mA && mB) {
        mA->preConcat(*mB);
    }
}

static jlong Matrix_Inverse(JNIEnv* env, jobject, jlong native_matrix) {
    if (auto* m = reinterpret_cast<SkM44*>(native_matrix)) {
        SkM44 inverse(SkM44::kUninitialized_Constructor);
        if (m->invert(&inverse)) {
            return reinterpret_cast<jlong>(new SkM44(inverse));
        }
    }
    return 0;
}

static jlong Matrix_Transpose(JNIEnv* env, jobject, jlong native_matrix) {
    if (auto* matrix = reinterpret_cast<SkM44*>(native_matrix)) {
        SkM44 trans(matrix->transpose());
        return reinterpret_cast<jlong>(new SkM44(trans));
    }
    return 0;
}

static jlong Matrix_Concat(JNIEnv* env, jobject, jlong native_matrixA, jlong native_matrixB) {
    if (auto* mA = reinterpret_cast<SkM44*>(native_matrixA),
            * mB = reinterpret_cast<SkM44*>(native_matrixB); mA && mB) {
        return reinterpret_cast<jlong>(new SkM44(*mA, *mB));
    }
    return 0;
}

static void Matrix_Translate(JNIEnv* env, jobject, jlong native_matrix, jfloat x, jfloat y, jfloat z) {
    if (auto* matrix = reinterpret_cast<SkM44*>(native_matrix)) {
        matrix->preTranslate(x, y, z);
    }
}

static void Matrix_Scale(JNIEnv* env, jobject, jlong native_matrix, jfloat x, jfloat y, jfloat z) {
    if (auto* matrix = reinterpret_cast<SkM44*>(native_matrix)) {
        matrix->preScale(x, y, z);
    }
}

static void Matrix_Rotate(JNIEnv* env, jobject, jlong native_matrix, jfloat x, jfloat y, jfloat z, jfloat rad) {
    if (auto* matrix = reinterpret_cast<SkM44*>(native_matrix)) {
        SkM44 rotate = SkM44::Rotate({x, y, z}, rad);
        matrix->preConcat(rotate);
    }
}

} // namespace

int register_androidkit_Matrix(JNIEnv* env) {
    static const JNINativeMethod methods[] = {
        {"nCreate"            , "(FFFFFFFFFFFFFFFF)J" , reinterpret_cast<void*>(Matrix_Create)},
        {"nCreateLookAt"      , "(FFFFFFFFF)J"        , reinterpret_cast<void*>(Matrix_CreateLookAt)},
        {"nCreatePerspective" , "(FFF)J"              , reinterpret_cast<void*>(Matrix_CreatePerspective)},
        {"nGetRowMajor"       , "(J)[F"               , reinterpret_cast<void*>(Matrix_GetRowMajor)},
        {"nRelease"           , "(J)V"                , reinterpret_cast<void*>(Matrix_Release)},
        {"nInverse"           , "(J)J"                , reinterpret_cast<void*>(Matrix_Inverse)},
        {"nTranspose"         , "(J)J"                , reinterpret_cast<void*>(Matrix_Transpose)},
        {"nPreConcat"         , "(JJ)V"               , reinterpret_cast<void*>(Matrix_PreConcat)},
        {"nConcat"            , "(JJ)J"               , reinterpret_cast<void*>(Matrix_Concat)},
        {"nTranslate"         , "(JFFF)V"             , reinterpret_cast<void*>(Matrix_Translate)},
        {"nScale"             , "(JFFF)V"             , reinterpret_cast<void*>(Matrix_Scale)},
        {"nRotate"            , "(JFFFF)V"            , reinterpret_cast<void*>(Matrix_Rotate)},
    };

    const auto clazz = env->FindClass("org/skia/androidkit/Matrix");
    return clazz
        ? env->RegisterNatives(clazz, methods, SK_ARRAY_COUNT(methods))
        : JNI_ERR;
}
