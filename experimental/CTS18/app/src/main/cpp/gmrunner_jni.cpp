#include "gmrunner_jni.h"
#include <gmrunner.h>

JNIEXPORT jstring JNICALL Java_org_skia_cts18_GMRunner_runGM(
  JNIEnv * env, jclass cls, jstring testName, jobject outImg) {

  const char *test = (*env).GetStringUTFChars(testName, nullptr);
  auto ret = gmrunner_run_test(test, nullptr);
  (*env).ReleaseStringUTFChars(testName, test);

  jstring result;
  result = (*env).NewStringUTF(ret.c_str());
  return result;
}
