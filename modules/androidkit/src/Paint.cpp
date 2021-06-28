/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <jni.h>

#include "include/core/SkColorFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkShader.h"
#include "include/effects/SkImageFilters.h"

namespace {

static jlong Paint_Create(JNIEnv*, jobject) {
    return reinterpret_cast<jlong>(new SkPaint);
}

static void Paint_Release(JNIEnv*, jobject, jlong native_paint) {
    delete reinterpret_cast<SkPaint*>(native_paint);
}

static void Paint_SetColor(JNIEnv*, jobject, jlong native_paint,
                           float r, float g, float b, float a) {
    if (auto* paint = reinterpret_cast<SkPaint*>(native_paint)) {
        paint->setColor4f({r, g, b, a});
    }
}

static void Paint_SetStroke(JNIEnv*, jobject, jlong native_paint, jboolean stroke) {
    if (auto* paint = reinterpret_cast<SkPaint*>(native_paint)) {
        paint->setStroke(stroke);
    }
}

static void Paint_SetStrokeWidth(JNIEnv*, jobject, jlong native_paint, jfloat width) {
    if (auto* paint = reinterpret_cast<SkPaint*>(native_paint)) {
        paint->setStrokeWidth(width);
    }
}

static void Paint_SetStrokeCap(JNIEnv* env, jobject, jlong native_paint, jint native_cap) {
    if (auto* paint = reinterpret_cast<SkPaint*>(native_paint)) {
        switch (native_cap)
        {
        case 0:
            paint->setStrokeCap(SkPaint::kButt_Cap);
            break;
        case 1:
            paint->setStrokeCap(SkPaint::kRound_Cap);
            break;
        case 2:
            paint->setStrokeCap(SkPaint::kSquare_Cap);
            break;
        default:
            break;
        }
    }
}

static void Paint_SetStrokeJoin(JNIEnv* env, jobject, jlong native_paint, jint native_join) {
    if (auto* paint = reinterpret_cast<SkPaint*>(native_paint)) {
        switch (native_join)
        {
        case 0:
            paint->setStrokeJoin(SkPaint::kMiter_Join);
            break;
        case 1:
            paint->setStrokeJoin(SkPaint::kRound_Join);
            break;
        case 2:
            paint->setStrokeJoin(SkPaint::kBevel_Join);
            break;
        default:
            break;
        }
    }
}

static void Paint_SetStrokeMiter(JNIEnv* env, jobject, jlong native_paint, jfloat limit) {
    if (auto* paint = reinterpret_cast<SkPaint*>(native_paint)) {
        paint->setStrokeMiter(limit);
    }
}

static void Paint_SetColorFilter(JNIEnv*, jobject, jlong native_paint, jlong native_cf) {
    if (auto* paint = reinterpret_cast<SkPaint*>(native_paint)) {
        paint->setColorFilter(sk_ref_sp(reinterpret_cast<SkColorFilter*>(native_cf)));
    }
}

static void Paint_SetShader(JNIEnv*, jobject, jlong native_paint, jlong native_shader) {
    if (auto* paint = reinterpret_cast<SkPaint*>(native_paint)) {
        paint->setShader(sk_ref_sp(reinterpret_cast<SkShader*>(native_shader)));
    }
}

static void Paint_SetImageFilter(JNIEnv*, jobject, jlong native_paint, jlong native_filter) {
    if (auto* paint = reinterpret_cast<SkPaint*>(native_paint)) {
        paint->setImageFilter(sk_ref_sp(reinterpret_cast<SkImageFilter*>(native_filter)));
    }
}

}  // namespace

int register_androidkit_Paint(JNIEnv* env) {
    static const JNINativeMethod methods[] = {
        {"nCreate"         , "()J"     , reinterpret_cast<void*>(Paint_Create)},
        {"nRelease"        , "(J)V"    , reinterpret_cast<void*>(Paint_Release)},
        {"nSetColor"       , "(JFFFF)V", reinterpret_cast<void*>(Paint_SetColor)},
        {"nSetStroke"      , "(JZ)V"   , reinterpret_cast<void*>(Paint_SetStroke)},
        {"nSetStrokeWidth" , "(JF)V"   , reinterpret_cast<void*>(Paint_SetStrokeWidth)},
        {"nSetStrokeCap"   , "(JI)V"   , reinterpret_cast<void*>(Paint_SetStrokeCap)},
        {"nSetStrokeJoin"  , "(JI)V"   , reinterpret_cast<void*>(Paint_SetStrokeJoin)},
        {"nSetStrokeMiter" , "(JF)V"   , reinterpret_cast<void*>(Paint_SetStrokeMiter)},
        {"nSetColorFilter" , "(JJ)V"   , reinterpret_cast<void*>(Paint_SetColorFilter)},
        {"nSetShader"      , "(JJ)V"   , reinterpret_cast<void*>(Paint_SetShader)},
        {"nSetImageFilter" , "(JJ)V"   , reinterpret_cast<void*>(Paint_SetImageFilter)},
    };

    const auto clazz = env->FindClass("org/skia/androidkit/Paint");
    return clazz
        ? env->RegisterNatives(clazz, methods, SK_ARRAY_COUNT(methods))
        : JNI_ERR;
}
