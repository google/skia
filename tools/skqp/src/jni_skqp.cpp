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
#include "include/private/base/SkTo.h"
#include "src/utils/SkOSPath.h"
#include "tools/ResourceFactory.h"

#include "tools/skqp/src/skqp.h"

////////////////////////////////////////////////////////////////////////////////
extern "C" {
JNIEXPORT void JNICALL Java_org_skia_skqp_SkQP_nInit(JNIEnv*, jobject, jobject, jstring);
JNIEXPORT jobjectArray JNICALL Java_org_skia_skqp_SkQP_nExecuteUnitTest(JNIEnv*, jobject, jint);
JNIEXPORT void JNICALL Java_org_skia_skqp_SkQP_nMakeReport(JNIEnv*, jobject);
}  // extern "C"
////////////////////////////////////////////////////////////////////////////////

static AAssetManager* sAAssetManager = nullptr;

static sk_sp<SkData> open_asset_data(const char* path) {
    sk_sp<SkData> data;
    if (sAAssetManager) {
        if (AAsset* asset = AAssetManager_open(sAAssetManager, path, AASSET_MODE_STREAMING)) {
            if (size_t size = SkToSizeT(AAsset_getLength(asset))) {
                data = SkData::MakeUninitialized(size);
                int ret = AAsset_read(asset, data->writable_data(), size);
                if (ret != SkToInt(size)) {
                    SK_ABORT("ERROR: AAsset_read != AAsset_getLength (%s)\n", path);
                }
            }
            AAsset_close(asset);
        }
    }
    return data;
}

namespace {
struct AndroidAssetManager : public SkQPAssetManager {
    sk_sp<SkData> open(const char* path) override {
        return open_asset_data(path);
    }

    std::vector<std::string> iterateDir(const char* directory, const char* extension) override {
        std::vector<std::string> paths;
        AAssetDir* assetDir = AAssetManager_openDir(sAAssetManager, directory);

        while (const char* filename = AAssetDir_getNextFileName(assetDir)) {
            const char* ext = strrchr(filename, '.');
            if (!ext) {
                continue;
            }
            if (0 != strcasecmp(extension, ext)) {
                continue;
            }
            SkString path = SkOSPath::Join(directory, filename);
            paths.push_back(path.c_str());
        }

        AAssetDir_close(assetDir);
        return paths;
    }
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

////////////////////////////////////////////////////////////////////////////////

static jobjectArray make_java_string_array(JNIEnv* env, jint arraySize) {
    jclass stringClass = env->FindClass("java/lang/String");
    jassert(env, stringClass, nullptr);
    jobjectArray jarray = env->NewObjectArray(arraySize, stringClass, nullptr);
    jassert(env, jarray != nullptr, nullptr);
    return jarray;
}

static void set_string_array_element(JNIEnv* env, jobjectArray a, const char* s, unsigned i) {
    jstring jstr = env->NewStringUTF(s);
    jassert(env, jstr != nullptr,);
    env->SetObjectArrayElement(a, (jsize)i, jstr);
    env->DeleteLocalRef(jstr);
}

template <typename T, typename F>
static jobjectArray to_java_string_array(JNIEnv* env,
                                         const std::vector<T>& array,
                                         F stringizeFn) {
    jobjectArray jarray = make_java_string_array(env, (jint)array.size());
    for (size_t i = 0; i < array.size(); ++i) {
        set_string_array_element(env, jarray, stringizeFn(array[i]), i);
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

static const char* get_sksl_error_name(const SkQP::SkSLErrorTest& t) {
    return t.name.c_str();
}

static const char* get_sksl_error_shader_text(const SkQP::SkSLErrorTest& t) {
    return t.shaderText.c_str();
}

void Java_org_skia_skqp_SkQP_nInit(JNIEnv* env, jobject object, jobject assetManager,
                                   jstring dataDir) {
    jclass SkQP_class = env->GetObjectClass(object);

    // tools/Resources
    gResourceFactory = &open_asset_data;

    std::string reportDirectory = to_string(env, dataDir);

    jassert(env, assetManager,);
    // This global must be set before using AndroidAssetManager
    sAAssetManager = AAssetManager_fromJava(env, assetManager);
    jassert(env, sAAssetManager,);

    std::lock_guard<std::mutex> lock(gMutex);
    gSkQP.init(&gAndroidAssetManager, reportDirectory.c_str());

    const std::vector<SkQP::UnitTest>& unitTests = gSkQP.getUnitTests();
    const std::vector<SkQP::SkSLErrorTest>& skslErrorTests = gSkQP.getSkSLErrorTests();

    constexpr char kStringArrayType[] = "[Ljava/lang/String;";
    env->SetObjectField(object,
                        env->GetFieldID(SkQP_class, "mUnitTests", kStringArrayType),
                        to_java_string_array(env, unitTests, &SkQP::GetUnitTestName));
    env->SetObjectField(object,
                        env->GetFieldID(SkQP_class, "mSkSLErrorTestName", kStringArrayType),
                        to_java_string_array(env, skslErrorTests, get_sksl_error_name));
    env->SetObjectField(object,
                        env->GetFieldID(SkQP_class, "mSkSLErrorTestShader", kStringArrayType),
                        to_java_string_array(env, skslErrorTests, get_sksl_error_shader_text));
}

jobjectArray Java_org_skia_skqp_SkQP_nExecuteUnitTest(JNIEnv* env,
                                                      jobject object,
                                                      jint index) {
    std::vector<std::string> errors;
    {
        std::lock_guard<std::mutex> lock(gMutex);
        jassert(env, index < (jint)gSkQP.getUnitTests().size(), nullptr);
        errors = gSkQP.executeTest(gSkQP.getUnitTests()[index]);
    }
    if (errors.empty()) {
        return nullptr;
    }
    jobjectArray array = make_java_string_array(env, errors.size());
    for (size_t i = 0; i < errors.size(); ++i) {
        set_string_array_element(env, array, errors[i].c_str(), i);
    }
    return (jobjectArray)env->NewGlobalRef(array);
}

void Java_org_skia_skqp_SkQP_nMakeReport(JNIEnv*, jobject) {
    std::lock_guard<std::mutex> lock(gMutex);
    gSkQP.makeReport();
}

////////////////////////////////////////////////////////////////////////////////

