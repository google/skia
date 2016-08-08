/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "org_skia_canvasproof_GaneshPictureRenderer.h"

#include "GrContext.h"
#include "JavaInputStream.h"
#include "SkCanvas.h"
#include "SkMatrix.h"
#include "SkPicture.h"
#include "SkRect.h"
#include "SkStream.h"
#include "SkSurface.h"

#define TAG "GaneshPictureRenderer.cpp: "

static void render_picture(GrContext* grContext,
                           int width,
                           int height,
                           const SkPicture* picture,
                           const SkMatrix& matrix) {
    SkASSERT(grContext);
    if (!picture) {
        SkDebugf(TAG "!picture\n");
        return;
    }
    // Render to the default framebuffer render target.
    GrBackendRenderTargetDesc desc;
    desc.fWidth = width;
    desc.fHeight = height;
    desc.fConfig = kSkia8888_GrPixelConfig;
    desc.fOrigin = kBottomLeft_GrSurfaceOrigin;
    SkSurfaceProps surfaceProps(SkSurfaceProps::kUseDeviceIndependentFonts_Flag,
                                kUnknown_SkPixelGeometry);
    // TODO:  Check to see if we can keep the surface between draw calls.
    SkAutoTUnref<SkSurface> surface(
            SkSurface::NewFromBackendRenderTarget(
                    grContext, desc, &surfaceProps));
    if (surface) {
        SkCanvas* canvas = surface->getCanvas();
        SkASSERT(canvas);
        canvas->clear(SK_ColorGRAY);
        canvas->concat(matrix);
        SkRect cullRect = picture->cullRect();
        canvas->clipRect(cullRect);
        picture->playback(canvas);
        canvas->flush();
    }
}

namespace {
struct GaneshPictureRendererImpl {
    SkAutoTUnref<GrContext> fGrContext;
    void render(int w, int h, const SkPicture* p, const SkMatrix& m) {
        if (!fGrContext) {
            // Cache the rendering context between frames.
            fGrContext.reset(GrContext::Create(kOpenGL_GrBackend, 0));
            if (!fGrContext) {
                SkDebugf(TAG "GrContext::Create - failed\n");
                return;
            }
        }
        render_picture(fGrContext, w, h, p, m);
    }
};
}  // namespace

/*
 * Class:     org_skia_canvasproof_GaneshPictureRenderer
 * Method:    DrawThisFrame
 * Signature: (IIFJ)V
 */
JNIEXPORT void JNICALL Java_org_skia_canvasproof_GaneshPictureRenderer_DrawThisFrame(
        JNIEnv*, jclass, jint width, jint height, jfloat scale, jlong ptr, jlong pic) {
    if (!ptr) { return; }
    SkMatrix matrix = SkMatrix::MakeScale((SkScalar)scale);
    GaneshPictureRendererImpl* impl =
        reinterpret_cast<GaneshPictureRendererImpl*>(ptr);
    SkPicture* picture = reinterpret_cast<SkPicture*>(pic);
    impl->render((int)width, (int)height, picture, matrix);
}

/*
 * Class:     org_skia_canvasproof_GaneshPictureRenderer
 * Method:    Ctor
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_org_skia_canvasproof_GaneshPictureRenderer_Ctor
  (JNIEnv *, jclass) {
    return reinterpret_cast<jlong>(new GaneshPictureRendererImpl);
}

/*
 * Class:     org_skia_canvasproof_GaneshPictureRenderer
 * Method:    CleanUp
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_org_skia_canvasproof_GaneshPictureRenderer_CleanUp
  (JNIEnv *, jclass, jlong ptr) {
    delete reinterpret_cast<GaneshPictureRendererImpl*>(ptr);
}

namespace {
struct AndroidRectHelper {
    jfieldID fLeft, fTop, fRight, fBottom;
    AndroidRectHelper()
        : fLeft(nullptr), fTop(nullptr), fRight(nullptr), fBottom(nullptr) {}
    void config(JNIEnv *env) {
        if (!fLeft) {
            jclass rectClass = env->FindClass("android/graphics/Rect");
            SkASSERT(rectClass);
            fLeft = env->GetFieldID(rectClass, "left", "I");
            fTop = env->GetFieldID(rectClass, "top", "I");
            fRight = env->GetFieldID(rectClass, "right", "I");
            fBottom = env->GetFieldID(rectClass, "bottom", "I");
        }
    }
};
} // namespace

/*
 * Class:     org_skia_canvasproof_GaneshPictureRenderer
 * Method:    GetCullRect
 * Signature: (Landroid/graphics/Rect;J)V
 */
JNIEXPORT void JNICALL Java_org_skia_canvasproof_GaneshPictureRenderer_GetCullRect
  (JNIEnv *env, jclass, jobject androidGraphicsRect, jlong picturePtr) {
    SkASSERT(androidGraphicsRect);
    const SkPicture* picture = reinterpret_cast<SkPicture*>(picturePtr);
    SkRect rect = SkRect::MakeEmpty();
    if (picture) {
        rect = picture->cullRect();
    }
    SkIRect iRect;
    rect.roundOut(&iRect);
    static AndroidRectHelper help;
    help.config(env);
    env->SetIntField(androidGraphicsRect, help.fLeft, (jint)(iRect.left()));
    env->SetIntField(androidGraphicsRect, help.fTop, (jint)(iRect.top()));
    env->SetIntField(androidGraphicsRect, help.fRight, (jint)(iRect.right()));
    env->SetIntField(androidGraphicsRect, help.fBottom, (jint)(iRect.bottom()));
}
