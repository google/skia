/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <android/log.h>
#include <jni.h>

#define REGISTER_NATIVES(class_name)                     \
extern int register_androidkit_##class_name(JNIEnv*);    \
if (auto rc = register_androidkit_##class_name(env)) {   \
    __android_log_print(ANDROID_LOG_ERROR, "AndroidKit", \
        "Failed to load natives: " #class_name);         \
    return rc;                                           \
}


JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    JNIEnv* env;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }

    REGISTER_NATIVES(Canvas)
    REGISTER_NATIVES(Paint)
    REGISTER_NATIVES(RuntimeShaderBuilder)
    REGISTER_NATIVES(Shader)
    REGISTER_NATIVES(Surface)

    return JNI_VERSION_1_6;
}
