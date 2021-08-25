/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <jni.h>

#include "experimental/sktext/src/Paint.h"
#include "modules/androidkit/src/Utils.h"
#include <string>

using namespace skia::text;

namespace {

// conversion function for passing jstrings to SkText static calls
static std::u16string JStringToU16String(JNIEnv* env, const jstring& jstr) {
    const jchar* u16 = env->GetStringChars(jstr, nullptr);
    std::u16string str(reinterpret_cast<const char16_t*>(u16), env->GetStringLength(jstr));
    env->ReleaseStringChars(jstr, u16);
    return str;
}

static void Text_RenderText(JNIEnv* env, jobject, jstring jtext,
                            jlong native_canvas, jlong native_fg_paint,
                            jfloat x, jfloat y) {
    auto* canvas = reinterpret_cast<SkCanvas*>(native_canvas);
    auto foreground = reinterpret_cast<SkPaint*>(native_fg_paint);
    std::u16string u16str = JStringToU16String(env, jtext);
    if (canvas && foreground) {
        SkPaint background(SkColors::kTransparent);
        Paint::drawText(u16str, canvas,
                        skia::text::TextDirection::kLtr, skia::text::TextAlign::kLeft,
                        *foreground, background, SkString("arial"), 50, SkFontStyle::Bold(), x, y);
    }
}

} //namespace

int register_androidkit_Text(JNIEnv* env) {
    static const JNINativeMethod methods[] = {
        {"nRenderText", "(Ljava/lang/String;JJFF)V", reinterpret_cast<void*>(Text_RenderText)},
    };

    const auto clazz = env->FindClass("org/skia/androidkit/Text");
    return clazz
        ? env->RegisterNatives(clazz, methods, SK_ARRAY_COUNT(methods))
        : JNI_ERR;
}
