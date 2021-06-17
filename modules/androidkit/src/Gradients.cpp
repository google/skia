/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <jni.h>

#include "include/core/SkM44.h"
#include "include/effects/SkGradientShader.h"
#include "modules/androidkit/src/Utils.h"

namespace {

static jlong MakeLinear(JNIEnv* env, jobject, jfloat x0, jfloat y0, jfloat x1, jfloat y1,
                        jfloatArray jcolors, jfloatArray jpos, jint jtm, jlong native_lm) {
    const auto count = env->GetArrayLength(jpos);
    SkASSERT(env->GetArrayLength(jcolors) == 4*count);

    auto* colors = env->GetFloatArrayElements(jcolors, nullptr);
    auto* pos    = env->GetFloatArrayElements(jpos, nullptr);

    const SkPoint pts[] = {{x0, y0}, {x1, y1}};

    const auto tm = androidkit::utils::TileMode(jtm);

    SkMatrix lm;
    if (const auto* lm44 = reinterpret_cast<const SkM44*>(native_lm)) {
        lm = lm44->asM33();
    }

    auto shader = SkGradientShader::MakeLinear(pts, reinterpret_cast<const SkColor4f*>(colors),
                                               nullptr, pos, count, tm, 0, &lm);

    env->ReleaseFloatArrayElements(jpos, pos, 0);
    env->ReleaseFloatArrayElements(jcolors, colors, 0);

    return reinterpret_cast<jlong>(shader.release());
}

} // namespace

int register_androidkit_LinearGradient(JNIEnv* env) {
    static const JNINativeMethod methods[] = {
        {"nMakeLinear", "(FFFF[F[FIJ)J", reinterpret_cast<void*>(MakeLinear)},
    };

    const auto clazz = env->FindClass("org/skia/androidkit/LinearGradient");
    return clazz
        ? env->RegisterNatives(clazz, methods, SK_ARRAY_COUNT(methods))
        : JNI_ERR;
}
