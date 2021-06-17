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

static void PathBuilder_LineTo(JNIEnv* env, jobject, jlong native_pathBuilder, jfloat x, jfloat y) {
    if (auto* pathBuilder = reinterpret_cast<SkPathBuilder*>(native_pathBuilder)) {
        pathBuilder->lineTo(x, y);
    }
}

static void PathBuilder_QuadTo(JNIEnv* env, jobject, jlong native_pathBuilder, jfloat x1, jfloat y1, jfloat x2, jfloat y2) {
    if (auto* pathBuilder = reinterpret_cast<SkPathBuilder*>(native_pathBuilder)) {
        pathBuilder->quadTo(x1, y1, x2, y2);
    }
}

static void PathBuilder_ConicTo(JNIEnv* env, jobject, jlong native_pathBuilder, jfloat x1, jfloat y1, jfloat x2, jfloat y2, jfloat w) {
    if (auto* pathBuilder = reinterpret_cast<SkPathBuilder*>(native_pathBuilder)) {
        pathBuilder->conicTo(x1, y1, x2, y2, w);
    }
}

static void PathBuilder_CubicTo(JNIEnv* env, jobject, jlong native_pathBuilder, jfloat x1, jfloat y1,
                                                                                jfloat x2, jfloat y2,
                                                                                jfloat x3, jfloat y3) {
    if (auto* pathBuilder = reinterpret_cast<SkPathBuilder*>(native_pathBuilder)) {
        pathBuilder->cubicTo(x1, y1, x2, y2, x3, y3);
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
            case 3:
                pathBuilder->setFillType(SkPathFillType::kInverseEvenOdd);
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
        {"nCreate"      , "()J"       , reinterpret_cast<void*>(PathBuilder_Create)},
        {"nRelease"     , "(J)V"      , reinterpret_cast<void*>(PathBuilder_Release)},
        {"nMoveTo"      , "(JFF)V"    , reinterpret_cast<void*>(PathBuilder_MoveTo)},
        {"nLineTo"      , "(JFF)V"    , reinterpret_cast<void*>(PathBuilder_LineTo)},
        {"nQuadTo"      , "(JFFFF)V"  , reinterpret_cast<void*>(PathBuilder_QuadTo)},
        {"nConicTo"     , "(JFFFFF)V" , reinterpret_cast<void*>(PathBuilder_ConicTo)},
        {"nCubicTo"     , "(JFFFFFF)V", reinterpret_cast<void*>(PathBuilder_CubicTo)},
        {"nClose"       , "(J)V"      , reinterpret_cast<void*>(PathBuilder_Close)},
        {"nSetFillType" , "(JI)V"     , reinterpret_cast<void*>(PathBuilder_SetFillType)},
        {"nMake"        , "(J)J"      , reinterpret_cast<void*>(PathBuilder_MakePath)},
    };

    const auto clazz = env->FindClass("org/skia/androidkit/PathBuilder");
    return clazz
        ? env->RegisterNatives(clazz, methods, SK_ARRAY_COUNT(methods))
        : JNI_ERR;
}
