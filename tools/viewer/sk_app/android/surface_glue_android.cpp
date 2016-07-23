/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "surface_glue_android.h"

#include <jni.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <unordered_map>

#include <android/input.h>
#include <android/keycodes.h>
#include <android/looper.h>
#include <android/native_window_jni.h>

#include "../Application.h"
#include "SkTypes.h"
#include "SkUtils.h"
#include "Window_android.h"
#include "SkTime.h"

namespace sk_app {

static const int LOOPER_ID_MESSAGEPIPE = 1;

static const std::unordered_map<int, Window::Key> ANDROID_TO_WINDOW_KEYMAP({
    {AKEYCODE_SOFT_LEFT, Window::Key::kLeft},
    {AKEYCODE_SOFT_RIGHT, Window::Key::kRight}
});

static const std::unordered_map<int, Window::InputState> ANDROID_TO_WINDOW_STATEMAP({
    {AMOTION_EVENT_ACTION_DOWN, Window::kDown_InputState},
    {AMOTION_EVENT_ACTION_POINTER_DOWN, Window::kDown_InputState},
    {AMOTION_EVENT_ACTION_UP, Window::kUp_InputState},
    {AMOTION_EVENT_ACTION_POINTER_UP, Window::kUp_InputState},
    {AMOTION_EVENT_ACTION_MOVE, Window::kMove_InputState},
    {AMOTION_EVENT_ACTION_CANCEL, Window::kUp_InputState},
});

SkiaAndroidApp::SkiaAndroidApp(JNIEnv* env, jobject androidApp) {
    env->GetJavaVM(&fJavaVM);
    fAndroidApp = env->NewGlobalRef(androidApp);
    jclass cls = env->GetObjectClass(fAndroidApp);
    fSetTitleMethodID = env->GetMethodID(cls, "setTitle", "(Ljava/lang/String;)V");
    fSetStateMethodID = env->GetMethodID(cls, "setState", "(Ljava/lang/String;)V");
    fNativeWindow = nullptr;
    pthread_create(&fThread, nullptr, pthread_main, this);
}

SkiaAndroidApp::~SkiaAndroidApp() {
    fPThreadEnv->DeleteGlobalRef(fAndroidApp);
    if (fWindow) {
        fWindow->detach();
    }
    if (fNativeWindow) {
        ANativeWindow_release(fNativeWindow);
        fNativeWindow = nullptr;
    }
    if (fApp) {
        delete fApp;
    }
}

void SkiaAndroidApp::setTitle(const char* title) const {
    jstring titleString = fPThreadEnv->NewStringUTF(title);
    fPThreadEnv->CallVoidMethod(fAndroidApp, fSetTitleMethodID, titleString);
    fPThreadEnv->DeleteLocalRef(titleString);
}

void SkiaAndroidApp::setUIState(const Json::Value& state) const {
    jstring jstr = fPThreadEnv->NewStringUTF(state.toStyledString().c_str());
    fPThreadEnv->CallVoidMethod(fAndroidApp, fSetStateMethodID, jstr);
    fPThreadEnv->DeleteLocalRef(jstr);
}

void SkiaAndroidApp::postMessage(const Message& message) const {
    SkDEBUGCODE(auto writeSize =) write(fPipes[1], &message, sizeof(message));
    SkASSERT(writeSize == sizeof(message));
}

void SkiaAndroidApp::readMessage(Message* message) const {
    SkDEBUGCODE(auto readSize =) read(fPipes[0], message, sizeof(Message));
    SkASSERT(readSize == sizeof(Message));
}

int SkiaAndroidApp::message_callback(int fd, int events, void* data) {
    auto skiaAndroidApp = (SkiaAndroidApp*)data;
    Message message;
    skiaAndroidApp->readMessage(&message);
    SkDebugf("message_callback %d", message.fType);
    SkASSERT(message.fType != kUndefined);

    switch (message.fType) {
        case kDestroyApp: {
            delete skiaAndroidApp;
            pthread_exit(nullptr);
            return 0;
        }
        case kContentInvalidated: {
            ((Window_android*)skiaAndroidApp->fWindow)->paintIfNeeded();
            break;
        }
        case kSurfaceCreated: {
            SkASSERT(!skiaAndroidApp->fNativeWindow && message.fNativeWindow);
            skiaAndroidApp->fNativeWindow = message.fNativeWindow;
            auto window_android = (Window_android*)skiaAndroidApp->fWindow;
            window_android->initDisplay(skiaAndroidApp->fNativeWindow);
            ((Window_android*)skiaAndroidApp->fWindow)->paintIfNeeded();
            break;
        }
        case kSurfaceChanged: {
            SkASSERT(message.fNativeWindow);
            int width = ANativeWindow_getWidth(skiaAndroidApp->fNativeWindow);
            int height = ANativeWindow_getHeight(skiaAndroidApp->fNativeWindow);
            auto window_android = (Window_android*)skiaAndroidApp->fWindow;
            if (message.fNativeWindow != skiaAndroidApp->fNativeWindow) {
                window_android->onDisplayDestroyed();
                ANativeWindow_release(skiaAndroidApp->fNativeWindow);
                skiaAndroidApp->fNativeWindow = message.fNativeWindow;
                window_android->initDisplay(skiaAndroidApp->fNativeWindow);
            }
            window_android->onResize(width, height);
            window_android->setContentRect(0, 0, width, height);
            window_android->paintIfNeeded();
            break;
        }
        case kSurfaceDestroyed: {
            if (skiaAndroidApp->fNativeWindow) {
                auto window_android = (Window_android*)skiaAndroidApp->fWindow;
                window_android->onDisplayDestroyed();
                ANativeWindow_release(skiaAndroidApp->fNativeWindow);
                skiaAndroidApp->fNativeWindow = nullptr;
            }
            break;
        }
        case kKeyPressed: {
            auto it = ANDROID_TO_WINDOW_KEYMAP.find(message.fKeycode);
            SkASSERT(it != ANDROID_TO_WINDOW_KEYMAP.end());
            // No modifier is supported so far
            skiaAndroidApp->fWindow->onKey(it->second, Window::kDown_InputState, 0);
            skiaAndroidApp->fWindow->onKey(it->second, Window::kUp_InputState, 0);
            break;
        }
        case kTouched: {
            auto it = ANDROID_TO_WINDOW_STATEMAP.find(message.fTouchState);
            SkASSERT(it != ANDROID_TO_WINDOW_STATEMAP.end());
            skiaAndroidApp->fWindow->onTouch(message.fTouchOwner, it->second, message.fTouchX,
                                             message.fTouchY);
            break;
        }
        case kUIStateChanged: {
            skiaAndroidApp->fWindow->onUIStateChanged(*message.stateName, *message.stateValue);
            delete message.stateName;
            delete message.stateValue;
            break;
        }
        default: {
            // do nothing
        }
    }

    return 1;  // continue receiving callbacks
}

static double now_ms() { return SkTime::GetMSecs(); }

void* SkiaAndroidApp::pthread_main(void* arg) {
    SkDebugf("pthread_main begins");

    auto skiaAndroidApp = (SkiaAndroidApp*)arg;

    // Because JNIEnv is thread sensitive, we need AttachCurrentThread to set our fPThreadEnv
    skiaAndroidApp->fJavaVM->AttachCurrentThread(&(skiaAndroidApp->fPThreadEnv), nullptr);

    ALooper* looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
    pipe(skiaAndroidApp->fPipes);
    ALooper_addFd(looper, skiaAndroidApp->fPipes[0], LOOPER_ID_MESSAGEPIPE, ALOOPER_EVENT_INPUT,
                  message_callback, skiaAndroidApp);

    skiaAndroidApp->fApp = Application::Create(0, nullptr, skiaAndroidApp);

    double currentTime = 0.0;
    double previousTime = 0.0;
    while (true) {
        const int ident = ALooper_pollAll(0, nullptr, nullptr, nullptr);

        if (ident >= 0) {
            SkDebugf("Unhandled ALooper_pollAll ident=%d !", ident);
        } else {
            previousTime = currentTime;
            currentTime = now_ms();
            skiaAndroidApp->fApp->onIdle(currentTime - previousTime);
        }
    }

    SkDebugf("pthread_main ends");

    return nullptr;
}

extern "C"  // extern "C" is needed for JNI (although the method itself is in C++)
    JNIEXPORT jlong JNICALL
    Java_org_skia_viewer_ViewerApplication_createNativeApp(JNIEnv* env, jobject application) {
    SkiaAndroidApp* skiaAndroidApp = new SkiaAndroidApp(env, application);
    return (jlong)((size_t)skiaAndroidApp);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_viewer_ViewerApplication_destroyNativeApp(
    JNIEnv* env, jobject application, jlong handle) {
    auto skiaAndroidApp = (SkiaAndroidApp*)handle;
    skiaAndroidApp->postMessage(Message(kDestroyApp));
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_viewer_ViewerActivity_onSurfaceCreated(
    JNIEnv* env, jobject activity, jlong handle, jobject surface) {
    auto skiaAndroidApp = (SkiaAndroidApp*)handle;
    Message message(kSurfaceCreated);
    message.fNativeWindow = ANativeWindow_fromSurface(env, surface);
    skiaAndroidApp->postMessage(message);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_viewer_ViewerActivity_onSurfaceChanged(
    JNIEnv* env, jobject activity, jlong handle, jobject surface) {
    auto skiaAndroidApp = (SkiaAndroidApp*)handle;
    Message message(kSurfaceChanged);
    message.fNativeWindow = ANativeWindow_fromSurface(env, surface);
    skiaAndroidApp->postMessage(message);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_viewer_ViewerActivity_onSurfaceDestroyed(
    JNIEnv* env, jobject activity, jlong handle) {
    auto skiaAndroidApp = (SkiaAndroidApp*)handle;
    skiaAndroidApp->postMessage(Message(kSurfaceDestroyed));
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_viewer_ViewerActivity_onKeyPressed(JNIEnv* env,
                                                                                   jobject activity,
                                                                                   jlong handle,
                                                                                   jint keycode) {
    auto skiaAndroidApp = (SkiaAndroidApp*)handle;
    Message message(kKeyPressed);
    message.fKeycode = keycode;
    skiaAndroidApp->postMessage(message);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_viewer_ViewerActivity_onTouched(
    JNIEnv* env, jobject activity, jlong handle, jint owner, jfloat x, jfloat y, jint state) {
    auto skiaAndroidApp = (SkiaAndroidApp*)handle;
    Message message(kTouched);
    message.fTouchOwner = owner;
    message.fTouchState = state;
    message.fTouchX = x;
    message.fTouchY = y;
    skiaAndroidApp->postMessage(message);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_viewer_ViewerActivity_onUIStateChanged(
    JNIEnv* env, jobject activity, jlong handle, jstring stateName, jstring stateValue) {
    auto skiaAndroidApp = (SkiaAndroidApp*)handle;
    Message message(kUIStateChanged);
    const char* nameChars = env->GetStringUTFChars(stateName, nullptr);
    const char* valueChars = env->GetStringUTFChars(stateValue, nullptr);
    message.stateName = new SkString(nameChars);
    message.stateValue = new SkString(valueChars);
    skiaAndroidApp->postMessage(message);
    env->ReleaseStringUTFChars(stateName, nameChars);
    env->ReleaseStringUTFChars(stateValue, valueChars);
}

}  // namespace sk_app
