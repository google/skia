/*
 * Copyright (C) 2011 Skia
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
 *
 */

#include "GrContext.h"
#include "GrGLInterface.h"
#include "SampleApp.h"
#include "SkApplication.h"
#include "SkCanvas.h"
#include "SkDevice.h"
#include "SkEvent.h"
#include "SkGpuCanvas.h"
#include "SkWindow.h"

#include <jni.h>
#include "utils/android/AndroidKeyToSkKey.h"


///////////////////////////////////////////
///////////////// Globals /////////////////
///////////////////////////////////////////

struct ActivityGlue {
    JNIEnv* m_env;
    jweak m_obj;
    jmethodID m_setTitle;
    jmethodID m_startTimer;
    jmethodID m_addToDownloads;
    ActivityGlue() {
        m_env = NULL;
        m_obj = NULL;
        m_setTitle = NULL;
        m_startTimer = NULL;
        m_addToDownloads = NULL;
    }
} gActivityGlue;

struct WindowGlue {
    jweak m_obj;
    jmethodID m_inval;
    jmethodID m_queueSkEvent;
    WindowGlue() {
        m_obj = NULL;
        m_inval = NULL;
        m_queueSkEvent = NULL;
    }
} gWindowGlue;

SampleWindow* gWindow;

///////////////////////////////////////////
///////////// SkOSWindow impl /////////////
///////////////////////////////////////////

void SkOSWindow::onSetTitle(const char title[])
{
    if (gActivityGlue.m_env) {
        JNIEnv* env = gActivityGlue.m_env;
        jstring string = env->NewStringUTF(title);
        env->CallVoidMethod(gActivityGlue.m_obj, gActivityGlue.m_setTitle,
                string);
        env->DeleteLocalRef(string);
    }
}

void SkOSWindow::onHandleInval(const SkIRect& rect)
{
    if (!gActivityGlue.m_env || !gWindowGlue.m_inval || !gWindowGlue.m_obj) {
        return;
    }
    gActivityGlue.m_env->CallVoidMethod(gWindowGlue.m_obj, gWindowGlue.m_inval);
}

void SkOSWindow::onPDFSaved(const char title[], const char desc[],
        const char path[])
{
    if (gActivityGlue.m_env) {
        JNIEnv* env = gActivityGlue.m_env;
        jstring jtitle = env->NewStringUTF(title);
        jstring jdesc = env->NewStringUTF(desc);
        jstring jpath = env->NewStringUTF(path);

        env->CallVoidMethod(gActivityGlue.m_obj, gActivityGlue.m_addToDownloads,
                jtitle, jdesc, jpath);

        env->DeleteLocalRef(jtitle);
        env->DeleteLocalRef(jdesc);
        env->DeleteLocalRef(jpath);
    }
}

///////////////////////////////////////////
/////////////// SkEvent impl //////////////
///////////////////////////////////////////

void SkEvent::SignalQueueTimer(SkMSec ms)
{
    if (!gActivityGlue.m_env || !gActivityGlue.m_startTimer
            || !gActivityGlue.m_obj || !ms) {
        return;
    }
    gActivityGlue.m_env->CallVoidMethod(gActivityGlue.m_obj,
            gActivityGlue.m_startTimer, ms);
}

void SkEvent::SignalNonEmptyQueue()
{
    if (!gActivityGlue.m_env || !gWindowGlue.m_queueSkEvent
            || !gWindowGlue.m_obj) {
        return;
    }
    gActivityGlue.m_env->CallVoidMethod(gWindowGlue.m_obj,
            gWindowGlue.m_queueSkEvent);
}

///////////////////////////////////////////
////////////////// JNI ////////////////////
///////////////////////////////////////////

static jmethodID GetJMethod(JNIEnv* env, jclass clazz, const char name[],
        const char signature[])
{
    jmethodID m = env->GetMethodID(clazz, name, signature);
    if (!m) SkDebugf("Could not find Java method %s\n", name);
    return m;
}

