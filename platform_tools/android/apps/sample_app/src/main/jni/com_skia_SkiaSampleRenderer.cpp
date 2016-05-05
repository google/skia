/*
 * Copyright 2011 Skia
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "com_skia_SkiaSampleRenderer.h"

#include "SampleApp.h"
#include "SkApplication.h"
#include "SkCanvas.h"
#include "SkDevice.h"
#include "SkEvent.h"
#include "SkWindow.h"

#include <jni.h>
#include "AndroidKeyToSkKey.h"


///////////////////////////////////////////
///////////////// Globals /////////////////
///////////////////////////////////////////

struct ActivityGlue {
    JNIEnv* m_env;
    jweak m_obj;
    jmethodID m_setTitle;
    jmethodID m_setSlideList;
    ActivityGlue() {
        m_env = nullptr;
        m_obj = nullptr;
        m_setTitle = nullptr;
        m_setSlideList = nullptr;
    }
} gActivityGlue;

struct WindowGlue {
    jweak m_obj;
    jmethodID m_inval;
    jmethodID m_queueSkEvent;
    jmethodID m_startTimer;
    jmethodID m_getMSAASampleCount;
    WindowGlue() {
        m_obj = nullptr;
        m_inval = nullptr;
        m_queueSkEvent = nullptr;
        m_startTimer = nullptr;
        m_getMSAASampleCount = nullptr;
    }
} gWindowGlue;

SampleWindow* gWindow;

///////////////////////////////////////////
///////////// SkOSWindow impl /////////////
///////////////////////////////////////////

SkOSWindow::SkOSWindow(void*) : fDestroyRequested(false) {
}

SkOSWindow::~SkOSWindow() {
}

bool SkOSWindow::attach(SkBackEndTypes /* attachType */, int /*msaaSampleCount*/,
                        bool /*deepColor*/, AttachmentInfo* info)
{
    JNIEnv* env = gActivityGlue.m_env;
    if (!env || !gWindowGlue.m_getMSAASampleCount || !gWindowGlue.m_obj) {
        return false;
    }
    if (env->IsSameObject(gWindowGlue.m_obj, nullptr)) {
        SkDebugf("ERROR: The JNI WeakRef to the Window is invalid");
        return false;
    }
    info->fSampleCount = env->CallIntMethod(gWindowGlue.m_obj, gWindowGlue.m_getMSAASampleCount);

    // This is the value requested in SkiaSampleView.java.
    info->fStencilBits = 8;
    return true;
}

void SkOSWindow::release() {
}

void SkOSWindow::present() {
}

void SkOSWindow::closeWindow() {
}

void SkOSWindow::setVsync(bool) {
}

void SkOSWindow::onSetTitle(const char title[])
{
    JNIEnv* env = gActivityGlue.m_env;
    if (!env) {
        return;
    }
    if (env->IsSameObject(gActivityGlue.m_obj, nullptr)) {
        SkDebugf("ERROR: The JNI WeakRef to the Activity is invalid");
        return;
    }

    jstring string = env->NewStringUTF(title);
    env->CallVoidMethod(gActivityGlue.m_obj, gActivityGlue.m_setTitle, string);
    env->DeleteLocalRef(string);
}

void SkOSWindow::onHandleInval(const SkIRect& rect)
{
    JNIEnv* env = gActivityGlue.m_env;
    if (!env || !gWindowGlue.m_inval || !gWindowGlue.m_obj) {
        return;
    }
    if (env->IsSameObject(gWindowGlue.m_obj, nullptr)) {
        SkDebugf("ERROR: The JNI WeakRef to the Window is invalid");
        return;
    }
    env->CallVoidMethod(gWindowGlue.m_obj, gWindowGlue.m_inval);
}

///////////////////////////////////////////
/////////////// SkEvent impl //////////////
///////////////////////////////////////////

