/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <mutex>
#include <vector>

#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include "gm_runner.h"
#include "gm_knowledge.h"
#include "skqp_asset_manager.h"
#include "SkStream.h"

////////////////////////////////////////////////////////////////////////////////
extern "C" {
JNIEXPORT void JNICALL Java_org_skia_skqp_SkQP_nInit(JNIEnv*, jobject, jobject, jstring);
JNIEXPORT jfloat JNICALL Java_org_skia_skqp_SkQP_nExecuteGM(JNIEnv*, jobject, jint, jint);
JNIEXPORT jobjectArray JNICALL Java_org_skia_skqp_SkQP_nExecuteUnitTest(JNIEnv*, jobject,
                                                                              jint);
JNIEXPORT void JNICALL Java_org_skia_skqp_SkQP_nMakeReport(JNIEnv*, jobject);
}  // extern "C"
////////////////////////////////////////////////////////////////////////////////

namespace {
struct AndroidAssetManager : public skqp::AssetManager {
    AAssetManager* fMgr = nullptr;
    std::unique_ptr<SkStreamAsset> open(const char* path) override {
        struct AAStrm : public SkStreamAsset {
            AAssetManager* fMgr;
            std::string fPath;
            AAsset* fAsset;
            AAStrm(AAssetManager* m, std::string p, AAsset* a)
                : fMgr(m), fPath(std::move(p)), fAsset(a) {}
            ~AAStrm() override { AAsset_close(fAsset); }
            size_t read(void* buffer, size_t size) override {
                size_t r = SkTMin(size, SkToSizeT(AAsset_getRemainingLength(fAsset)));
                if (buffer) {
                    return SkToSizeT(AAsset_read(fAsset, buffer, r));
                } else {
                    this->move(SkTo<long>(r));
                    return r;
                }
            }
            size_t getLength() const  override { return SkToSizeT(AAsset_getLength(fAsset)); }
            size_t peek(void* buffer, size_t size) const override {
                size_t r = const_cast<AAStrm*>(this)->read(buffer, size);
                const_cast<AAStrm*>(this)->move(-(long)r);
                return r;
            }
            bool isAtEnd() const override { return 0 == AAsset_getRemainingLength(fAsset); }
            bool rewind() override { return this->seek(0); }
            size_t getPosition() const override {
                return SkToSizeT(AAsset_seek(fAsset, 0, SEEK_CUR));
            }
            bool seek(size_t position) override {
                return -1 != AAsset_seek(fAsset, SkTo<off_t>(position), SEEK_SET);
            }
            bool move(long offset) override {
                return -1 != AAsset_seek(fAsset, SkTo<off_t>(offset), SEEK_CUR);
            }
            SkStreamAsset* onDuplicate() const override {
                AAsset* dupAsset = AndroidAssetManager::OpenAsset(fMgr, fPath.c_str());
                return dupAsset ? new AAStrm(fMgr, fPath, dupAsset) : nullptr;
            }
            SkStreamAsset* onFork() const override {
                SkStreamAsset* dup = this->onDuplicate();
                if (dup) { (void)dup->seek(this->getPosition()); }
                return dup;
            }
        };
        AAsset* asset = AndroidAssetManager::OpenAsset(fMgr, path);
        return asset ? std::unique_ptr<SkStreamAsset>(new AAStrm(fMgr, std::string(path), asset))
                     : nullptr;
    }
    static AAsset* OpenAsset(AAssetManager* mgr, const char* path) {
        std::string fullPath = std::string("gmkb/") + path;
        return mgr ? AAssetManager_open(mgr, fullPath.c_str(), AASSET_MODE_STREAMING) : nullptr;
    }
};
}

static void set_string_array_element(JNIEnv* env, jobjectArray a, const char* s, unsigned i) {
    jstring jstr = env->NewStringUTF(s);
    env->SetObjectArrayElement(a, (jsize)i, jstr);
    env->DeleteLocalRef(jstr);
}

