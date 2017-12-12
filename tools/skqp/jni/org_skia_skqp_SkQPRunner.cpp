/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <cassert>
#include <mutex>
#include <vector>

#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include "gm_runner.h"
#include "skqp_asset_manager.h"
#include "SkStream.h"

////////////////////////////////////////////////////////////////////////////////
extern "C" {
JNIEXPORT void JNICALL Java_org_skia_skqp_SkQPRunner_nInit(JNIEnv*, jobject, jobject);
JNIEXPORT jfloat JNICALL Java_org_skia_skqp_SkQPRunner_nExecuteGM(JNIEnv*, jobject, jint, jint);
JNIEXPORT jobjectArray JNICALL Java_org_skia_skqp_SkQPRunner_nExecuteUnitTests(JNIEnv*, jobject,
                                                                               jint);
}  // extern "C"
////////////////////////////////////////////////////////////////////////////////

namespace {
struct AndroidAssetManager : public AssetManager {
    AAssetManager* fMgr = nullptr;
    std::unique_ptr<SkStreamAsset> open(const char* path) override {
        AAsset* asset = AndroidAssetManager::OpenAsset(fMgr, path);
        if (!asset) {
            return nullptr;
        }
        off_t start, length;
        int fd = AAsset_openFileDescriptor(asset, &start, &length);
        SkASSERT(0 == start);
        if (fd < 0) {
            SkASSERT(false);
            AAsset_close(asset);
            return nullptr;
        }
        FILE* fptr = fdopen(fd, "r");
        if (!fptr) {
            SkASSERT(false);
            AAsset_close(asset);
            return nullptr;
        }  // TODO: verify that fclose(fptr) will release asset.
        return std::unique_ptr<SkStreamAsset>(new SkFILEStream(fptr));
    }
    static AAsset* OpenAsset(AAssetManager* mgr, const char* path) {
        std::string fullPath = std::string("gmkb/") + path;
        return mgr ? AAssetManager_open(mgr, fullPath.c_str(), AASSET_MODE_STREAMING) : nullptr;
    }
};
}

#define jassert(env, cond)                                                \
    do { if (!(cond)) {                                                   \
        (void)env->ThrowNew((env)->FindClass("java/lang/AssertionError"), \
                            "assert(" #cond ")");                         \
    } } while (false)

static void set_string_array_element(JNIEnv* env, jobjectArray a, const char* s, unsigned i) {
    jstring jstr = env->NewStringUTF(s);
    env->SetObjectArrayElement(a, (jsize)i, jstr);
    env->DeleteLocalRef(jstr);
}

////////////////////////////////////////////////////////////////////////////////

static std::mutex gMutex;
static std::vector<gm_runner::SkiaBackend> gBackends;
static std::vector<gm_runner::GMFactory> gGMs;
static std::vector<gm_runner::UnitTest> gUnitTests;
static AndroidAssetManager gAssetManager;
static std::string gReportDirectory; // FIXME

////////////////////////////////////////////////////////////////////////////////

void Java_org_skia_skqp_SkQPRunner_nInit(JNIEnv* env, jobject object, jobject assetManager) {
    jclass clazz = env->GetObjectClass(object);
    jassert(env, assetManager);
    std::lock_guard<std::mutex> lock(gMutex);
    gAssetManager.fMgr = AAssetManager_fromJava(env, assetManager);
    jassert(env, gAssetManager.fMgr);
    //gReportDirectory = // FIXME
    gBackends = gm_runner::GetSupportedBackends();
    gGMs = gm_runner::GetGMFactories(&gAssetManager);
    gUnitTests = gm_runner::GetUnitTests();
    jclass stringClass = env->FindClass("java/lang/String");

    jobjectArray array;
    constexpr char stringArrayType[] = "[Ljava/lang/String;";
    array = env->NewObjectArray((jint)gBackends.size(), stringClass, nullptr);
    for (unsigned i = 0; i < gBackends.size(); ++i) {
        set_string_array_element(env, array, gm_runner::GetBackendName(gBackends[i]), i);
    }
    env->SetObjectField(object, env->GetFieldID(clazz, "mBackends", stringArrayType), array);

    array = env->NewObjectArray((jint)gGMs.size(), stringClass, nullptr);
    for (unsigned i = 0; i < gGMs.size(); ++i) {
        set_string_array_element(env, array, gm_runner::GetGMName(gGMs[i]).c_str(), i);
    }
    env->SetObjectField(object, env->GetFieldID(clazz, "mGMs", stringArrayType), array);

    array = env->NewObjectArray((jint)gUnitTests.size(), stringClass, nullptr);
    for (unsigned i = 0; i < gUnitTests.size(); ++i) {
        set_string_array_element(env, array, gm_runner::GetUnitTestName(gUnitTests[i]), i);
    }
    env->SetObjectField(object, env->GetFieldID(clazz, "mUnitTests", stringArrayType), array);
}

jfloat Java_org_skia_skqp_SkQPRunner_nExecuteGM(JNIEnv* env,
                                               jobject object,
                                               jint gmIndex,
                                               jint backendIndex) {
    gm_runner::GMFactory gm;
    gm_runner::SkiaBackend backend;
    std::string reportDirectoryPath;
    {
        std::lock_guard<std::mutex> lock(gMutex);
        backend = gBackends.at(backendIndex);
        gm = gGMs.at(gmIndex);
        reportDirectoryPath = gReportDirectory;
    }
    float result;
    gm_runner::Error error;
    std::tie(result, error) = gm_runner::EvaluateGM(backend, gm, &gAssetManager, "");
    if (error != gm_runner::Error::None) {
        const char* errorString = gm_runner::GetErrorString(error);
        jclass skQPException = env->FindClass("org/skia/skqp/SkQPRunner$SkQPException");
        jmethodID methodID = env->GetMethodID(skQPException, "<init>",
                                              "(Lorg/skia/skqp/SkQPRunner;Ljava/lang/String;)V");
        jobject exception = env->NewObject(skQPException, methodID, object,
                                           env->NewStringUTF(errorString));
        (void)env->Throw((jthrowable)exception);
    }
    return result;
}

jobjectArray Java_org_skia_skqp_SkQPRunner_nExecuteUnitTests(JNIEnv* env,
                                                            jobject object,
                                                            jint index) {
    gm_runner::UnitTest test;
    {
        std::lock_guard<std::mutex> lock(gMutex);
        test = gUnitTests.at(index);
    }
    std::vector<std::string> errors = gm_runner::ExecuteTest(test);
    if (errors.size() == 0) {
        return nullptr;
    }
    jobjectArray array = env->NewObjectArray(errors.size(),
                                             env->FindClass("java/lang/String"), nullptr);
    for (unsigned i = 0; i < errors.size(); ++i) {
        jstring jstr = env->NewStringUTF(errors[i].c_str());
        env->SetObjectArrayElement(array, (jsize)i, jstr);
        env->DeleteLocalRef(jstr);
    }
    return (jobjectArray)env->NewGlobalRef(array);
}

////////////////////////////////////////////////////////////////////////////////

