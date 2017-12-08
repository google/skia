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
        auto testNameStr = env->NewStringUTF(i->c_str());
        env->SetObjectArrayElement(ret, idx, testNameStr);
        env->DeleteLocalRef(testNameStr);
    }
    return ret;
}


JNIEXPORT jstring JNICALL Java_org_skia_cts18_GMRunner_runGM(
  JNIEnv * env, jclass cls, jstring testName, jobject outImg) {
    // Run the requested GM.
    const char *test = (*env).GetStringUTFChars(testName, nullptr);
    ImageData imgData;
    std::string err;
    gmrunner_run_test(test, &imgData, err);
    (*env).ReleaseStringUTFChars(testName, test);

    // Return an error if necessary.
    jstring result = env->NewStringUTF(err.c_str());
    if (err != "") {
        return result;
    }

    // Copy the image data to a Java object.
    jbyteArray jArr = env->NewByteArray(imgData.pix.size());
    env->SetByteArrayRegion (jArr, 0, imgData.pix.size(),
                             reinterpret_cast<jbyte*>(&imgData.pix[0]));

    jclass clsObj = env->GetObjectClass(outImg);
    auto fid = env->GetFieldID(clsObj, "pix", "[B");
    env->SetObjectField(outImg,fid, jArr);
    env->DeleteLocalRef(jArr);

    fid = env->GetFieldID(clsObj, "width", "I");
    env->SetIntField(outImg, fid, imgData.width);

    fid = env->GetFieldID(clsObj, "height", "I");
    env->SetIntField(outImg, env->GetFieldID(clsObj, "height", "I"), imgData.height);

    fid = env->GetFieldID(clsObj, "byteOrder", "I");
    env->SetIntField(outImg, env->GetFieldID(clsObj, "byteOrder", "I"), imgData.byteOrder);
    return result;
}


