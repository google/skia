/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <mutex>

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <jni.h>
#include <sys/stat.h>

#include "include/core/SkStream.h"
#include "include/private/SkTo.h"
#include "tools/ResourceFactory.h"

#include "tools/skqp/src/skqp.h"

////////////////////////////////////////////////////////////////////////////////
extern "C" {
JNIEXPORT void JNICALL Java_org_skia_skqp_SkQP_nInit(JNIEnv*, jobject, jobject, jstring);
JNIEXPORT jlong JNICALL Java_org_skia_skqp_SkQP_nExecuteGM(JNIEnv*, jobject, jint, jint);
JNIEXPORT jobjectArray JNICALL Java_org_skia_skqp_SkQP_nExecuteUnitTest(JNIEnv*, jobject, jint);
JNIEXPORT void JNICALL Java_org_skia_skqp_SkQP_nMakeReport(JNIEnv*, jobject);
}  // extern "C"
////////////////////////////////////////////////////////////////////////////////

static AAssetManager* gAAssetManager = nullptr;

static sk_sp<SkData> open_asset_data(const char* path) {
    sk_sp<SkData> data;
    if (gAAssetManager) {
        if (AAsset* asset = AAssetManager_open(gAAssetManager, path, AASSET_MODE_STREAMING)) {
            if (size_t size = SkToSizeT(AAsset_getLength(asset))) {
                data = SkData::MakeUninitialized(size);
                int ret = AAsset_read(asset, data->writable_data(), size);
                if (ret != SkToInt(size)) {
                    SkDebugf("ERROR: AAsset_read != AAsset_getLength (%s)\n", path);
                }
            }
            AAsset_close(asset);
        }
    }
    return data;
}

namespace {
struct AndroidAssetManager : public SkQPAssetManager {
    sk_sp<SkData> open(const char* path) override { return open_asset_data(path); }
};
}

// TODO(halcanary): Should not have global variables; SkQP Java object should
// own pointers and manage concurency.
static AndroidAssetManager gAndroidAssetManager;
static std::mutex gMutex;
static SkQP gSkQP;

#define jassert(env, cond, ret) do { if (!(cond)) { \
    (env)->ThrowNew((env)->FindClass("java/lang/Exception"), \
                    __FILE__ ": assert(" #cond ") failed."); \
    return ret; } } while (0)

static void set_string_array_element(JNIEnv* env, jobjectArray a, const char* s, unsigned i) {
    jstring jstr = env->NewStringUTF(s);
    jassert(env, jstr != nullptr,);
    env->SetObjectArrayElement(a, (jsize)i, jstr);
    env->DeleteLocalRef(jstr);
}

////////////////////////////////////////////////////////////////////////////////

sk_sp<SkData> get_resource(const char* resource) {
    return open_asset_data((std::string("resources/")  + resource).c_str());
}

////////////////////////////////////////////////////////////////////////////////

template <typename T, typename F>
jobjectArray to_java_string_array(JNIEnv* env,
                                  const std::vector<T>& array,
                                  F toString) {
    jclass stringClass = env->FindClass("java/lang/String");
    jassert(env, stringClass, nullptr);
    jobjectArray jarray = env->NewObjectArray((jint)array.size(), stringClass, nullptr);
    jassert(env, jarray != nullptr, nullptr);
    for (unsigned i = 0; i < array.size(); ++i) {
        set_string_array_element(env, jarray, std::string(toString(array[i])).c_str(), i);
    }
    return jarray;
}

static std::string to_string(JNIEnv* env, jstring jString) {
    const char* utf8String = env->GetStringUTFChars(jString, nullptr);
    jassert(env, utf8String && utf8String[0], "");
    std::string sString(utf8String);
    env->ReleaseStringUTFChars(jString, utf8String);
    return sString;
}

void Java_org_skia_skqp_SkQP_nInit(JNIEnv* env, jobject object, jobject assetManager,
                                   jstring dataDir) {
    jclass SkQP_class = env->GetObjectClass(object);

    // tools/Resources
    gResourceFactory = &get_resource;

    std::string reportDirectory = to_string(env, dataDir);

    jassert(env, assetManager,);
    // This global must be set before using AndroidAssetManager
    gAAssetManager = AAssetManager_fromJava(env, assetManager);
    jassert(env, gAAssetManager,);

    std::lock_guard<std::mutex> lock(gMutex);
    gSkQP.init(&gAndroidAssetManager, reportDirectory.c_str());

    auto backends = gSkQP.getSupportedBackends();
    jassert(env, backends.size() > 0,);
    auto gms = gSkQP.getGMs();
    jassert(env, gms.size() > 0,);
    auto unitTests = gSkQP.getUnitTests();
    jassert(env, unitTests.size() > 0,);

    constexpr char kStringArrayType[] = "[Ljava/lang/String;";
    env->SetObjectField(object, env->GetFieldID(SkQP_class, "mBackends", kStringArrayType),
                        to_java_string_array(env, backends, SkQP::GetBackendName));
    env->SetObjectField(object, env->GetFieldID(SkQP_class, "mUnitTests", kStringArrayType),
                        to_java_string_array(env, unitTests, SkQP::GetUnitTestName));
    env->SetObjectField(object, env->GetFieldID(SkQP_class, "mGMs", kStringArrayType),
                        to_java_string_array(env, gms, SkQP::GetGMName));
}

jlong Java_org_skia_skqp_SkQP_nExecuteGM(JNIEnv* env,
                                          jobject object,
                                          jint gmIndex,
                                          jint backendIndex) {
    SkQP::RenderOutcome outcome;
    std::string except;
    {
        std::lock_guard<std::mutex> lock(gMutex);
        jassert(env, backendIndex < (jint)gSkQP.getSupportedBackends().size(), -1);
        jassert(env, gmIndex < (jint)gSkQP.getGMs().size(), -1);
        SkQP::SkiaBackend backend = gSkQP.getSupportedBackends()[backendIndex];
        SkQP::GMFactory gm = gSkQP.getGMs()[gmIndex];
        std::tie(outcome, except) = gSkQP.evaluateGM(backend, gm);
    }

    if (!except.empty()) {
        (void)env->ThrowNew(env->FindClass("org/skia/skqp/SkQPException"), except.c_str());
    }
    return (jlong)outcome.fTotalError;
}

jobjectArray Java_org_skia_skqp_SkQP_nExecuteUnitTest(JNIEnv* env,
                                                      jobject object,
                                                      jint index) {
    std::vector<std::string> errors;
    {
        jassert(env, index < (jint)gSkQP.getUnitTests().size(), nullptr);
        std::lock_guard<std::mutex> lock(gMutex);
        errors = gSkQP.executeTest(gSkQP.getUnitTests()[index]);
    }
    if (errors.size() == 0) {
        return nullptr;
    }
    jclass stringClass = env->FindClass("java/lang/String");
    jassert(env, stringClass, nullptr);
    jobjectArray array = env->NewObjectArray(errors.size(), stringClass, nullptr);
    for (unsigned i = 0; i < errors.size(); ++i) {
        set_string_array_element(env, array, errors[i].c_str(), i);
    }
    return (jobjectArray)env->NewGlobalRef(array);
}

void Java_org_skia_skqp_SkQP_nMakeReport(JNIEnv*, jobject) {
    std::lock_guard<std::mutex> lock(gMutex);
    gSkQP.makeReport();
}

////////////////////////////////////////////////////////////////////////////////

