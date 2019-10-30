/*
 * Copyright 2017 Google Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef C_ARCORE_HELLOE_AR_JNI_INTERFACE_H_
#define C_ARCORE_HELLOE_AR_JNI_INTERFACE_H_

#include <jni.h>
/**
 * Helper functions to provide access to Java from C via JNI.
 */
extern "C" {

// Helper function used to access the jni environment on the current thread.
// In this sample, no consideration is made for detaching the thread when the
// thread exits. This can cause memory leaks, so production applications should
// detach when the thread no longer needs access to the JVM.
JNIEnv *GetJniEnv();

jclass FindClass(const char *classname);
}  // extern "C"
#endif
