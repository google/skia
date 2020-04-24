/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef surface_glue_android_DEFINED
#define surface_glue_android_DEFINED

#include <pthread.h>

#include <android/native_window_jni.h>

#include "include/core/SkString.h"

#include "tools/sk_app/Application.h"
#include "tools/sk_app/Window.h"

namespace sk_app {

enum MessageType {
    kUndefined,
    kSurfaceCreated,
    kSurfaceChanged,
    kSurfaceDestroyed,
    kDestroyApp,
    kContentInvalidated,
    kKeyPressed,
    kTouched,
    kUIStateChanged,
};

struct Message {
    MessageType fType = kUndefined;
    ANativeWindow* fNativeWindow = nullptr;
    int fKeycode = 0;
    int fTouchOwner, fTouchState;
    float fTouchX, fTouchY;

    SkString* stateName;
    SkString* stateValue;

    Message() {}
    Message(MessageType t) : fType(t) {}
};

struct SkiaAndroidApp {
    Application* fApp;
    Window* fWindow;
    jobject fAndroidApp;

    SkiaAndroidApp(JNIEnv* env, jobject androidApp);

    void postMessage(const Message& message) const;
    void readMessage(Message* message) const;

    // These must be called in SkiaAndroidApp's own pthread because the JNIEnv is thread sensitive
    void setTitle(const char* title) const;
    void setUIState(const char* state) const;

private:
    pthread_t fThread;
    ANativeWindow* fNativeWindow;
    int fPipes[2];  // 0 is the read message pipe, 1 is the write message pipe
    JavaVM* fJavaVM;
    JNIEnv* fPThreadEnv;
    jmethodID fSetTitleMethodID, fSetStateMethodID;

    // This must be called in SkiaAndroidApp's own pthread because the JNIEnv is thread sensitive
    ~SkiaAndroidApp();

    static int message_callback(int fd, int events, void* data);
    static void* pthread_main(void*);
};

}  // namespace sk_app

#endif
