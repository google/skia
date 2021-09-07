/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <jni.h>

#include "include/core/SkFont.h"
#include "modules/androidkit/src/Utils.h"

namespace {

//TODO: replace jstring fontFamily with TypeFace object. Wrap Typeface in JetSki
static jlong Font_Create(JNIEnv* env, jobject, jstring jFontFamily, float size) {
    const androidkit::utils::CString cFontFamily(env, jFontFamily);
    sk_sp<SkTypeface> typeface = SkTypeface::MakeFromName(cFontFamily, SkFontStyle());
    return reinterpret_cast<jlong>(new SkFont(std::move(typeface), size));
}

static void Font_Release(JNIEnv*, jobject, jlong native_Font) {
    delete reinterpret_cast<SkFont*>(native_Font);
}

}  // namespace

int register_androidkit_Font(JNIEnv* env) {
    static const JNINativeMethod methods[] = {
        {"nCreate" , "(Ljava/lang/String;F)J" , reinterpret_cast<void*>(Font_Create)},
        {"nRelease", "(J)V"                  , reinterpret_cast<void*>(Font_Release)},
    };

    const auto clazz = env->FindClass("org/skia/androidkit/ToyFont");
    return clazz
        ? env->RegisterNatives(clazz, methods, SK_ARRAY_COUNT(methods))
        : JNI_ERR;
}
