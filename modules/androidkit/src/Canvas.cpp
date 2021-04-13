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

SkPaint skpaint(JNIEnv* env, jobject jpaint) {
    SkPaint paint;

    // TODO: reflect jpaint
    paint.setColor(0xff00ff00);

    return paint;
}

void Canvas_DrawRect(JNIEnv* env, jobject, jlong native_instance,
                     jfloat left, jfloat top, jfloat right, jfloat bottom, jobject paint) {
    if (auto* canvas = reinterpret_cast<SkCanvas*>(native_instance)) {
        canvas->drawRect(SkRect::MakeLTRB(left, top, right, bottom), skpaint(env, paint));
    }
}

}  // namespace

int register_androidkit_Canvas(JNIEnv* env) {
    static const JNINativeMethod methods[] = {
        {"nDrawRect", "(JFFFFLandroid/graphics/Paint;)V", reinterpret_cast<void*>(Canvas_DrawRect)},
    };

    const auto clazz = env->FindClass("org/skia/androidkit/Canvas");
    return clazz
        ? env->RegisterNatives(clazz, methods, SK_ARRAY_COUNT(methods))
        : JNI_ERR;
}
