/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <jni.h>
#include <string>
#include "generator/jnigenerator.h"
#include "generator/skia_api_idl.h"

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkiaJNI_generateJNI(JNIEnv* env,
                                                                    jclass jthis,
                                                                    jstring jpath) {
    const char* path = env->GetStringUTFChars(jpath, NULL);
    generator::genJavaAPI(generator::skiaJavaConv, path);
    env->ReleaseStringUTFChars(jpath, path);
}