void SkEvent::SignalQueueTimer(SkMSec ms)
{
    JNIEnv* env = gActivityGlue.m_env;
    if (!env || !gWindowGlue.m_startTimer || !gWindowGlue.m_obj || !ms) {
        return;
    }
    if (env->IsSameObject(gWindowGlue.m_obj, nullptr)) {
        SkDebugf("ERROR: The JNI WeakRef to the Window is invalid");
        return;
    }
    env->CallVoidMethod(gWindowGlue.m_obj,
            gWindowGlue.m_startTimer, ms);
}

void SkEvent::SignalNonEmptyQueue()
{
    JNIEnv* env = gActivityGlue.m_env;
    if (!env || !gWindowGlue.m_queueSkEvent || !gWindowGlue.m_obj) {
        return;
    }
    if (env->IsSameObject(gWindowGlue.m_obj, nullptr)) {
        SkDebugf("ERROR: The JNI WeakRef to the Window is invalid");
        return;
    }
    env->CallVoidMethod(gWindowGlue.m_obj, gWindowGlue.m_queueSkEvent);
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

JNIEXPORT void JNICALL Java_com_skia_SkiaSampleRenderer_init(JNIEnv* env,
        jobject thiz, jobject jsampleActivity, jstring cmdLineFlags, jint msaaSampleCount)
{
    // setup jni hooks to the java activity
    gActivityGlue.m_env = env;
    jclass clazz = env->FindClass("com/skia/SkiaSampleActivity");
    gActivityGlue.m_obj = env->NewWeakGlobalRef(jsampleActivity);
    gActivityGlue.m_setTitle = GetJMethod(env, clazz, "setTitle", "(Ljava/lang/CharSequence;)V");
    gActivityGlue.m_setSlideList = GetJMethod(env, clazz, "setSlideList", "([Ljava/lang/String;)V");
    env->DeleteLocalRef(clazz);

    // setup jni hooks to the java renderer
    clazz = env->FindClass("com/skia/SkiaSampleRenderer");
    gWindowGlue.m_obj = env->NewWeakGlobalRef(thiz);
    gWindowGlue.m_inval = GetJMethod(env, clazz, "requestRender", "()V");
    gWindowGlue.m_queueSkEvent = GetJMethod(env, clazz, "queueSkEvent", "()V");
    gWindowGlue.m_startTimer = GetJMethod(env, clazz, "startTimer", "(I)V");
    gWindowGlue.m_getMSAASampleCount = GetJMethod(env, clazz, "getMSAASampleCount", "()I");
    env->DeleteLocalRef(clazz);

    application_init();

    const char* flags = env->GetStringUTFChars(cmdLineFlags, JNI_FALSE);
    SkTArray<SkString> flagEntries;
    SkStrSplit(flags, " ", &flagEntries);

    SkTArray<const char*> args;
    args.push_back("SampleApp");
    for (int i = 0; i < flagEntries.count(); i++) {
        SkDebugf(flagEntries[i].c_str());
        args.push_back(flagEntries[i].c_str());
    }

    SkString msaaSampleCountString;
    if (msaaSampleCount > 0) {
        args.push_back("--msaa");
        msaaSampleCountString.appendS32(static_cast<uint32_t>(msaaSampleCount));
        args.push_back(msaaSampleCountString.c_str());
    }

    if (gWindow) {
        SkDebugf("The sample window already exists.");
    } else {
        gWindow = new SampleWindow(nullptr, args.count(), const_cast<char**>(args.begin()), nullptr);
    }

    // cleanup the command line flags
    env->ReleaseStringUTFChars(cmdLineFlags, flags);

    // send the list of slides up to the activity
    const int slideCount = gWindow->sampleCount();
    jobjectArray slideList = env->NewObjectArray(slideCount, env->FindClass("java/lang/String"), env->NewStringUTF(""));
    for (int i = 0; i < slideCount; i++) {
        jstring slideTitle = env->NewStringUTF(gWindow->getSampleTitle(i).c_str());
        env->SetObjectArrayElement(slideList, i, slideTitle);
        env->DeleteLocalRef(slideTitle);
    }
    env->CallVoidMethod(gActivityGlue.m_obj, gActivityGlue.m_setSlideList, slideList);
    env->DeleteLocalRef(slideList);
}

JNIEXPORT void JNICALL Java_com_skia_SkiaSampleRenderer_term(JNIEnv* env,
        jobject thiz)
{
    delete gWindow;
    gWindow = nullptr;
    application_term();
    if (gWindowGlue.m_obj) {
        env->DeleteWeakGlobalRef(gWindowGlue.m_obj);
        gWindowGlue.m_obj = nullptr;
    }
    if (gActivityGlue.m_obj) {
        env->DeleteWeakGlobalRef(gActivityGlue.m_obj);
        gActivityGlue.m_obj = nullptr;
    }
}

JNIEXPORT void JNICALL Java_com_skia_SkiaSampleRenderer_draw(
        JNIEnv* env, jobject thiz)
{
    if (!gWindow) return;
    gWindow->update(nullptr);
}

JNIEXPORT void JNICALL Java_com_skia_SkiaSampleRenderer_updateSize(JNIEnv* env,
        jobject thiz, jint w, jint h)
{
    gWindow->resize(w, h);
}

JNIEXPORT void JNICALL Java_com_skia_SkiaSampleRenderer_handleClick(JNIEnv* env,
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
    gWindow->handleClick(x, y, state, reinterpret_cast<void*>(owner));
}

JNIEXPORT void JNICALL Java_com_skia_SkiaSampleRenderer_nextSample(
        JNIEnv* env, jobject thiz)
{
    gWindow->nextSample();
}

JNIEXPORT void JNICALL Java_com_skia_SkiaSampleRenderer_previousSample(
        JNIEnv* env, jobject thiz)
{
    gWindow->previousSample();
}

JNIEXPORT void JNICALL Java_com_skia_SkiaSampleRenderer_goToSample(
        JNIEnv* env, jobject thiz, jint position)
{
    gWindow->goToSample(position);
}

JNIEXPORT void JNICALL Java_com_skia_SkiaSampleRenderer_toggleRenderingMode(
        JNIEnv* env, jobject thiz)
{
    gWindow->toggleRendering();
}

JNIEXPORT void JNICALL Java_com_skia_SkiaSampleRenderer_showOverview(
        JNIEnv* env, jobject thiz)
{
    gWindow->showOverview();
}

JNIEXPORT void JNICALL Java_com_skia_SkiaSampleRenderer_toggleSlideshow(
        JNIEnv* env, jobject thiz)
{
    gWindow->toggleSlideshow();
}

JNIEXPORT void JNICALL Java_com_skia_SkiaSampleRenderer_toggleFPS(
        JNIEnv* env, jobject thiz)
{
    gWindow->toggleFPS();
}

JNIEXPORT void JNICALL Java_com_skia_SkiaSampleRenderer_toggleTiling(
        JNIEnv* env, jobject thiz)
{
    gWindow->handleChar('t');
}

JNIEXPORT void JNICALL Java_com_skia_SkiaSampleRenderer_toggleBBox(
        JNIEnv* env, jobject thiz)
{
    gWindow->handleChar('b');
}

JNIEXPORT void JNICALL Java_com_skia_SkiaSampleRenderer_processSkEvent(
        JNIEnv* env, jobject thiz)
{
    if (SkEvent::ProcessEvent()) {
        SkEvent::SignalNonEmptyQueue();
    }
}

JNIEXPORT void JNICALL Java_com_skia_SkiaSampleRenderer_serviceQueueTimer(
        JNIEnv* env, jobject thiz)
{
    SkEvent::ServiceQueueTimer();
}

JNIEXPORT void JNICALL Java_com_skia_SkiaSampleRenderer_saveToPDF(
        JNIEnv* env, jobject thiz)
{
    gWindow->saveToPdf();
}

JNIEXPORT void JNICALL Java_com_skia_SkiaSampleRenderer_postInval(
        JNIEnv* env, jobject thiz)
{
    gWindow->postInvalDelay();
}
