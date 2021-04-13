/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"

#include <jni.h>

namespace {

void Canvas_DrawRect(JNIEnv* env, jobject, jlong native_instance,
                     jfloat left, jfloat top, jfloat right, jfloat bottom,
                     jlong native_paint) {
    auto* canvas = reinterpret_cast<SkCanvas*>(native_instance);
    auto* paint  = reinterpret_cast<SkPaint* >(native_paint);
    if (canvas && paint) {
        canvas->drawRect(SkRect::MakeLTRB(left, top, right, bottom), *paint);
    }
}

}  // namespace

int register_androidkit_Canvas(JNIEnv* env) {
    static const JNINativeMethod methods[] = {
        {"nDrawRect", "(JFFFFJ)V", reinterpret_cast<void*>(Canvas_DrawRect)},
    };

    const auto clazz = env->FindClass("org/skia/androidkit/Canvas");
    return clazz
        ? env->RegisterNatives(clazz, methods, SK_ARRAY_COUNT(methods))
        : JNI_ERR;
}
