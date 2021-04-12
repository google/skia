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

}  // namespace

extern "C" JNIEXPORT void
JNICALL
Java_org_skia_androidkit_Canvas_nDrawRect(JNIEnv* env, jobject, jlong native_instance,
                                          jfloat left, jfloat top, jfloat right, jfloat bottom,
                                          jobject paint) {
    if (auto* canvas = reinterpret_cast<SkCanvas*>(native_instance)) {
        canvas->drawRect(SkRect::MakeLTRB(left, top, right, bottom), skpaint(env, paint));
    }
}
