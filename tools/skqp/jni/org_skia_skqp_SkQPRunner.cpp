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
JNIEXPORT void JNICALL Java_org_skia_skqp_SkQPRunner_init(JNIEnv*, jobject);
JNIEXPORT jstring JNICALL Java_org_skia_skqp_SkQPRunner_getGMName(JNIEnv*, jobject, jint);
JNIEXPORT jstring JNICALL Java_org_skia_skqp_SkQPRunner_getBackendName(JNIEnv*, jobject, jint);
JNIEXPORT jstring JNICALL Java_org_skia_skqp_SkQPRunner_getUnitTestName(JNIEnv*, jobject, jint);
JNIEXPORT jfloat JNICALL Java_org_skia_skqp_SkQPRunner_executeGM(JNIEnv*, jobject, jint, jint);
JNIEXPORT jobjectArray JNICALL Java_org_skia_skqp_SkQPRunner_executeUnitTests(JNIEnv*, jobject,
                                                                              jint);
}  // extern "C"
////////////////////////////////////////////////////////////////////////////////

static jstring to_global_jstr(JNIEnv* env, const char* cstr) {
    return (jstring)env->NewGlobalRef(env->NewStringUTF(cstr));
}

static jobject get_object_field(JNIEnv* env, jobject object,
                                const char *name, const char *sig) {
    jclass clazz = env->GetObjectClass(object);
    return env->GetObjectField(object, env->GetFieldID(clazz, name, sig));
}

static void set_int_field(JNIEnv* env, jobject object, const char* field, jint value) {
    jclass clazz = env->GetObjectClass(object);
    env->SetIntField(object, env->GetFieldID(clazz, field, "I"), value);
}

template <typename T>
T get_locked_t(std::mutex& mutex, std::vector<T>& array, int index) {
    std::lock_guard<std::mutex> lock(mutex); // Only hold the mutex a short while.
    return array.at(index);
}

////////////////////////////////////////////////////////////////////////////////

struct AndroidAssetManager : public AssetManager {
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

#define jassert(env, cond)                                                \
    do { if (!(cond)) {                                                   \
        (void)env->ThrowNew((env)->FindClass("java/lang/AssertionError"), \
                            "assert(" #cond ")");                         \
    } } while (false)

////////////////////////////////////////////////////////////////////////////////

static std::mutex gMutex;
static std::vector<gm_runner::SkiaBackend> gBackends;
static std::vector<gm_runner::GMFactory> gGMs;
static std::vector<gm_runner::UnitTest> gUnitTests;
static AndroidAssetManager gAssetManager;
static std::string gReportDirectory; // FIXME

////////////////////////////////////////////////////////////////////////////////

void Java_org_skia_skqp_SkQPRunner_init(JNIEnv* env,
                                        jobject object) {
    //   private android.content.res.AssetManager mAssetManager;
    jobject assetManager = get_object_field(env, object, "mAssetManager",
                                            "Landroid/content/res/AssetManager;");
    jassert(env, assetManager);
    std::lock_guard<std::mutex> lock(gMutex);
    gAssetManager.fMgr = AAssetManager_fromJava(env, assetManager);
    jassert(env, gAssetManager.fMgr);
    //gReportDirectory =  "REPORT DIREC"
    gBackends = gm_runner::GetSupportedBackends();
    gGMs = gm_runner::GetGMFactories(&gAssetManager);
    gUnitTests = gm_runner::GetUnitTests();
    set_int_field(env, object, "mBackendCount",  (jint)gBackends.size());
    set_int_field(env, object, "mGMCount",       (jint)gGMs.size());
    set_int_field(env, object, "mUnitTestCount", (jint)gUnitTests.size());
}


jstring Java_org_skia_skqp_SkQPRunner_getGMName(JNIEnv* env, jobject, jint index) {
    return to_global_jstr(env, gm_runner::GetGMName(get_locked_t(gMutex, gGMs, index)).c_str());
}

jstring Java_org_skia_skqp_SkQPRunner_getBackendName(JNIEnv* env, jobject, jint index) {
    return to_global_jstr(env, gm_runner::GetBackendName(get_locked_t(gMutex, gBackends, index)));
}

jstring Java_org_skia_skqp_SkQPRunner_getUnitTestName(JNIEnv* env, jobject, jint index) {
    return to_global_jstr(env, gm_runner::GetUnitTestName(get_locked_t(gMutex, gUnitTests, index)));
}

jfloat Java_org_skia_skqp_SkQPRunner_executeGM(JNIEnv* env,
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

jobjectArray Java_org_skia_skqp_SkQPRunner_executeUnitTests(JNIEnv* env,
                                                            jobject object,
                                                            jint index) {
    gm_runner::UnitTest test = get_locked_t(gMutex, gUnitTests, index);
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