extern "C" {
JNIEXPORT void JNICALL Java_com_skia_sampleapp_SampleApp_draw(
        JNIEnv* env, jobject thiz);
JNIEXPORT void JNICALL Java_com_skia_sampleapp_SampleApp_init(
        JNIEnv* env, jobject thiz);
JNIEXPORT void JNICALL Java_com_skia_sampleapp_SampleApp_term(
        JNIEnv* env, jobject thiz);
JNIEXPORT void JNICALL Java_com_skia_sampleapp_SampleApp_updateSize(
        JNIEnv* env, jobject thiz, jint w, jint h);
JNIEXPORT bool JNICALL Java_com_skia_sampleapp_SampleApp_handleKeyDown(
        JNIEnv* env, jobject thiz, jint keyCode, jint uni);
JNIEXPORT bool JNICALL Java_com_skia_sampleapp_SampleApp_handleKeyUp(
        JNIEnv* env, jobject thiz, jint keyCode);
JNIEXPORT void JNICALL Java_com_skia_sampleapp_SampleApp_handleClick(
        JNIEnv* env, jobject thiz, jint owner, jfloat x, jfloat y, jint state);
JNIEXPORT void JNICALL Java_com_skia_sampleapp_SampleApp_createOSWindow(
        JNIEnv* env, jobject thiz, jobject jsampleView);
JNIEXPORT void JNICALL Java_com_skia_sampleapp_SampleApp_setZoomCenter(
        JNIEnv* env, jobject thiz, jfloat x, jfloat y);
JNIEXPORT void JNICALL Java_com_skia_sampleapp_SampleApp_zoom(
        JNIEnv* env, jobject thiz, jfloat factor);
JNIEXPORT void JNICALL Java_com_skia_sampleapp_SampleApp_nextSample(
        JNIEnv* env, jobject thiz, jboolean fprevious);
JNIEXPORT void JNICALL Java_com_skia_sampleapp_SampleApp_toggleRendering(
        JNIEnv* env, jobject thiz);
JNIEXPORT void JNICALL Java_com_skia_sampleapp_SampleApp_toggleSlideshow(
        JNIEnv* env, jobject thiz);
JNIEXPORT void JNICALL Java_com_skia_sampleapp_SampleApp_toggleFps(
        JNIEnv* env, jobject thiz);
JNIEXPORT void JNICALL Java_com_skia_sampleapp_SampleApp_processSkEvent(
        JNIEnv* env, jobject thiz);
JNIEXPORT void JNICALL Java_com_skia_sampleapp_SampleApp_serviceQueueTimer(
        JNIEnv* env, jobject thiz);
JNIEXPORT void JNICALL Java_com_skia_sampleapp_SampleApp_saveToPdf(
        JNIEnv* env, jobject thiz);
JNIEXPORT void JNICALL Java_com_skia_sampleapp_SampleApp_postInval(
        JNIEnv* env, jobject thiz);
};

JNIEXPORT bool JNICALL Java_com_skia_sampleapp_SampleApp_handleKeyDown(
        JNIEnv* env, jobject thiz, jint keyCode, jint uni)
{
    bool handled = gWindow->handleKey(AndroidKeycodeToSkKey(keyCode));
    handled |= gWindow->handleChar((SkUnichar) uni);
    return handled;
}

JNIEXPORT bool JNICALL Java_com_skia_sampleapp_SampleApp_handleKeyUp(JNIEnv* env,
        jobject thiz, jint keyCode)
{
    return gWindow->handleKeyUp(AndroidKeycodeToSkKey(keyCode));
}

JNIEXPORT void JNICALL Java_com_skia_sampleapp_SampleApp_handleClick(JNIEnv* env,
        jobject thiz, jint owner, jfloat x, jfloat y, jint jstate)
{
    SkView::Click::State state;
    switch(jstate) {
        case 0:     // MotionEvent.ACTION_DOWN
            state = SkView::Click::kDown_State;
            break;
        case 1:     // MotionEvent.ACTION_UP
        case 3:     // MotionEvent.ACTION_CANCEL
            state = SkView::Click::kUp_State;
            break;
        case 2:     // MotionEvent.ACTION_MOVE
            state = SkView::Click::kMoved_State;
            break;
        default:
            SkDebugf("motion event ignored\n");
            return;
    }
    gWindow->handleClick(x, y, state, (void*) owner);
}

JNIEXPORT void JNICALL Java_com_skia_sampleapp_SampleApp_updateSize(JNIEnv* env,
        jobject thiz, jint w, jint h)
{
    gWindow->resize(w, h);
}

JNIEXPORT void JNICALL Java_com_skia_sampleapp_SampleApp_createOSWindow(
        JNIEnv* env, jobject thiz, jobject jsampleView)
{
    gWindow = new SampleWindow(NULL);
    jclass clazz = gActivityGlue.m_env->FindClass(
            "com/skia/sampleapp/SampleView");
    gWindowGlue.m_obj = gActivityGlue.m_env->NewWeakGlobalRef(jsampleView);
    gWindowGlue.m_inval = GetJMethod(gActivityGlue.m_env, clazz,
            "requestRender", "()V");
    gWindowGlue.m_queueSkEvent = GetJMethod(gActivityGlue.m_env, clazz,
            "queueSkEvent", "()V");
    gActivityGlue.m_env->DeleteLocalRef(clazz);
}

