/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <jni.h>

#include "include/core/SkColorFilter.h"
#include "modules/jetski/src/Utils.h"

namespace {

static jlong MakeMatrix(JNIEnv* env, jobject, jfloatArray jcm) {
    SkASSERT(env->GetArrayLength(jcm) == 20);

    auto cf = SkColorFilters::Matrix(jetski::utils::CFloats(env, jcm));

    return reinterpret_cast<jlong>(cf.release());
}

static jlong MakeHSLAMatrix(JNIEnv* env, jobject, jfloatArray jcm) {
    SkASSERT(env->GetArrayLength(jcm) == 20);

    auto cf = SkColorFilters::HSLAMatrix(jetski::utils::CFloats(env, jcm));

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

int register_jetski_ColorFilter(JNIEnv* env) {
    static const JNINativeMethod methods[] = {
        {"nRelease"         , "(J)V" , reinterpret_cast<void*>(ColorFilter_Release)         },
    };

    const auto clazz = env->FindClass("org/skia/jetski/ColorFilter");
    return clazz
        ? env->RegisterNatives(clazz, methods, std::size(methods))
        : JNI_ERR;
}

int register_jetski_MatrixColorFilter(JNIEnv* env) {
    static const JNINativeMethod methods[] = {
        {"nMakeMatrix", "([F)J", reinterpret_cast<void*>(MakeMatrix)},
    };

    const auto clazz = env->FindClass("org/skia/jetski/MatrixColorFilter");
    return clazz
        ? env->RegisterNatives(clazz, methods, std::size(methods))
        : JNI_ERR;
}

int register_jetski_HSLAMatrixColorFilter(JNIEnv* env) {
    static const JNINativeMethod methods[] = {
        {"nMakeHSLAMatrix", "([F)J", reinterpret_cast<void*>(MakeHSLAMatrix)},
    };

    const auto clazz = env->FindClass("org/skia/jetski/HSLAMatrixColorFilter");
    return clazz
        ? env->RegisterNatives(clazz, methods, std::size(methods))
        : JNI_ERR;
}

int register_jetski_ComposeColorFilter(JNIEnv* env) {
    static const JNINativeMethod methods[] = {
        {"nMakeCompose", "(JJ)J", reinterpret_cast<void*>(MakeCompose)},
    };

    const auto clazz = env->FindClass("org/skia/jetski/ComposeColorFilter");
    return clazz
        ? env->RegisterNatives(clazz, methods, std::size(methods))
        : JNI_ERR;
}
