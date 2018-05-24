
/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <jni.h>
#include <string>
#include <utility>
#include <core/SkColor.h>
#include <core/SkCanvas.h>
#include <core/SkBitmap.h>
#include <core/SkSurface.h>
#include <core/SkTime.h>

#include <gpu/GrContextOptions.h>
#include <gpu/GrContext.h>
#include <gpu/gl/GrGLInterface.h>
#include <gpu/GrBackendSurface.h>
#include <gpu/gl/GrGLTypes.h>

#include <skottie/Skottie.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <GLES3/gl3.h>

#define STENCIL_BUFFER_SIZE 8


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
        //LOG_ALWAYS_FATAL("Failed to get JNIEnv for JavaVM: %p", jvm);
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
        //LOG_ALWAYS_FATAL_IF(env->GetJavaVM(&jvm) != JNI_OK);
        if (env->GetJavaVM(&jvm) != JNI_OK) {
            //TODO: crash
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

static SkStreamRewindable* CopyJavaInputStream(JNIEnv* env, jobject stream,
                                        jbyteArray storage) {
    std::unique_ptr<SkStream> adaptor(CreateJavaInputStreamAdaptor(env, stream, storage));
    if (NULL == adaptor.get()) {
        return NULL;
    }
    return adaptor_to_mem_stream(adaptor.get());
}



static int register_android_graphics_CreateJavaStreamAdaptor(JNIEnv* env) {
    jclass inputStream_Clazz = findClassCheck(env, "java/io/InputStream");
    gInputStream_readMethodID = getMethodIDCheck(env, inputStream_Clazz, "read", "([BII)I");
    gInputStream_skipMethodID = getMethodIDCheck(env, inputStream_Clazz, "skip", "(J)J");

    return 0;
}

extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
    JNIEnv* env;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return -1;
    }

    register_android_graphics_CreateJavaStreamAdaptor(env);

    // Get jclass with env->FindClass.
    // Register methods with env->RegisterNatives.

    return JNI_VERSION_1_6;
}

class TempResourceProvider : public skottie::ResourceProvider {
public:
    std::unique_ptr<SkStream> openStream(const char resource[]) const override {
        //TODO: Implement resource manager which knows how to load android assets.
        //TODO: This is needed to load external resources like images for image layers.
        return std::unique_ptr<SkStream>();
    }
};

static TempResourceProvider RP;

struct SkottieRunner {
    sk_sp<GrContext> mGrContext;
};

extern "C" JNIEXPORT jlong
JNICALL
Java_org_skia_skottie_SkottieRunner_nCreateProxy(JNIEnv *env, jclass clazz) {
    sk_sp<const GrGLInterface> glInterface(GrGLCreateNativeInterface());
    if (!glInterface.get()) {
        return 0;
    }

    GrContextOptions options;
    options.fDisableDistanceFieldPaths = true;
    sk_sp<GrContext> grContext = GrContext::MakeGL(std::move(glInterface), options);
    if (!grContext.get()) {
        return 0;
    }

    SkottieRunner* skottie = new SkottieRunner();
    skottie->mGrContext = grContext;

    return (jlong) skottie;
}

extern "C" JNIEXPORT void
JNICALL
Java_org_skia_skottie_SkottieRunner_nDeleteProxy(JNIEnv *env, jclass clazz, jlong nativeProxy) {
    if (!nativeProxy) {
        return;
    }
    SkottieRunner* skottie = reinterpret_cast<SkottieRunner*>(nativeProxy);
    if (skottie->mGrContext) {
        skottie->mGrContext->releaseResourcesAndAbandonContext();
        skottie->mGrContext.reset();
    }
    delete skottie;
}

struct SkottieView {
    SkottieRunner *mRunner;
    std::unique_ptr<SkStream> mStream;
    skottie::Animation::Stats mAnimationStats;
    sk_sp<skottie::Animation> mAnimation;
    double                    mTimeBase;
};

