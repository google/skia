/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <jni.h>

#include "include/core/SkPathBuilder.h"

namespace {

static jlong PathBuilder_Create(JNIEnv* env, jobject) {
    return reinterpret_cast<jlong>(new SkPathBuilder());
}

static void PathBuilder_Release(JNIEnv* env, jobject, jlong native_pathBuilder) {
    delete reinterpret_cast<SkPathBuilder*>(native_pathBuilder);
}

static void PathBuilder_MoveTo(JNIEnv* env, jobject, jlong native_pathBuilder, jfloat x, jfloat y) {
    if (auto* pathBuilder = reinterpret_cast<SkPathBuilder*>(native_pathBuilder)) {
        pathBuilder->moveTo(x, y);
    }
}

static void PathBuilder_QuadTo(JNIEnv* env, jobject, jlong native_pathBuilder, jfloat x1, jfloat y1, jfloat x2, jfloat y2) {
    if (auto* pathBuilder = reinterpret_cast<SkPathBuilder*>(native_pathBuilder)) {
        pathBuilder->quadTo(x1, y1, x2, y2);
    }
}

static void PathBuilder_Close(JNIEnv* env, jobject, jlong native_pathBuilder) {
    if (auto* pathBuilder = reinterpret_cast<SkPathBuilder*>(native_pathBuilder)) {
        pathBuilder->close();
    }
}

static void PathBuilder_SetFillType(JNIEnv* env, jobject, jlong native_pathBuilder, jint fill) {
    if (auto* pathBuilder = reinterpret_cast<SkPathBuilder*>(native_pathBuilder)) {
        switch(fill) {
            case 0:
                pathBuilder->setFillType(SkPathFillType::kWinding);
                break;
            case 1:
                pathBuilder->setFillType(SkPathFillType::kEvenOdd);
                break;
            case 2:
                pathBuilder->setFillType(SkPathFillType::kInverseWinding);
                break;
        }
    }
}

static jlong PathBuilder_MakePath(JNIEnv* env, jobject, jlong native_pathBuilder) {
    if (auto* pathBuilder = reinterpret_cast<SkPathBuilder*>(native_pathBuilder)) {
        SkPath* path = new SkPath(pathBuilder->detach());
        return reinterpret_cast<jlong>(path);
    }

    return 0;
}

}  // namespace

int register_androidkit_PathBuilder(JNIEnv* env) {
    static const JNINativeMethod methods[] = {
        {"nCreate"      , "()J"      , reinterpret_cast<void*>(PathBuilder_Create)},
        {"nRelease"     , "(J)V"     , reinterpret_cast<void*>(PathBuilder_Release)},
        {"nMoveTo"      , "(JFF)V"   , reinterpret_cast<void*>(PathBuilder_MoveTo)},
        {"nQuadTo"      , "(JFFFF)V" , reinterpret_cast<void*>(PathBuilder_QuadTo)},
        {"nClose"       , "(J)V"     , reinterpret_cast<void*>(PathBuilder_Close)},
        {"nSetFillType" , "(JI)V"    , reinterpret_cast<void*>(PathBuilder_SetFillType)},
        {"nMake"        , "(J)J"     , reinterpret_cast<void*>(PathBuilder_MakePath)},
    };

    const auto clazz = env->FindClass("org/skia/androidkit/PathBuilder");
    return clazz
        ? env->RegisterNatives(clazz, methods, SK_ARRAY_COUNT(methods))
        : JNI_ERR;
}
