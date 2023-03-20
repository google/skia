/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <jni.h>

#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkM44.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkShader.h"
#include "include/core/SkTileMode.h"
#include "include/private/base/SkTo.h"
#include "modules/jetski/src/Utils.h"

namespace {

jlong Image_Create(JNIEnv* env, jobject, jbyteArray jdata) {
    auto  size = env->GetArrayLength(jdata);
    auto* data = env->GetByteArrayElements(jdata, nullptr);
    auto image = SkImage::MakeFromEncoded(SkData::MakeWithCopy(data, SkToSizeT(size)));

    env->ReleaseByteArrayElements(jdata, data, 0);

    return reinterpret_cast<jlong>(image.release());
}

void Image_Release(JNIEnv*, jobject, jlong native_instance) {
    SkSafeUnref(reinterpret_cast<const SkImage*>(native_instance));
}

jint Image_GetWidth(JNIEnv*, jobject, jlong native_instance) {
    const auto* image = reinterpret_cast<const SkImage*>(native_instance);
    return image ? image->width() : 0;
}

jint Image_GetHeight(JNIEnv*, jobject, jlong native_instance) {
    const auto* image = reinterpret_cast<const SkImage*>(native_instance);
    return image ? image->height() : 0;
}

jlong Image_MakeShader(JNIEnv*, jobject, jlong native_instance, jint jtmx, jint jtmy,
                       jint sampling_desc, jfloat sampling_b, jfloat sampling_c,
                       jlong native_matrix) {
    sk_sp<SkShader> shader;

    if (const auto* image = reinterpret_cast<const SkImage*>(native_instance)) {
        const auto tmx = jetski::utils::TileMode(jtmx),
                   tmy = jetski::utils::TileMode(jtmy);
        const auto sampling = jetski::utils::SamplingOptions(sampling_desc,
                                                                 sampling_b, sampling_c);

        const auto* lm = reinterpret_cast<const SkM44*>(native_matrix);
        shader = lm
            ? image->makeShader(tmx, tmy, sampling, lm->asM33())
            : image->makeShader(tmx, tmy, sampling);
    }

    return reinterpret_cast<jlong>(shader.release());
}

} // namespace

int register_jetski_Image(JNIEnv* env) {
    static const JNINativeMethod methods[] = {
        {"nCreate"    , "([B)J"      , reinterpret_cast<void*>(Image_Create)   },
        {"nRelease"   , "(J)V"       , reinterpret_cast<void*>(Image_Release)  },

        {"nGetWidth"  , "(J)I"       , reinterpret_cast<void*>(Image_GetWidth) },
        {"nGetHeight" , "(J)I"       , reinterpret_cast<void*>(Image_GetHeight)},

        {"nMakeShader", "(JIIIFFJ)J" , reinterpret_cast<void*>(Image_MakeShader)},
    };

    const auto clazz = env->FindClass("org/skia/jetski/Image");
    return clazz
        ? env->RegisterNatives(clazz, methods, std::size(methods))
        : JNI_ERR;
}
