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

#include "ResourceFactory.h"
#include "SkStream.h"

#include "skqp.h"

////////////////////////////////////////////////////////////////////////////////
extern "C" {
JNIEXPORT void JNICALL Java_org_skia_skqp_SkQP_nInit(JNIEnv*, jobject, jobject, jstring, jboolean);
JNIEXPORT jfloat JNICALL Java_org_skia_skqp_SkQP_nExecuteGM(JNIEnv*, jobject, jint, jint);
JNIEXPORT jobjectArray JNICALL Java_org_skia_skqp_SkQP_nExecuteUnitTest(JNIEnv*, jobject, jint);
JNIEXPORT void JNICALL Java_org_skia_skqp_SkQP_nMakeReport(JNIEnv*, jobject);
}  // extern "C"
////////////////////////////////////////////////////////////////////////////////

static AAssetManager* gAAssetManager = nullptr;
static std::mutex gMutex;
static SkQP gSkQP;
static jclass gStringClass = nullptr;

static sk_sp<SkData> open_asset_data(const char* path) {
    sk_sp<SkData> data;
    if (gAAssetManager) {
        if (AAsset* asset = AAssetManager_open(gAAssetManager, path, AASSET_MODE_STREAMING)) {
            if (size_t size = SkToSizeT(AAsset_getLength(asset))) {
                data = SkData::MakeUninitialized(size);
                (void)AAsset_read(asset, data->writable_data(), size);
            }
            AAsset_close(asset);
        }
    }
    return data;
}

static std::unique_ptr<SkStreamAsset> open_asset_stream(const char* path) {
    #if 0
    sk_sp<SkData> data = open_asset_data(path);
    return data ? std::unique_ptr<SkStreamAsset>(new SkMemoryStream(std::move(data))) : nullptr;
    #else
    struct AAStrm : public SkStreamAsset {
        std::string fPath;
        AAsset* fAsset;
        AAStrm(std::string p, AAsset* a) : fPath(std::move(p)), fAsset(a) {}
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
            return open_asset_stream(fPath.c_str()).release();
        }
        SkStreamAsset* onFork() const override {
            SkStreamAsset* dup = this->onDuplicate();
            if (dup) { (void)dup->seek(this->getPosition()); }
            return dup;
        }
    };
    AAsset* asset =
        gAAssetManager ? AAssetManager_open(gAAssetManager, path, AASSET_MODE_STREAMING) : nullptr;
    return asset ? std::unique_ptr<SkStreamAsset>(new AAStrm(std::string(path), asset))
                 : nullptr;
    #endif
}

namespace {
struct AndroidAssetManager : public SkQPAssetManager {
    std::unique_ptr<SkStreamAsset> open(const char* path) override {
        return open_asset_stream(path);
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

sk_sp<SkData> get_resource(const char* resource) {
    return open_asset_data((std::string("resources/")  + resource).c_str());
}

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

std::string report_directory(JNIEnv* env, jstring dataDir) {
    const char* dataDirString = env->GetStringUTFChars(dataDir, nullptr);
    jassert(env, dataDirString && dataDirString[0]);
    std::string reportDirectory = std::string(dataDirString) + "/skqp_report";
    int mkdirRetval = mkdir(reportDirectory.c_str(), 0777);
    jassert(env, 0 == mkdirRetval);
    env->ReleaseStringUTFChars(dataDir, dataDirString);
    return reportDirectory;
}

void Java_org_skia_skqp_SkQP_nInit(JNIEnv* env, jobject object, jobject assetManager,
                                   jstring dataDir, jboolean experimentalMode) {
    jclass clazz = env->GetObjectClass(object);

    // tools/Resources
    gResourceFactory = &get_resource;

    std::string reportDirectory = report_directory(env, dataDir);

    jassert(env, assetManager);
    // This global must be set before using AndroidAssetManager
    gAAssetManager = AAssetManager_fromJava(env, assetManager);
    jassert(env, assetManager);

    std::lock_guard<std::mutex> lock(gMutex);
    gSkQP.init(std::unique_ptr<SkQPAssetManager>(new AndroidAssetManager),
               reportDirectory.c_str(), 0 != experimentalMode);

    auto backends = gSkQP.getSupportedBackends();
    jassert(env, backends.size() > 0);
    auto gms = gSkQP.getGMs();
    jassert(env, gms.size() > 0);
    auto unitTests = gSkQP.getUnitTests();
    jassert(env, unitTests.size() > 0);
    gStringClass = env->FindClass("java/lang/String");
    jassert(env, gStringClass);

    constexpr char stringArrayType[] = "[Ljava/lang/String;";
    env->SetObjectField(object, env->GetFieldID(clazz, "mBackends", stringArrayType),
                        to_java_string_array(env, backends, SkQP::GetBackendName));
    env->SetObjectField(object, env->GetFieldID(clazz, "mUnitTests", stringArrayType),
                        to_java_string_array(env, unitTests, SkQP::GetUnitTestName));
    env->SetObjectField(object, env->GetFieldID(clazz, "mGMs", stringArrayType),
                        to_java_string_array(env, gms, SkQP::GetGMName));
}

jfloat Java_org_skia_skqp_SkQP_nExecuteGM(JNIEnv* env,
                                          jobject object,
                                          jint gmIndex,
                                          jint backendIndex) {
    int maxError;
    int errorCount;
    std::string except;
    {
        std::lock_guard<std::mutex> lock(gMutex);
        jassert(env, gmIndex < (jint)gSkQP.getSupportedBackends().size());
        jassert(env, backendIndex < (jint)gSkQP.getGMs().size());
        SkQP::SkiaBackend backend = gSkQP.getSupportedBackends()[backendIndex];
        SkQP::GMFactory gm = gSkQP.getGMs()[gmIndex];
        std::tie(maxError, errorCount, except) = gSkQP.evaluateGM(backend, gm);
    }

    if (!except.empty()) {
        (void)env->ThrowNew(env->FindClass("org/skia/skqp/SkQPException"), except.c_str());
    }
    return (float)maxError;  // TODO: switch to int.
}

jobjectArray Java_org_skia_skqp_SkQP_nExecuteUnitTest(JNIEnv* env,
                                                      jobject object,
                                                      jint index) {
    std::vector<std::string> errors;
    {
        jassert(env, index < (jint)gSkQP.getUnitTests().size());
        std::lock_guard<std::mutex> lock(gMutex);
        errors = gSkQP.executeTest(gSkQP.getUnitTests()[index]);
    }
    if (errors.size() == 0) {
        return nullptr;
    }
    jclass stringClass = env->FindClass("java/lang/String");
    jassert(env, stringClass);
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

