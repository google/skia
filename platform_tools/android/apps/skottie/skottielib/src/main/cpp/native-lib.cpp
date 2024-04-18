
/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/codec/SkCodec.h"
#include "include/codec/SkGifDecoder.h"
#include "include/codec/SkJpegDecoder.h"
#include "include/codec/SkPngDecoder.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "include/ports/SkFontMgr_empty.h"

#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrContextOptions.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/gpu/ganesh/gl/GrGLBackendSurface.h"
#include "include/gpu/ganesh/gl/GrGLDirectContext.h"
#include "include/gpu/gl/GrGLInterface.h"
#include "include/gpu/gl/GrGLTypes.h"
#include "include/gpu/ganesh/gl/egl/GrGLMakeEGLInterface.h"

#include "modules/skottie/include/Skottie.h"
#include "modules/skresources/include/SkResources.h"
#include "modules/sksg/include/SkSGInvalidationController.h"
#include "modules/skshaper/utils/FactoryHelpers.h"

#include <jni.h>
#include <math.h>
#include <string>
#include <utility>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <GLES3/gl3.h>

#define STENCIL_BUFFER_SIZE 8

/*#define ATRACE_NAME(name) ScopedTrace ___tracer(name)

// ATRACE_CALL is an ATRACE_NAME that uses the current function name.
#define ATRACE_CALL() ATRACE_NAME(__FUNCTION__)
namespace {
    class ScopedTrace {
    public:
        inline ScopedTrace(const char *name) {
            ATrace_beginSection(name);
        }

        inline ~ScopedTrace() {
            ATrace_endSection();
        }
    };

}*/

//disable atrace
#define ATRACE_NAME(name)
#define ATRACE_CALL()

struct SkottieRunner {
    sk_sp<GrDirectContext> mDContext;
};

static JavaVM* sJVM = nullptr;

static void release_global_jni_ref(const void* /*data*/, void* context) {
    JNIEnv* env;
    if (sJVM->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        SK_ABORT("Attempting to release a JNI ref on a thread without a JVM attached.");
    }
    jobject obj = reinterpret_cast<jobject>(context);
    env->DeleteGlobalRef(obj);
}

extern "C" JNIEXPORT jlong
JNICALL
Java_org_skia_skottie_SkottieRunner_nCreateProxy(JNIEnv *env, jclass clazz) {
    sk_sp<const GrGLInterface> glInterface = GrGLInterfaces::MakeEGL();
    if (!glInterface) {
        return 0;
    }

    GrContextOptions options;
    options.fDisableDistanceFieldPaths = true;
    sk_sp<GrDirectContext> dContext = GrDirectContexts::MakeGL(std::move(glInterface), options);
    if (!dContext) {
        return 0;
    }

    SkottieRunner* skottie = new SkottieRunner();
    skottie->mDContext = std::move(dContext);

    return (jlong) skottie;
}

extern "C" JNIEXPORT void
JNICALL
Java_org_skia_skottie_SkottieRunner_nDeleteProxy(JNIEnv *env, jclass clazz, jlong nativeProxy) {
    if (!nativeProxy) {
        return;
    }
    SkottieRunner* skottie = reinterpret_cast<SkottieRunner*>(nativeProxy);
    if (skottie->mDContext) {
        skottie->mDContext->releaseResourcesAndAbandonContext();
        skottie->mDContext.reset();
    }
    delete skottie;
}


extern "C" JNIEXPORT void
JNICALL
Java_org_skia_skottie_SkottieRunner_nSetMaxCacheSize(JNIEnv *env, jclass clazz, jint maxCacheSize,
                                                                                jlong nativeProxy) {
    if (!nativeProxy) {
            return;
    }
    SkottieRunner* skottie = reinterpret_cast<SkottieRunner*>(nativeProxy);
    if (skottie->mDContext) {
        skottie->mDContext->setResourceCacheLimit(maxCacheSize);
    }
}

struct SkottieAnimation {
    SkottieRunner *mRunner;
    std::unique_ptr<SkStream> mStream;
    sk_sp<skottie::Animation> mAnimation;
    long                      mTimeBase;
    float                     mDuration; //in milliseconds
};

