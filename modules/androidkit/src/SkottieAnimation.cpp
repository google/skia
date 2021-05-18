/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <jni.h>

#include "modules/androidkit/src/Utils.h"
#include "modules/skottie/include/Skottie.h"

using namespace skottie;

namespace {

static jlong Animation_Create(JNIEnv* env, jobject, jstring jjson) {
    const androidkit::utils::CString cstr(env, jjson);

    // TODO: more builder opts
    auto animation = Animation::Builder().make(cstr, strlen(cstr));

    return reinterpret_cast<jlong>(animation.release());
}

static void Animation_Release(JNIEnv, jobject, jlong native_animation) {
    SkSafeUnref(reinterpret_cast<Animation*>(native_animation));
}

static jdouble Animation_GetDuration(JNIEnv, jobject, jlong native_animation) {
    const auto* animation = reinterpret_cast<const Animation*>(native_animation);
    return animation ? animation->duration() : 0;
}

static jdouble Animation_GetFrameCnt(JNIEnv, jobject, jlong native_animation) {
    const auto* animation = reinterpret_cast<const Animation*>(native_animation);
    return animation ? animation->outPoint() : 0;
}

static jfloat Animation_GetWidth(JNIEnv, jobject, jlong native_animation) {
    const auto* animation = reinterpret_cast<const Animation*>(native_animation);
    return animation ? animation->size().width() : 0;
}

static jfloat Animation_GetHeight(JNIEnv, jobject, jlong native_animation) {
    const auto* animation = reinterpret_cast<const Animation*>(native_animation);
    return animation ? animation->size().height() : 0;
}

static void Animation_SeekFrame(JNIEnv, jobject, jlong native_animation, jdouble frame) {
    if (auto* animation = reinterpret_cast<Animation*>(native_animation)) {
        animation->seekFrame(frame);
    }
}

static void Animation_SeekTime(JNIEnv, jobject, jlong native_animation, jdouble t) {
    if (auto* animation = reinterpret_cast<Animation*>(native_animation)) {
        animation->seekFrameTime(t);
    }
}

static void Animation_Render(JNIEnv, jobject, jlong native_animation, jlong native_canvas) {
    const auto* animation = reinterpret_cast<const Animation*>(native_animation);
    auto* canvas = reinterpret_cast<SkCanvas*>(native_canvas);
    if (animation && canvas) {
        animation->render(canvas);
    }
}

} // namespace

int register_androidkit_SkottieAnimation(JNIEnv* env) {
    static const JNINativeMethod methods[] = {
        {"nCreate"       , "(Ljava/lang/String;)J", reinterpret_cast<void*>(Animation_Create)     },
        {"nRelease"      , "(J)V"                 , reinterpret_cast<void*>(Animation_Release)    },

        {"nGetDuration"  , "(J)D"                 , reinterpret_cast<void*>(Animation_GetDuration)},
        {"nGetFrameCount", "(J)D"                 , reinterpret_cast<void*>(Animation_GetFrameCnt)},
        {"nGetWidth"     , "(J)F"                 , reinterpret_cast<void*>(Animation_GetWidth)   },
        {"nGetHeight"    , "(J)F"                 , reinterpret_cast<void*>(Animation_GetHeight)  },

        {"nSeekFrame"    , "(JD)V"                , reinterpret_cast<void*>(Animation_SeekFrame)  },
        {"nSeekTime"     , "(JD)V"                , reinterpret_cast<void*>(Animation_SeekTime)   },
        {"nRender"       , "(JJ)V"                , reinterpret_cast<void*>(Animation_Render)     },
    };

    const auto clazz = env->FindClass("org/skia/androidkit/SkottieAnimation");
    return clazz
        ? env->RegisterNatives(clazz, methods, SK_ARRAY_COUNT(methods))
        : JNI_ERR;
}