#define jassert(env, cond) do { if (!(cond)) { \
    (env)->ThrowNew((env)->FindClass("java/lang/Exception"), \
                    __FILE__ ": assert(" #cond ") failed."); } } while (0)

////////////////////////////////////////////////////////////////////////////////

static std::mutex gMutex;
static std::vector<gm_runner::SkiaBackend> gBackends;
static std::vector<gm_runner::GMFactory> gGMs;
static std::vector<gm_runner::UnitTest> gUnitTests;
static AndroidAssetManager gAssetManager;
static std::string gReportDirectory;
static jclass gStringClass = nullptr;

////////////////////////////////////////////////////////////////////////////////

template <typename T, typename F>
jobjectArray to_java_string_array(JNIEnv* env,
                                  const std::vector<T>& array,
                                  F toString) {
    jobjectArray jarray = env->NewObjectArray((jint)array.size(), gStringClass, nullptr);
    for (unsigned i = 0; i < array.size(); ++i) {
        set_string_array_element(env, jarray, std::string(toString(array[i])).c_str(), i);
    }
    return jarray;
}

void Java_org_skia_skqp_SkQP_nInit(JNIEnv* env, jobject object, jobject assetManager,
                                         jstring dataDir) {
    jclass clazz = env->GetObjectClass(object);
    jassert(env, assetManager);

    gm_runner::InitSkia();

    std::lock_guard<std::mutex> lock(gMutex);
    gAssetManager.fMgr = AAssetManager_fromJava(env, assetManager);
    jassert(env, gAssetManager.fMgr);

    const char* dataDirString = env->GetStringUTFChars(dataDir, nullptr);
    gReportDirectory =  std::string(dataDirString) + "/skqp_report";
    env->ReleaseStringUTFChars(dataDir, dataDirString);

    gBackends = gm_runner::GetSupportedBackends();
    gGMs = gm_runner::GetGMFactories(&gAssetManager);
    gUnitTests = gm_runner::GetUnitTests();
    gStringClass = env->FindClass("java/lang/String");

    constexpr char stringArrayType[] = "[Ljava/lang/String;";
    env->SetObjectField(object, env->GetFieldID(clazz, "mBackends", stringArrayType),
                        to_java_string_array(env, gBackends, gm_runner::GetBackendName));
    env->SetObjectField(object, env->GetFieldID(clazz, "mUnitTests", stringArrayType),
                        to_java_string_array(env, gUnitTests, gm_runner::GetUnitTestName));
    env->SetObjectField(object, env->GetFieldID(clazz, "mGMs", stringArrayType),
                        to_java_string_array(env, gGMs, gm_runner::GetGMName));
}

jfloat Java_org_skia_skqp_SkQP_nExecuteGM(JNIEnv* env,
                                                jobject object,
                                                jint gmIndex,
                                                jint backendIndex) {
    jassert(env, gmIndex < (jint)gGMs.size());
    jassert(env, backendIndex < (jint)gBackends.size());
    gm_runner::GMFactory gm;
    gm_runner::SkiaBackend backend;
    std::string reportDirectoryPath;
    {
        std::lock_guard<std::mutex> lock(gMutex);
        backend = gBackends[backendIndex];
        gm = gGMs[gmIndex];
        reportDirectoryPath = gReportDirectory;
    }
    float result;
    gm_runner::Error error;
    std::tie(result, error) = gm_runner::EvaluateGM(backend, gm, &gAssetManager,
                                                    reportDirectoryPath.c_str());
    if (error != gm_runner::Error::None) {
        (void)env->ThrowNew(env->FindClass("org/skia/skqp/SkQPException"),
                            gm_runner::GetErrorString(error));
    }
    return result;
}

jobjectArray Java_org_skia_skqp_SkQP_nExecuteUnitTest(JNIEnv* env,
                                                            jobject object,
                                                            jint index) {
    jassert(env, index < (jint)gUnitTests.size());
    gm_runner::UnitTest test;
    {
        std::lock_guard<std::mutex> lock(gMutex);
        test = gUnitTests[index];
    }
    std::vector<std::string> errors = gm_runner::ExecuteTest(test);
    if (errors.size() == 0) {
        return nullptr;
    }
    jobjectArray array = env->NewObjectArray(errors.size(), gStringClass, nullptr);
    for (unsigned i = 0; i < errors.size(); ++i) {
        set_string_array_element(env, array, errors[i].c_str(), i);
    }
    return (jobjectArray)env->NewGlobalRef(array);
}

void Java_org_skia_skqp_SkQP_nMakeReport(JNIEnv*, jobject) {
    std::string reportDirectoryPath;
    {
        std::lock_guard<std::mutex> lock(gMutex);
        reportDirectoryPath = gReportDirectory;
    }
    (void)gmkb::MakeReport(reportDirectoryPath.c_str());
}

////////////////////////////////////////////////////////////////////////////////

