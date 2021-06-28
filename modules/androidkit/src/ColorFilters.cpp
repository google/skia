/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <jni.h>

#include "include/core/SkColorFilter.h"
#include "modules/androidkit/src/Utils.h"

namespace {

static jlong MakeMatrix(JNIEnv* env, jobject, jfloatArray jcm) {
    SkASSERT(env->GetArrayLength(jcm) == 20);

    auto cf = SkColorFilters::Matrix(androidkit::utils::CFloats(env, jcm));

    return reinterpret_cast<jlong>(cf.release());
}

static jlong MakeHSLAMatrix(JNIEnv* env, jobject, jfloatArray jcm) {
    SkASSERT(env->GetArrayLength(jcm) == 20);

    auto cf = SkColorFilters::HSLAMatrix(androidkit::utils::CFloats(env, jcm));

    return reinterpret_cast<jlong>(cf.release());
}

static jlong MakeCompose(JNIEnv*, jobject, jlong outer, jlong inner) {
    auto cf = SkColorFilters::Compose(sk_ref_sp(reinterpret_cast<SkColorFilter*>(outer)),
                                      sk_ref_sp(reinterpret_cast<SkColorFilter*>(inner)));

    return reinterpret_cast<jlong>(cf.release());
}

static void ColorFilter_Release(JNIEnv*, jobject, jlong native_cf) {
    SkSafeUnref(reinterpret_cast<SkColorFilter*>(native_cf));
}

}  // namespace

int register_androidkit_ColorFilter(JNIEnv* env) {
    static const JNINativeMethod methods[] = {
        {"nRelease"         , "(J)V" , reinterpret_cast<void*>(ColorFilter_Release)         },
    };

    const auto clazz = env->FindClass("org/skia/androidkit/ColorFilter");
    return clazz
        ? env->RegisterNatives(clazz, methods, SK_ARRAY_COUNT(methods))
        : JNI_ERR;
}

int register_androidkit_MatrixColorFilter(JNIEnv* env) {
    static const JNINativeMethod methods[] = {
        {"nMakeMatrix", "([F)J", reinterpret_cast<void*>(MakeMatrix)},
    };

    const auto clazz = env->FindClass("org/skia/androidkit/MatrixColorFilter");
    return clazz
        ? env->RegisterNatives(clazz, methods, SK_ARRAY_COUNT(methods))
        : JNI_ERR;
}

int register_androidkit_HSLAMatrixColorFilter(JNIEnv* env) {
    static const JNINativeMethod methods[] = {
        {"nMakeHSLAMatrix", "([F)J", reinterpret_cast<void*>(MakeHSLAMatrix)},
    };

    const auto clazz = env->FindClass("org/skia/androidkit/HSLAMatrixColorFilter");
    return clazz
        ? env->RegisterNatives(clazz, methods, SK_ARRAY_COUNT(methods))
        : JNI_ERR;
}

int register_androidkit_ComposeColorFilter(JNIEnv* env) {
    static const JNINativeMethod methods[] = {
        {"nMakeCompose", "(JJ)J", reinterpret_cast<void*>(MakeCompose)},
    };

    const auto clazz = env->FindClass("org/skia/androidkit/ComposeColorFilter");
    return clazz
        ? env->RegisterNatives(clazz, methods, SK_ARRAY_COUNT(methods))
        : JNI_ERR;
}
