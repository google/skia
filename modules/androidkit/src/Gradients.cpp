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

// Helper for common gradient data access.
class GradientData {
public:
    GradientData(JNIEnv* env, const jfloatArray& jcolors, const jfloatArray& jpos,
                 jint jtm, jlong native_lm)
        : fEnv(env)
        , fJColors(jcolors)
        , fJPos(jpos)
        , fColors(env->GetFloatArrayElements(jcolors, nullptr))
        , fPos(env->GetFloatArrayElements(jpos, nullptr))
        , fCount(env->GetArrayLength(jpos))
        , fTileMode(androidkit::utils::TileMode(jtm))
        , fLocalMatrix(native_lm ? reinterpret_cast<const SkM44*>(native_lm)->asM33() : SkMatrix())
    {
        SkASSERT(env->GetArrayLength(jcolors) == 4*fCount);
    }

    ~GradientData() {
        fEnv->ReleaseFloatArrayElements(fJPos, fPos, 0);
        fEnv->ReleaseFloatArrayElements(fJColors, fColors, 0);
    }

    int count() const { return fCount; }

    const SkColor4f*  colors()      const { return reinterpret_cast<const SkColor4f*>(fColors); }
    const float*      pos()         const { return fPos; }
    const SkTileMode& tileMode()    const { return fTileMode; }
    const SkMatrix&   localMatrix() const { return fLocalMatrix; }

private:
    JNIEnv*            fEnv;
    const jfloatArray& fJColors;
    const jfloatArray& fJPos;
    float*             fColors;
    float*             fPos;
    const int          fCount;
    const SkTileMode   fTileMode;
    const SkMatrix     fLocalMatrix;
};

static jlong MakeLinear(JNIEnv* env, jobject, jfloat x0, jfloat y0, jfloat x1, jfloat y1,
                        jfloatArray jcolors, jfloatArray jpos, jint jtm, jlong native_lm) {
    const GradientData gdata(env, jcolors, jpos, jtm, native_lm);
    const SkPoint pts[] = {{x0, y0}, {x1, y1}};

    auto shader = SkGradientShader::MakeLinear(pts,
                                               gdata.colors(),
                                               nullptr,
                                               gdata.pos(),
                                               gdata.count(),
                                               gdata.tileMode(),
                                               0,
                                               &gdata.localMatrix());

    return reinterpret_cast<jlong>(shader.release());
}

static jlong MakeRadial(JNIEnv* env, jobject, jfloat x, jfloat y, jfloat r,
                        jfloatArray jcolors, jfloatArray jpos, jint jtm, jlong native_lm) {
    const GradientData gdata(env, jcolors, jpos, jtm, native_lm);

    auto shader = SkGradientShader::MakeRadial({x,y}, r,
                                               gdata.colors(),
                                               nullptr,
                                               gdata.pos(),
                                               gdata.count(),
                                               gdata.tileMode(),
                                               0,
                                               &gdata.localMatrix());

    return reinterpret_cast<jlong>(shader.release());
}

static jlong MakeTwoPointConical(JNIEnv* env, jobject,
                                 jfloat x0, jfloat y0, jfloat r0,
                                 jfloat x1, jfloat y1, jfloat r1,
                                 jfloatArray jcolors, jfloatArray jpos, jint jtm, jlong native_lm) {
    const GradientData gdata(env, jcolors, jpos, jtm, native_lm);

    auto shader = SkGradientShader::MakeTwoPointConical({x0,y0}, r0,
                                                        {x1,y1}, r1,
                                                        gdata.colors(),
                                                        nullptr,
                                                        gdata.pos(),
                                                        gdata.count(),
                                                        gdata.tileMode(),
                                                        0,
                                                        &gdata.localMatrix());

    return reinterpret_cast<jlong>(shader.release());
}

static jlong MakeSweep(JNIEnv* env, jobject, jfloat x, jfloat y, jfloat sa, jfloat ea,
                       jfloatArray jcolors, jfloatArray jpos, jint jtm, jlong native_lm) {
    const GradientData gdata(env, jcolors, jpos, jtm, native_lm);

    auto shader = SkGradientShader::MakeSweep(x, y,
                                              gdata.colors(),
                                              nullptr,
                                              gdata.pos(),
                                              gdata.count(),
                                              gdata.tileMode(),
                                              sa, ea,
                                              0,
                                              &gdata.localMatrix());

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

int register_androidkit_RadialGradient(JNIEnv* env) {
    static const JNINativeMethod methods[] = {
        {"nMakeRadial", "(FFF[F[FIJ)J", reinterpret_cast<void*>(MakeRadial)},
    };

    const auto clazz = env->FindClass("org/skia/androidkit/RadialGradient");
    return clazz
        ? env->RegisterNatives(clazz, methods, SK_ARRAY_COUNT(methods))
        : JNI_ERR;
}

int register_androidkit_TwoPointConicalGradient(JNIEnv* env) {
    static const JNINativeMethod methods[] = {
        {"nMakeTwoPointConical", "(FFFFFF[F[FIJ)J", reinterpret_cast<void*>(MakeTwoPointConical)},
    };

    const auto clazz = env->FindClass("org/skia/androidkit/TwoPointConicalGradient");
    return clazz
        ? env->RegisterNatives(clazz, methods, SK_ARRAY_COUNT(methods))
        : JNI_ERR;
}

int register_androidkit_SweepGradient(JNIEnv* env) {
    static const JNINativeMethod methods[] = {
        {"nMakeSweep", "(FFFF[F[FIJ)J", reinterpret_cast<void*>(MakeSweep)},
    };

    const auto clazz = env->FindClass("org/skia/androidkit/SweepGradient");
    return clazz
        ? env->RegisterNatives(clazz, methods, SK_ARRAY_COUNT(methods))
        : JNI_ERR;
}
