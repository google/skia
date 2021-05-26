/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <android/log.h>
#include <jni.h>

#include "include/core/SkM44.h"
#include "include/effects/SkRuntimeEffect.h"
#include "modules/androidkit/src/Utils.h"

namespace {

static jlong ShaderBuilder_Create(JNIEnv* env, jobject, jstring jsksl) {
    auto [eff,err] = SkRuntimeEffect::MakeForShader(
                         SkString(androidkit::utils::CString(env, jsksl)));

    if (!eff) {
        // TODO: throw exception?
        __android_log_print(ANDROID_LOG_ERROR, "AndroidKit", "Failed to compile shader: %s\n",
                            err.c_str());
        return 0;
    }

    return reinterpret_cast<jlong>(new SkRuntimeShaderBuilder(std::move(eff)));
}

static void ShaderBuilder_Release(JNIEnv* env, jobject, jlong native_instance) {
    if (auto* builder = reinterpret_cast<SkRuntimeShaderBuilder*>(native_instance)) {
        delete builder;
    }
}

static void ShaderBuilder_SetUniformFloat(JNIEnv* env, jobject, jlong native_instance, jstring jname, float val) {
    if (auto* builder = reinterpret_cast<SkRuntimeShaderBuilder*>(native_instance)) {
        builder->uniform(androidkit::utils::CString(env, jname)) = val;
    }
}

static void ShaderBuilder_SetUniformFloat3(JNIEnv* env, jobject, jlong native_instance, jstring jname, float valX, float valY, float valZ) {
    if (auto* builder = reinterpret_cast<SkRuntimeShaderBuilder*>(native_instance)) {
        builder->uniform(androidkit::utils::CString(env, jname)) = SkV3{valX, valY, valZ};
    }
}

static void ShaderBuilder_SetUniformMatrix(JNIEnv* env, jobject, jlong native_instance, jstring jname, jlong native_matrix) {
    if (auto* builder = reinterpret_cast<SkRuntimeShaderBuilder*>(native_instance)) {
        if (auto* matrix = reinterpret_cast<SkM44*>(native_matrix)) {
            builder->uniform(androidkit::utils::CString(env, jname)) = *matrix;
        }
    }
}

static jlong ShaderBuilder_MakeShader(JNIEnv* env, jobject, jlong native_instance) {
    if (auto* builder = reinterpret_cast<SkRuntimeShaderBuilder*>(native_instance)) {
        auto shader = builder->makeShader(nullptr, false);
        return reinterpret_cast<jlong>(shader.release());
    }

    return 0;
}

}  // namespace

int register_androidkit_RuntimeShaderBuilder(JNIEnv* env) {
    static const JNINativeMethod methods[] = {
        {"nCreate"          , "(Ljava/lang/String;)J"       , reinterpret_cast<void*>(ShaderBuilder_Create)},
        {"nRelease"         , "(J)V"                        , reinterpret_cast<void*>(ShaderBuilder_Release)},

        {"nSetUniformFloat" , "(JLjava/lang/String;F)V"     , reinterpret_cast<void*>(ShaderBuilder_SetUniformFloat)},
        {"nSetUniformFloat3", "(JLjava/lang/String;FFF)V"   , reinterpret_cast<void*>(ShaderBuilder_SetUniformFloat3)},
        {"nSetUniformMatrix", "(JLjava/lang/String;J)V"     , reinterpret_cast<void*>(ShaderBuilder_SetUniformMatrix)},
        {"nMakeShader"      , "(J)J"                        , reinterpret_cast<void*>(ShaderBuilder_MakeShader)},
    };

    const auto clazz = env->FindClass("org/skia/androidkit/RuntimeShaderBuilder");
    return clazz
        ? env->RegisterNatives(clazz, methods, SK_ARRAY_COUNT(methods))
        : JNI_ERR;
}
