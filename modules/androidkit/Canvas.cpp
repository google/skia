/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include <jni.h>


/*
 * Takes in a native instance of bitmap and returns a pointer to the raster canvas.
 *
 * The native instance of bitmap provided by Android is not an SkBitmap,
 * so we ignore it for now.
 *
 */
extern "C" JNIEXPORT jlong
JNICALL
Java_org_skia_androidkit_Canvas_nInitRaster(JNIEnv* env, jobject, SkBitmap* /*bitmap*/) {
    SkCanvas* canvas = new SkCanvas();
    return (jlong) canvas;
}

extern "C" JNIEXPORT void
JNICALL
Java_org_skia_androidkit_Canvas_nDrawRect(JNIEnv* env, jobject, jlong canvasProxy,
                                                        jfloat left, jfloat top, jfloat right,
                                                        jfloat bottom, jlong paintProxy) {
    SkCanvas* canvas = reinterpret_cast<SkCanvas*>(canvasProxy);
    SkPaint* paint = reinterpret_cast<SkPaint*>(paintProxy);
    SkScalar left_ = SkFloatToScalar(left);
    SkScalar top_ = SkFloatToScalar(top);
    SkScalar right_ = SkFloatToScalar(right);
    SkScalar bottom_ = SkFloatToScalar(bottom);

    canvas->drawRect(SkRect{left_, top_, right_, bottom_}, *paint);
}
