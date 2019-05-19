/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include "include/core/SkTypes.h"
#include "platform_tools/android/apps/skottie/src/main/cpp/JavaInputStreamAdaptor.h"

static jclass findClassCheck(JNIEnv* env, const char classname[]) {
    jclass clazz = env->FindClass(classname);
    SkASSERT(!env->ExceptionCheck());
    return clazz;
}

static jmethodID getMethodIDCheck(JNIEnv* env, jclass clazz,
                                  const char methodname[], const char type[]) {
    jmethodID id = env->GetMethodID(clazz, methodname, type);
    SkASSERT(!env->ExceptionCheck());
    return id;
}

static jmethodID    gInputStream_readMethodID;
static jmethodID    gInputStream_skipMethodID;

static JNIEnv* get_env_or_die(JavaVM* jvm) {
    JNIEnv* env;
    if (jvm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        char errorMessage[256];
        sprintf(errorMessage, "Failed to get JNIEnv for JavaVM: %p", jvm);
        SK_ABORT(errorMessage);
    }
    return env;
}

/**
 *  Wrapper for a Java InputStream.
 */
class JavaInputStreamAdaptor : public SkStream {
    JavaInputStreamAdaptor(JavaVM* jvm, jobject js, jbyteArray ar, jint capacity,
                           bool swallowExceptions)
            : fJvm(jvm)
            , fJavaInputStream(js)
            , fJavaByteArray(ar)
            , fCapacity(capacity)
            , fBytesRead(0)
            , fIsAtEnd(false)
            , fSwallowExceptions(swallowExceptions) {}

public:
    static JavaInputStreamAdaptor* Create(JNIEnv* env, jobject js, jbyteArray ar,
                                          bool swallowExceptions) {
        JavaVM* jvm;
        if (env->GetJavaVM(&jvm) != JNI_OK) {
            SK_ABORT("Failed to get JavaVM");
        }

        js = env->NewGlobalRef(js);
        if (!js) {
            return nullptr;
        }

        ar = (jbyteArray) env->NewGlobalRef(ar);
        if (!ar) {
            env->DeleteGlobalRef(js);
            return nullptr;
        }

        jint capacity = env->GetArrayLength(ar);
        return new JavaInputStreamAdaptor(jvm, js, ar, capacity, swallowExceptions);
    }

    ~JavaInputStreamAdaptor() override {
        auto* env = get_env_or_die(fJvm);
        env->DeleteGlobalRef(fJavaInputStream);
        env->DeleteGlobalRef(fJavaByteArray);
    }

    size_t read(void* buffer, size_t size) override {
        auto* env = get_env_or_die(fJvm);
        if (!fSwallowExceptions && checkException(env)) {
            // Just in case the caller did not clear from a previous exception.
            return 0;
        }
        if (NULL == buffer) {
            if (0 == size) {
                return 0;
            } else {
                /*  InputStream.skip(n) can return <=0 but still not be at EOF
                    If we see that value, we need to call read(), which will
                    block if waiting for more data, or return -1 at EOF
                 */
                size_t amountSkipped = 0;
                do {
                    size_t amount = this->doSkip(size - amountSkipped, env);
                    if (0 == amount) {
                        char tmp;
                        amount = this->doRead(&tmp, 1, env);
                        if (0 == amount) {
                            // if read returned 0, we're at EOF
                            fIsAtEnd = true;
                            break;
                        }
                    }
                    amountSkipped += amount;
                } while (amountSkipped < size);
                return amountSkipped;
            }
        }
        return this->doRead(buffer, size, env);
    }

    bool isAtEnd() const override { return fIsAtEnd; }

private:
    size_t doRead(void* buffer, size_t size, JNIEnv* env) {
        size_t bytesRead = 0;
        // read the bytes
        do {
            jint requested = 0;
            if (size > static_cast<size_t>(fCapacity)) {
                requested = fCapacity;
            } else {
                // This is safe because requested is clamped to (jint)
                // fCapacity.
                requested = static_cast<jint>(size);
            }

            jint n = env->CallIntMethod(fJavaInputStream,
                                        gInputStream_readMethodID, fJavaByteArray, 0, requested);
            if (checkException(env)) {
                SkDebugf("---- read threw an exception\n");
                return bytesRead;
            }

            if (n < 0) { // n == 0 should not be possible, see InputStream read() specifications.
                fIsAtEnd = true;
                break;  // eof
            }

            env->GetByteArrayRegion(fJavaByteArray, 0, n,
                                    reinterpret_cast<jbyte*>(buffer));
            if (checkException(env)) {
                SkDebugf("---- read:GetByteArrayRegion threw an exception\n");
                return bytesRead;
            }

            buffer = (void*)((char*)buffer + n);
            bytesRead += n;
            size -= n;
            fBytesRead += n;
        } while (size != 0);

        return bytesRead;
    }

    size_t doSkip(size_t size, JNIEnv* env) {
        jlong skipped = env->CallLongMethod(fJavaInputStream,
                                            gInputStream_skipMethodID, (jlong)size);
        if (checkException(env)) {
            SkDebugf("------- skip threw an exception\n");
            return 0;
        }
        if (skipped < 0) {
            skipped = 0;
        }

        return (size_t)skipped;
    }

    bool checkException(JNIEnv* env) {
        if (!env->ExceptionCheck()) {
            return false;
        }

        env->ExceptionDescribe();
        if (fSwallowExceptions) {
            env->ExceptionClear();
        }

        // There is no way to recover from the error, so consider the stream
        // to be at the end.
        fIsAtEnd = true;

        return true;
    }

    JavaVM*     fJvm;
    jobject     fJavaInputStream;
    jbyteArray  fJavaByteArray;
    const jint  fCapacity;
    size_t      fBytesRead;
    bool        fIsAtEnd;
    const bool  fSwallowExceptions;
};

static SkStream* CreateJavaInputStreamAdaptor(JNIEnv* env, jobject stream, jbyteArray storage,
                                       bool swallowExceptions = true) {
    return JavaInputStreamAdaptor::Create(env, stream, storage, swallowExceptions);
}

static SkMemoryStream* adaptor_to_mem_stream(SkStream* stream) {
    SkASSERT(stream != NULL);
    size_t bufferSize = 4096;
    size_t streamLen = 0;
    size_t len;
    char* data = (char*)sk_malloc_throw(bufferSize);

    while ((len = stream->read(data + streamLen,
                               bufferSize - streamLen)) != 0) {
        streamLen += len;
        if (streamLen == bufferSize) {
            bufferSize *= 2;
            data = (char*)sk_realloc_throw(data, bufferSize);
        }
    }
    data = (char*)sk_realloc_throw(data, streamLen);

    SkMemoryStream* streamMem = new SkMemoryStream();
    streamMem->setMemoryOwned(data, streamLen);
    return streamMem;
}

SkStreamRewindable* CopyJavaInputStream(JNIEnv* env, jobject stream,
                                        jbyteArray storage) {
    std::unique_ptr<SkStream> adaptor(CreateJavaInputStreamAdaptor(env, stream, storage));
    if (NULL == adaptor.get()) {
        return NULL;
    }
    return adaptor_to_mem_stream(adaptor.get());
}


extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
    JNIEnv* env;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return -1;
    }

    jclass inputStream_Clazz = findClassCheck(env, "java/io/InputStream");
    gInputStream_readMethodID = getMethodIDCheck(env, inputStream_Clazz, "read", "([BII)I");
    gInputStream_skipMethodID = getMethodIDCheck(env, inputStream_Clazz, "skip", "(J)J");

    return JNI_VERSION_1_6;
}
