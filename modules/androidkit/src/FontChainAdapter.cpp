/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/sktext/include/Text.h"
#include "experimental/sktext/include/Types.h"
#include "include/core/SkCanvas.h"
#include "modules/androidkit/src/Utils.h"

#include <jni.h>

namespace {
static jmethodID gFontChain_countMethodID;
static jmethodID gFontChain_getAtMethodID;
static jmethodID gFontChain_fontSizeMethodID;
static jmethodID gFontChain_localeMethodID;

class FontChainAdapter : public skia::text::FontChain {
    public:
        FontChainAdapter(JNIEnv* env, jobject jFontChain)
            : fEnv(env)
            , fFontChainImpl(jFontChain)
        { }

        size_t count() const override {
            return fEnv->CallIntMethod(fFontChainImpl, gFontChain_countMethodID);
        }

        sk_sp<SkTypeface> operator[](size_t index) const  override {
            SkASSERT(index < this->count());
            SkFont* font = reinterpret_cast<SkFont*>(fEnv->CallLongMethod(fFontChainImpl, gFontChain_getAtMethodID, index));
            if (font) {
                return sk_ref_sp(font->getTypeface());
            }
            return nullptr;
        }

        float fontSize() const override {
            return fEnv->CallFloatMethod(fFontChainImpl, gFontChain_fontSizeMethodID);
        }

        SkString locale() const override {
            jstring jLocale = (jstring)fEnv->CallObjectMethod(fFontChainImpl, gFontChain_localeMethodID);

            const androidkit::utils::CString cstr(fEnv, jLocale);
            SkString str(cstr);
            return str;
        }

    private:
        // TODO: Check, can we hold on to env here or do we have to get it every time?
        JNIEnv* fEnv;
        jobject fFontChainImpl;
};

static jlong FontChain_Create(JNIEnv* env, jobject, jobject jFontChain) {
    return reinterpret_cast<jlong>(new FontChainAdapter(env, jFontChain));
}

static void FontChain_Release(JNIEnv* env, jobject, jlong native_FontChain) {
    SkSafeUnref(reinterpret_cast<FontChainAdapter*>(native_FontChain));
}

} // namespace

int register_androidkit_FontChain(JNIEnv* env) {
    const auto clazz = env->FindClass("org/skia/androidkit/FontChain");

    gFontChain_countMethodID = env->GetMethodID(clazz, "count", "()I");
    gFontChain_getAtMethodID = env->GetMethodID(clazz, "getAt", "(I)J");
    gFontChain_fontSizeMethodID = env->GetMethodID(clazz, "fontSize", "()F");
    gFontChain_localeMethodID = env->GetMethodID(clazz, "locale", "()Ljava/lang/String;");

    if (!gFontChain_countMethodID || !gFontChain_getAtMethodID ||
        !gFontChain_fontSizeMethodID || !gFontChain_localeMethodID) {
            return JNI_ERR;
        }

    static const JNINativeMethod methods[] = {
        {"nCreate" , "(Lorg/skia/androidkit/FontChain;)J", reinterpret_cast<void*>(FontChain_Create)},
        {"nRelease", "(J)V",                               reinterpret_cast<void*>(FontChain_Release)},
    };

    return clazz
        ? env->RegisterNatives(clazz, methods, SK_ARRAY_COUNT(methods))
        : JNI_ERR;
}