extern "C" JNIEXPORT jlong
JNICALL
Java_org_skia_skottie_SkottieAnimation_nCreateProxy(JNIEnv *env,
                                                    jobject clazz,
                                                    jlong runner,
                                                    jobject bufferObj) {

    if (!runner) {
        return 0;
    }
    SkottieRunner *skottieRunner = reinterpret_cast<SkottieRunner*>(runner);

    const void* buffer = env->GetDirectBufferAddress(bufferObj);
    jlong bufferSize = env->GetDirectBufferCapacity(bufferObj);
    if (buffer == nullptr || bufferSize <= 0) {
        return 0;
    }

    env->GetJavaVM(&sJVM);
    jobject bufferRef = env->NewGlobalRef(bufferObj);
    if (bufferRef == nullptr) {
        return 0;
    }

    sk_sp<SkData> data(SkData::MakeWithProc(buffer, bufferSize, release_global_jni_ref,
                                            reinterpret_cast<void*>(bufferRef)));
    std::unique_ptr<SkStream> stream = SkMemoryStream::Make(data);
    if (!stream) {
        // Cannot create a stream
        return 0;
    }

    SkottieAnimation* skottieAnimation = new SkottieAnimation();
    skottieAnimation->mRunner = skottieRunner;
    skottieAnimation->mStream = std::move(stream);

    sk_sp<SkFontMgr> freetypeMgr = SkFontMgr_New_Custom_Empty();

    SkCodecs::Register(SkPngDecoder::Decoder());
    SkCodecs::Register(SkGifDecoder::Decoder());
    SkCodecs::Register(SkJpegDecoder::Decoder());

    skottieAnimation->mAnimation = skottie::Animation::Builder()
        // Note, this nullptr ResourceProvider will only be able to decode base64 encoded images
        // (using the above registered codecs) or base64 encoded FreeType typefaces.
        .setResourceProvider(skresources::DataURIResourceProviderProxy::Make(nullptr,
            skresources::ImageDecodeStrategy::kPreDecode, freetypeMgr))
        .setTextShapingFactory(SkShapers::BestAvailable())
        .make(skottieAnimation->mStream.get());
    skottieAnimation->mTimeBase  = 0.0f; // force a time reset
    skottieAnimation->mDuration = 1000 * skottieAnimation->mAnimation->duration();

    if (!skottieAnimation->mAnimation) {
        //failed to load Bodymovin animation
        delete skottieAnimation;
        return 0;
    }

    return (jlong) skottieAnimation;
}

extern "C" JNIEXPORT void
JNICALL
Java_org_skia_skottie_SkottieAnimation_nDeleteProxy(JNIEnv *env, jclass clazz,
                                                    jlong nativeProxy) {
    if (!nativeProxy) {
        return;
    }
    SkottieAnimation* skottieAnimation = reinterpret_cast<SkottieAnimation*>(nativeProxy);
    delete skottieAnimation;
}

extern "C" JNIEXPORT bool
JNICALL
Java_org_skia_skottie_SkottieAnimation_nDrawFrame(JNIEnv *env, jclass clazz,
                                                  jlong nativeProxy, jint width,
                                                  jint height,
                                                  jboolean wideColorGamut,
                                                  jfloat progress,
                                                  jint backgroundColor,
                                                  jboolean forceDraw) {
    ATRACE_NAME("SkottieDrawFrame");
    if (!nativeProxy) {
        return false;
    }
    SkottieAnimation* skottieAnimation = reinterpret_cast<SkottieAnimation*>(nativeProxy);

    auto dContext = skottieAnimation->mRunner->mDContext.get();

    if (!dContext) {
        return false;
    }

    sksg::InvalidationController ic;

    if (skottieAnimation->mAnimation) {
        skottieAnimation->mAnimation->seek(progress, &ic);
        if (!forceDraw && ic.bounds().isEmpty()) {
            return false;
        }
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
    fboInfo.fProtected = skgpu::Protected::kNo;
    auto backendRT = GrBackendRenderTargets::MakeGL(width, height, 0, STENCIL_BUFFER_SIZE, fboInfo);

    SkSurfaceProps props(0, kUnknown_SkPixelGeometry);

    sk_sp<SkSurface> renderTarget(SkSurfaces::WrapBackendRenderTarget(
            dContext, backendRT, kBottomLeft_GrSurfaceOrigin, colorType, nullptr, &props));

    auto canvas = renderTarget->getCanvas();
    canvas->clear(backgroundColor);

    SkAutoCanvasRestore acr(canvas, true);
    SkRect bounds = SkRect::MakeWH(width, height);
    skottieAnimation->mAnimation->render(canvas, &bounds);
    dContext->flushAndSubmit();
    return true;
}

extern "C" JNIEXPORT jlong
JNICALL
Java_org_skia_skottie_SkottieAnimation_nGetDuration(JNIEnv *env,
                                                    jclass clazz,
                                                    jlong nativeProxy) {
    if (!nativeProxy) {
        return 0;
    }
    SkottieAnimation* skottieAnimation = reinterpret_cast<SkottieAnimation*>(nativeProxy);
    return (jlong) skottieAnimation->mDuration;
}
