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

#include <android/keycodes.h>
#include <android/looper.h>
#include <android/native_window_jni.h>

#include "../Application.h"
#include "SkTypes.h"
#include "SkUtils.h"
#include "Window_android.h"

namespace sk_app {

static const int LOOPER_ID_MESSAGEPIPE = 1;

static const std::unordered_map<int, Window::Key> ANDROID_TO_WINDOW_KEYMAP({
    {AKEYCODE_SOFT_LEFT, Window::Key::kLeft},
    {AKEYCODE_SOFT_RIGHT, Window::Key::kRight}
});

void* pthread_main(void* arg);

SkiaAndroidApp::SkiaAndroidApp() {
    fNativeWindow = nullptr;
    pthread_create(&fThread, nullptr, pthread_main, this);
}

SkiaAndroidApp::~SkiaAndroidApp() {
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

void SkiaAndroidApp::paintIfNeeded() {
    if (fNativeWindow && fWindow) {
        fWindow->onPaint();
    }
}

void SkiaAndroidApp::postMessage(const Message& message) {
    auto writeSize = write(fPipes[1], &message, sizeof(message));
    SkASSERT(writeSize == sizeof(message));
}

void SkiaAndroidApp::readMessage(Message* message) {
    auto readSize = read(fPipes[0], message, sizeof(Message));
    SkASSERT(readSize == sizeof(Message));
}

static int message_callback(int fd, int events, void* data) {
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
            skiaAndroidApp->paintIfNeeded();
            break;
        }
        case kSurfaceCreated: {
            SkASSERT(!skiaAndroidApp->fNativeWindow && message.fNativeWindow);
            skiaAndroidApp->fNativeWindow = message.fNativeWindow;
            auto window_android = (Window_android*)skiaAndroidApp->fWindow;
            window_android->initDisplay(skiaAndroidApp->fNativeWindow);
            skiaAndroidApp->paintIfNeeded();
            break;
        }
        case kSurfaceChanged: {
            SkASSERT(message.fNativeWindow == skiaAndroidApp->fNativeWindow &&
                     message.fNativeWindow);
            int width = ANativeWindow_getWidth(skiaAndroidApp->fNativeWindow);
            int height = ANativeWindow_getHeight(skiaAndroidApp->fNativeWindow);
            auto window_android = (Window_android*)skiaAndroidApp->fWindow;
            window_android->setContentRect(0, 0, width, height);
            skiaAndroidApp->paintIfNeeded();
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
            auto it = ANDROID_TO_WINDOW_KEYMAP.find(message.keycode);
            SkASSERT(it != ANDROID_TO_WINDOW_KEYMAP.end());
            // No modifier is supported so far
            skiaAndroidApp->fWindow->onKey(it->second, Window::kDown_InputState, 0);
            skiaAndroidApp->fWindow->onKey(it->second, Window::kUp_InputState, 0);
            break;
        }
        default: {
            // do nothing
        }
    }

    return 1;  // continue receiving callbacks
}

void* pthread_main(void* arg) {
    SkDebugf("pthread_main begins");

    auto skiaAndroidApp = (SkiaAndroidApp*)arg;

    ALooper* looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
    pipe(skiaAndroidApp->fPipes);
    ALooper_addFd(looper, skiaAndroidApp->fPipes[0], LOOPER_ID_MESSAGEPIPE, ALOOPER_EVENT_INPUT,
                  message_callback, skiaAndroidApp);

    int ident;
    int events;
    struct android_poll_source* source;

    skiaAndroidApp->fApp = Application::Create(0, nullptr, skiaAndroidApp);

    while ((ident = ALooper_pollAll(-1, nullptr, &events, (void**)&source)) >= 0) {
        SkDebugf("ALooper_pollAll ident=%d", ident);
    }

    return nullptr;
}

extern "C"  // extern "C" is needed for JNI (although the method itself is in C++)
    JNIEXPORT jlong JNICALL
    Java_org_skia_viewer_ViewerApplication_createNativeApp(JNIEnv* env, jobject activity) {
    SkiaAndroidApp* skiaAndroidApp = new SkiaAndroidApp;
    return (jlong)((size_t)skiaAndroidApp);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_viewer_ViewerApplication_destroyNativeApp(
    JNIEnv* env, jobject activity, jlong handle) {
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
    message.keycode = keycode;
    skiaAndroidApp->postMessage(message);
}

}  // namespace sk_app