JNIEXPORT void JNICALL Java_com_skia_sampleapp_SampleApp_init(JNIEnv* env,
        jobject thiz)
{
    gActivityGlue.m_env = env;
    jclass clazz = env->FindClass("com/skia/sampleapp/SampleApp");
    gActivityGlue.m_obj = env->NewWeakGlobalRef(thiz);
    gActivityGlue.m_setTitle = GetJMethod(env, clazz, "setTitle",
            "(Ljava/lang/CharSequence;)V");
    gActivityGlue.m_addToDownloads = GetJMethod(env, clazz, "addToDownloads",
            "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
    gActivityGlue.m_startTimer = GetJMethod(gActivityGlue.m_env, clazz,
            "startTimer", "(I)V");
    env->DeleteLocalRef(clazz);

    application_init();
}

JNIEXPORT void JNICALL Java_com_skia_sampleapp_SampleApp_term(JNIEnv* env,
        jobject thiz)
{
    delete gWindow;
    gWindow = NULL;
    application_term();
    if (gWindowGlue.m_obj) {
        env->DeleteWeakGlobalRef(gWindowGlue.m_obj);
        gWindowGlue.m_obj = NULL;
    }
    if (gActivityGlue.m_obj) {
        env->DeleteWeakGlobalRef(gActivityGlue.m_obj);
        gActivityGlue.m_obj = NULL;
    }
}


JNIEXPORT void JNICALL Java_com_skia_sampleapp_SampleApp_draw(
        JNIEnv* env, jobject thiz)
{
    if (!gWindow) return;
    gWindow->update(NULL);

    // Copy the bitmap to the screen in raster mode
    if (!gWindow->drawsToHardware()) {

        SkBitmap bitmap = gWindow->getBitmap();

        GrContext* context = gWindow->getGrContext();
        if (!context) {
            context = GrContext::Create(kOpenGL_Shaders_GrEngine, NULL);
            if (!context || !gWindow->setGrContext(context)) {
                return;
            }
            context->unref();
        }

        GrRenderTarget* renderTarget;

        GrPlatformSurfaceDesc desc;
        desc.reset();
        desc.fSurfaceType = kRenderTarget_GrPlatformSurfaceType;
        desc.fWidth = bitmap.width();
        desc.fHeight = bitmap.height();
        desc.fConfig = kRGBA_8888_GrPixelConfig;
        desc.fStencilBits = 8;
        GrGLint buffer;
        GR_GL_GetIntegerv(GR_GL_FRAMEBUFFER_BINDING, &buffer);
        desc.fPlatformRenderTarget = buffer;

        renderTarget = static_cast<GrRenderTarget*>(
                context->createPlatformSurface(desc));

        SkGpuCanvas* gpuCanvas = new SkGpuCanvas(context, renderTarget);

        SkDevice* device = gpuCanvas->createDevice(SkBitmap::kARGB_8888_Config,
                                          bitmap.width(), bitmap.height(),
                                          false, false);
        gpuCanvas->setDevice(device)->unref();
        gpuCanvas->drawBitmap(bitmap, 0, 0);

        SkSafeUnref(renderTarget);
        SkSafeUnref(gpuCanvas);
    }
}

JNIEXPORT void JNICALL Java_com_skia_sampleapp_SampleApp_setZoomCenter(
        JNIEnv* env, jobject thiz, jfloat x, jfloat y)
{
    gWindow->setZoomCenter(x, y);
}

JNIEXPORT void JNICALL Java_com_skia_sampleapp_SampleApp_zoom(
        JNIEnv* env, jobject thiz, jfloat factor)
{
    gWindow->changeZoomLevel(factor);
}

JNIEXPORT void JNICALL Java_com_skia_sampleapp_SampleApp_nextSample(
        JNIEnv* env, jobject thiz, jboolean fprevious)
{
    if (fprevious) {
        gWindow->previousSample();
    } else {
        gWindow->nextSample();
    }
}

JNIEXPORT void JNICALL Java_com_skia_sampleapp_SampleApp_toggleRendering(
        JNIEnv* env, jobject thiz)
{
    gWindow->toggleRendering();
}

JNIEXPORT void JNICALL Java_com_skia_sampleapp_SampleApp_toggleSlideshow(
        JNIEnv* env, jobject thiz)
{
    gWindow->toggleSlideshow();
}

JNIEXPORT void JNICALL Java_com_skia_sampleapp_SampleApp_toggleFps(
        JNIEnv* env, jobject thiz)
{
    gWindow->toggleFPS();
}

JNIEXPORT void JNICALL Java_com_skia_sampleapp_SampleApp_processSkEvent(
        JNIEnv* env, jobject thiz)
{
    if (SkEvent::ProcessEvent()) {
        SkEvent::SignalNonEmptyQueue();
    }
}

JNIEXPORT void JNICALL Java_com_skia_sampleapp_SampleApp_serviceQueueTimer(
        JNIEnv* env, jobject thiz)
{
    SkEvent::ServiceQueueTimer();
}

JNIEXPORT void JNICALL Java_com_skia_sampleapp_SampleApp_saveToPdf(
        JNIEnv* env, jobject thiz)
{
    gWindow->saveToPdf();
}

JNIEXPORT void JNICALL Java_com_skia_sampleapp_SampleApp_postInval(
        JNIEnv* env, jobject thiz)
{
    gWindow->postInvalDelay();    
}
