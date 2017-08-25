#include "gmrunner_jni.h"
#include <gmrunner.h>

#include <list>
#include <string>

JNIEXPORT jobjectArray JNICALL Java_org_skia_cts18_GMRunner_init(
  JNIEnv *env, jclass cls) {
    std::list<std::string> testNames;
    gmrunner_init(testNames);

    jobjectArray ret = (jobjectArray)env->NewObjectArray(testNames.size(),
                                                         env->FindClass("java/lang/String"),
                                                         env->NewStringUTF(""));

    int idx = 0;
    for(auto i=testNames.begin(); i != testNames.end(); i++, idx++) {
        env->SetObjectArrayElement(ret, idx, env->NewStringUTF(i->c_str()));
    }
    return ret;
}


JNIEXPORT jstring JNICALL Java_org_skia_cts18_GMRunner_runGM(
  JNIEnv * env, jclass cls, jstring testName, jobject outImg) {
    const char *test = (*env).GetStringUTFChars(testName, nullptr);
    ImageData imgData;
    auto ret = gmrunner_run_test(test, &imgData);
    (*env).ReleaseStringUTFChars(testName, test);

    jstring result;
    result = (*env).NewStringUTF(ret.c_str());
    return result;
}