extern "C" JNIEXPORT jlong
JNICALL
Java_org_skia_skottie_SkottieView_nCreateProxy(JNIEnv *env, jclass clazz, jlong runner, jobject is,
                                               jbyteArray storage) {

    if (!runner) {
        return 0;
    }
    SkottieRunner *skottieRunner = reinterpret_cast<SkottieRunner*>(runner);
    std::unique_ptr<SkStream> stream(CopyJavaInputStream(env, is, storage));
    if (!stream.get()) {
        // Cannot create a stream
        return 0;
    }

    SkottieView* skottieView = new SkottieView();
    skottieView->mRunner = skottieRunner;
    skottieView->mStream = std::move(stream);

    skottieView->mAnimation = skottie::Animation::Make(skottieView->mStream.get(), RP, &skottieView->mAnimationStats);
    skottieView->mTimeBase  = 0.0f; // force a time reset

    if (skottieView->mAnimation) {
        skottieView->mAnimation->setShowInval(false);
        /*ALOGI("loaded Bodymovin animation v: %s, size: [%f %f], fr: %f",
              fAnimation->version().c_str(),
              fAnimation->size().width(),
              fAnimation->size().height(),
              fAnimation->frameRate());*/
    } else {
        //failed to load Bodymovin animation
        delete skottieView;
        return 0;
    }

    return (jlong) skottieView;
}

extern "C" JNIEXPORT void
JNICALL
Java_org_skia_skottie_SkottieView_nDeleteProxy(JNIEnv *env, jclass clazz, jlong nativeProxy) {
    if (!nativeProxy) {
        return;
    }
    SkottieView* skottieView = reinterpret_cast<SkottieView*>(nativeProxy);
    delete skottieView;
}

extern "C" JNIEXPORT void
JNICALL
Java_org_skia_skottie_SkottieView_nDrawFrame(JNIEnv *env, jclass clazz, jlong nativeProxy, jint width,
                                             jint height, jboolean wideColorGamut) {
    if (!nativeProxy) {
        return;
    }
    SkottieView* skottieView = reinterpret_cast<SkottieView*>(nativeProxy);

    auto grContext = skottieView->mRunner->mGrContext;

    if (!grContext) {
        return;
    }

    SkColorType colorType;
    // setup surface for fbo0
    GrGLFramebufferInfo fboInfo;
    fboInfo.fFBOID = 0;
    if (wideColorGamut) {
        fboInfo.fFormat = GL_RGBA16F;
        colorType = kRGBA_F16_SkColorType;
    } else {
        fboInfo.fFormat = GL_RGBA8;
        colorType = kN32_SkColorType;
    }
    GrBackendRenderTarget backendRT(width, height, 0, STENCIL_BUFFER_SIZE, fboInfo);

    SkSurfaceProps props(0, kUnknown_SkPixelGeometry);

    sk_sp<SkSurface> renderTarget(SkSurface::MakeFromBackendRenderTarget(
            grContext.get(), backendRT, kBottomLeft_GrSurfaceOrigin, colorType,
            nullptr, &props));

    auto canvas = renderTarget->getCanvas();
    canvas->clear(SK_ColorTRANSPARENT);
    if (skottieView->mAnimation) {
        const double currentTime = SkTime::GetMSecs();

        double t = 0;
        if (skottieView->mTimeBase == 0.0f) {
            // Reset the animation time.
            skottieView->mTimeBase = currentTime;
        } else {
            t = currentTime - skottieView->mTimeBase;
        }

        //TODO: control repeat count [0, (outP - inP) * 1000 / framerate]
        skottieView->mAnimation->animationTick((SkMSec)t);

        SkAutoCanvasRestore acr(canvas, true);
        SkRect bounds = SkRect::MakeWH(width, height);
        skottieView->mAnimation->render(canvas, &bounds);
    }
    canvas->flush();
}
