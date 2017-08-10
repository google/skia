/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL
// DO NOT USE -- FOR INTERNAL TESTING ONLY

#ifndef gmrunner_DEFINED
#define gmrunner_DEFINED

#include <jni.h>

bool gmrunner_init();
bool gmrunner_run_test(const char* testName);


SK_C_PLUS_PLUS_BEGIN_GUARD


// extern "C" is needed for JNI (although the method itself is in C++)
extern "C" {
  // JNI adapter for gmrunner_init
  JNIEXPORT jlong JNICALL Java_org_skia_cts18_GMRunner_init(JNIEnv* env, jobject gmRunner);

  // JNI adapter for gmrunner_run.
  JNIEXPORT jlong JNICALL Java_org_skia_cts18_GMRunner_runTest(JNIEnv* env, jobject gmRunner);
}

#endif

// use:

// tools/ok_srcs.cpp
// tools/ok_dsts.cpp
// tools/ok_vias.cpp
