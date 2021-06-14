/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <jni.h>

#include "include/core/SkPath.h"

namespace {

static jlong Path_Create(JNIEnv* env, jobject) {
    return reinterpret_cast<jlong>(new SkPath);
}

static void Path_Release(JNIEnv* env, jobject, jlong native_path) {
    delete reinterpret_cast<SkPath*>(native_path);
}

static void Path_MoveTo(JNIEnv* env, jobject, jlong native_path, jfloat x, jfloat y) {
    if (auto* path = reinterpret_cast<SkPath*>(native_path)) {
        path->moveTo(x, y);
    }
}

static void Path_QuadTo(JNIEnv* env, jobject, jlong native_path, jfloat x1, jfloat y1, jfloat x2, jfloat y2) {
    if (auto* path = reinterpret_cast<SkPath*>(native_path)) {
        path->quadTo(x1, y1, x2, y2);
    }
}

static void Path_Close(JNIEnv* env, jobject, jlong native_path) {
    if (auto* path = reinterpret_cast<SkPath*>(native_path)) {
        path->close();
    }
}

static void Path_SetFillType(JNIEnv* env, jobject, jlong native_path, jint fill) {
    if (auto* path = reinterpret_cast<SkPath*>(native_path)) {
        switch(fill) {
            case 0:
                path->setFillType(SkPathFillType::kWinding);
                break;
            case 1:
                path->setFillType(SkPathFillType::kEvenOdd);
                break;
            case 2:
                path->setFillType(SkPathFillType::kInverseWinding);
                break;
        }
    }
}

}  // namespace

int register_androidkit_Path(JNIEnv* env) {
    static const JNINativeMethod methods[] = {
        {"nCreate"      , "()J"      , reinterpret_cast<void*>(Path_Create)},
        {"nRelease"     , "(J)V"     , reinterpret_cast<void*>(Path_Release)},
        {"nMoveTo"      , "(JFF)V"   , reinterpret_cast<void*>(Path_MoveTo)},
        {"nQuadTo"      , "(JFFFF)V" , reinterpret_cast<void*>(Path_QuadTo)},
        {"nClose"       , "(J)V"     , reinterpret_cast<void*>(Path_Close)},
        {"nSetFillType" , "(JI)V"    , reinterpret_cast<void*>(Path_SetFillType)},
    };

    const auto clazz = env->FindClass("org/skia/androidkit/Path");
    return clazz
        ? env->RegisterNatives(clazz, methods, SK_ARRAY_COUNT(methods))
        : JNI_ERR;
}
