/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <android/log.h>
#include <jni.h>

#include "include/effects/SkRuntimeEffect.h"

namespace {

static jlong Create(JNIEnv* env, jobject, jstring jsksl) {
    const char* sksl = env->GetStringUTFChars(jsksl, nullptr);
    const auto result = SkRuntimeEffect::Make(SkString(sksl));
    env->ReleaseStringUTFChars(jsksl, sksl);

    if (!result.effect) {
        // TODO: throw exception?
        __android_log_print(ANDROID_LOG_ERROR, "AndroidKit", "Failed to compile shader: %s\n",
                            result.errorText.c_str());
        return 0;
    }

    return reinterpret_cast<jlong>(new SkRuntimeShaderBuilder(std::move(result.effect)));
}

static void Release(JNIEnv* env, jobject, jlong native_instance) {
    if (auto* builder = reinterpret_cast<SkRuntimeShaderBuilder*>(native_instance)) {
        delete builder;
    }
}

static void SetUniform(JNIEnv* env, jobject, jlong native_instance, jstring jname, float val) {
    if (auto* builder = reinterpret_cast<SkRuntimeShaderBuilder*>(native_instance)) {
        const char* name = env->GetStringUTFChars(jname, nullptr);
        builder->uniform(name) = val;
        env->ReleaseStringUTFChars(jname, name);
    }
}

static jlong MakeShader(JNIEnv* env, jobject, jlong native_instance) {
    if (auto* builder = reinterpret_cast<SkRuntimeShaderBuilder*>(native_instance)) {
        auto shader = builder->makeShader(nullptr, false);
        return reinterpret_cast<jlong>(shader.release());
    }

    return 0;
}

}  // namespace

int register_androidkit_RuntimeShaderBuilder(JNIEnv* env) {
    static const JNINativeMethod methods[] = {
        {"nCreate"    , "(Ljava/lang/String;)J"  , reinterpret_cast<void*>(Create)    },
        {"nRelease"   , "(J)V"                   , reinterpret_cast<void*>(Release)   },

        {"nSetUniform", "(JLjava/lang/String;F)V", reinterpret_cast<void*>(SetUniform)},
        {"nMakeShader", "(J)J"                   , reinterpret_cast<void*>(MakeShader)},
    };

    const auto clazz = env->FindClass("org/skia/androidkit/RuntimeShaderBuilder");
    return clazz
        ? env->RegisterNatives(clazz, methods, SK_ARRAY_COUNT(methods))
        : JNI_ERR;
}
