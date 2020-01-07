
/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTime.h"
#include <jni.h>
#include <math.h>
#include <string>
#include <utility>

#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrContext.h"
#include "include/gpu/GrContextOptions.h"
#include "include/gpu/gl/GrGLInterface.h"
#include "include/gpu/gl/GrGLTypes.h"

#include "modules/skottie/include/Skottie.h"

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <GLES3/gl3.h>
#include <android/trace.h>
#include "platform_tools/android/apps/skottie/src/main/cpp/JavaInputStreamAdaptor.h"

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
    sk_sp<GrContext> mGrContext;
};

extern "C" JNIEXPORT jlong
JNICALL
Java_org_skia_skottie_SkottieRunner_nCreateProxy(JNIEnv *env, jclass clazz) {
    sk_sp<const GrGLInterface> glInterface = GrGLMakeNativeInterface();
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

struct SkottieAnimation {
    SkottieRunner *mRunner;
    std::unique_ptr<SkStream> mStream;
    sk_sp<skottie::Animation> mAnimation;
    long                      mTimeBase;
    float                     mDuration; //in milliseconds
};

extern "C" JNIEXPORT jlong
JNICALL
Java_org_skia_skottie_SkottieRunner_00024SkottieAnimationImpl_nCreateProxy(JNIEnv *env, jobject clazz,
                                                                       jlong runner, jobject is,
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

    SkottieAnimation* skottieAnimation = new SkottieAnimation();
    skottieAnimation->mRunner = skottieRunner;
    skottieAnimation->mStream = std::move(stream);

    skottieAnimation->mAnimation = skottie::Animation::Make(skottieAnimation->mStream.get());
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
Java_org_skia_skottie_SkottieRunner_00024SkottieAnimationImpl_nDeleteProxy(JNIEnv *env, jclass clazz,
                                                                       jlong nativeProxy) {
    if (!nativeProxy) {
        return;
    }
    SkottieAnimation* skottieAnimation = reinterpret_cast<SkottieAnimation*>(nativeProxy);
    delete skottieAnimation;
}

extern "C" JNIEXPORT void
JNICALL
Java_org_skia_skottie_SkottieRunner_00024SkottieAnimationImpl_nDrawFrame(JNIEnv *env, jclass clazz,
                                                                     jlong nativeProxy, jint width,
                                                                     jint height,
                                                                     jboolean wideColorGamut,
                                                                     jfloat progress) {
    ATRACE_NAME("SkottieDrawFrame");
    if (!nativeProxy) {
        return;
    }
    SkottieAnimation* skottieAnimation = reinterpret_cast<SkottieAnimation*>(nativeProxy);

    auto grContext = skottieAnimation->mRunner->mGrContext;

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
    if (skottieAnimation->mAnimation) {
        skottieAnimation->mAnimation->seek(progress);

        SkAutoCanvasRestore acr(canvas, true);
        SkRect bounds = SkRect::MakeWH(width, height);
        skottieAnimation->mAnimation->render(canvas, &bounds);
    }

    canvas->flush();
}

extern "C" JNIEXPORT jlong
JNICALL
Java_org_skia_skottie_SkottieRunner_00024SkottieAnimationImpl_nGetDuration(JNIEnv *env,
                                                                           jclass clazz,
                                                                           jlong nativeProxy) {
    if (!nativeProxy) {
        return 0;
    }
    SkottieAnimation* skottieAnimation = reinterpret_cast<SkottieAnimation*>(nativeProxy);
    return (jlong) skottieAnimation->mDuration;
}
